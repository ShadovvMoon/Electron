//
//  MRenderView.m
//  Gallium
//
//  Created by Samuco on 6/23/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#import "MRenderView.h"
#import "SharedStructures.h"
#import "ProtonMap.h"
#import <MetalKit/MetalKit.h>
#import "camera.h"
#import "bitmaps.h"
#import <string>
#include "defines.h"
#import <simd/simd.h>
#import <ModelIO/ModelIO.h>
#import <GLKit/GLKit.h>

#include "mshader.hpp"
#include "bsp.h"
#include "object.h"
#include "sky.h"
#include "metal.hpp"

// The max number of command buffers in flight
static const NSUInteger kMaxInflightBuffers = 3;

// Max API memory buffer size.
static const size_t kMaxBytesPerFrame = sizeof(float) * 256;

// Controls
typedef struct {
    bool forward;
    bool forwardSlow;
    bool left;
    bool right;
    bool back;
    bool shift;
    bool control;
} Control;

@implementation MRenderView
{
    // view
    MTKView *_view;
    
    // camera
    Camera *camera;
    shader_options *options;
    Control *controls;
    MetalShaderManager *shaders;
    BSP *bsp;
    ObjectManager *objects;
    SkyManager *skies;
    MetalPipeline *pipeline;
    bool ready;
    
    // controller
    dispatch_semaphore_t _inflight_semaphore;
    id <MTLBuffer> _dynamicConstantBuffer;
    uint8_t _constantDataBufferIndex;
    
    // renderer
    id <MTLDevice> _device;
    id <MTLCommandQueue> _commandQueue;
    id <MTLLibrary> _defaultLibrary;
    id <MTLRenderPipelineState> _pipelineState;
    id <MTLDepthStencilState> _depthState;
    id <MTLSamplerState> _sampler;
    
    // uniforms
    matrix_float4x4 _projectionMatrix;
    matrix_float4x4 _viewMatrix;
    uniforms_t _uniform_buffer;
    float _rotation;
    
    // meshes
    MTKMesh *_boxMesh;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
}

- (void)setupMetal {
    _constantDataBufferIndex = 0;
    _inflight_semaphore = dispatch_semaphore_create(3);
    
    // Create Camera
    camera = new Camera;
    options = new shader_options;
    controls = new Control;
    
    // Create Metal
    [self _setupMetal];
    if(_device)
    {
        [self _setupView];
        [self _loadAssets];
        [self _reshape];
        
        // First responder
        [[[self view] window] makeFirstResponder:self];
        
    }
    else // Fallback to a blank NSView, an application could also fallback to OpenGL here.
    {
        NSLog(@"Metal is not supported on this device");
        self.view = [[NSView alloc] initWithFrame:self.view.frame];
    }
}

-(void)setView:(NSView*)view {
    NSLog(@"Set view");
    [super setView:view];
    [self setupMetal];
}

- (void)_setupView
{
    _view = (MTKView *)self.view;
    _view.delegate = self;
    _view.device = _device;
    
    // Setup the render target, choose values based on your app
    _view.sampleCount = 4;
    _view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
}

- (void)_setupMetal
{
    // Set the view to use the default device
    _device = MTLCreateSystemDefaultDevice();
    
    // Create a new command queue
    _commandQueue = [_device newCommandQueue];
    
    // Load all the shader files with a metal file extension in the project
    _defaultLibrary = [_device newDefaultLibrary];
}

