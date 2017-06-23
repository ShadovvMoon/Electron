//
//  GameViewController.m
//  Metallic
//
//  Created by Samuco on 23/6/16.
//  Copyright (c) 2016 Samuco. All rights reserved.
//

#define RENDER_CORE_32 // Disables camera gl operations
#import "MRenderView.h"
#import "SharedStructures.h"
#import "ProtonMap.h"
#import "camera.h"
#import "bitmaps.h"

#import <simd/simd.h>
#import <ModelIO/ModelIO.h>
#import <GLKit/GLKit.h>

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

#pragma mark Textures
#define BITM_FORMAT_A8			0x00
#define BITM_FORMAT_Y8			0x01
#define BITM_FORMAT_AY8			0x02
#define BITM_FORMAT_A8Y8		0x03
#define BITM_FORMAT_R5G6B5		0x06
#define BITM_FORMAT_A1R5G5B5	0x08
#define BITM_FORMAT_A4R4G4B4	0x09
#define BITM_FORMAT_X8R8G8B8	0x0A
#define BITM_FORMAT_A8R8G8B8	0x0B
#define BITM_FORMAT_DXT1		0x0E
#define BITM_FORMAT_DXT2AND3	0x0F
#define BITM_FORMAT_DXT4AND5	0x10
#define BITM_FORMAT_P8			0x11

// Types
#define BITM_TYPE_2D			0x00
#define BITM_TYPE_3D			0x01
#define BITM_TYPE_CUBEMAP		0x02

// Flags
#define BITM_FLAG_LINEAR		(1 << 4)

// Structs
typedef struct
{
    uint16_t type;
    uint16_t format;
    //uint16_t usage;
    int32_t unknown[0x14];
    HaloTagReflexive reflexive_to_first;
    HaloTagReflexive image_reflexive;
} bitm_header_t;

typedef struct
{
    int							unknown[16];
    
} bitm_first_t;

typedef struct
{
    int							id;			// 'bitm' 0x0
    short						width;      //04
    short						height;     //06
    short						depth;      //08
    short						type;       //10
    short						format;     //12
    //short						flags;
    char						flag0;      //14
    char						internalized; //15
    short						reg_point_x; //16
    short						reg_point_y; //18
    short						num_mipmaps; //20
    short						pixel_offset; //22
    int         				offset; //24
    int							size;
    int							unknown8;
    int							unknown9;	// always 0xFFFFFFFF?
    int							unknown10;	// always 0x00000000?
    int							unknown11;	// always 0x024F0040?
} bitm_image_t;

// Conversion
typedef struct
{
    unsigned int r, g, b, a;
} rgba_color_t;

void DecodeLinearR5G6B5 (int width, int height, const char *texdata, unsigned int *outdata)
{
    //CSLog(@"BITM_FORMAT_A8R8G8B8");
    unsigned short cdata;
    int x,y;
    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++){
            cdata = ((unsigned short *)texdata)[(y * width) + x]; //crash if no bitmaps
            outdata[(y * width) + x]  = 0xFF000000 |
            ((cdata & 0xF800) >> 8)|
            ((cdata & 0x07e0) << 5)|
            ((cdata & 0x1f) << 19); //rgba_to_int (color);
        }
}



/*================================
 * DecodeLinearX8R8G8B8
 ================================*/
void DecodeLinearX8R8G8B8 (int width, int height, const char *texdata, unsigned int *outdata)
{
    rgba_color_t	color;
    unsigned int	cdata;
    int x,y;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            cdata = ((int *)texdata)[(y * width) + x];
            color.a = 255;
            color.r = (cdata >> 16) & 0xFF;
            color.g = (cdata >>  8) & 0xFF;
            color.b = (cdata >>  0) & 0xFF;
            
            outdata[(y * width) + x] = (255 << 24) | (color.r << 16) | (color.g << 8) | (color.b); //0xff000000 | cdata & 0xffffff;
            
            outdata[(y * width) + x]  = 0xFF000000 |
            (((cdata) & 0xff0000) >> 16)|
            ((cdata) & 0xff00)|
            (((cdata) & 0xff) << 16);
            
        }
    }
}
/*================================
 * DecodeLinearA8R8G8B8
 ================================*/
