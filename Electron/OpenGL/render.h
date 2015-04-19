//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef ____render__
#define ____render__

#include "defines.h"
#include "bsp.h"
#include "math.h"
#include "camera.h"
#include "shader.h"

class ERenderer {
private:
    ProtonMap *map;
    BSP *bsp;
    Camera *camera;
    bool ready = false;
public:
    void setup();
    void resize(float width, float height);
    void render();
    void setMap(ProtonMap *map);
};

#endif /* defined(____render__) */
