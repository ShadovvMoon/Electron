//
//  shader.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef ____EShadersenv_reflect__
#define ____EShadersenv_reflect__

#include "../glshader.h"
class senv_reflect : public shader {
private:
    GLint baseTexture;
    GLint primaryDetailMap;
    GLint secondaryDetailMap;
    GLint lightMap;
    GLint bumpMap;
    GLint cubeMap;
    
    GLint fog;
    GLint fogSettings;
    
    GLint ProjectionMatrix;
    GLint ModelViewMatrix;
    
    GLuint tDiffuse;
    GLuint tPosition;
    GLuint tNormals;
    GLuint tDepth;
public:
    GLint maps;
    GLint maps2;
    GLint maps3;
    GLint reflectionScale;
    GLint scale;
    
    void setup(std::string path);
    void start(shader_options *options);
    void update(shader_options *options);
    void stop();
};

#endif