void DecodeLinearA8R8G8B8 (int width, int height, const char *texdata, unsigned int *outdata)
{
    rgba_color_t	color;
    unsigned int	cdata;
    int x,y;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            cdata = ((int *)texdata)[(y * width) + x];
            //cdata = bmpEndianSwap32(cdata);
            
            color.a = (cdata >> 24) & 0xFF;
            color.r = (cdata >> 16) & 0xFF;
            color.g = (cdata >>  8) & 0xFF;
            color.b = (cdata >>  0) & 0xFF;
            
            // //Pretty sky!
            outdata[(y * width) + x]  = (((cdata >> 24) & 0xff) << 24 ) |
            (((cdata) & 0xff0000) >> 16)|
            ((cdata) & 0xff00)|
            (((cdata) & 0xff) << 16);
        }
    }
}

@interface TextureManager : NSObject
{
    NSMutableDictionary *textures;
}
@end
@implementation TextureManager
-(NSArray*)createTextureWithDevice:(id <MTLDevice>)device Map:(ProtonMap*)map tag:(HaloTagDependency)bitm {
    // Check
    if (bitm.tag_id.tag_index == NULLED_TAG_ID) {
        printf("invalid bitmap\n");
        return nil;
    }
    
    // Load the texture (texture 0 for now)
    ProtonTag *bitmapTag = map->tags.at(bitm.tag_id.tag_index).get();
    if (bitmapTag) {
        bitm_header_t *bitmData = (bitm_header_t *)bitmapTag->Data();
        
        
        NSMutableArray *textureLevels = [[NSMutableArray alloc] init];
        int i;
        for (i=0; i < bitmData->image_reflexive.count; i++) {
            
            bitm_image_t *image = (bitm_image_t *)(bitmapTag->Data() + bitmapTag->PointerToOffset(bitmData->image_reflexive.address) + sizeof(bitm_image_t) * i);
            char *input = (char*)bitmapTag->ResourcesData() + image->offset;
            if (image == NULL) continue;
            
            // If the image is in bitmaps.map, read from that instead
            if (image->internalized == 1) {
                input = (char*)bitmap_data() + image->offset;
            };
            
            
            // Create the texture descriptor
            MTLTextureDescriptor *textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:image->width height:image->height mipmapped:YES];
            id<MTLTexture> texture = [device newTextureWithDescriptor:textureDescriptor];
            MTLRegion region = MTLRegionMake2D(0, 0, image->width, image->height);
            
            // Convert textures into desired format
            if (image->format == BITM_FORMAT_X8R8G8B8) {
                char *output = (char*)malloc(4 * image->width * image->height);
                DecodeLinearX8R8G8B8(image->width,image->height,input,(unsigned int*)output);
                [texture replaceRegion:region mipmapLevel:0 withBytes:output bytesPerRow:4 * image->width];
                free(output);
            } else if (image->format == BITM_FORMAT_A8R8G8B8) {
                char *output = (char*)malloc(4 * image->width * image->height);
                DecodeLinearA8R8G8B8(image->width,image->height,input,(unsigned int*)output);
                [texture replaceRegion:region mipmapLevel:0 withBytes:output bytesPerRow:4 * image->width];
                free(output);
            } else if (image->format == BITM_FORMAT_R5G6B5) {
                char *output = (char*)malloc(4 * image->width * image->height);
                DecodeLinearR5G6B5(image->width,image->height,input,(unsigned int*)output);
                [texture replaceRegion:region mipmapLevel:0 withBytes:output bytesPerRow:4 * image->width];
                free(output);
            } else {
                printf("unknown format\n");
                return @[];
            }
            [textureLevels addObject:texture];
        }
        return textureLevels;
    }
    
    printf("missing bitmap\n");
    return @[];
}
@end

