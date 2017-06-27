//
//  mtexture.h
//  Electron
//
//  Created by Samuco on 6/23/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#ifndef mtexture_h
#define mtexture_h

#import "pipeline.hpp"
class metal_texture: public texture {
public:
    std::vector<uint>textures;
    uint tex = 0;
    metal_texture(ProtonMap *map, HaloTagDependency bitm);
    void bind();
    void bind(int index);
};

class metal_texture_cubemap: public texture_cubemap {
public:
    uint tex;
    void load_side(uint side_target, uint i, ProtonTag *bitmapTag);
    void bind();
    metal_texture_cubemap(ProtonMap *map, HaloTagDependency bitm);
};

class MetalTextureManager: public TextureManager {
private:
    std::map<uint16_t, texture*> textures;
    std::map<uint16_t, texture_cubemap*> textures_cube;
public:
    texture *create_texture(ProtonMap *map, HaloTagDependency bitm);
    texture_cubemap *create_cubemap(ProtonMap *map, HaloTagDependency bitm);
};

#endif /* mtexture_h */
