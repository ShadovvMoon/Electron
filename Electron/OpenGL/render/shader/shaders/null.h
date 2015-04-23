//
//  shader.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef ____EShaderNull__
#define ____EShaderNull__

#include "../shader.h"
class null : public shader {
private:
    GLuint program;
public:
    void setup(std::string path);
    void start();
    void stop();
};

class null_object : public shader_object {
private:
public:
    void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag);
    void render();
    void setBaseUV(float u, float v);
    bool is(ShaderType type);
};

#endif
