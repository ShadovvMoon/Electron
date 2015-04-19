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
#include "tags/ZZTHaloScenarioTag.h"

class ERenderer {
private:
    ProtonMap *map;
    const void *data;
    bool ready;
    
    void *map2mem(uint32_t pointer);
    void renderBSP(Reflexive bsp);
public:
    void setup();
    void resize(float width, float height);
    void render();
    void setMap(ProtonMap *map);
};

#endif /* defined(____render__) */
