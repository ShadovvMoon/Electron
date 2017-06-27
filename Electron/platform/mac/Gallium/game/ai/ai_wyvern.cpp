//
//  ai_wyvern.cpp
//  HaloBot
//
//  Created by Samuco on 12/12/2015.
//  Copyright © 2015 samuco. All rights reserved.
//

#include "ai_wyvern.hpp"
#include <math.h>
#import "ZZTHaloWeaponTag.h"
#import "ZZTHaloProjectileTag.h"
#import "ZZTHaloDamageTag.h"

float angleBetweenLinesInRad(float l1sx, float l1sy, float l1ex, float l1ey,
                             float l2sx, float l2sy, float l2ex, float l2ey) {
    float a = l1ex - l1sx;
    float b = l1ey - l1sy;
    float c = l2ex - l2sx;
    float d = l2ey - l2sy;
    
    float atanA = atan2(a, b);
    float atanB = atan2(c, d);
    return atanA - atanB;
}

uint8_t* tag2mem(ProtonTag *scenario, uint32_t address) {
    return (uint8_t*)(scenario->Data() + scenario->PointerToOffset(address));
}

ai_wyvern::ai_wyvern(BSP *mesh, ProtonMap *map, ProtonTag *scenario) {
    bsp = mesh;
    this->map = map;
    this->scenario = scenario;
}

bool ai_wyvern::canSee(vector3d *me, vector3d *location) {
    vector3d *intersect = bsp->intersect(me, location, map, scenario);
    bool visible = true;
    if (intersect != nullptr) {
        visible = false;
    }
    delete intersect;
    return visible;
}

float weapRange(ProtonMap *map, ProtonTag *tag) {
    HaloWeaponData *data = (HaloWeaponData *)tag->Data();
    if (data) {
        HaloTagReflexive triggers = data->triggers;
        int i;
        for (i=0; i < triggers.count; i++) {
            HaloWeaponTrigger *trigger = (HaloWeaponTrigger *)(tag2mem(tag, (uint32_t)(uint64_t)triggers.address) + i * sizeof(HaloWeaponTrigger));
            if (trigger) {
                if (trigger->projectile.tag_id.tag_index != NULLED_TAG_ID) {
                    ProtonTag *projTag = map->tags.at(trigger->projectile.tag_id.tag_index).get();
                    if (projTag) {
                        HaloProjectileTag *projectile = (HaloProjectileTag *)projTag->Data();
                        if (projectile) {
                            return projectile->detonationMaximumRange;
                        }
                    }
                }
            }
        }
    }
    return -1.0;
}

float netWeaponRange(ProtonMap *map, NetworkObject *weapon) {
    if (weapon && weapon->table_id >= 0 && weapon->table_id < map->tags.size()) {
        ProtonTag *weaponTag = map->tags.at(weapon->table_id).get();
        return weapRange(map, weaponTag);
    }
    return -2.0;
}

