//
//  senv.hpp
//  Electron
//
//  Created by Samuco on 6/23/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#ifndef msenv_hpp
#define msenv_hpp

#include "../mshader.hpp"
#include "SharedStructures.h"

class msenv : public shader {
private:
    id <MTLRenderPipelineState> _pipelineState;
public:
    id <MTLBuffer> _dynamicConstantBuffer;
    void setup(id <MTLDevice> device, MTKView *view, id <MTLLibrary> library);
    void start(shader_options *options);
    void update(shader_options *options);
    void stop();
};

class msenv_object : public shader_object {
private:
    msenv *shader;
    senv_options options;
    texture *baseMap = nullptr;
    texture *primaryDetailMap = nullptr;
    texture *secondaryDetailMap;
    texture *bumpMap;
    texture_cubemap *cubeMap = nullptr;
public:
    bool useLight = false;
    void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag);
    bool render(ShaderType type, Pipeline *pipeline);
    void setBaseUV(float u, float v);
    void setFogSettings(float r, float g, float b, float distance, float cutoff);
    bool is(ShaderType type);
};
#endif /* msenv_hpp */
