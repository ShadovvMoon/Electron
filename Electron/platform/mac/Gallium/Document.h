//
//  Document.h
//  Gallium
//
//  Created by Samuco on 6/23/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MRenderView.h"

@interface Document : NSDocument
{
    IBOutlet MRenderView *controller;
    NSData *map_data;
}

@end

