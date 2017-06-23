//
//  senv.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "scex.h"

std::chrono::milliseconds timems() {
    return std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::high_resolution_clock::now().time_since_epoch());
}

// Global instance
void scex::setup(std::string path) {
    printf("scex setup\n");
    GLuint vertex_shader, fragment_shader;
    vertex_shader   = make_shader(GL_VERTEX_SHADER,   (path + "/scex.vert").c_str());
    fragment_shader = make_shader(GL_FRAGMENT_SHADER, (path + "/scex.frag").c_str());
    program         = make_program(vertex_shader, fragment_shader);
    
    // Bind attributes
    baseMapUV        = glGetUniformLocation(program, "baseMapUV");
    useMap           = glGetUniformLocation(program, "useMap");
    mapTexture0       = glGetUniformLocation(program, "mapTexture0");
    mapTexture1       = glGetUniformLocation(program, "mapTexture1");
    mapTexture2       = glGetUniformLocation(program, "mapTexture2");
    mapTexture3       = glGetUniformLocation(program, "mapTexture3");
    uScale           = glGetUniformLocation(program, "uScale");
    vScale           = glGetUniformLocation(program, "vScale");
    uOffset          = glGetUniformLocation(program, "uOffset");
    vOffset          = glGetUniformLocation(program, "vOffset");
    colorFunction    = glGetUniformLocation(program, "colorFunction");
    alphaFunction    = glGetUniformLocation(program, "alphaFunction");
    mapCount         = glGetUniformLocation(program, "mapCount");
    fog              = glGetUniformLocation(program, "fog");
    fogSettings      = glGetUniformLocation(program, "fogSettings");
    ProjectionMatrix    = glGetUniformLocation(program, "ProjectionMatrix");
    ModelViewMatrix     = glGetUniformLocation(program, "ModelViewMatrix");
}

void scex::start(shader_options *options) {
    glUseProgram(program);
    glUniform1i(mapTexture0,0);
    glUniform1i(mapTexture1,1);
    glUniform1i(mapTexture2,2);
    glUniform1i(mapTexture3,3);
    glUniform4f(fog, options->fogr, options->fogg, options->fogb, 1.0);
    glUniform2f(fogSettings, options->fogdist, options->fogcut);
#ifdef RENDER_CORE_32
    glUniformMatrix4fv(ProjectionMatrix, 1, false, options->perspective);
    glUniformMatrix4fv(ModelViewMatrix , 1, false, options->modelview);
#endif
}
void scex::update(shader_options *options) {
#ifdef RENDER_CORE_32
    glUniformMatrix4fv(ModelViewMatrix , 1, false, options->modelview);
#endif
}

void scex::stop() {
    
}

void scex_object::setBaseUV(float u, float v) {
    uscale = u;
    vscale = v;
}
void scex_object::setFogSettings(float r, float g, float b, float distance, float cutoff) {
    fogr = r;
    fogg = g;
    fogb = b;
    fogdist = distance;
    fogcut = cutoff;
}

Stage4Renderable::Stage4Renderable() {
}

