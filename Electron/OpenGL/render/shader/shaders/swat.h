//
//  shader.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef ____EShaderSwat__
#define ____EShaderSwat__

#include "../shader.h"
#include "scex.h"

class swat : public shader {
private:
    GLuint program;
    GLuint baseTexture;
    GLuint bumpMap;
public:
    GLuint frameSize;
    GLuint floats;
    GLuint texOffset;
    
    void setup(std::string path);
    void start();
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
    
    void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag);
    bool render();
    void setBaseUV(float u, float v);
    bool is(ShaderType type);
};

#endif
