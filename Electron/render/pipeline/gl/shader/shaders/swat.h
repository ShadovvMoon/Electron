//
//  shader.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef ____EShaderSwat__
#define ____EShaderSwat__

#include "../glshader.h"
#include "scex.h"

class swat : public shader {
private:
    GLuint program;
    GLuint baseTexture;
    GLuint bumpMap;
    GLint fog;
    GLint fogSettings;
public:
    GLuint frameSize;
    GLuint floats;
    GLuint texOffset;
    
    void setup(std::string path);
    void start(shader_options *options);
    void update(shader_options *options);
    void stop();
};

class swat_object : public shader_object {
public:
    texture *bumpMap = nullptr;
    bool useBump = false;
    float bumpScale;
    float uOffset, vOffset;
    std::chrono::milliseconds prev;
    GLuint frameSize;
    GLuint zRef;
    GLuint floats;
    GLuint texOffset;
    
    float fogr = 0.0, fogg = 0.0, fogb = 0.0, fogdist = 1.0, fogcut = 0.0;
    void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag);
    bool render(ShaderType type, Pipeline *pipeline);
    void setBaseUV(float u, float v);
    void setFogSettings(float r, float g, float b, float distance, float cutoff);
    bool is(ShaderType type);
};

#endif
