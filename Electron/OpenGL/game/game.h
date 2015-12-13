//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef __game__
#define __game__

#define kMaxPlayers 17
#define kMaxBipeds  17
#define kMaxObjects 1024

#include "defines.h"
#include "vector3d.h"
#include <array>

typedef struct {
    bool right;
    bool left;
    bool backwards;
    bool forwards;
} MoveMask;

typedef struct {
    bool crouch;
    bool jump;
    bool flash;
    bool action;
    bool melee;
    bool unk0;
    bool shoot;
    bool unk1;
    bool unk2;
    bool longact;
} JumpMask;

typedef struct {
    float current;
} Look;

typedef struct {
    //unk0
    JumpMask jumping;
    float look_x;
    float look_y;
    MoveMask movement;
    bool shooting;
    bool tab;
} ControlMask;


class NetworkObject {
public:
    int16_t tag_index;
    int16_t table_id;
    vector3d position = *new vector3d(0.0,0.0,0.0);
    vector3d rotation = *new vector3d(0.0,0.0,0.0);
};

class Biped {
public:
    uint16_t object_index;
    uint16_t dead_tick = 0;
    
    int last_update_tick = 0;
    int team = 0;
    bool alive = false;
    ControlMask controls;
    vector3d *position = nullptr;
};

class Player {
public:
    int team = 0;
    bool alive = false;
    char *name = NULL;
    uint16_t biped_number;
};


class Game {
private:
    std::array<NetworkObject, kMaxObjects> objects;
    std::array<Player, kMaxPlayers> players;
    std::array<Biped , kMaxBipeds>  bipeds;
public:
    Player *getPlayer(int index);
    Biped *getBiped(int index);
    NetworkObject *getObject(int index);
    
    Game();
    ~Game();

};

#endif /* defined(____render__) */
