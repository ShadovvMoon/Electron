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
    virtual void bind(Pipeline *pipeline) = 0;
    virtual void bind(int index, Pipeline *pipeline) = 0;
};

class texture_cubemap {
public:
    virtual void load_side(uint side_target, uint i, ProtonTag *bitmapTag) = 0;
    virtual void bind(Pipeline *pipeline) = 0;
};

class TextureManager {
public:
    virtual texture *create_texture(ProtonMap *map, HaloTagDependency bitm) = 0;
    virtual texture_cubemap *create_cubemap(ProtonMap *map, HaloTagDependency bitm) = 0;
};






class gltexture: public texture {
public:
    std::vector<uint>textures;
    uint tex = 0;
    gltexture(ProtonMap *map, HaloTagDependency bitm);
    void bind(Pipeline *pipeline);
    void bind(int index, Pipeline *pipeline);
};

class gltexture_cubemap: public texture_cubemap {
public:
    uint tex;
    void load_side(uint side_target, uint i, ProtonTag *bitmapTag);
    void bind(Pipeline *pipeline);
    gltexture_cubemap(ProtonMap *map, HaloTagDependency bitm);
};

class GLTextureManager: public TextureManager {
private:
    std::map<uint16_t, texture*> textures;
    std::map<uint16_t, texture_cubemap*> textures_cube;
public:
    texture *create_texture(ProtonMap *map, HaloTagDependency bitm);
    texture_cubemap *create_cubemap(ProtonMap *map, HaloTagDependency bitm);
};


#endif
