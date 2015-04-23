/*
 
 Copyright (c) 2015, Samuel Colbran
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * Neither the name of Paul Whitcomb nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 */
//
//  ZZTHaloScenarioTag.h
//  Electron
//
//  Created by Samuel Colbran on 19/4/15.
//  Copyright (c) 2015 Samuco. All rights reserved.
//

#include "HaloData.h"

#ifndef _ZZTHaloScenarioTag_h
#define _ZZTHaloScenarioTag_h

typedef struct {
    uint8_t unk0[0x30];     //0x0
    HaloTagReflexive skyBox;       //0x30
    uint32_t unk1;          //0x3C
    HaloTagReflexive children;     //0x40
    uint8_t unk2[0xA0];     //0x4C
    HaloTagReflexive resources;    //0xEC
    HaloTagReflexive functions;    //0xF8
    uint32_t unk4[2];       //0x104
    HaloTagReflexive unknown;      //0x110
    HaloTagReflexive comments;     //0x118
    uint8_t unk5[0xE0];     //0x124
    HaloTagReflexive objectNames;  //0x204
    HaloTagReflexive scen;
    HaloTagReflexive scenRef;
    HaloTagReflexive bipd;
    HaloTagReflexive bipdRef;
    HaloTagReflexive vehi;
    HaloTagReflexive vehiRef;
    HaloTagReflexive eqip;
    HaloTagReflexive eqipRef;
    HaloTagReflexive weap;
    HaloTagReflexive weapRef;
    HaloTagReflexive deviceGroups;
    HaloTagReflexive mach;
    HaloTagReflexive machRef;
    HaloTagReflexive cont;
    HaloTagReflexive contRef;
    HaloTagReflexive light;
    HaloTagReflexive lightRef;
    HaloTagReflexive sound;
    HaloTagReflexive soundRef;
    HaloTagReflexive unk6;
    HaloTagReflexive unk7;
    HaloTagReflexive unk8;
    HaloTagReflexive unk9;
    HaloTagReflexive unk10;
    HaloTagReflexive unk11;
    HaloTagReflexive unk12;
    HaloTagReflexive playerStartingProfile;
    HaloTagReflexive playerSpawn;
    HaloTagReflexive triggerVolumes;
    HaloTagReflexive animations;
    HaloTagReflexive multiplayerFlags;
    HaloTagReflexive mpEquip;
    HaloTagReflexive startEquip;
    HaloTagReflexive bspSwitchTriggers;
    HaloTagReflexive decal;
    HaloTagReflexive decalRef;
    HaloTagReflexive dobcRef;
    HaloTagReflexive unk13;
    HaloTagReflexive unk14;
    HaloTagReflexive unk15;
    HaloTagReflexive unk16;
    HaloTagReflexive unk17;
    HaloTagReflexive unk18;
    HaloTagReflexive unk19;
    HaloTagReflexive actorRef;
    HaloTagReflexive encounters;
    HaloTagReflexive commandlists;
    HaloTagReflexive unk20;
    HaloTagReflexive AiStartingLocations;
    HaloTagReflexive AiPlatoons;
    HaloTagReflexive AiConversations;
    uint32_t scriptDataSize; //0x474
    uint32_t unk21;          //0x478
    HaloTagReflexive scripts;       //0x47C
    uint32_t unk22[2];       //0x488
    HaloTagReflexive commands;      //0x490
    HaloTagReflexive points;        //0x49C
    HaloTagReflexive AiAnimationRef;//0x4A8
    HaloTagReflexive globals;       //0x4B4
    uint32_t unk23[9];       //0x4C0
    HaloTagReflexive AiRecordingRef;//0x4E4
    HaloTagReflexive unk24;         //0x4F0
    HaloTagReflexive participants;  //0x4FC
    uint32_t unk25[9];       //0x508
    HaloTagReflexive lines;         //0x52C
    uint8_t unk26[0x6C];     //0x538
    HaloTagReflexive bsp;           //0x5A4
} HaloScenarioTag;

