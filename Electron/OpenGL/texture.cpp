//
//  shader.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "texture.h"
#include "squish.h"
using namespace squish;

#define BITM_FORMAT_A8			0x00
#define BITM_FORMAT_Y8			0x01
#define BITM_FORMAT_AY8			0x02
#define BITM_FORMAT_A8Y8		0x03
#define BITM_FORMAT_R5G6B5		0x06
#define BITM_FORMAT_A1R5G5B5	0x08
#define BITM_FORMAT_A4R4G4B4	0x09
#define BITM_FORMAT_X8R8G8B8	0x0A
#define BITM_FORMAT_A8R8G8B8	0x0B
#define BITM_FORMAT_DXT1		0x0E
#define BITM_FORMAT_DXT2AND3	0x0F
#define BITM_FORMAT_DXT4AND5	0x10
#define BITM_FORMAT_P8			0x11

// Types
#define BITM_TYPE_2D			0x00
#define BITM_TYPE_3D			0x01
#define BITM_TYPE_CUBEMAP		0x02

// Flags
#define BITM_FLAG_LINEAR		(1 << 4)

// Structs
typedef struct
{
    int32_t unknown[0x15];
    HaloTagReflexive reflexive_to_first;
    HaloTagReflexive image_reflexive;
} bitm_header_t;

typedef struct
{
    int							unknown[16];
    
} bitm_first_t;


//0x10 + 1 + 8 = 0x19

typedef struct
{
    int							id;			// 'bitm' 0x0
    short						width;      //04
    short						height;     //06
    short						depth;      //08
    short						type;       //10
    short						format;     //12
    //short						flags;
    char						flag0;      //14
    char						internalized; //15
    short						reg_point_x; //16
    short						reg_point_y; //18
    short						num_mipmaps; //20
    short						pixel_offset; //22
    int         				offset; //24
    int							size;
    int							unknown8;
    int							unknown9;	// always 0xFFFFFFFF?
    int							unknown10;	// always 0x00000000?
    int							unknown11;	// always 0x024F0040?
} bitm_image_t;

texture::texture(ProtonMap *map, HaloTagDependency bitm) {
    glDeleteTextures(1, &tex);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    // Params
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    
    // Load the texture (texture 0 for now)
    ProtonTag *bitmapTag = map->tags.at(bitm.tag_id.tag_index).get();
    if (bitmapTag) {
        bitm_header_t *bitmData = (bitm_header_t *)bitmapTag->Data();
        if (bitmData->image_reflexive.count > 0) {
            
            int i = 0;
            bitm_image_t *image = (bitm_image_t *)(bitmapTag->Data() + bitmapTag->PointerToOffset(bitmData->image_reflexive.address) + sizeof(bitm_image_t) * i);
            char *input = (char*)bitmapTag->ResourcesData() + image->offset;
            
            GLint format = GL_RGBA;
            GLint internalFormat = GL_RGBA;
            
            // If the image is in bitmaps.map, read from that instead
            if (image->internalized == 1) {
                input = (char*)bitmap_data() + image->offset;
            };
            
            printf("loading %s %d %d %d %d 0x%x -> %d\n", bitmapTag->Name(), image->width, image->height, image->format, image->internalized, bitmapTag->ResourcesData(), tex);
            int size = 0;
            if (image->format == BITM_FORMAT_DXT1) {
                format = GL_RGB;
                internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
                size = (image->width >> 2) * (image->height >> 2) * 8;
            } else if (image->format == BITM_FORMAT_DXT2AND3) {
                format = GL_RGBA;
                internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
                size = (image->width >> 2) * (image->height >> 2) * 16;
            } else if (image->format == BITM_FORMAT_DXT4AND5) {
                format = GL_RGBA;
                internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
                size = (image->width >> 2) * (image->height >> 2) * 16;
            } else {
                return;
            }
            
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image->width, image->height, 0, size, input);
            /*glTexImage2D(GL_TEXTURE_2D,
                         0,
                         internalFormat,
                         image->width,
                         image->height,
                         0,
                         format,
                         GL_UNSIGNED_BYTE,
                         input);
             */
            
            
        }
    } else {
        printf("missing bitmap\n");
    }
}
void texture::bind() {
    glBindTexture(GL_TEXTURE_2D, tex);
}

texture *TextureManager::create_texture(ProtonMap *map, HaloTagDependency bitm) {
    // Has this bitmap been loaded before? Check the cache
    std::map<uint16_t, texture*>::iterator iter = textures.find(bitm.tag_id.tag_index);
    if (iter != textures.end()) {
        return iter->second;
    }
    
    // Create a new texture
    texture *tex = new texture(map, bitm);
    textures[bitm.tag_id.tag_index] = tex;
    return tex;
}