// Senv object
void scex_object::setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) {
    
    // Should we render this?
    //0x5B578D is in charge of lighting
    uint32_t flags = *(uint32_t*)(shaderTag->Data() + 0x29);
    if (((flags>>(31-27)) & 1) != 0) {
        skip = true;
        return;
    }
    skip = false;
    
    // Use  4 stage maps
    stage4Maps.resize(4);
    for (int i=0; i < 4; i++) {
        Stage4Renderable *renderable = new Stage4Renderable;
        renderable->map = nullptr;
        renderable->uScale = 0.0;
        renderable->vScale = 0.0;
        renderable->uOffset = 0.0;
        renderable->vOffset = 0.0;
        stage4Maps[i] = renderable;
    }
    
    printf("scex object setup %s\n", shaderTag->Name());
    HaloTagReflexive stage4 = *(HaloTagReflexive*)(shaderTag->Data() + 0x54);
    
    mapsCount = 0;
    for (int i=0; i < stage4.count; i++) {
        printf("scex setup map %d 0x%x\n", i, stage4.address);
        uint8_t *stageMap = (uint8_t*)(shaderTag->Data() + shaderTag->PointerToOffset(stage4.address) + 220 * i);
        texture *mapTex = manager->texture_manager()->create_texture(map, *(HaloTagDependency*)(stageMap + 0x6C));
        stage4Maps[i]->map = mapTex;
        stage4Maps[i]->uScale = *(float*)(stageMap + 0x54); //stageMap->uScale;
        stage4Maps[i]->vScale = *(float*)(stageMap + 0x58); //stageMap->vScale;
        stage4Maps[i]->cFunc = *(uint16_t*)(stageMap + 0x2C);
        stage4Maps[i]->aFunc = *(uint16_t*)(stageMap + 0x2E);
        
        stage4Maps[i]->uAnimate = *(uint16_t*)(stageMap + 0xA6);
        stage4Maps[i]->vAnimate = *(uint16_t*)(stageMap + 0xB6);
        stage4Maps[i]->uPeriod = *(float*)(stageMap + 0xA8);
        stage4Maps[i]->vPeriod = *(float*)(stageMap + 0xB8);
        stage4Maps[i]->uAScale = *(float*)(stageMap + 0xB0);
        stage4Maps[i]->vAScale = *(float*)(stageMap + 0xC0);
        stage4Maps[i]->prev = timems();
        mapsCount++;
    }

    printf("shader setup\n");
    scex *shader = (scex *)(manager->get_shader(shader_SCEX));
    baseMapUV        = shader->baseMapUV;
    useMap           = shader->useMap;
    mapTexture0       = shader->mapTexture0;
    mapTexture1       = shader->mapTexture1;
    mapTexture2       = shader->mapTexture2;
    mapTexture3       = shader->mapTexture3;
    uScale           = shader->uScale;
    vScale           = shader->vScale;
    uOffset          = shader->uOffset;
    vOffset          = shader->vOffset;
    colorFunction    = shader->colorFunction;
    alphaFunction    = shader->alphaFunction;
    mapCount         = shader->mapCount;
};

bool scex_object::is(ShaderType type) {
    return (type == shader_SCEX);
}
bool scex_object::render(ShaderType type) {
    if (skip) {
        return false;
    }
    
    // Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    #ifndef RENDER_CORE_32
    // Texturing
    glEnable(GL_TEXTURE_2D);
    #endif
    
    int i;
    for (i=0; i < mapsCount; i++) {
        switch (i) {
            case 0:
                glActiveTexture(GL_TEXTURE0);
                break;
            case 1:
                glActiveTexture(GL_TEXTURE1);
                break;
            case 2:
                glActiveTexture(GL_TEXTURE2);
                break;
            case 3:
                glActiveTexture(GL_TEXTURE3);
                break;
            default:
                break;
        }
        
        Stage4Renderable *renderable = stage4Maps[i];
        renderable->map->bind();
        
        // Update the tick
        std::chrono::milliseconds now = timems();
        double seconds = (now.count() - stage4Maps[i]->prev.count()) / 1000.0;
        stage4Maps[i]->prev = now;
        
        switch (stage4Maps[i]->uAnimate) {
            case 6: // Slide
                stage4Maps[i]->uOffset += (seconds / stage4Maps[i]->uPeriod)  * stage4Maps[i]->uAScale;
                break;
            default:
                break;
        }
    
        switch (stage4Maps[i]->vAnimate) {
            case 6: // Slide
                stage4Maps[i]->vOffset += (seconds / stage4Maps[i]->vPeriod) * stage4Maps[i]->vAScale;
                break;
            default:
                break;
        }
    }
    
    glUniform1i(mapCount, mapsCount);
    glUniform2f(baseMapUV, uscale, vscale);
    glUniform4f(uScale, stage4Maps[0]->uScale, stage4Maps[1]->uScale, stage4Maps[2]->uScale, stage4Maps[3]->uScale);
    glUniform4f(vScale, stage4Maps[0]->vScale, stage4Maps[1]->vScale, stage4Maps[2]->vScale, stage4Maps[3]->vScale);
    glUniform4f(uOffset, stage4Maps[0]->uOffset, stage4Maps[1]->uOffset, stage4Maps[2]->uOffset, stage4Maps[3]->uOffset);
    glUniform4f(vOffset, stage4Maps[0]->vOffset, stage4Maps[1]->vOffset, stage4Maps[2]->vOffset, stage4Maps[3]->vOffset);
    glUniform4i(colorFunction, stage4Maps[0]->cFunc, stage4Maps[1]->cFunc, stage4Maps[2]->cFunc, stage4Maps[3]->cFunc);
    glUniform4i(alphaFunction, stage4Maps[0]->aFunc, stage4Maps[1]->aFunc, stage4Maps[2]->aFunc, stage4Maps[3]->aFunc);
    return true;
}