ProtonMap *map;
-(void)setData:(NSData*)data {
    NSLog(@"Loading map...");
    map = new ProtonMap([data bytes]);
    
    // Initilisation
    camera    = new Camera;
    shaders   = new MetalShaderManager(_device, _commandQueue, _view, _defaultLibrary);
    options   = new shader_options;
    bsp       = new BSP(shaders);
    objects   = new ObjectManager(camera, bsp);
    skies     = new SkyManager;
    pipeline  = new MetalPipeline(_view, _device);
    options->pipeline = pipeline;
    
    // Start the tick
    tick = mnow();
    forward_tick = mnow();
    strafe_tick = mnow();
    
    // Load the map
    uint16_t scenarioTag = map->principal_tag;
    if (scenarioTag != NULLED_TAG_ID) {
        ProtonTag *scenarioTag = map->tags.at(map->principal_tag).get();
        bsp->setup(map, scenarioTag, pipeline);
        objects->read(shaders, map, scenarioTag);
        skies->read(objects, map, scenarioTag);
        
        // Set the fog
        skies->options(options, map, scenarioTag);
    }
    ready = true;
}

-(NSData *)getData {
    ProtonCacheFile cache = map->ToCacheFile();
    return [[NSData alloc] initWithBytes:cache.Data() length:cache.Length()];
}

- (void)_loadAssets
{
    // Create stencil
    MTLDepthStencilDescriptor *depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
    depthStateDesc.depthWriteEnabled = YES;
    _depthState = [_device newDepthStencilStateWithDescriptor:depthStateDesc];
    
    // Create sampler
    MTLSamplerDescriptor *samplerDescriptor = [MTLSamplerDescriptor new];
    samplerDescriptor.sAddressMode = MTLSamplerAddressModeRepeat;
    samplerDescriptor.tAddressMode = MTLSamplerAddressModeRepeat;
    samplerDescriptor.minFilter = MTLSamplerMinMagFilterLinear;
    samplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;
    _sampler = [_device newSamplerStateWithDescriptor:samplerDescriptor];
    
    // Ready
    ready = false;
    printf("ready\n");
}

- (void)_render
{
    if (!ready) {
        return;
    }
    dispatch_semaphore_wait(_inflight_semaphore, DISPATCH_TIME_FOREVER);
    
    [self _update];
    
    // Create a new command buffer for each renderpass to the current drawable
    id <MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    commandBuffer.label = @"MyCommand";
    
    // Call the view's completion handler which is required by the view since it will signal its semaphore and set up the next buffer
    __block dispatch_semaphore_t block_sema = _inflight_semaphore;
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
        dispatch_semaphore_signal(block_sema);
    }];
    
    // Obtain a renderPassDescriptor generated from the view's drawable textures
    MTLRenderPassDescriptor* renderPassDescriptor = _view.currentRenderPassDescriptor;
    if(renderPassDescriptor != nil) // If we have a valid drawable, begin the commands to render into it
    {
        // Create a render command encoder so we can render into something
        id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        renderEncoder.label = @"GalliumEncoder";
        [renderEncoder setDepthStencilState:_depthState];
        [renderEncoder setFragmentSamplerState:_sampler atIndex:0];
        [renderEncoder pushDebugGroup:@"GalliumRender"];
        pipeline->encoder = renderEncoder;
        
        // Render into the encoder
        uint16_t scenarioTag = map->principal_tag;
        if (scenarioTag != NULLED_TAG_ID) {
            // ProtonTag *scenarioTag = map->tags.at(map->principal_tag).get();
            
            // Render each pass
            for (int pass = ShaderStart; pass <= ShaderEnd; pass++ )
            {
                if (pass != shader_SENV) continue;
                ShaderType type = static_cast<ShaderType>(pass);
                shader *shader = shaders->get_shader(type);
                shader->start(options);
                bsp->render(type, pipeline);
                // uint number = 0;
                // objects->render(&number, nullptr, type, options);
                shader->stop();
            }
        }
        
        // We're done encoding commands
        [renderEncoder popDebugGroup];
        [renderEncoder endEncoding];
        
        // Schedule a present once the framebuffer is complete using the current drawable
        [commandBuffer presentDrawable:_view.currentDrawable];
    } else {
        std::cout << "missing render pass descriptor" << std::endl;
    }
    
    // The render assumes it can now increment the buffer index and that the previous index won't be touched until we cycle back around to the same index
    _constantDataBufferIndex = (_constantDataBufferIndex + 1) % kMaxInflightBuffers;
    
    // Finalize rendering here & push the command buffer to the GPU
    [commandBuffer commit];
}

