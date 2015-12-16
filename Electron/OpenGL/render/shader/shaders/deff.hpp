//
//  shader.h
//
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef ____EShaderDeff__
#define ____EShaderDeff__

#include "../shader.h"
class deff : public shader {
private:
    GLuint program;
    GLuint tDiffuse;
    GLuint tPosition;
    GLuint tNormals;
    GLuint tDepth;
    GLuint tSSAO;
    GLint ProjectionMatrix;
    GLint UBOCamera;
public:
    void setup(std::string path);
    void start(shader_options *options);
    void update(shader_options *options);
    void stop();
};

class deff_object : public shader_object {
private:
public:
    void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag);
    bool render(ShaderType type);
    void setBaseUV(float u, float v);
    void setFogSettings(float r, float g, float b, float distance, float cutoff);
    bool is(ShaderType type);
};

#endif
