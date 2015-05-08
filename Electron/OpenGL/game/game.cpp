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

Game::Game() {
    int i;
    for (i=0; i < kMaxPlayers; i++) {
        players[i].alive = false;
        players[i].position = new vector3d(0.0,0.0,0.0);
    }
}

// Cleanup
Game::~Game() {
    printf("deleting game\n");
}