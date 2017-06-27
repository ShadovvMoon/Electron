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
    

    
    // Create a threaded client
    //char pname[255];
    //
    //int i;
    //for (i=0; i < 1; i++) {
    //    sprintf(pname, "Dragon%d", i+1);
    //    Client *c0  = new Client(pname, "107.191.57.51", 5000);
    //}
    
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender
{
    return NO;
}

@end
