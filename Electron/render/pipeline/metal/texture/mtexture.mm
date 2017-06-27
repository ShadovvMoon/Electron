//
//  mtexture.cpp
//  Metallic
//
//  Created by Samuco on 6/23/17.
//  Copyright © 2017 Samuco. All rights reserved.
//

#include "mtexture.h"
#import "SharedStructures.h"
#import "ProtonMap.h"
#import "camera.h"
#import "bitmaps.h"
#import "metal.hpp"
#import "squish.h"
using namespace squish;

metal_texture::metal_texture(id<MTLDevice> device, id <MTLCommandQueue> queue, ProtonMap *map, HaloTagDependency bitm) {
    textureLevels = [[NSMutableArray alloc] init];
    
    // Check
    if (bitm.tag_id.tag_index == NULLED_TAG_ID) {
        printf("invalid bitmap\n");
        return;
    }
    
    // Load the texture (texture 0 for now)
    ProtonTag *bitmapTag = map->tags.at(bitm.tag_id.tag_index).get();
    if (!bitmapTag) {
        printf("missing bitmap\n");
        return;
    }
    
    // Extract each layer
    bitm_header_t *bitmData = (bitm_header_t *)bitmapTag->Data();
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
        if (texture == nil) {
            std::cerr << "metal texture is nil" << std::endl;
            continue;
        }
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
        } else if (image->format == BITM_FORMAT_DXT1 ||
                   image->format == BITM_FORMAT_DXT2AND3 ||
                   image->format == BITM_FORMAT_DXT4AND5) {
            
            // Select block size
            int format = kDxt1;
            if (image->format == BITM_FORMAT_DXT2AND3) {
                format = kDxt3;
            } else if (image->format == BITM_FORMAT_DXT4AND5) {
                format = kDxt5;
            }
            
            // Conversion
            char *output = (char*)malloc(4 * image->width * image->height);
            DecompressImage(
                (uint8_t*)output,
                image->width,
                image->height,
                input,
                format
            );
            [texture replaceRegion:region mipmapLevel:0 withBytes:output bytesPerRow:4 * image->width];
            free(output);
        } else {
            printf("unknown format\n");
            return;
        }
        
        
        id <MTLCommandBuffer> commandBuffer = [queue commandBuffer];
        id <MTLBlitCommandEncoder> commandEncoder = [commandBuffer blitCommandEncoder];
        [commandEncoder generateMipmapsForTexture:texture];
        [commandEncoder endEncoding];
        [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
            // texture is now ready for use
        }];
        [commandBuffer commit];
        [textureLevels addObject:texture];
    }
}

void metal_texture::bind(Pipeline *pipeline) {
    bind(0, pipeline);
}

void metal_texture::bind(int i, Pipeline *pipeline) {
    MetalPipeline *mpipe = (MetalPipeline*)pipeline;
    id <MTLRenderCommandEncoder> renderEncoder = mpipe->encoder;
    if (i < 0 || i >= [textureLevels count]) {
        return;
    }
    id<MTLTexture> texture = [textureLevels objectAtIndex:i];
    [renderEncoder setFragmentTexture:texture atIndex:mpipe->activeTexture];
}

void metal_texture_cubemap::load_side(GLenum side_target, GLenum i, ProtonTag *bitmapTag) {
}

metal_texture_cubemap::metal_texture_cubemap(ProtonMap *map, HaloTagDependency bitm) {
}

void metal_texture_cubemap::bind(Pipeline *pipeline) {
}

texture *MetalTextureManager::create_texture(ProtonMap *map, HaloTagDependency bitm) {
    printf("%d\n", bitm.tag_id.tag_index);
    
    // Has this bitmap been loaded before? Check the cache
    std::map<uint16_t, texture*>::iterator iter = textures.find(bitm.tag_id.tag_index);
    if (iter != textures.end()) {
        return iter->second;
    }
    
    // Create a new texture
    texture *tex = new metal_texture(device, mipmapQueue, map, bitm);
    textures[bitm.tag_id.tag_index] = tex;
    return tex;
}

texture_cubemap *MetalTextureManager::create_cubemap(ProtonMap *map, HaloTagDependency bitm) {
    return nullptr;
}
 
MetalTextureManager::MetalTextureManager(id<MTLDevice> device, id <MTLCommandQueue> _commandQueue) {
    this->device = device;
    this->mipmapQueue = _commandQueue;
}
