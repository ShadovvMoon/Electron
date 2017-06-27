//
//  senv.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "senv_reflect.h"

// Global instance
void senv_reflect::setup(std::string path) {
    printf("senv_reflect setup\n");
    GLuint vertex_shader, fragment_shader;
    vertex_shader   = make_shader(GL_VERTEX_SHADER,   (path + "/senv_reflect.vert").c_str());
    fragment_shader = make_shader(GL_FRAGMENT_SHADER, (path + "/senv_reflect.frag").c_str());
    program         = make_program(vertex_shader, fragment_shader);
    
    // Bind attributes
    baseTexture         = glGetUniformLocation(program, "baseTexture");
    primaryDetailMap    = glGetUniformLocation(program, "primaryDetailMap");
    secondaryDetailMap  = glGetUniformLocation(program, "secondaryDetailMap");
    lightMap            = glGetUniformLocation(program, "lightMap");
    cubeMap             = glGetUniformLocation(program, "cubeTextureMap");
    bumpMap             = glGetUniformLocation(program, "bumpMap");
    scale               = glGetUniformLocation(program, "scale");
    fog                 = glGetUniformLocation(program, "fog");
    fogSettings         = glGetUniformLocation(program, "fogSettings");
    ProjectionMatrix    = glGetUniformLocation(program, "ProjectionMatrix");
    ModelViewMatrix     = glGetUniformLocation(program, "ModelViewMatrix");
    tDiffuse        = glGetUniformLocation(program, "tDiffuse");
    tPosition       = glGetUniformLocation(program, "tPosition");
    tNormals        = glGetUniformLocation(program, "tNormals");
    tDepth          = glGetUniformLocation(program, "tDepth");
    
    maps = glGetUniformLocation(program, "maps");
    maps2 = glGetUniformLocation(program, "maps2");
    maps3 = glGetUniformLocation(program, "maps3");
    reflectionScale = glGetUniformLocation(program, "reflectionScale");
    //glBindAttribLocation(program, 1, "texCoord_buffer");
    //glBindAttribLocation(program, 2, "texCoord_buffer_light");
    //glBindAttribLocation(program, 3, "normal_buffer");
}

void senv_reflect::start(shader_options *options) {
    glUseProgram(program);
    glUniform1i(baseTexture, 0);
    glUniform1i(primaryDetailMap, 1);
    glUniform1i(secondaryDetailMap, 2);
    glUniform1i(lightMap, 3);
    glUniform1i(cubeMap, 4);
    glUniform1i(bumpMap, 5);
    
    
    // Reflection textures
    glUniform1i(tNormals, 4);
    glUniform1i(tDiffuse, 5);
    glUniform1i(tPosition, 6);
    glUniform1i(tDepth, 7);
    
    glUniform4f(fog, options->fogr, options->fogg, options->fogb, 1.0);
    glUniform2f(fogSettings, options->fogdist, options->fogcut);
    
    glUniformMatrix4fv(ProjectionMatrix, 1, false, options->perspective);
    #ifdef RENDER_CORE_32
    glUniformMatrix4fv(ModelViewMatrix , 1, false, options->modelview);
    #endif
}
void senv_reflect::update(shader_options *options) {
    
}

void senv_reflect::stop() {
    
}
