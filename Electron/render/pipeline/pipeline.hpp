//
//  pipeline.hpp
//  Electron
//
//  Created by Samuco on 6/22/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#ifndef pipeline_hpp
#define pipeline_hpp

#include <stdio.h>
#include "texture.h"

// Pipeline


// Texture
#define BITM_FORMAT_A8            0x00
#define BITM_FORMAT_Y8            0x01
#define BITM_FORMAT_AY8            0x02
#define BITM_FORMAT_A8Y8        0x03
#define BITM_FORMAT_R5G6B5        0x06
#define BITM_FORMAT_A1R5G5B5    0x08
#define BITM_FORMAT_A4R4G4B4    0x09
#define BITM_FORMAT_X8R8G8B8    0x0A
#define BITM_FORMAT_A8R8G8B8    0x0B
#define BITM_FORMAT_DXT1        0x0E
#define BITM_FORMAT_DXT2AND3    0x0F
#define BITM_FORMAT_DXT4AND5    0x10
#define BITM_FORMAT_P8            0x11

// Types
#define BITM_TYPE_2D            0x00
#define BITM_TYPE_3D            0x01
#define BITM_TYPE_CUBEMAP        0x02

// Flags
#define BITM_FLAG_LINEAR        (1 << 4)

// Structs
typedef struct
{
    uint16_t type;
    uint16_t format;
    //uint16_t usage;
    int32_t unknown[0x14];
    HaloTagReflexive reflexive_to_first;
    HaloTagReflexive image_reflexive;
} bitm_header_t;

typedef struct
{
    int                            unknown[16];
    
} bitm_first_t;

typedef struct
{
    int                            id;            // 'bitm' 0x0
    short                        width;      //04
    short                        height;     //06
    short                        depth;      //08
    short                        type;       //10
    short                        format;     //12
    //short                        flags;
    char                        flag0;      //14
    char                        internalized; //15
    short                        reg_point_x; //16
    short                        reg_point_y; //18
    short                        num_mipmaps; //20
    short                        pixel_offset; //22
    int                         offset; //24
    int                            size;
    int                            unknown8;
    int                            unknown9;    // always 0xFFFFFFFF?
    int                            unknown10;    // always 0x00000000?
    int                            unknown11;    // always 0x024F0040?
} bitm_image_t;

// Conversion
typedef struct
{
    unsigned int r, g, b, a;
} rgba_color_t;

void DecodeLinearA8R8G8B8 (int width, int height, const char *texdata, unsigned int *outdata);
void DecodeLinearX8R8G8B8 (int width, int height, const char *texdata, unsigned int *outdata);
void DecodeLinearR5G6B5 (int width, int height, const char *texdata, unsigned int *outdata);

// Shaders
class shader {
public:
    int program;
    virtual void start(shader_options *options) = 0;
    virtual void update(shader_options *options) = 0;
    virtual void stop() = 0;
};

class ShaderManager;
class shader_object {
public:
    virtual void setup(ShaderManager *manager, ProtonMap *map, ProtonTag *shaderTag) = 0;
    virtual void setBaseUV(float u, float v) = 0;
    virtual void setFogSettings(float r, float g, float b, float distance, float cutoff) = 0;
    virtual bool render(ShaderType type, Pipeline *pipeline) = 0;
    virtual bool is(ShaderType type) = 0;
};

class ShaderManager {
public:
    //virtual ShaderManager(const char *resources) = 0;
    virtual TextureManager *texture_manager() = 0;
    virtual shader *get_shader(ShaderType pass) = 0;
    virtual bool needs_reflection() = 0;
    virtual float reflection_height() = 0;
    virtual void set_needs_reflection(bool reflect) = 0;
    virtual void set_reflection_height(float height) = 0;
    virtual uint get_reflection(int index) = 0;
    virtual shader_object *create_shader(ProtonMap *map, HaloTagDependency shader) = 0;
};

// Mesh
class Submesh {
public:
    virtual void bindTexture(int index, texture *texture, int mipmap) = 0;
};

class Pipeline;
enum VertexBuffer {
    VBPosition = 0,
    VBTextureCoordinate = 1,
    VBLightCoordinate = 2,
    VBNormal = 3,
    VBBinormal = 4,
    VBTangent = 5,
    VBCount = 7
};

class Mesh {
public:
    virtual void render(ShaderType pass, Pipeline *pipeline) = 0;
    virtual void writeIndexData(int i, void *array, int vertices) = 0;
    virtual void writeVertexData(VertexBuffer buffer, int uniform, int count, void *array, int vertices) = 0;
    virtual Submesh* addSubmesh(shader_object *shader, int vertexOffset, int vertexCount, int indexOffset, int indexCount) = 0;
};

class Pipeline {
public:
    virtual Mesh *createMesh(int count, int verts) = 0;
};

#endif /* pipeline_hpp */
