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

class senv : public shader {
private:
    GLuint program;
    GLint attribute_coord2d;
public:
    void setup();
    void start();
    void stop();
};

class senv_object : public shader_object {
private:
    GLuint baseMap;
public:
    void setup(ProtonMap *map, ProtonTag *shaderTag);
    void render();
};

#endif
