//
//  shader.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef ____EShaderSchi__
#define ____EShaderSchi__

#include "../glshader.h"
#include "scex.h"

class schi : public shader {
public:
    GLuint baseMapUV;
    GLuint useMap;
    GLuint mapTexture0 = 0;
    GLuint mapTexture1 = 0;
    GLuint mapTexture2 = 0;
    GLuint mapTexture3 = 0;
    GLuint uScale;
    GLuint vScale;
    GLuint uOffset;
    GLuint vOffset;
    GLuint colorFunction;
    GLuint alphaFunction;
    GLuint mapCount;
    GLint ProjectionMatrix;
    GLint ModelViewMatrix;
    
    GLint fog;
    GLint fogSettings;
    void setup(std::string path);
    void start(shader_options *options);
    void update(shader_options *options);
    void stop();
};

class schi_object : public shader_object {
private:
    texture *baseMap = nullptr;
    std::vector<Stage4Renderable*> stage4Maps;
    float uscale = 1.0;
    float vscale = 1.0;
    GLuint baseMapUV = 0;
    GLuint useMap = 0;
    GLuint mapTexture0 = 0;
    GLuint mapTexture1 = 0;
    GLuint mapTexture2 = 0;
    GLuint mapTexture3 = 0;
    GLuint uScale = 0;
    GLuint vScale = 0;
    GLuint uOffset = 0;
    GLuint vOffset = 0;
    GLuint colorFunction = 0;
    GLuint alphaFunction = 0;
    GLuint mapCount = 0;
    int mapsCount = 0;
    
    float fogr = 0.0, fogg = 0.0, fogb = 0.0, fogdist = 1.0, fogcut = 0.0;
public:
    void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag);
    bool render(ShaderType type, Pipeline *pipeline);
    void setBaseUV(float u, float v);
    void setFogSettings(float r, float g, float b, float distance, float cutoff);
    bool is(ShaderType type);
};

#endif
