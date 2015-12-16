//
//  shader.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef ____EShaderSgla__
#define ____EShaderSgla__

#include "../shader.h"
class sgla : public shader {
private:
    GLuint program;
    GLint multipurposeMap;
    GLint cubeMap;
    GLint fog;
    GLint fogSettings;
public:
    GLint scale;
    GLint maps;
    GLint reflectionScale;
    void setup(std::string path);
    void start(shader_options *options);
    void update(shader_options *options);
    void stop();
};

class sgla_object : public shader_object {
private:
    bool useMulti = false;
    texture *multipurposeMap = nullptr;
    
    bool useCube = false;
    texture_cubemap *cubeMap = nullptr;
    float reflectionPerpendicular;
    float reflectionParallel;
    
    float uscale = 1.0;
    float vscale = 1.0;
    float bumpScale = 1.0;
    GLint mapsId;
    GLint scaleId;
    GLint reflectionScaleId;
    float fogr = 0.0, fogg = 0.0, fogb = 0.0, fogdist = 1.0, fogcut = 0.0;
public:
    void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag);
    bool render(ShaderType type);
    void setBaseUV(float u, float v);
    void setFogSettings(float r, float g, float b, float distance, float cutoff);
    bool is(ShaderType type);
};

#endif
