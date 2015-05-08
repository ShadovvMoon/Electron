//
//  AppDelegate.m
//  Electron
//
//  Created by Samuco on 19/04/2015.
//  Copyright (c) 2015 Samuco. All rights reserved.
//

#import "AppDelegate.h"
#import "bitmaps.h"
#import "client.h"

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    load_bitmaps([[NSString stringWithFormat:@"%@/Library/Application Support/HaloMD/GameData/Maps/bitmaps.map", NSHomeDirectory()] cStringUsingEncoding:NSUTF8StringEncoding]);
    
    // Disable app nap
    //if ([[NSProcessInfo processInfo] respondsToSelector:@selector(beginActivityWithOptions:reason:)]) {
    //    id activity;
    //    activity = [[NSProcessInfo processInfo] beginActivityWithOptions:0x00FFFFFF reason:@"receiving OSC messages"];
    //    [activity retain];
    //}
    
    // Create a threaded client
    //char pname[255];
    //
    //int i;
    //for (i=0; i < 1; i++) {
    //    sprintf(pname, "Dragon%d", i+1);
    //    Client *c0  = new Client(pname, "107.191.57.51", 5000);
    //}
    
    
    
    //Client *c1  = new Client("107.191.57.51", 5000);
    //Client *c2  = new Client("107.191.57.51", 5000);
    //Client *c3  = new Client("107.191.57.51", 5000);

    //Client *c = new Client("10.0.1.28", 2345);
    //Client *c = new Client("127.0.0.1", 4000);w
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender
{
    return NO;
}

@end
