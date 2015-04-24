//
//  HaloData.h
//  Proton
//
//  Created by Paul Whitcomb on 2/13/15.
//  Copyright (c) 2015 Paul Whitcomb. All rights reserved.
//

#ifndef Proton_HaloData_h
#define Proton_HaloData_h

#include <cstdlib>
#include <cstdint>

using std::uint16_t;
using std::uint32_t;

#define NULLED_TAG_ID 0xFFFF
#define TAGS          0x74616773
#define GFOT          0x47666f74
#define EHED          0x45686564
#define FOOT          0x47666f74
#define HEAD          0x68656164

typedef enum HaloMapGame : uint32_t {
    HALO_MAP_GAME_PC = 7,
    HALO_MAP_GAME_DEMO = 6,
    HALO_MAP_GAME_XBOX = 5,
    HALO_MAP_GAME_CE = 0x261,
    HALO_MAP_GAME_CUSTOM_PROTON_TAG_ARRAY = 0xFF01
} HaloMapGame;

typedef enum HaloMapType : uint16_t {
    HALO_MAP_TYPE_SINGLEPLAYER = 0,
    HALO_MAP_TYPE_MULTIPLAYER = 1,
    HALO_MAP_TYPE_USER_INTERFACE = 2, 
    HALO_MAP_TYPE_CUSTOM_PROTON_TAG_ARRAY = 0xFF01
} HaloMapType;

struct HaloTagID {
    uint16_t tag_index;
    uint16_t table_id;
    HaloTagID();
    HaloTagID(uint16_t tagIndex);
};

struct HaloTagDependency {
    char tag_class[4];
    uint32_t name_address;
    uint32_t reserved_data;
    HaloTagID tag_id;
};

struct HaloTagReflexive {
    uint32_t count;
    uint32_t address;
    uint32_t reserved_data;
};


struct HaloCacheFileHeaderDemo {
    char padding1[2] = {};              //0x0
    HaloMapType mapType;                //0x2
    char padding2[0x2BC] = {};          //0x4 We're going to be seeing a lot of these in the demo header.
    uint32_t head = EHED;         //0x2C0 Ehed
    uint32_t tagDataSize;               //0x2C4
    char mapBuild[32] = {};             //0x2C8
    char padding3[0x2A0];               //0x2E8
    HaloMapGame mapGame;                //0x588
    char mapName[32] = {};              //0x58C
    char padding4[4] = {};              //0x5AC
    uint32_t fileCRC32;                 //0x5B0
    char padding5[0x34] = {};           //0x5B4
    uint32_t fileSize;                  //0x5E8
    uint32_t tagDataOffset;             //0x5EC
    uint32_t foot = GFOT;         //0x5F0 Gfot
    char padding6[0x20C]={};            //0x5F4
    
    struct HaloCacheFileHeader asStandardHeader();
};

struct HaloCacheFileHeader {
    uint32_t head = HEAD;         //0x0 head (not a real head)
    HaloMapGame mapGame;                //0x4
    uint32_t fileSize;                  //0x8 file size when decompressed
    char padding1[4] = {};              //0xC
    uint32_t tagDataOffset;             //0x10
    uint32_t tagDataSize;               //0x14
    char padding2[8] = {};              //0x18
    char mapName[32] = {};              //0x20
    char mapBuild[32] = {};             //0x40
    HaloMapType mapType;                //0x60
    char padding3[2] = {};              //0x62
    uint32_t fileCRC32;                 //0x64 crc32 when decompressed
    char padding4[0x794]={};            //0x68
    uint32_t foot = FOOT;         //0x7FC foot
    
    struct HaloCacheFileHeaderDemo asDemoHeader();
};

struct HaloCacheFileTagDataHeader {
    uint32_t tagArrayAddress;
    HaloTagID principalScenarioTag;
    uint32_t randomNumber;
    uint32_t tagCount;
    uint32_t partCountA;
    uint32_t modelDataOffset;
    uint32_t partCountB;
    uint32_t vertexSize;
    uint32_t modelSize;
    uint32_t tags = TAGS;
};

struct HaloCacheFileTagArrayIndex {
    char tagClassA[4];
    char tagClassB[4];
    char tagClassC[4];
    HaloTagID tagID;
    uint32_t nameAddress;
    uint32_t dataAddress;
    uint32_t notInMap;
    uint32_t padding;
};

struct HaloScnrBSPIndex {
    uint32_t sbspFileOffset;
    uint32_t sbspSize;
    uint32_t sbspAddress;
    char reserved[4];
    HaloTagDependency sbsp;
};

#endif
