//
//  senv.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "senv.h"

// Global instance
void senv::setup(std::string path) {
    printf("senv setup\n");
    GLuint vertex_shader, fragment_shader;
    vertex_shader   = make_shader(GL_VERTEX_SHADER,   (path + "/senv.vert").c_str());
    fragment_shader = make_shader(GL_FRAGMENT_SHADER, (path + "/senv.frag").c_str());
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
    
    maps = glGetUniformLocation(program, "maps");
    maps2 = glGetUniformLocation(program, "maps2");
    maps3 = glGetUniformLocation(program, "maps3");
    reflectionScale = glGetUniformLocation(program, "reflectionScale");
    //glBindAttribLocation(program, 1, "texCoord_buffer");
    //glBindAttribLocation(program, 2, "texCoord_buffer_light");
    //glBindAttribLocation(program, 3, "normal_buffer");
}

void senv::start(shader_options *options) {
    glUseProgram(program);
    glUniform1i(baseTexture, 0);
    glUniform1i(primaryDetailMap, 1);
    glUniform1i(secondaryDetailMap, 2);
    glUniform1i(lightMap, 3);
    glUniform1i(cubeMap, 4);
    glUniform1i(bumpMap, 5);
    
    glUniform4f(fog, options->fogr, options->fogg, options->fogb, 1.0);
    glUniform2f(fogSettings, options->fogdist, options->fogcut);
    
    #ifdef RENDER_CORE_32
    glUniformMatrix4fv(ProjectionMatrix, 1, false, options->perspective);
    glUniformMatrix4fv(ModelViewMatrix , 1, false, options->modelview);
    #endif
}
void senv::update(shader_options *options) {
    
}

void senv::stop() {
    
}

void senv_object::setBaseUV(float u, float v) {
    uscale = u;
    vscale = v;
}

void senv_object::setFogSettings(float r, float g, float b, float distance, float cutoff) {
    fogr = r;
    fogg = g;
    fogb = b;
    fogdist = distance;
    fogcut = cutoff;
}

// Senv object
void senv_object::setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) {
    printf("senv object setup\n");
    baseMap = manager->texture_manager()->create_texture(map, *(HaloTagDependency*)(shaderTag->Data() + 0x88));
    
    bitmask16 environmentFlags = *(bitmask16*)(shaderTag->Data() + 0x28);
    useBlend = (environmentFlags & 15);
    
    HaloTagDependency primary = *(HaloTagDependency*)(shaderTag->Data() + 0xB8);
    if (primary.tag_id.tag_index != NULLED_TAG_ID) {
        primaryScale = *(float*)(shaderTag->Data() + 0xB4);
        primaryDetailMap = manager->texture_manager()->create_texture(map, primary);
        usePrimary = true;
    }
    
    printf("secondary setup\n");
    HaloTagDependency secondary = *(HaloTagDependency*)(shaderTag->Data() + 0xCC);
    if (secondary.tag_id.tag_index != NULLED_TAG_ID) {
        secondaryScale = *(float*)(shaderTag->Data() + 0xC8);
        secondaryDetailMap = manager->texture_manager()->create_texture(map, secondary);
        useSecondary = true;
    }
    
    printf("bump setup\n");
    HaloTagDependency bump = *(HaloTagDependency*)(shaderTag->Data() + 0x128);
    if (bump.tag_id.tag_index != NULLED_TAG_ID) {
        bumpScale = *(float*)(shaderTag->Data() + 0x124);
        bumpMap = manager->texture_manager()->create_texture(map, bump);
        useBump = true;
    }
    
    printf("cube setup\n");
    HaloTagDependency cube = *(HaloTagDependency*)(shaderTag->Data() + 0x324);
    if (cube.tag_id.tag_index != NULLED_TAG_ID) {
        cubeMap = manager->texture_manager()->create_cubemap(map, cube);
        useCube = true;
    }
    reflectionPerpendicular = *(float*)(shaderTag->Data() + 0x2F4);
    reflectionParallel = *(float*)(shaderTag->Data() + 0x2F8);
    
    printf("shader setup\n");
    senv *shader = (senv *)(manager->get_shader(shader_SENV));
    mapsId = shader->maps;
    maps2Id = shader->maps2;
    maps3Id = shader->maps3;
    scaleId = shader->scale;
    reflectionScaleId = shader->reflectionScale;
};

bool senv_object::is(ShaderType type) {
    return (type == shader_SENV || type == shader_SENV_REFLECT);
}

bool senv_object::render(ShaderType type, Pipeline *pipeline) {

    // Texturing
    //glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    baseMap->bind(nullptr);
    
    glActiveTexture(GL_TEXTURE1);
    if (usePrimary) {
        primaryDetailMap->bind(nullptr);
    }
    
    glActiveTexture(GL_TEXTURE2);
    if (useSecondary) {
        secondaryDetailMap->bind(nullptr);
    }
    
  
    if (type == shader_SENV) {
        glActiveTexture(GL_TEXTURE4);
        if (useCube) {
            cubeMap->bind(nullptr);
        }
        
        
        glActiveTexture(GL_TEXTURE5);
        if (useBump) {
            bumpMap->bind(nullptr);
        }
        
    // Blending
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Scales
    
        glUniform2f(scaleId, uscale, vscale);
        glUniform4f(mapsId, b2f(usePrimary), primaryScale, b2f(useSecondary), secondaryScale);
        glUniform4f(maps2Id, b2f(useLight), b2f(useBlend), b2f(useCube), b2f(useBump));
        glUniform1f(maps3Id, bumpScale);
        glUniform2f(reflectionScaleId, reflectionPerpendicular, reflectionParallel);
    }
    return true;
}
