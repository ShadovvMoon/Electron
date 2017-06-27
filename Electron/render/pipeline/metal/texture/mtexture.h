//
//  mtexture.h
//  Electron
//
//  Created by Samuco on 6/23/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#ifndef mtexture_h
#define mtexture_h

#import <MetalKit/MetalKit.h>
#import "pipeline.hpp"

class metal_texture: public texture {
    NSMutableArray *textureLevels;
public:
    std::vector<uint>textures;
    uint tex = 0;
    metal_texture(id<MTLDevice> device, id <MTLCommandQueue> queue, ProtonMap *map, HaloTagDependency bitm);
    void bind(Pipeline *pipeline);
    void bind(int index, Pipeline *pipeline);
};

class metal_texture_cubemap: public texture_cubemap {
public:
    uint tex;
    void load_side(uint side_target, uint i, ProtonTag *bitmapTag);
    void bind(Pipeline *pipeline);
    metal_texture_cubemap(ProtonMap *map, HaloTagDependency bitm);
};

class MetalTextureManager: public TextureManager {
private:
    id<MTLDevice> device;
    id <MTLCommandQueue> mipmapQueue;
    std::map<uint16_t, texture*> textures;
    std::map<uint16_t, texture_cubemap*> textures_cube;
public:
    MetalTextureManager(id<MTLDevice> device, id <MTLCommandQueue> queue);
    texture *create_texture(ProtonMap *map, HaloTagDependency bitm);
    texture_cubemap *create_cubemap(ProtonMap *map, HaloTagDependency bitm);
};

#endif /* mtexture_h */
