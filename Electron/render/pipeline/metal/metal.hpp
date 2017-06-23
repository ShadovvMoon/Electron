//
//  metal.hpp
//  Electron
//
//  Created by Samuco on 6/22/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#ifndef metal_hpp
#define metal_hpp
#include "defines.h"
#include <stdio.h>
class CoreRenderer {
public:
};

class MetalMesh {
    
};
class MetalTexture {
    
};

class MetalRenderer: CoreRenderer {
    public:
    MetalRenderer(void);
    
    // Render
    void resize(float width, float height);
    void render();
    
    // Setup
    void addMesh();
    void addTexture();
};















#endif /* metal_hpp */
