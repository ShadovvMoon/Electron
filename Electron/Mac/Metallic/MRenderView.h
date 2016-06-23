//
//  GameViewController.h
//  Metallic
//
//  Created by Samuco on 23/6/16.
//  Copyright (c) 2016 Samuco. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

@interface MRenderView : NSViewController<MTKViewDelegate>
{
    
}
-(void)setData:(NSData*)data;
@end
