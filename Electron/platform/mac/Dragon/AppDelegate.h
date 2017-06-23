//
//  AppDelegate.h
//  Dragon
//
//  Created by Samuco on 9/05/2015.
//  Copyright (c) 2015 Samuco. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "client.h"
#import "ai_test.hpp"
#define kMaxClients 32

@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    Client *clients[kMaxClients];
}

@end

