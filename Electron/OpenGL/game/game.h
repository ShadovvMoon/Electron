//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef __game__
#define __game__

#define kMaxPlayers 16
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

class Player {
public:
    bool alive = false;
    ControlMask controls;
    vector3d *position = nullptr;
};


class Game {
private:
    std::array<Player, kMaxPlayers> players;
public:
    Player *getPlayer(int index);
    
    Game();
    ~Game();

};

#endif /* defined(____render__) */
