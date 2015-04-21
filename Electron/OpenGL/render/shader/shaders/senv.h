//
//  shader.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef ____EShaderSenv__
#define ____EShaderSenv__

#include "shader.h"
#include "texture.h"

class senv : public shader {
private:
    GLuint program;
    GLint baseTexture;
    GLint primaryDetailMap;
    GLint secondaryDetailMap;
    GLint lightMap;
public:
    GLint maps;
    GLint maps2;
    
    void setup(std::string path);
    void start();
    void stop();
};

class senv_object : public shader_object {
private:
    texture *baseMap = nullptr;
    bool usePrimary = false;
    float primaryScale;
    texture *primaryDetailMap = nullptr;
    
    bool useSecondary = false;
    float secondaryScale;
    texture *secondaryDetailMap;
    
    GLint mapsId;
    GLint maps2Id;
public:
    bool useLight = false;
    
    void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag);
    void render();
    void setBaseUV(float u, float v);
    bool is(ShaderType type);
};

#endif
