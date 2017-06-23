//
//  ai_sniper.hpp
//  HaloBot
//
//  Created by Samuco on 12/12/2015.
//  Copyright © 2015 samuco. All rights reserved.
//

#include "ai.hpp"
#ifndef ai_test_cpp
#define ai_test_cpp

#include <stdio.h>
class ai_wyvern : public ai {
    BSP *bsp;
    ProtonMap *map;
    ProtonTag *scenario;
    bool canSee(vector3d *me, vector3d *location);
    
    int pathTick = 0;
    int pointTick = 0;
    float *path = NULL;
public:
    ai_wyvern(BSP *mesh, ProtonMap *map, ProtonTag *scenario);
    void tick(Game *game, uint16_t bno);
};

#endif /* ai_test_cpp */