#pragma mark Shaders
@interface Shader : NSObject
-(id)initWithMap:(ProtonMap*)map tag:(HaloTagDependency)shader;
@end
@implementation Shader
-(id)initWithMap:(ProtonMap*)map tag:(HaloTagDependency)shader {
    self = [super init];
    if (self) {
        //baseMap = manager->texture_manager()->create_texture(map, *(HaloTagDependency*)(shaderTag->Data() + 0x88));
        
    }
    return self;
}
@end

// senv
@interface Shader_senv : Shader @end
@implementation Shader_senv
-(id)initWithMap:(ProtonMap*)map tag:(HaloTagDependency)shader {
    self = [super init];
    if (self) {
        
    }
    return self;
}
@end

#pragma mark Renderer
@implementation MRenderView
{
    // view
    MTKView *_view;
    
    // camera
    Camera *camera;
    shader_options *options;
    Control *controls;
    
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

#include "defines.h"

uint8_t* map2mem(ProtonTag *scenario, uint32_t address) {
    return (uint8_t*)(scenario->Data() + scenario->PointerToOffset(address));
}

ProtonMap *map;
-(void)setData:(NSData*)data {
    NSLog(@"Loading map...");
    map = new ProtonMap([data bytes]);
}

-(NSData *)getData {
    ProtonCacheFile cache = map->ToCacheFile();
    return [[NSData alloc] initWithBytes:cache.Data() length:cache.Length()];
}


NSMutableDictionary *shaders = [[NSMutableDictionary alloc] init];
-(Shader*)loadShader:(HaloTagDependency) shader {
    
    // Do we already have this shader?
    NSNumber *key = [NSNumber numberWithInt:shader.tag_id.tag_index];
    Shader *object = [shaders objectForKey:key];
    if (object) {
        return object;
    }
    
    ProtonTag *shaderTag = map->tags.at((uint16_t)shader.tag_id.tag_index).get();
    if(strncmp(shaderTag->tag_classes, "vnes", 4) == 0) { // senv shader
        object = [[Shader_senv alloc] initWithMap:map tag:shader];
    } else {
        object = [[Shader alloc] initWithMap:map tag:shader];
    }
    [shaders setObject:object forKey:key];
    return object;
}

-(void)loadMap {
    if (map == NULL) {
        return;
    }
    
    // Load the map
    uint16_t scenarioTag = map->principal_tag;
    if (scenarioTag != NULLED_TAG_ID) {
        ProtonTag *scenarioTag = map->tags.at(map->principal_tag).get();
        ProtonTag *scenario = scenarioTag;
        HaloTagReflexive bsp = ((HaloScenarioTag*)scenario->Data())->bsp;

        /// Indices for buffer bind points.
        enum AAPLBufferIndex  {
            AAPLMeshVertexBuffer      = 0,
            AAPLFrameUniformBuffer    = 1,
            AAPLMaterialUniformBuffer = 2,
        };
        enum AAPLVertexAttributes {
            AAPLVertexAttributePosition = 0,
            AAPLVertexAttributeNormal   = 1,
            AAPLVertexAttributeBinormal = 1,
            AAPLVertexAttributeTangent  = 1,
            AAPLVertexAttributeTexcoord = 2,
        };
        
        // Set up the vertex descriptor
        MTLVertexDescriptor *mtlVertexDescriptor = [[MTLVertexDescriptor alloc] init];
        
        // Positions.
        mtlVertexDescriptor.attributes[AAPLVertexAttributePosition].format = MTLVertexFormatFloat3;
        mtlVertexDescriptor.attributes[AAPLVertexAttributePosition].offset = 0;
        mtlVertexDescriptor.attributes[AAPLVertexAttributePosition].bufferIndex = AAPLMeshVertexBuffer;
        
        // Normals
        mtlVertexDescriptor.attributes[AAPLVertexAttributeNormal].format = MTLVertexFormatFloat3;
        mtlVertexDescriptor.attributes[AAPLVertexAttributeNormal].offset = 3 * sizeof(GLfloat);
        mtlVertexDescriptor.attributes[AAPLVertexAttributeNormal].bufferIndex = AAPLMeshVertexBuffer;
        
        // Binormals
        mtlVertexDescriptor.attributes[AAPLVertexAttributeBinormal].format = MTLVertexFormatFloat3;
        mtlVertexDescriptor.attributes[AAPLVertexAttributeBinormal].offset = 6 * sizeof(GLfloat);
        mtlVertexDescriptor.attributes[AAPLVertexAttributeBinormal].bufferIndex = AAPLMeshVertexBuffer;
        
        // Tangents
        mtlVertexDescriptor.attributes[AAPLVertexAttributeTangent].format = MTLVertexFormatFloat3;
        mtlVertexDescriptor.attributes[AAPLVertexAttributeTangent].offset = 9 * sizeof(GLfloat);
        mtlVertexDescriptor.attributes[AAPLVertexAttributeTangent].bufferIndex = AAPLMeshVertexBuffer;
        
        // Texture Coordinates
        mtlVertexDescriptor.attributes[AAPLVertexAttributeTexcoord].format = MTLVertexFormatFloat2;
        mtlVertexDescriptor.attributes[AAPLVertexAttributeTexcoord].offset = 12 * sizeof(GLfloat);
        mtlVertexDescriptor.attributes[AAPLVertexAttributeTexcoord].bufferIndex = AAPLMeshVertexBuffer;
        
        // Layout
        mtlVertexDescriptor.layouts[AAPLMeshVertexBuffer].stride = 14 * sizeof(GLfloat);
        mtlVertexDescriptor.layouts[AAPLMeshVertexBuffer].stepRate = 1;
        mtlVertexDescriptor.layouts[AAPLMeshVertexBuffer].stepFunction = MTLVertexStepFunctionPerVertex;
        
        // Conversion
        MDLVertexDescriptor *mdlVertexDescriptor = MTKModelIOVertexDescriptorFromMetal(mtlVertexDescriptor);
        mdlVertexDescriptor.attributes[AAPLVertexAttributePosition].name = MDLVertexAttributePosition;
        mdlVertexDescriptor.attributes[AAPLVertexAttributeNormal].name   = MDLVertexAttributeNormal;
        mdlVertexDescriptor.attributes[AAPLVertexAttributeBinormal].name = MDLVertexAttributeBinormal;
        mdlVertexDescriptor.attributes[AAPLVertexAttributeTangent].name = MDLVertexAttributeTangent;
        mdlVertexDescriptor.attributes[AAPLVertexAttributeTexcoord].name = MDLVertexAttributeTextureCoordinate;
        
        // Load meshes
        //NSMutableArray *buffers = [[NSMutableArray alloc] init];
        NSMutableArray *meshes  = [[NSMutableArray alloc] init];
        MTKMeshBufferAllocator *allocator = [[MTKMeshBufferAllocator alloc] initWithDevice:_device];
        NSMutableData *vertexData = [[NSMutableData alloc] init];
        
        int maxVert = -1;
        int maxInde = 0;
        
        long vertices = 0;
        for (int i=0; i < bsp.count; i++) {
            BSP_CHUNK *chunk = (BSP_CHUNK *)(map2mem(scenario, bsp.address) + sizeof(BSP_CHUNK) * i); // VERIFIED
            ProtonTag *bspTag = map->tags.at((uint16_t)(chunk->tagId)).get();
            uint32_t mesh_offset = *(uint32_t *)(bspTag->Data());
            BSP_MESH *mesh = (BSP_MESH *)map2mem(bspTag, mesh_offset);
            
            for (int m=0; m < mesh->submeshHeader.count; m++) {
                BSP_SUBMESH *submesh = (BSP_SUBMESH *)(map2mem(bspTag, mesh->submeshHeader.address) + sizeof(BSP_SUBMESH) * m);
                for (int n=0; n < submesh->material.count; n++) {
                    MATERIAL_SUBMESH_HEADER *material = (MATERIAL_SUBMESH_HEADER *)(map2mem(bspTag, submesh->material.address) + sizeof(MATERIAL_SUBMESH_HEADER) * n);
                    uint8_t *vertIndexOffset = (uint8_t *)((sizeof(TRI_INDICES) * material->VertIndexOffset) + map2mem(bspTag, mesh->submeshIndices.address));
                    uint8_t *PcVertexDataOffset = map2mem(bspTag, material->PcVertexDataOffset);
                    int indexCount = material->VertIndexCount * 3;
                    
                    // Create the shader
                    HaloTagDependency shader = material->ShaderTag;
                    
                    // Create the index mesh
                    NSData *index = [[NSData alloc] initWithBytes:vertIndexOffset length:indexCount * sizeof(uint16_t)];
                    uint16_t *bytePtr = (uint16_t *)[index bytes];
                    for (int v = 0; v < indexCount; v++) {
                        bytePtr[v] += vertices;
                    }
                    MDLMeshBufferData *indexData = [allocator newBufferWithData:index type:MDLMeshBufferTypeIndex];
                    MDLSubmesh *submesh = [[MDLSubmesh alloc] initWithIndexBuffer:indexData indexCount:indexCount indexType:MDLIndexBitDepthUint16 geometryType:MDLGeometryTypeTriangles material:NULL];
                    
                    // Append the vertex mesh
                    NSData *verts = [[NSData alloc] initWithBytes:PcVertexDataOffset length:material->VertexCount1 * sizeof(UNCOMPRESSED_BSP_VERT)];
                    vertices += material->VertexCount1;
                    [vertexData appendData:verts];
                    
                    // Add the mesh
                    [meshes addObject:submesh];
                    
                    // Find largest mesh
                    if (indexCount > maxVert) {
                        maxVert = indexCount;
                        maxInde = meshes.count - 1;
                    }
                }
            }
        }
        
        // Assemble the BSP mesh
        NSLog(@"Total vertices: %ld", vertices);
        MDLMeshBufferData *vertexBuffer = [allocator newBufferWithData:vertexData type:MDLMeshBufferTypeVertex];
        MDLMesh *bspMesh = [[MDLMesh alloc] initWithVertexBuffer:vertexBuffer vertexCount:vertices descriptor:mdlVertexDescriptor submeshes:meshes];
        
        //bspMesh = [[MDLMesh alloc] initMeshBySubdividingMesh:bspMesh submeshIndex:maxInde subdivisionLevels:1 allocator:bspMesh.allocator];
        
        _boxMesh = [[MTKMesh alloc] initWithMesh:bspMesh device:_device error:nil];
        
        
        
        
        
        //bsp->setup(map, scenarioTag);
        //objects->read(shaders, map, scenarioTag);
        //skies->read(objects, map, scenarioTag);
        
        // Set the fog
        //skies->options(options, map, scenarioTag);
    }
    //ready = true;
}

- (void)_loadAssets
{
    // Generate meshes
    [self loadMap];
    
    if (!_boxMesh) {
        //mdl = [[MDLMesh alloc] initMeshBySubdividingMesh:mdl submeshIndex:0 subdivisionLevels:1 allocator:mdl.allocator];
        MDLMesh *mdl = [MDLMesh newBoxWithDimensions:(vector_float3){1,1,1} segments:(vector_uint3){1,1,1}
                                        geometryType:MDLGeometryTypeTriangles inwardNormals:NO
                                           allocator:[[MTKMeshBufferAllocator alloc] initWithDevice: _device]];
        
        NSError *err = nil;
        _boxMesh = [[MTKMesh alloc] initWithMesh:mdl device:_device error:&err];
        if (err) {
            NSLog(@"ERROR: %@", [err description]);
            return;
        }
    }
    
    // Allocate one region of memory for the uniform buffer
    _dynamicConstantBuffer = [_device newBufferWithLength:kMaxBytesPerFrame options:0];
    _dynamicConstantBuffer.label = @"UniformBuffer";
    
    // Load the fragment program into the library
    id <MTLFunction> fragmentProgram = [_defaultLibrary newFunctionWithName:@"lighting_fragment"];
    
    // Load the vertex program into the library
    id <MTLFunction> vertexProgram = [_defaultLibrary newFunctionWithName:@"lighting_vertex"];
    
    // Create a vertex descriptor from the MTKMesh
    MTLVertexDescriptor *vertexDescriptor = MTKMetalVertexDescriptorFromModelIO(_boxMesh.vertexDescriptor);
    vertexDescriptor.layouts[0].stepRate = 1;
    vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    
    // Create a reusable pipeline state
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"MyPipeline";
    pipelineStateDescriptor.sampleCount = _view.sampleCount;
    pipelineStateDescriptor.vertexFunction = vertexProgram;
    pipelineStateDescriptor.fragmentFunction = fragmentProgram;
    pipelineStateDescriptor.vertexDescriptor = vertexDescriptor;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = _view.colorPixelFormat;
    pipelineStateDescriptor.depthAttachmentPixelFormat = _view.depthStencilPixelFormat;
    pipelineStateDescriptor.stencilAttachmentPixelFormat = _view.depthStencilPixelFormat;
    
    NSError *error = NULL;
    _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
    if (!_pipelineState) {
        NSLog(@"Failed to created pipeline state, error %@", error);
    }
    
    MTLDepthStencilDescriptor *depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
    depthStateDesc.depthWriteEnabled = YES;
    _depthState = [_device newDepthStencilStateWithDescriptor:depthStateDesc];
}

- (void)_render
{
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
        renderEncoder.label = @"MyRenderEncoder";
        [renderEncoder setDepthStencilState:_depthState];

        // Set context state
        [renderEncoder pushDebugGroup:@"DrawCube"];
        [renderEncoder setRenderPipelineState:_pipelineState];
        [renderEncoder setVertexBuffer:_boxMesh.vertexBuffers[0].buffer offset:_boxMesh.vertexBuffers[0].offset atIndex:0 ];
        [renderEncoder setVertexBuffer:_dynamicConstantBuffer offset:(sizeof(uniforms_t) * _constantDataBufferIndex) atIndex:1 ];
        
        int i;
        for (i = 0; i < _boxMesh.submeshes.count; i++) {
            MTKSubmesh* submesh = _boxMesh.submeshes[i];
            [renderEncoder drawIndexedPrimitives:submesh.primitiveType indexCount:submesh.indexCount indexType:submesh.indexType indexBuffer:submesh.indexBuffer.buffer indexBufferOffset:submesh.indexBuffer.offset];
        }
        
        // Pop the debug group
        [renderEncoder popDebugGroup];
        
        // We're done encoding commands
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
    
    _viewMatrix = matrix_identity_float4x4;
}

#include <chrono>
using namespace std::chrono;

// movement
milliseconds tick;
milliseconds forward_tick;
milliseconds strafe_tick;

milliseconds now() {
    return duration_cast< milliseconds >(high_resolution_clock::now().time_since_epoch());
}

-(void)applyControls {
    milliseconds current = now();
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
        forward_tick = now();
    }
    
    if (controls->left && !controls->right) {
        camera->strafe(-sspeed);
    } else if (controls->right) {
        camera->strafe(+sspeed);
    } else {
        strafe_tick = now();
    }
    tick = now();
}


