//
//  senv.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "schi.h"


// Global instance
void schi::setup(std::string path) {
    printf("schi setup\n");
    GLuint vertex_shader, fragment_shader;
    vertex_shader   = make_shader(GL_VERTEX_SHADER,   (path + "/schi.vert").c_str());
    fragment_shader = make_shader(GL_FRAGMENT_SHADER, (path + "/schi.frag").c_str());
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
}

void schi::start() {
    glUseProgram(program);
    glUniform1i(mapTexture0,0);
    glUniform1i(mapTexture1,1);
    glUniform1i(mapTexture2,2);
    glUniform1i(mapTexture3,3);
    errorCheck();
}

void schi::stop() {
    
}

void schi_object::setBaseUV(float u, float v) {
    uscale = u;
    vscale = v;
}

// Senv object
void schi_object::setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) {
    
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
    schi *shader = (schi *)(manager->get_shader(shader_SCHI));
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

bool schi_object::is(ShaderType type) {
    return (type == shader_SCHI);
}

bool schi_object::render() {
    
    // Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Texturing
    glEnable(GL_TEXTURE_2D);
    
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
