//
//  ERenderView.m
//  Electron
//
//  Created by Samuco on 19/04/2015.
//  Copyright (c) 2015 Samuco. All rights reserved.
//

#import "ERenderView.h"
#import "../../OpenGL/render/render.h"

@implementation ERenderView
ERenderer *renderer;
Control *controls = (Control *)malloc(sizeof(Control));

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
        
        renderer = new ERenderer;
        renderer->setup();
        
        // Start the draw timer
        int fps = 60;
        drawTimer = [NSTimer scheduledTimerWithTimeInterval:(1.0/fps) target:self selector:@selector(timerTick:) userInfo:nil repeats:YES];
    }
    return self;
}

-(BOOL)acceptsFirstResponder {
    return YES;
}

-(void)updateMasks:(NSEvent *)theEvent {
    controls->shift = (([theEvent modifierFlags] & NSShiftKeyMask) != 0);
    controls->control = (([theEvent modifierFlags] & NSControlKeyMask) != 0);
}

CGPoint prevDown;
CGPoint prevDownLeft;
-(void)mouseDown:(NSEvent *)theEvent {
    [self updateMasks:theEvent];
    prevDownLeft = [NSEvent mouseLocation];
    renderer->mouseDown([theEvent locationInWindow].x, [theEvent locationInWindow].y);
}
- (void)mouseDragged:(NSEvent *)theEvent {
    [self updateMasks:theEvent];
    NSPoint dragPoint = [NSEvent mouseLocation];
    renderer->mouseDrag((dragPoint.x - prevDownLeft.x), (dragPoint.y - prevDownLeft.y));
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
        renderer->rightMouseDrag((dragPoint.x - prevDown.x), (dragPoint.y - prevDown.y));
    
    prevDown = dragPoint;
}

-(void)keyDown:(NSEvent *)theEvent {
    unichar character = [[theEvent characters] characterAtIndex:0];
    switch (character)
    {
        case 'w':
            controls->forward = true;
            break;
        case 's':
            controls->back = true;
            break;
        case 'a':
            controls->left = true;
            break;
        case 'd':
            controls->right = true;
            break;
    }
    [self updateMasks:theEvent];
}

-(void)keyUp:(NSEvent *)theEvent {
    
    unichar character = [[theEvent characters] characterAtIndex:0];
    switch (character)
    {
        case 'w':
            controls->forward = false;
            break;
        case 's':
            controls->back = false;
            break;
        case 'a':
            controls->left = false;
            break;
        case 'd':
            controls->right = false;
            break;
    }
    [self updateMasks:theEvent];
}

- (void)timerTick:(NSTimer *)timer
{
    [[self openGLContext] update];
    
    renderer->applyControl(controls);
    [self setNeedsDisplay:YES];
}

-(void)awakeFromNib {
    
}

ProtonMap *map;
-(void)setData:(NSData*)data {
    map = new ProtonMap([data bytes]);
    renderer->read(map);
}

-(NSData *)getData {
    renderer->write();
    ProtonCacheFile cache = map->ToCacheFile();
    return [[NSData alloc] initWithBytes:cache.Data() length:cache.Length()];
}

-(void)dealloc {
    delete map;
    delete renderer;
    [super dealloc];
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