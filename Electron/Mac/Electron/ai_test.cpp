//
//  ai_test.cpp
//  Electron
//
//  Created by Samuco on 21/07/2015.
//  Copyright © 2015 Samuco. All rights reserved.
//

#include "ai_test.hpp"
#include <math.h>

ai_test::ai_test(BSP *mesh, ProtonMap *map, ProtonTag *scenario) {
    bsp = mesh;
    this->map = map;
    this->scenario = scenario;
}

bool ai_test::canSee(vector3d *me, vector3d *location) {
    vector3d *intersect = bsp->intersect(me, location, map, scenario);
    bool visible = true;
    if (intersect != nullptr) {
        visible = false;
    }
    delete intersect;
    return visible;
}

void ai_test::tick(Game *game, uint16_t bno) {
    Biped *meb = game->getBiped(0);
    if (meb) {
        int i; float minDist = 500.0; Biped *closest = nullptr;
        for (i=1; i < kMaxBipeds; i++) {
            Biped *pd = game->getBiped(i);
            if (pd != nullptr && pd->alive) {
                float distance = pd->position->distance(meb->position);
                if (distance < minDist) {
                    minDist = distance;
                    closest = pd;
                }
            }
        }
        
        if (closest) {
            vector3d *me    = new vector3d(meb->position);
            vector3d *enemy = new vector3d(closest->position);
            me->z    +=     meb->controls.jumping.crouch ? 0.5 : 0.75;
            enemy->z += closest->controls.jumping.crouch ? 0.5 : 0.75;;

            float dx = enemy->x - me->x;
            float dy = enemy->y - me->y;
            float dz = enemy->z - me->z;
            float dist = enemy->distance(me);
            
            float rx = 0.0;
            if (dx > 0 && dy > 0) {
                rx = atan2f(dy, dx);
            } else if (dx < 0 && dy > 0) {
                rx = M_PI - atan2f(dy, -dx);
            } else if (dx < 0 && dy < 0) {
                rx = M_PI + atan2f(-dy, -dx);
            } else if (dx > 0 && dy < 0) {
                rx = -atan2f(-dy, dx);
            }
            
            float ry = atan2f(dz, dist);
            meb->controls.look_y = ry;
            meb->controls.look_x = rx;
            meb->controls.movement.left     = false;
            meb->controls.movement.right    = false;
            meb->controls.movement.forwards = minDist > 5;
            meb->controls.jumping.crouch    = false;
            meb->controls.jumping.jump      = false;
            meb->controls.jumping.shoot     = (minDist < 10) && (arc4random() % 2) == 0 && canSee(me, enemy);
            meb->controls.jumping.melee     = false;
            
            delete me;
            delete enemy;
        } else {
            meb->controls.movement.left     = false;
            meb->controls.movement.right    = false;
            meb->controls.movement.backwards= false;
            meb->controls.movement.forwards = false;
            meb->controls.jumping.shoot     = false;
            meb->controls.jumping.crouch    = false;
            meb->controls.jumping.jump      = false;
        }
    }
}