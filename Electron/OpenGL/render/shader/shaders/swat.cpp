//
//  senv.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "swat.h"

// Global instance
void swat::setup(std::string path) {
    printf("swat setup\n");
    GLuint vertex_shader, fragment_shader;
    vertex_shader   = make_shader(GL_VERTEX_SHADER,   (path + "/swat.vert").c_str());
    fragment_shader = make_shader(GL_FRAGMENT_SHADER, (path + "/swat.frag").c_str());
    program         = make_program(vertex_shader, fragment_shader);
    printf("done\n");
    
    // Bind attributes
    baseTexture = glGetUniformLocation(program, "baseTexture");
    bumpMap     = glGetUniformLocation(program, "bumpMap");
    frameSize   = glGetUniformLocation(program, "frameSize");
    floats      = glGetUniformLocation(program, "bumpScales");
    texOffset   = glGetUniformLocation(program, "timerOffset");
    fog                 = glGetUniformLocation(program, "fog");
    fogSettings         = glGetUniformLocation(program, "fogSettings");
}

void swat::start(shader_options *options) {
    glUseProgram(program);
    glUniform1i(baseTexture, 0);
    glUniform1i(bumpMap, 1);
    glUniform4f(fog, options->fogr, options->fogg, options->fogb, 1.0);
    glUniform2f(fogSettings, options->fogdist, options->fogcut);
}
void swat::update(shader_options *options) {
    
}

void swat::stop() {
    
}

void swat_object::setBaseUV(float u, float v) {
}
void swat_object::setFogSettings(float r, float g, float b, float distance, float cutoff) {
    fogr = r;
    fogg = g;
    fogb = b;
    fogdist = distance;
    fogcut = cutoff;
}

// Senv object
void swat_object::setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) {
    
    printf("swat setup\n");
    HaloTagDependency bump = *(HaloTagDependency*)(shaderTag->Data() + 0xC8);
    if (bump.tag_id.tag_index != NULLED_TAG_ID) {
        bumpScale = *(float*)(shaderTag->Data() + 0xC4);
        bumpMap = manager->texture_manager()->create_texture(map, bump);
    }
    
    uOffset = 0.0;
    vOffset = 0.0;
	prev = timems();
    
#ifdef RENDER_SWAT_REFLECTION
    manager->set_needs_reflection(true);
#endif
    swat *shader = (swat *)(manager->get_shader(shader_SWAT));
    frameSize = shader->frameSize;
    floats = shader->floats;
    texOffset = shader->texOffset;
    zRef = manager->get_reflection(reflection_z);
};

bool swat_object::is(ShaderType type) {
    return (type == shader_SWAT);
}
bool swat_object::render() {
    // Update the tick
    std::chrono::milliseconds now = timems();
    double seconds = (now.count() - prev.count()) / 1000.0;
    prev = now;
    uOffset += (seconds) / 2.0;
    vOffset += (seconds)  / 2.0;
    
    // Enable the textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, zRef);
    
    // Enable the bump texture
    glActiveTexture(GL_TEXTURE1);
    bumpMap->bind();
    
    // Update the uniforms
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
    glUniform4f(frameSize, viewport[2], viewport[3], NextHighestPowerOf2(viewport[2]), NextHighestPowerOf2(viewport[3]));
    glUniform1f(floats, bumpScale*2);
    glUniform2f(texOffset, uOffset, vOffset);
    return true;
}
