//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef __BSP__
#define __BSP__

#include "defines.h"

class BSP {
public:
    void setup(ProtonMap *map, ProtonTag *scenario);
    void render(ProtonMap *map, ProtonTag *scenario);
};

#endif /* defined(__BSP__) */
