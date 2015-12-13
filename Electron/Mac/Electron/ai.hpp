//
//  ai.hpp
//  Electron
//
//  Created by Samuco on 21/07/2015.
//  Copyright © 2015 Samuco. All rights reserved.
//

#ifndef ai_cpp
#define ai_cpp

#include "bsp.h"
#include "game.h"
#include <stdio.h>

class ai {
public:
    virtual void tick(Game *game, uint16_t bno) = 0;
};
#endif /* ai_cpp */
