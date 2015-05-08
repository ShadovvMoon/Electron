//
//  shader.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef ____EShaderScex__
#define ____EShaderScex__

#include <vector>
#include "../shader.h"
typedef struct {
    bitmask16 flags;
    uint16_t colour;
    uint16_t alpha;
    float uScale;
    float vScale;
    float uOffset;
    float vOffset;
    float mapRotation;
    float mipmapBias;
    HaloTagDependency map;
    uint16_t uSource;
    uint16_t uAnimate;
    float uPeriod;
    float uPhase;
    float uAnimateScale;
    uint16_t vSource;
    uint16_t vAnimate;
    float vPeriod;
    float vPhase;
    float vAnimateScale;
} Stage4Map;

class scex : public shader {
private:
    GLuint program;
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
    GLint fog;
    GLint fogSettings;
    
    void setup(std::string path);
    void start(shader_options *options);
    void stop();
};

class Stage4Renderable {
public:
    texture *map = nullptr;
    float uScale = 0.0;
    float vScale = 0.0;
    float uOffset = 0.0;
    float vOffset = 0.0;
    int cFunc;
    int aFunc;
    
    uint16_t uAnimate;
    uint16_t vAnimate;
    float vPeriod;
    float uPeriod;
    float uAScale;
    float vAScale;
    std::chrono::milliseconds prev;
    
    Stage4Renderable();
};

class scex_object : public shader_object {
private:
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
    bool skip;
    float fogr = 0.0, fogg = 0.0, fogb = 0.0, fogdist = 1.0, fogcut = 0.0;
public:
    void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag);
    bool render();
    void setBaseUV(float u, float v);
    void setFogSettings(float r, float g, float b, float distance, float cutoff);
    bool is(ShaderType type);
};

std::chrono::milliseconds timems();
#endif