- (void)_update
{
    [self applyControls];
    
    /*
    matrix_float4x4 base_model = matrix_multiply(matrix_from_translation(0.0f, 0.0f, 5.0f), matrix_from_rotation(_rotation, 0.0f, 1.0f, 0.0f));
    matrix_float4x4 base_mv = matrix_multiply(_viewMatrix, base_model);
    matrix_float4x4 modelViewMatrix = matrix_multiply(base_mv, matrix_from_rotation(_rotation, 1.0f, 1.0f, 1.0f));
    
    camera->look(options);
    
    // Load constant buffer data into appropriate buffer at current index
    uniforms_t *uniforms = &((uniforms_t *)[_dynamicConstantBuffer contents])[_constantDataBufferIndex];
    
    uniforms->normal_matrix = matrix_invert(matrix_transpose(modelViewMatrix));
    uniforms->modelview_projection_matrix = matrix_multiply(_projectionMatrix, modelViewMatrix);
    
    _rotation = 106.198952f;
    
    
    camera->look(options);
    */
    
    
    
    
    matrix_float4x4 base_model = matrix_multiply(matrix_from_translation(0.0f, 0.0f, 5.0f), matrix_from_rotation(_rotation, 0.0f, 1.0f, 0.0f));
    matrix_float4x4 base_mv = matrix_multiply(_viewMatrix, base_model);
    //matrix_float4x4 modelViewMatrix = matrix_multiply(base_mv, matrix_from_rotation(_rotation, 1.0f, 1.0f, 1.0f));
    
    
    //matrix_float4x4 m = matrix_identity_float4x4;
    
    camera->look(options);
    
    /*
    NSLog(@"%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n",
          options->modelview[0], options->modelview[4], options->modelview[8 ], options->modelview[12],
          options->modelview[1], options->modelview[5], options->modelview[9 ], options->modelview[13],
          options->modelview[2], options->modelview[6], options->modelview[10], options->modelview[14],
          options->modelview[3], options->modelview[7], options->modelview[11], options->modelview[15]);
    */
    
    matrix_float4x4 look = matrix_identity_float4x4;
    /*
    look.columns[0] = (vector_float4) { options->modelview[0], options->modelview[4], options->modelview[8 ], options->modelview[12] };
    look.columns[1] = (vector_float4) { options->modelview[1], options->modelview[5], options->modelview[9 ], options->modelview[13] };
    look.columns[2] = (vector_float4) { options->modelview[2], options->modelview[6], options->modelview[10], options->modelview[14] };
    look.columns[3] = (vector_float4) { options->modelview[3], options->modelview[7], options->modelview[11], options->modelview[15] };
    */
    
    look.columns[0] = (vector_float4) { options->modelview[0], options->modelview[1], options->modelview[2], options->modelview[3] };
    look.columns[1] = (vector_float4) { options->modelview[4], options->modelview[5], options->modelview[6], options->modelview[7] };
    look.columns[2] = (vector_float4) { options->modelview[8], options->modelview[9], options->modelview[10], options->modelview[11] };
    look.columns[3] = (vector_float4) { options->modelview[12], options->modelview[13], options->modelview[14], options->modelview[15] };
    
    
    //matrix_float4x4 base_model = matrix_multiply(matrix_from_translation(0.0f, 0.0f, 5.0f), matrix_from_rotation(_rotation, 0.0f, 1.0f, 0.0f));
    //matrix_float4x4 base_mv = matrix_multiply(_viewMatrix, base_model);
    matrix_float4x4 modelViewMatrix = look; //matrix_multiply(base_mv, look);
    //modelViewMatrix = matrix_identity_float4x4;
    
    //m.columns[3] = (vector_float4) { x, y, z, 1.0 };
    //return m;
    
    //modelViewMatrix = options->modelview;
    
    // Load constant buffer data into appropriate buffer at current index
    uniforms_t *uniforms = &((uniforms_t *)[_dynamicConstantBuffer contents])[_constantDataBufferIndex];
    uniforms->normal_matrix = matrix_invert(matrix_transpose(modelViewMatrix));
    uniforms->modelview_projection_matrix = matrix_multiply(_projectionMatrix, modelViewMatrix);
    _rotation = 106.198952f;
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
