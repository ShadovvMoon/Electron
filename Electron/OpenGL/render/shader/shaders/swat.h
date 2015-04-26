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
class swat : public shader {
private:
    GLuint program;
public:
    void setup(std::string path);
    void start();
    void stop();
};

class swat_object : public shader_object {
private:
public:
    void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag);
    void render();
    void setBaseUV(float u, float v);
    bool is(ShaderType type);
};

#endif
