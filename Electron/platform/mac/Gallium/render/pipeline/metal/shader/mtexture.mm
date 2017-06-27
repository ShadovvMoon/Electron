//
//  mtexture.cpp
//  Metallic
//
//  Created by Samuco on 6/23/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#include "texture.h"
#import "SharedStructures.h"
#import "ProtonMap.h"
#import "camera.h"
#import "bitmaps.h"
#import <MetalKit/MetalKit.h>

/*
metal_texture::metal_texture(ProtonMap *map, HaloTagDependency bitm) {
    // Check
    if (bitm.tag_id.tag_index == NULLED_TAG_ID) {
        printf("invalid bitmap\n");
        return nil;
    }
    
    // Load the texture (texture 0 for now)
    ProtonTag *bitmapTag = map->tags.at(bitm.tag_id.tag_index).get();
    if (bitmapTag) {
        bitm_header_t *bitmData = (bitm_header_t *)bitmapTag->Data();
        NSMutableArray *textureLevels = [[NSMutableArray alloc] init];
        int i;
        for (i=0; i < bitmData->image_reflexive.count; i++) {
            
            bitm_image_t *image = (bitm_image_t *)(bitmapTag->Data() + bitmapTag->PointerToOffset(bitmData->image_reflexive.address) + sizeof(bitm_image_t) * i);
            char *input = (char*)bitmapTag->ResourcesData() + image->offset;
            if (image == NULL) continue;
            
            // If the image is in bitmaps.map, read from that instead
            if (image->internalized == 1) {
                input = (char*)bitmap_data() + image->offset;
            };
            
            // Create the texture descriptor
            MTLTextureDescriptor *textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:image->width height:image->height mipmapped:YES];
            id<MTLTexture> texture = [device newTextureWithDescriptor:textureDescriptor];
            MTLRegion region = MTLRegionMake2D(0, 0, image->width, image->height);
            
            // Convert textures into desired format
            if (image->format == BITM_FORMAT_X8R8G8B8) {
                char *output = (char*)malloc(4 * image->width * image->height);
                DecodeLinearX8R8G8B8(image->width,image->height,input,(unsigned int*)output);
                [texture replaceRegion:region mipmapLevel:0 withBytes:output bytesPerRow:4 * image->width];
                free(output);
            } else if (image->format == BITM_FORMAT_A8R8G8B8) {
                char *output = (char*)malloc(4 * image->width * image->height);
                DecodeLinearA8R8G8B8(image->width,image->height,input,(unsigned int*)output);
                [texture replaceRegion:region mipmapLevel:0 withBytes:output bytesPerRow:4 * image->width];
                free(output);
            } else if (image->format == BITM_FORMAT_R5G6B5) {
                char *output = (char*)malloc(4 * image->width * image->height);
                DecodeLinearR5G6B5(image->width,image->height,input,(unsigned int*)output);
                [texture replaceRegion:region mipmapLevel:0 withBytes:output bytesPerRow:4 * image->width];
                free(output);
            } else {
                printf("unknown format\n");
                return @[];
            }
            [textureLevels addObject:texture];
        }
        return textureLevels;
    }
    
    printf("missing bitmap\n");
    return @[];
}

void texture::bind() {}
void texture::bind(int i) {}
void texture_cubemap::load_side(GLenum side_target, GLenum i, ProtonTag *bitmapTag) {}
texture_cubemap::texture_cubemap(ProtonMap *map, HaloTagDependency bitm) {}
void texture_cubemap::bind() {}

texture *TextureManager::create_texture(ProtonMap *map, HaloTagDependency bitm) {
    printf("%d\n", bitm.tag_id.tag_index);
    
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

texture_cubemap *TextureManager::create_cubemap(ProtonMap *map, HaloTagDependency bitm) {
    return nullptr;
}
*/

