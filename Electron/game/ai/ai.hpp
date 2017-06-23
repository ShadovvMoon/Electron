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
    ProtonMap *map;
    virtual void tick(Game *game, uint16_t bno) = 0;
    
    int stringPull(const float* portals, int nportals,
                   float* pts, const int maxPts);
    float *findPath(BSP *bsp, ProtonMap *map, vector3d *start, vector3d *end);
    void debug( const char* format, ... );
};
#endif /* ai_cpp */
