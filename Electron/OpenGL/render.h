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
    bool ready;
    void renderBSP(ProtonTag *scenario);
public:
    void setup();
    void resize(float width, float height);
    void render();
    void setMap(ProtonMap *map);
};

#endif /* defined(____render__) */
