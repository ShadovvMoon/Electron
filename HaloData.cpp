//
//  HaloData.cpp
//  Proton
//
//  Created by Paul Whitcomb on 2/20/15.
//  Copyright (c) 2015 Paul Whitcomb. All rights reserved.
//

#include <stdio.h>
#include <string>
#include "HaloData.h"


HaloCacheFileHeaderDemo HaloCacheFileHeader::asDemoHeader() {
    HaloCacheFileHeaderDemo demoHeader = {};
    
    demoHeader.fileCRC32 = this->fileCRC32;
    demoHeader.fileSize = this->fileSize;
    memcpy(demoHeader.foot,"tofG",4);
    memcpy(demoHeader.head,"dehE",4);
    memcpy(demoHeader.mapBuild,this->mapBuild,32);
    memcpy(demoHeader.mapName,this->mapName,32);
    demoHeader.mapGame = this->mapGame;
    demoHeader.mapType = this->mapType;
    demoHeader.tagDataOffset = this->tagDataOffset;
    demoHeader.tagDataSize = this->tagDataSize;
    
    return demoHeader;
}
HaloCacheFileHeader HaloCacheFileHeaderDemo::asStandardHeader() {
    HaloCacheFileHeader header = {};
    
    header.fileCRC32 = this->fileCRC32;
    header.fileSize = this->fileSize;
    memcpy(header.foot,"toof",4);
    memcpy(header.head,"daeh",4);
    memcpy(header.mapBuild,this->mapBuild,32);
    memcpy(header.mapName,this->mapName,32);
    header.mapGame = this->mapGame;
    header.mapType = this->mapType;
    header.tagDataOffset = this->tagDataOffset;
    header.tagDataSize = this->tagDataSize;
    
    return header;
}

HaloTagID::HaloTagID() {}
HaloTagID::HaloTagID(uint16_t tagIndex) {
    this->tag_index = tagIndex;
    if(tagIndex == HALO_TAGID_NULL)
        this->table_id = tagIndex;
    else
        this->table_id = tagIndex + 0xE174;
}