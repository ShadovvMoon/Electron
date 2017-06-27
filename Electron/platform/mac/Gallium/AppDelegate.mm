//
//  AppDelegate.m
//  Gallium
//
//  Created by Samuco on 6/23/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#import "AppDelegate.h"
#import "bitmaps.h"

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    const char *bitmap = [[NSString stringWithFormat:@"%@/Library/Application Support/HaloMD/GameData/Maps/bitmaps.map", NSHomeDirectory()] cStringUsingEncoding:NSUTF8StringEncoding];
    
    // Insert code here to initialize your application
    load_bitmaps(bitmap);
    NSLog(@"bitmaps loaded");
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}


@end
