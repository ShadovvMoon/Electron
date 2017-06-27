//
//  game.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "game.h"

Player *Game::getPlayer(int index) {
    if (index < 0 || index >= kMaxPlayers) return nullptr;
    return &players[index];
}

Biped *Game::getBiped(int index) {
    if (index < 0 || index >= kMaxBipeds) return nullptr;
    return &bipeds[index];
}

NetworkObject *Game::getObject(int index) {
    if (index < 0 || index >= kMaxObjects) return nullptr;
    return &objects[index];
}

Game::Game(ProtonMap *map) {
    this->map = map;
    
    int i;
    for (i=0; i < kMaxPlayers; i++) {
        players[i].alive = false;
        players[i].biped_number = -1;
    }
    for (i=0; i < kMaxBipeds; i++) {
        bipeds[i].alive = false;
        bipeds[i].object_index = -1;
        bipeds[i].position = new vector3d(0.0,0.0,0.0);
    }
    for (i=0; i < kMaxObjects; i++) {
        objects[i].tag_index = -1;
        objects[i].table_id  = -1;
    }
}

// Cleanup
Game::~Game() {
    printf("deleting game\n");
}