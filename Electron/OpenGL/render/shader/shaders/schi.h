//
//  shader.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef ____EShaderSchi__
#define ____EShaderSchi__

#include "../shader.h"
class schi : public shader {
private:
    GLuint program;
    GLint baseTexture;
public:
    GLint maps;
    
    void setup(std::string path);
    void start();
    void stop();
};

class schi_object : public shader_object {
private:
    texture *baseMap = nullptr;
    GLint mapsId;
public:
    void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag);
    void render();
    void setBaseUV(float u, float v);
    bool is(ShaderType type);
};

#endif
