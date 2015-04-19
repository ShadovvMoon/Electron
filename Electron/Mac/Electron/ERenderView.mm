//
//  ERenderView.m
//  Electron
//
//  Created by Samuco on 19/04/2015.
//  Copyright (c) 2015 Samuco. All rights reserved.
//

#import "ERenderView.h"
#import "../../OpenGL/render.h"

@implementation ERenderView
ERenderer *renderer;

+(NSOpenGLPixelFormat*)pixelFormat {
    // Setup the OpenGL context
    NSOpenGLPixelFormatAttribute attr[] =
    {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize,
        (NSOpenGLPixelFormatAttribute)BITS_PER_PIXEL,
        NSOpenGLPFADepthSize,
        (NSOpenGLPixelFormatAttribute)DEPTH_SIZE,
        NSOpenGLPFAAccelerated,
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
        
        renderer = new ERenderer();
        renderer->setup();
        
        // Start the draw timer
        int fps = 100;
        drawTimer = [NSTimer scheduledTimerWithTimeInterval:(1.0/fps) target:self selector:@selector(timerTick:) userInfo:nil repeats:YES];
    }
    return self;
}

- (void)timerTick:(NSTimer *)timer
{
    [[self openGLContext] update];
    [self setNeedsDisplay:YES];
}

-(void)awakeFromNib {
    
}

-(void)setData:(NSData*)data {
    ProtonMap *map = new ProtonMap([data bytes]);
    renderer->setMap(map);
}

- (void)drawRect:(NSRect)dirtyRect {
    if ([[self window] isMainWindow]) {
        [[self openGLContext] makeCurrentContext];
    
        NSSize sceneBounds = [self bounds].size;
        renderer->resize(sceneBounds.width, sceneBounds.height);
        renderer->render();
        [[self openGLContext] flushBuffer];
    }
}

@end
