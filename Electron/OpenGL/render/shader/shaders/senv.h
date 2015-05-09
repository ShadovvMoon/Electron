//
//  shader.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef ____EShaderSenv__
#define ____EShaderSenv__

#include "../shader.h"
class senv : public shader {
private:
    GLint baseTexture;
    GLint primaryDetailMap;
    GLint secondaryDetailMap;
    GLint lightMap;
    GLint bumpMap;
    GLint cubeMap;
    
    GLint fog;
    GLint fogSettings;
    
    GLint ProjectionMatrix;
    GLint ModelViewMatrix;
public:
    GLint maps;
    GLint maps2;
    GLint maps3;
    GLint reflectionScale;
    GLint scale;
    
    void setup(std::string path);
    void start(shader_options *options);
    void update(shader_options *options);
    void stop();
};

class senv_object : public shader_object {
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
    
    GLint mapsId;
    GLint maps2Id;
    GLint maps3Id;
    GLint scaleId;
    GLint reflectionScaleId;
    float fogr = 0.0, fogg = 0.0, fogb = 0.0, fogdist = 1.0, fogcut = 0.0;
public:
    bool useLight = false;
    
    void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag);
    bool render();
    void setBaseUV(float u, float v);
    void setFogSettings(float r, float g, float b, float distance, float cutoff);
    bool is(ShaderType type);
};

#endif
