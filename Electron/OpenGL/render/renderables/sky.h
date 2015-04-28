//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef __SKY__
#define __SKY__

#include "defines.h"
#include "model.h"
#include "object.h"

class SkyManager {
private:
    std::vector<Model*> skies;
public:
    ModelManager *modelManager;
    void read(ObjectManager *objects, ProtonMap *map, ProtonTag *scenario);
    void render(ShaderType pass);
};

#endif /* defined(__BSP__) */