- (void)_reshape
{
    // When reshape is called, update the view and projection matricies since this means the view orientation or size changed
    float aspect = fabs(self.view.bounds.size.width / self.view.bounds.size.height);
    _projectionMatrix = matrix_from_perspective_fov_aspectLH(65.0f * (M_PI / 180.0f), aspect, 0.1f, 1000.0f);
    
    options->perspective[0]  = _projectionMatrix.columns[0][0];
    options->perspective[1]  = _projectionMatrix.columns[0][1];
    options->perspective[2]  = _projectionMatrix.columns[0][2];
    options->perspective[3]  = _projectionMatrix.columns[0][3];
    options->perspective[4]  = _projectionMatrix.columns[1][0];
    options->perspective[5]  = _projectionMatrix.columns[1][1];
    options->perspective[6]  = _projectionMatrix.columns[1][2];
    options->perspective[7]  = _projectionMatrix.columns[1][3];
    options->perspective[8]  = _projectionMatrix.columns[2][0];
    options->perspective[9]  = _projectionMatrix.columns[2][1];
    options->perspective[10] = _projectionMatrix.columns[2][2];
    options->perspective[11] = _projectionMatrix.columns[2][3];
    options->perspective[12] = _projectionMatrix.columns[3][0];
    options->perspective[13] = _projectionMatrix.columns[3][1];
    options->perspective[14] = _projectionMatrix.columns[3][2];
    options->perspective[15] = _projectionMatrix.columns[3][3];
    
    
    _viewMatrix = matrix_identity_float4x4;
}

#include <chrono>
using namespace std::chrono;

// movement
milliseconds tick;
milliseconds forward_tick;
milliseconds strafe_tick;

milliseconds mnow() {
    return duration_cast< milliseconds >(high_resolution_clock::now().time_since_epoch());
}

-(void)applyControls {
    milliseconds current = mnow();
    //double seconds = (current.count() - tick.count()) / 1000.0;
    double forward_seconds = (current.count() - forward_tick.count()) / 1000.0;
    double  strafe_seconds = (current.count() -  strafe_tick.count()) / 1000.0;
    
    float speed = 1;
    float fspeed = speed * forward_seconds;
    float sspeed = speed * strafe_seconds;
    if (controls->forward && !controls->back) {
        camera->move(-fspeed);
    } else if (controls->forwardSlow && !controls->back) {
        camera->move(0.05);
    } else if (controls->back) {
        camera->move(fspeed);
    } else {
        forward_tick = mnow();
    }
    
    if (controls->left && !controls->right) {
        camera->strafe(-sspeed);
    } else if (controls->right) {
        camera->strafe(+sspeed);
    } else {
        strafe_tick = mnow();
    }
    tick = mnow();
}

- (void)_update
{
    [self applyControls];
    camera->look(options);
    
    //_rotation = 106.198952f;
}

// Called whenever view changes orientation or layout is changed
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
    [self _reshape];
}

// Called whenever the view needs to render
- (void)drawInMTKView:(nonnull MTKView *)view
{
    @autoreleasepool {
        [self _render];
    }
}

#pragma mark Controls
CGPoint prevDown;
CGPoint prevDownLeft;
-(void)mouseDown:(NSEvent *)theEvent {
    prevDownLeft = [NSEvent mouseLocation];
    //[theEvent locationInWindow].x, [theEvent locationInWindow].y
}
- (void)mouseDragged:(NSEvent *)theEvent {
    NSPoint dragPoint = [NSEvent mouseLocation];
    //(dragPoint.x - prevDownLeft.x), (dragPoint.y - prevDownLeft.y)
    prevDownLeft = dragPoint;
}

