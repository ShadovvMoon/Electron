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
    void start(shader_options *options);
    void stop();
};

class null_object : public shader_object {
private:
public:
    void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag);
    bool render();
    void setBaseUV(float u, float v);
    void setFogSettings(float r, float g, float b, float distance, float cutoff);
    bool is(ShaderType type);
};

#endif
