//
//  BitmSndMap.h
//  Proton
//
//  Created by Paul Whitcomb on 1/12/15.
//  Copyright (c) 2015 Paul Whitcomb. All rights reserved.
//

#include <cstdlib>
#include "HaloData.h"

#ifndef Resource_Imploder_BitmSndMap_h
#define Resource_Imploder_BitmSndMap_h


// Bitmaps.map, Sounds.map, Loc.map stuff
struct HaloResourceMapHeader {
    uint32_t type;                  // 0x0; 1 = bitmaps, 2 = sounds, 3 = loc
    uint32_t names_offset;          // 0x4; File offset of names
    uint32_t resource_index_offset; // 0x8; File offset of resource array
    uint32_t resource_count;        // 0xC; Number of resources
};

struct HaloResourceMapResource {
    uint32_t resource_name;         // 0x0; name is relative to names_offset
    uint32_t resource_size;         // 0x4; size of the resource data in bytes
    uint32_t resource_data_offset;  // 0x8; file offset of resource data
};

struct HaloMapTagID {
    uint16_t tag_index;
    uint16_t tag_index_different;
};

struct HaloMapTagIndex {
    char tag_class_a[4];
    char tag_class_b[4];
    char tag_class_c[4];
    HaloTagID tag_id;
    uint32_t name_offset;
    uint32_t data_offset;
    uint32_t not_in_map;
    uint32_t padding2;
};

struct HaloMapReflexive {
    uint32_t count;
    uint32_t address;
    uint32_t padding;
};

// Not mapped out. Just basically the offset to the resource data.
struct HaloMapBitmTag {
    char offset[0x60];
    HaloMapReflexive bitmaps;
};

// Same as above
struct HaloMapBitmBitmaps {
    char offset[0xF];
    char not_internalized : 1;
    char offset2 : 7;
    char offset3[0x8];
    uint32_t data_offset;
    uint32_t length;
    char offset4[0x10];
};

// Same again.
struct HaloMapSndTag {
    char offset[0x98];
    HaloMapReflexive ranges;
};

// Yep.
struct HaloMapSndTagRange {
    char offset[0x3C];
    HaloMapReflexive permutations;
};

// I'm Zippy the Zebra! How do you like my stripes?
struct HaloMapSndTagPermutation {
    char offset[0x40];
    uint32_t length;
    char not_internalized : 1;
    char offset1 : 7;
    char offset2[3];
    uint32_t data_offset;
    char offset3[0x30];
};

struct HaloMapMod2GeoIndex {
    uint32_t count; //size in bytes = 0x4 + 0x2 x count
    uint32_t index_offsetA;
    uint32_t index_offsetB;
};

struct HaloMapMod2GeoVertex {
    uint32_t count; //size in bytes = 0x44 x count
    char offset[0x8];
    uint32_t vertex_offset;
};

struct HaloMapMod2GeoPart {
    char offset[0x48];
    HaloMapMod2GeoIndex index;
    char offset1[0x4];
    HaloMapMod2GeoVertex vertex;
    char offset2[0x1C];
};

struct HaloMapMod2Geo {
    char offset[0x24];
    HaloTagReflexive parts;
}; //reflexive is offset 0xD0 in mod2 tag

struct HaloMapMod2 {
    char offset[0xD0];
    HaloTagReflexive geos;
};


#endif
