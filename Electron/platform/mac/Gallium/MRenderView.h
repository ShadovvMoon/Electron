//
//  MRenderView.h
//  Gallium
//
//  Created by Samuco on 6/23/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>

@interface MRenderView : NSViewController<MTKViewDelegate>
{
    
}
-(void)setData:(NSData*)data;
@end