void ai_wyvern::tick(Game *game, uint16_t bno) {
    Biped *meb = game->getBiped(bno);
    if (meb) {
        double height = 0.68;
        double crouch = 0.5;
        vector3d *me    = new vector3d(meb->position);
        me->z    +=     meb->controls.jumping.crouch ? crouch : height;
        
        // Classify our weapon information
        float weaponSpeed = 1000.0;
        float weaponRange = 5.0;
        
        NetworkObject *weapon = game->getObject(meb->weapon_object_id);
        weaponRange = netWeaponRange(map, weapon);
 
        vector3d *aimTarget  = nullptr;
        vector3d *moveTarget = nullptr;
        bool targetAttack = false;
        
        // Search nearby 'avoid' objects
        int i;
        for (i=0; i < kMaxObjects; i++) {
            NetworkObject *object = game->getObject(i);
            
        }
        
        // Search for a player to atttack
        float closestDistance = 1000.0;
        bool hasEnemy = false;
        
        for (i=0; i < kMaxBipeds; i++) {
            if (i == bno) {
                continue;
            }
            Biped *pd = game->getBiped(i);
            
            // Account crouching in the z offset
            vector3d *enemy = new vector3d(pd->position);
            enemy->z += pd->controls.jumping.crouch ? crouch : height;
            
            // Find an alive player, not on our team
            if (pd != nullptr && pd->alive && pd->team != meb->team) {
                float distance = pd->position->distance(meb->position);
                if (distance < weaponRange / 2.0) {
        
                    // Can they see us? Are they aiming near us? If so, force their attack
                    
                    
                }
                
                // Target this player (and force for the next few ticks)
                if (distance < closestDistance) {
                    closestDistance = distance;
                    aimTarget = new vector3d(enemy);
                    moveTarget = new vector3d(enemy);
                    targetAttack = true;
                }
            }
            delete enemy;
        }
        
        // Find a path to the target
        if (moveTarget != nullptr) {
            if (moveTarget->distance(meb->position) > 0.5) {
                if (path == NULL || pathTick > 250) {
                    float *newPath = findPath(bsp, map, meb->position, moveTarget);
                    if (newPath != NULL) {
                        if (path != NULL) {
                            free(path);
                        }
                        path = newPath;
                        pathTick = 0;
                        pointTick = 0;
                    }
                }
                
                delete moveTarget;
                if (path != NULL && pointTick < 30) {
                    moveTarget = new vector3d(path[pointTick*3+0], path[pointTick*3+1], path[pointTick*3+2]);
                    if (moveTarget->distance(meb->position) < 0.5) {
                        pointTick++;
                    }
                } else {
                    moveTarget = nullptr;
                }
            }
        }
        pathTick++;
        
        // If we have a target, aim towards it
        if (aimTarget != nullptr && moveTarget != nullptr) {
            
            // Aim at the target
            float dx = aimTarget->x - me->x;
            float dy = aimTarget->y - me->y;
            float dz = aimTarget->z - me->z;
            float dist = aimTarget->distance(me);
            
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
            if (!targetAttack) {
                ry = 0;
            }
            aimTarget->z -= -height; // tweak for up canSee
            
            meb->controls.look_y = ry;
            meb->controls.look_x = rx;
            meb->controls.jumping.crouch    = false;
            meb->controls.jumping.jump      = false;
            
            // Shoot at the target if applicable
            meb->controls.jumping.shoot     = canSee(aimTarget, me) && dist < weaponRange && arc4random()%2 == 1;
            meb->controls.jumping.melee     = false;
            meb->controls.jumping.action    = false;
            
            // Change weapons if the other one is better
            NetworkObject *weap1 = game->getObject(meb->weapon_slots[0]);
            NetworkObject *weap2 = game->getObject(meb->weapon_slots[1]);
            
            float range1 = netWeaponRange(map, weap1);
            float range2 = netWeaponRange(map, weap2);
            int currSlot = meb->weapon_object_id == meb->weapon_slots[0] ? 1 : 2;
            int goalSlot = (range1 > range2) ? 1 : 2;
            meb->controls.tab = currSlot != goalSlot;
            ((void (*)(int, const char*, ...))0x1588a8)(0x343aa0, "Weapon range %f %f. Switch = %d", range1, range2, meb->controls.tab?1:0);
            
            // Random zoom to increase accuracy
            meb->controls.zooming = !meb->controls.zooming;
            
            // Move toward the target
            float x  = meb->position->x;
            float y  = meb->position->y;
            meb->controls.movement.left     = false;
            meb->controls.movement.right    = false;
            meb->controls.movement.backwards= false;
            meb->controls.movement.forwards = false;
            
            float dot2 = dx * (moveTarget->x-x) + dy * (moveTarget->y-y);
            if (dot2 > 0) {
                meb->controls.movement.forwards  = true;
            } else {
                meb->controls.movement.backwards = true;
            }
            float dot = (dx) * -(moveTarget->y-y) + dy * (moveTarget->x-x);
            if (dot > 0) {
                meb->controls.movement.right = true;
            } else {
                meb->controls.movement.left  = true;
            }

            delete aimTarget;
            delete moveTarget;
        } else {
            debug("Missing target");
            meb->controls.jumping.shoot     = false;
            meb->controls.jumping.crouch    = false;
            meb->controls.jumping.jump      = false;
            meb->controls.movement.left     = false;
            meb->controls.movement.right    = false;
            meb->controls.movement.backwards= false;
            meb->controls.movement.forwards = true;
        }
        
        delete me;
    }
}
