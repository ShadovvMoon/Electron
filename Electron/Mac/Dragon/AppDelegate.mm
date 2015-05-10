//
//  AppDelegate.m
//  Dragon
//
//  Created by Samuco on 9/05/2015.
//  Copyright (c) 2015 Samuco. All rights reserved.
//

#import "AppDelegate.h"
#import "bitmaps.h"
#import "ERenderView.h"
#import "../../OpenGL/render/render.h"

@interface AppDelegate ()

@property IBOutlet NSWindow *window;
@end

@implementation AppDelegate

NSDate *start = [NSDate date];
int tick = 0;
ERenderView *renderView;

-(void)render {
    //printf("render %d\n", tick);
    NSTimeInterval timeInterval = -[start timeIntervalSinceNow];
    start = [NSDate date];
    tick++;
    
    Client *main = clients[0];
    Game *game = main->game;
    if (game != nullptr) {
        for (int i=0; i<kMaxPlayers; i++) {
            Player *p = game->getPlayer(i);
            if (p && p->alive) {
                
                glUseProgram(0);
                glEnable(GL_COLOR_MATERIAL);
                if (p->team == 0) {
                    glColor4f(1.0, 0.0, 0.0, 1.0);
                } else {
                    glColor4f(0.0, 0.0, 1.0, 1.0);
                }
                
                glLineWidth(2.0);
                glPushMatrix();
                glTranslatef(p->position->x, p->position->y, p->position->z);
                GLUquadric *sphere=gluNewQuadric();
                gluQuadricDrawStyle( sphere, GLU_FILL);
                gluQuadricNormals( sphere, GLU_SMOOTH);
                gluQuadricOrientation( sphere, GLU_OUTSIDE);
                gluQuadricTexture( sphere, GL_TRUE);
                gluSphere(sphere,0.5,10,10);
                gluDeleteQuadric ( sphere );
                glPopMatrix();
                
                float distance = 1.0;
                float x = distance * cos(p->controls.look_y) * cos(p->controls.look_x);
                float y = distance * cos(p->controls.look_y) * sin(p->controls.look_x);
                float z = distance * sin(p->controls.look_y);
                
                // Move the player
                float forwardSpeed = 2.25 * timeInterval;
                float backwardsSpeed = 2 * timeInterval;
                if (p->controls.movement.forwards == true) {
                    p->position->x += x * forwardSpeed;
                    p->position->y += y * forwardSpeed;
                    //p->position->z += z * forwardSpeed;
                }
                if (p->controls.movement.backwards == true) {
                    p->position->x -= x * backwardsSpeed;
                    p->position->y -= y * backwardsSpeed;
                    //p->position->z -= z * backwardsSpeed;
                }
                
                // Fall to the bsp
                ERenderer *render = (ERenderer *)[renderView renderer];
                
                float gravity = 5.0 * timeInterval;
                uint16_t scenarioTag = render->map->principal_tag;
                if (scenarioTag != NULLED_TAG_ID) {
                    ProtonTag *scenarioTag = render->map->tags.at(render->map->principal_tag).get();
                    
                    float search = 2.0;
                    vector3d *position      = new vector3d(p->position->x, p->position->y, p->position->z+search);
                    vector3d *position_down = new vector3d(p->position->x, p->position->y, p->position->z-search);
                    vector3d *intersect = render->bsp->intersect(position_down, position, render->map, scenarioTag);
                    if (intersect != nullptr) {
                        if (p->position->z > intersect->z) {
                            p->position->z -= gravity;
                        } else {
                            p->position->z = intersect->z;
                        }
                    } else {
                        p->position->z -= gravity;
                    }
                }
                
                float line = 100.0;
                glBegin(GL_LINES);
                glVertex3f(p->position->x, p->position->y, p->position->z);
                glVertex3f(p->position->x+line*x, p->position->y+line*y, p->position->z+line*z);
                glEnd();
            }
        }
    }
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    
    // Insert code here to initialize your application
    load_bitmaps([[NSString stringWithFormat:@"%@/Library/Application Support/HaloMD/GameData/Maps/bitmaps.map", NSHomeDirectory()] cStringUsingEncoding:NSUTF8StringEncoding]);
    NSString *bloodgulch = [NSString stringWithFormat:@"%@/Library/Application Support/HaloMD/GameData/Maps/bloodgulch.map", NSHomeDirectory()];
    
    // Create a rendering view
    renderView = [[ERenderView alloc] initWithFrame:NSZeroRect];
    [renderView setFrame:[[_window contentView] bounds]];
    [renderView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [renderView setData:[[NSData alloc] initWithContentsOfFile:bloodgulch]];
    [renderView setDelegate:self];
    [[_window contentView] addSubview:renderView];
     
    // Disable app nap
    if ([[NSProcessInfo processInfo] respondsToSelector:@selector(beginActivityWithOptions:reason:)]) {
        id activity;
        activity = [[NSProcessInfo processInfo] beginActivityWithOptions:0x00FFFFFF reason:@"receiving OSC messages"];
        [activity retain];
    }
    
    char pname[255];
    sprintf(pname, "Dragon(test)");
    clients[0]  = new Client(pname, "107.191.57.51", 5000);
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

@end
