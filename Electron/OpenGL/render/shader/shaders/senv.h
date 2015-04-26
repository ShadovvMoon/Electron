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
    GLuint program;
    GLint baseTexture;
    GLint primaryDetailMap;
    GLint secondaryDetailMap;
    GLint lightMap;
    GLint bumpMap;
    GLint cubeMap;
public:
    GLint maps;
    GLint maps2;
    GLint maps3;
    GLint reflectionScale;
    GLint scale;
    
    void setup(std::string path);
    void start();
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
public:
    bool useLight = false;
    
    void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag);
    void render();
    void setBaseUV(float u, float v);
    bool is(ShaderType type);
};

#endif
