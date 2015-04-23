//
//  ERenderView.h
//  Electron
//
//  Created by Samuco on 19/04/2015.
//  Copyright (c) 2015 Samuco. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface ERenderView : NSOpenGLView
{
    NSTimer *drawTimer;
}
-(void)setData:(NSData*)data;
-(NSData *)getData;
@end
