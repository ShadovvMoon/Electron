//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef ____render__
#define ____render__

#include <stdio.h>
#include "ProtonMap.h"

class ERenderer {
public:
    void setup();
    void render();
    void setMap(ProtonMap *map);
};

#endif /* defined(____render__) */
