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
public:
    void setup();
    void start();
    void stop();
};

class senv_object : public shader_object {
private:
    texture *baseMap;
public:
    void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag);
    void render();
};

#endif
