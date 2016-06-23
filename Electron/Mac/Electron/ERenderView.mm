//
//  ERenderView.m
//  Electron
//
//  Created by Samuco on 19/04/2015.
//  Copyright (c) 2015 Samuco. All rights reserved.
//

#import "ERenderView.hpp"
#import "../../OpenGL/render/render.h"

#include <mutex>          // std::mutex
std::mutex mtx;           // mutex for critical section

@implementation ERenderView

//E((ERenderer*)renderer) *((ERenderer*)renderer);

-(void)setDelegate:(id)del {
    delegate = del;
}

-(void*)renderer {
    return ((ERenderer*)renderer);
}

- (void)prepareOpenGL
{
    // Synchronize buffer swaps with vertical refresh rate
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
    
    // Create a display link capable of being used with all active displays
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    
    // Set the ((ERenderer*)renderer) output callback function
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
    
    // Set the display link for the current ((ERenderer*)renderer)
    CGLContextObj cglContext = [[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = [[self pixelFormat] CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    
    // Activate the display link
    CVDisplayLinkStart(displayLink);
}

// This is the ((ERenderer*)renderer) output callback function
static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
    CVReturn result = [(ERenderView*)displayLinkContext getFrameForTime:outputTime];
    return result;
}

- (CVReturn)getFrameForTime:(const CVTimeStamp*)outputTime
{
    // Add your drawing codes here
    ((ERenderer*)renderer)->applyControl(((Control*)controls));
    if ([[self window] isMainWindow]) {
        mtx.lock();
        //NSDate *start = [NSDate date];
        [[self openGLContext] makeCurrentContext];
        NSSize sceneBounds = [self bounds].size;
        ((ERenderer*)renderer)->resize(sceneBounds.width, sceneBounds.height);
        ((ERenderer*)renderer)->render();
        if (delegate != NULL) {
            [delegate render];
        }
        [[self openGLContext] flushBuffer];
        //NSTimeInterval timeInterval = -[start timeIntervalSinceNow];
        //frames++;
        //totalTime += timeInterval;
        //printf("%f %f (%f %f)\n", totalTime, frames/totalTime, timeInterval, 1.0/timeInterval);
        mtx.unlock();
    }
    return kCVReturnSuccess;
}

+(NSOpenGLPixelFormat*)pixelFormat {
    // Setup the OpenGL context
    NSOpenGLPixelFormatAttribute attr[] =
    {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize,
        (NSOpenGLPixelFormatAttribute)BITS_PER_PIXEL,
        NSOpenGLPFADepthSize,
        (NSOpenGLPixelFormatAttribute)DEPTH_SIZE,
#ifdef RENDER_GPU
        NSOpenGLPFAAccelerated,
#else
        NSOpenGLPFA((ERenderer*)renderer)ID, kCGL((ERenderer*)renderer)GenericID,
#endif
#ifdef RENDER_CORE_32
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
#endif
        NSOpenGLPFASampleBuffers, 1,
        NSOpenGLPFASamples, 4,
        0
    };
    
    // Next, we initialize the NSOpenGLPixelFormat itself
    NSOpenGLPixelFormat *nsglFormat;
    nsglFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attr];
    
    // Check for errors in the creation of the NSOpenGLPixelFormat
    // If we could not create one, return nil (the OpenGL is not initialized, and
    // we should send an error message to the user at this point)
    if(!nsglFormat) { NSLog(@"Invalid NSOpenGLPixelFormat"); return nil; }
    
    // Now we create the the CocoaGL instance, using our initial frame and the NSOpenGLPixelFormat
    return nsglFormat;
}

-(id)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect pixelFormat:[ERenderView pixelFormat]];
    if (self) {
        [self setPostsFrameChangedNotifications: YES];
        
        [[self openGLContext] makeCurrentContext];
        [[self openGLContext] setView:self];
        
        NSString *resourcesPath = [[NSBundle mainBundle] resourcePath];
        delegate = NULL;
        renderer = new ERenderer;
        controls = new Control; //(Control *)malloc(sizeof(Control));
        
        ((ERenderer*)renderer)->setup([resourcesPath cStringUsingEncoding:NSUTF8StringEncoding]);
    }
    return self;
}

-(BOOL)acceptsFirstResponder {
    return YES;
}

-(void)updateMasks:(NSEvent *)theEvent {
    ((Control*)controls)->shift = (([theEvent modifierFlags] & NSShiftKeyMask) != 0);
    ((Control*)controls)->control = (([theEvent modifierFlags] & NSControlKeyMask) != 0);
}

CGPoint prevDown;
CGPoint prevDownLeft;
-(void)mouseDown:(NSEvent *)theEvent {
    [self updateMasks:theEvent];
    prevDownLeft = [NSEvent mouseLocation];
    mtx.lock();
    ((ERenderer*)renderer)->mouseDown([theEvent locationInWindow].x, [theEvent locationInWindow].y);
    mtx.unlock();
}
- (void)mouseDragged:(NSEvent *)theEvent {
    [self updateMasks:theEvent];
    NSPoint dragPoint = [NSEvent mouseLocation];
    ((ERenderer*)renderer)->mouseDrag((dragPoint.x - prevDownLeft.x), (dragPoint.y - prevDownLeft.y));
    prevDownLeft = dragPoint;
}

- (void)rightMouseDown:(NSEvent *)event {
    [self updateMasks:event];
    prevDown = [NSEvent mouseLocation];
}

- (void)rightMouseDragged:(NSEvent *)theEvent {
    [self updateMasks:theEvent];
    NSPoint dragPoint = [NSEvent mouseLocation];
    
    if ((([theEvent modifierFlags] & NSControlKeyMask) == 0))
        ((ERenderer*)renderer)->rightMouseDrag((dragPoint.x - prevDown.x), (dragPoint.y - prevDown.y));
    
    prevDown = dragPoint;
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
        case 'q':
            ((ERenderer*)renderer)->useSSAO = !((ERenderer*)renderer)->useSSAO;
            break;
    }
    [self updateMasks:theEvent];
}

-(IBAction)generateLightmaps:(id)sender {
    uint16_t scenarioTag = ((ERenderer*)renderer)->map->principal_tag;
    if (scenarioTag != NULLED_TAG_ID) {
        ProtonTag *scenarioTag = ((ERenderer*)renderer)->map->tags.at(((ERenderer*)renderer)->map->principal_tag).get();
        ((ERenderer*)renderer)->bsp->generate_lightmap(((ERenderer*)renderer)->camera->position, ((ERenderer*)renderer)->map, scenarioTag);
    }
}

- (void)timerTick:(NSTimer *)timer
{
    //[[self openGLContext] update];
    
    //((ERenderer*)renderer)->applyControl(controls);
    //[self setNeedsDisplay:YES];
}

-(void)awakeFromNib {
    
}

ProtonMap *map;
-(void)setData:(NSData*)data {
    map = new ProtonMap([data bytes]);
    ((ERenderer*)renderer)->read(map);
}

-(NSData *)getData {
    ((ERenderer*)renderer)->write();
    ProtonCacheFile cache = map->ToCacheFile();
    return [[NSData alloc] initWithBytes:cache.Data() length:cache.Length()];
}

-(void)dealloc {
    delete map;
    delete ((ERenderer*)renderer);
    [super dealloc];
}

/*
- (void)drawRect:(NSRect)dirtyRect {
    
}
*/

@end