- (void)rightMouseDown:(NSEvent *)event {
    //[theEvent locationInWindow].x, [theEvent locationInWindow].y
    prevDown = [NSEvent mouseLocation];
}

- (void)rightMouseDragged:(NSEvent *)theEvent {
    NSPoint dragPoint = [NSEvent mouseLocation];
    camera->drag(-(dragPoint.x - prevDown.x), -(dragPoint.y - prevDown.y));
    prevDown = dragPoint;
}

-(void)updateMasks:(NSEvent *)theEvent {
    ((Control*)controls)->shift = (([theEvent modifierFlags] & NSShiftKeyMask) != 0);
    ((Control*)controls)->control = (([theEvent modifierFlags] & NSControlKeyMask) != 0);
}

-(void)keyDown:(NSEvent *)theEvent {
    unichar character = [[theEvent characters] characterAtIndex:0];
    switch (character)
    {
        case 'w':
            ((Control*)controls)->forward = true;
            break;
        case 'e':
            ((Control*)controls)->forwardSlow = true;
            break;
        case 's':
            ((Control*)controls)->back = true;
            break;
        case 'a':
            ((Control*)controls)->left = true;
            break;
        case 'd':
            ((Control*)controls)->right = true;
            break;
    }
    [self updateMasks:theEvent];
}

-(void)keyUp:(NSEvent *)theEvent {
    
    unichar character = [[theEvent characters] characterAtIndex:0];
    switch (character)
    {
        case 'w':
            ((Control*)controls)->forward = false;
            break;
        case 'e':
            ((Control*)controls)->forwardSlow = false;
            break;
        case 's':
            ((Control*)controls)->back = false;
            break;
        case 'a':
            ((Control*)controls)->left = false;
            break;
        case 'd':
            ((Control*)controls)->right = false;
            break;
    }
    [self updateMasks:theEvent];
}


-(BOOL)acceptsFirstResponder {
    return YES;
}

#pragma mark Utilities

static matrix_float4x4
matrix_from_perspective_fov_aspectLH(const float fovY, const float aspect, const float nearZ, const float farZ)
{
    float yscale = 1.0f / tanf(fovY * 0.5f); // 1 / tan == cot
    float xscale = yscale / aspect;
    float q = farZ / (farZ - nearZ);
    
    matrix_float4x4 m = {
        .columns[0] = { xscale, 0.0f, 0.0f, 0.0f },
        .columns[1] = { 0.0f, yscale, 0.0f, 0.0f },
        .columns[2] = { 0.0f, 0.0f, q, 1.0f },
        .columns[3] = { 0.0f, 0.0f, q * -nearZ, 0.0f }
    };
    
    return m;
}

static matrix_float4x4 matrix_from_translation(float x, float y, float z)
{
    matrix_float4x4 m = matrix_identity_float4x4;
    m.columns[3] = (vector_float4) { x, y, z, 1.0 };
    return m;
}

static matrix_float4x4 matrix_from_rotation(float radians, float x, float y, float z)
{
    vector_float3 v = vector_normalize(((vector_float3){x, y, z}));
    float cos = cosf(radians);
    float cosp = 1.0f - cos;
    float sin = sinf(radians);
    
    matrix_float4x4 m = {
        .columns[0] = {
            cos + cosp * v.x * v.x,
            cosp * v.x * v.y + v.z * sin,
            cosp * v.x * v.z - v.y * sin,
            0.0f,
        },
        
        .columns[1] = {
            cosp * v.x * v.y - v.z * sin,
            cos + cosp * v.y * v.y,
            cosp * v.y * v.z + v.x * sin,
            0.0f,
        },
        
        .columns[2] = {
            cosp * v.x * v.z + v.y * sin,
            cosp * v.y * v.z - v.x * sin,
            cos + cosp * v.z * v.z,
            0.0f,
        },
        
        .columns[3] = { 0.0f, 0.0f, 0.0f, 1.0f
        }
    };
    return m;
}


@end
