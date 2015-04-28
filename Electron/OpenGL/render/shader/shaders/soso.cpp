//
//  senv.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "soso.h"

// Global instance
void soso::setup(std::string path) {
    printf("soso setup\n");
    GLuint vertex_shader, fragment_shader;
    vertex_shader   = make_shader(GL_VERTEX_SHADER,   (path + "/soso.vert").c_str());
    fragment_shader = make_shader(GL_FRAGMENT_SHADER, (path + "/soso.frag").c_str());
    program         = make_program(vertex_shader, fragment_shader);
    
    // Bind attributes
    baseTexture     = glGetUniformLocation(program, "baseTexture");
    multipurposeMap = glGetUniformLocation(program, "multipurposeMap");
    detailMap       = glGetUniformLocation(program, "detailMap");
    cubeMap         = glGetUniformLocation(program, "cubeTextureMap");
    maps            = glGetUniformLocation(program, "maps");
    scale           = glGetUniformLocation(program, "scale");
    reflectionScale = glGetUniformLocation(program, "reflectionScale");
    
	//glBindAttribLocation(program, 1, "texCoord_buffer");
    //glBindAttribLocation(program, 2, "normal_buffer");
    printf("done %d %d %d %d\n", baseTexture, multipurposeMap, detailMap, cubeMap);
}

void soso::start() {
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.5);
    glUseProgram(program);
    glUniform1i(baseTexture, 0);
    glUniform1i(detailMap, 1);
    glUniform1i(multipurposeMap, 2);
    glUniform1i(cubeMap, 3);
}

void soso::stop() {
    glDisable(GL_ALPHA_TEST);
}

void soso_object::setBaseUV(float u, float v) {
    uscale = u;
    vscale = v;
}

// Senv object
void soso_object::setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) {
    printf("soso object setup\n");
    baseMap = manager->texture_manager()->create_texture(map, *(HaloTagDependency*)(shaderTag->Data() + 0xA4));
    uscale = *(float*)(shaderTag->Data() + 0x9C);
    vscale = *(float*)(shaderTag->Data() + 0xA0);
    
    printf("detail setup\n");
    HaloTagDependency detail = *(HaloTagDependency*)(shaderTag->Data() + 0xDC);
    if (detail.tag_id.tag_index != NULLED_TAG_ID) {
        detailMap = manager->texture_manager()->create_texture(map, detail);
        detailScale  = *(float*)(shaderTag->Data() + 0xD8);
        detailScaleV = *(float*)(shaderTag->Data() + 0xEC);
        useDetail = true;
    }
    
    printf("multi setup\n");
    HaloTagDependency multi = *(HaloTagDependency*)(shaderTag->Data() + 0xBC);
    if (multi.tag_id.tag_index != NULLED_TAG_ID) {
        multipurposeMap = manager->texture_manager()->create_texture(map, multi);
        useMulti = true;
    }
    
    printf("cube setup\n");
    HaloTagDependency cube = *(HaloTagDependency*)(shaderTag->Data() + 0x164);
    if (cube.tag_id.tag_index != NULLED_TAG_ID) {
        cubeMap = manager->texture_manager()->create_cubemap(map, cube);
        useCube = true;
    }
    reflectionPerpendicular = *(float*)(shaderTag->Data() + 0x144);
    reflectionParallel = *(float*)(shaderTag->Data() + 0x154);
    
    printf("shader setup\n");
    soso *shader = (soso *)(manager->get_shader(shader_SOSO));
    mapsId  = shader->maps;
    scaleId = shader->scale;
    reflectionScaleId = shader->reflectionScale;
};

bool soso_object::is(ShaderType type) {
    return (type == shader_SOSO);
}
void soso_object::render() {
    
    // Texturing
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    baseMap->bind();
    
    glActiveTexture(GL_TEXTURE1);
    if (useDetail) {
        detailMap->bind();
    }
    
    glActiveTexture(GL_TEXTURE2);
    if (useMulti) {
        multipurposeMap->bind();
    }
    
    glActiveTexture(GL_TEXTURE3);
    if (useCube) {
        cubeMap->bind();
    }
    
    // Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Scales
    glUniform4f(scaleId, uscale, vscale, detailScale, detailScaleV);
    glUniform3f(mapsId , b2f(useMulti), b2f(useDetail), useCube ? 0.5:0.0);
    glUniform2f(reflectionScaleId, reflectionPerpendicular, reflectionParallel);
}
