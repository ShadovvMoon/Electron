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
class msenv : public shader {
private:
public:
    void setup(id <MTLLibrary> library);
    void start(shader_options *options);
    void update(shader_options *options);
    void stop();
};

class msenv_object : public shader_object {
private:
    float uscale = 1.0;
    float vscale = 1.0;
    texture *baseMap = nullptr;
    bool useBlend = false;
    bool usePrimary = false;
    float primaryScale;
    texture *primaryDetailMap = nullptr;
    bool useSecondary = false;
    float secondaryScale;
    texture *secondaryDetailMap;
    bool useBump = false;
    float bumpScale;
    texture *bumpMap;
    bool useCube = false;
    texture_cubemap *cubeMap = nullptr;
    float reflectionPerpendicular;
    float reflectionParallel;
public:
    bool useLight = false;
    void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag);
    bool render(ShaderType type);
    void setBaseUV(float u, float v);
    void setFogSettings(float r, float g, float b, float distance, float cutoff);
    bool is(ShaderType type);
};
#endif /* msenv_hpp */
