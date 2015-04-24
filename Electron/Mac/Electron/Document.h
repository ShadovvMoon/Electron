//
//  Document.h
//  Electron
//
//  Created by Samuco on 19/04/2015.
//  Copyright (c) 2015 Samuco. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "ERenderView.h"

@interface Document : NSDocument
{
    IBOutlet ERenderView *renderView;
}

@end

