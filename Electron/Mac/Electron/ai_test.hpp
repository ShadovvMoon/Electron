//
//  ai_test.hpp
//  Electron
//
//  Created by Samuco on 21/07/2015.
//  Copyright © 2015 Samuco. All rights reserved.
//
#include "ai.hpp"
#ifndef ai_test_cpp
#define ai_test_cpp

#include <stdio.h>
class ai_test : public ai {
    BSP *bsp;
    ProtonMap *map;
    ProtonTag *scenario;
    bool canSee(vector3d *me, vector3d *location);
public:
    ai_test(BSP *mesh, ProtonMap *map, ProtonTag *scenario);
    void tick(Game *game, uint16_t bno);
};

#endif /* ai_test_cpp */
