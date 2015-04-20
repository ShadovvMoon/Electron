//
//  shader.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "defines.h"
#include <map>
#include "bitmaps.h"

#ifndef __texture__
#define __texture__

class texture {
public:
    GLuint tex;
    texture(ProtonMap *map, HaloTagDependency bitm);
    void bind();
};

class TextureManager {
private:
    std::map<uint16_t, texture*> textures;
public:
    texture *create_texture(ProtonMap *map, HaloTagDependency bitm);
};


#endif
