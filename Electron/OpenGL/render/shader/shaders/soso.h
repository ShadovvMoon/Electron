//
//  shader.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef ____EShaderSoso__
#define ____EShaderSoso__

#include "../shader.h"
class soso : public shader {
private:
    GLint baseTexture;
    GLint multipurposeMap;
    GLint detailMap;
    GLint cubeMap;
    GLint fog;
    GLint fogSettings;
    GLint ProjectionMatrix; GLint ModelViewMatrix;
    GLint Position; GLint Rotation;
public:
    GLint scale;
    GLint maps;
    GLint reflectionScale;
    
    void setup(std::string path);
    void start(shader_options *options);
    void update(shader_options *options);
    void stop();
};

class soso_object : public shader_object {
private:
    float uscale = 1.0;
    float vscale = 1.0;
    texture *baseMap = nullptr;
    
    bool useMulti = false;
    texture *multipurposeMap = nullptr;
    
    bool useDetail = false;
    float detailScale = 1.0;
    float detailScaleV = 1.0;
    texture *detailMap = nullptr;
    
    bool useCube = false;
    texture_cubemap *cubeMap = nullptr;
    float reflectionPerpendicular;
    float reflectionParallel;
    
    GLint mapsId;
    GLint scaleId;
    GLint reflectionScaleId;
    float fogr = 0.0, fogg = 0.0, fogb = 0.0, fogdist = 1.0, fogcut = 0.0;
public:
    void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag);
    bool render();
    void setBaseUV(float u, float v);
    void setFogSettings(float r, float g, float b, float distance, float cutoff);
    bool is(ShaderType type);
};

#endif
