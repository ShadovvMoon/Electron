//
//  shader.h
//
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef ____EShaderBlur__
#define ____EShaderBlur__

#include "../glshader.h"
class blur : public shader {
private:
    GLuint program;
    GLuint tDiffuse;
    GLuint tPosition;
    GLuint tNormals;
    GLuint tDepth;
public:
    void setup(std::string path);
    void start(shader_options *options);
    void update(shader_options *options);
    void stop();
};

class blur_object : public shader_object {
private:
public:
    void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag);
    bool render(ShaderType type, Pipeline *pipeline);
    void setBaseUV(float u, float v);
    void setFogSettings(float r, float g, float b, float distance, float cutoff);
    bool is(ShaderType type);
};

#endif