typedef struct {
    uint32_t bsp_mapfile_start;
    uint32_t bsp_size;
    uint32_t bsp_magic;
    uint32_t zero;
    char sbsp[4];
    uint32_t name_address;
    uint32_t reserved;
    uint16_t tagId;
    uint16_t halodoesntevenreadthisvaluewtfbungiewhydoyoudothistome;
} BSP_CHUNK;

typedef struct {
    HaloTagDependency lightmaps;
    uint32_t unk0[0x25];
    HaloTagReflexive shaders;
    HaloTagReflexive collBSP;
    HaloTagReflexive nodes;
    uint32_t unk1[0x6];
    HaloTagReflexive leaves;
    HaloTagReflexive surfaces;
    HaloTagReflexive submeshIndices;
    HaloTagReflexive submeshHeader;
    HaloTagReflexive unk2;
    HaloTagReflexive unk3;
    HaloTagReflexive unk4;
    HaloTagReflexive clusters;
    uint32_t clusterSize;
    uint32_t unk5;
    HaloTagReflexive unk6;
    HaloTagReflexive clusterPortals;
    HaloTagReflexive unk7;
    HaloTagReflexive breakableSurfaces;
    HaloTagReflexive fogPlanes;
    HaloTagReflexive fogRegions;
    HaloTagReflexive weatherPalette;
    HaloTagReflexive unk8;
    HaloTagReflexive unk9;
    HaloTagReflexive weather;
    HaloTagReflexive weatherPolyhedra;
    HaloTagReflexive unk10;
    HaloTagReflexive unk11;
    HaloTagReflexive pathfinding;
    HaloTagReflexive unk12;
    HaloTagReflexive backgroundSound;
    HaloTagReflexive environmentSound;
    uint32_t soundSize;
    uint32_t unk13;
    HaloTagReflexive unk14;
    HaloTagReflexive unk15;
    HaloTagReflexive unk16;
    HaloTagReflexive markers;
    HaloTagReflexive dobc;
    HaloTagReflexive decals;
    uint32_t unk17[0x9];
} BSP_MESH;

typedef struct
{
    short LightmapIndex;
    short unk1;
    uint32_t unknown[4];
    HaloTagReflexive material;
} BSP_SUBMESH;

typedef struct
{
    HaloTagDependency ShaderTag;
    uint32_t UnkZero2;
    uint32_t VertIndexOffset;
    uint32_t VertIndexCount;
    float Centroid[3];
    float AmbientColor[3];
    uint32_t DistLightCount;
    float DistLight1[6];
    float DistLight2[6];
    float unkFloat2[3];
    float ReflectTint[4];
    float ShadowVector[3];
    float ShadowColor[3];
    float Plane[4];
    uint32_t UnkFlag2;
    uint32_t UnkCount1;
    uint32_t VertexCount1;
    uint32_t UnkZero4;
    uint32_t VertexOffset;
    uint32_t Vert_HaloTagReflexive;
    uint32_t UnkAlways3;
    uint32_t VertexCount2;
    uint32_t UnkZero9;
    uint32_t UnkLightmapOffset;
    uint32_t CompVert_HaloTagReflexive;
    uint32_t UnkZero5[2];
    uint32_t SomeOffset1;
    uint32_t PcVertexDataOffset;
    uint32_t UnkZero6;
    uint32_t CompVertBufferSize;
    uint32_t UnkZero7;
    uint32_t SomeOffset2;
    uint32_t VertexDataOffset;
    uint32_t UnkZero8;
} MATERIAL_SUBMESH_HEADER;

typedef struct
{
    uint32_t comp_normal;
    short comp_uv[2];
}COMPRESSED_LIGHTMAP_VERT;
typedef struct
{
    float normal[3];
    float uv[2];
}UNCOMPRESSED_LIGHTMAP_VERT;


typedef struct
{
    float vertex_k[3]; //0
    float normal[3];   //0xC
    float binormal[3]; //0x18
    float tangent[3];  //0x24
    float uv[2]; //0x30
}UNCOMPRESSED_BSP_VERT;

typedef struct
{
    float vertex_k[3];
    uint32_t  comp_normal;
    uint32_t  comp_binormal;
    uint32_t  comp_tangent;
    float uv[2];
}COMPRESSED_BSP_VERT;
typedef struct
{
    unsigned short tri_ind[3];
}TRI_INDICES;

#endif
