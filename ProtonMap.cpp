//
//  ProtonMap.cpp
//  Proton
//
//  Created by Paul Whitcomb on 2/20/15.
//  Copyright (c) 2015 Paul Whitcomb. All rights reserved.
//

#include <iostream>
#include "ProtonMap.h"
#include "BitmSndMap.h"

ProtonMap::ProtonMap(const void *cache_file) {
    const char *cache_file_c = (const char *)(cache_file);
    HaloCacheFileHeader header = *(HaloCacheFileHeader *)(cache_file_c);
    this->meta_address = 0x40440000;
    if(memcmp(header.head,"daeh",4) != 0) {
        HaloCacheFileHeaderDemo demoHeader = *(HaloCacheFileHeaderDemo *)(cache_file_c);
        if(memcmp(demoHeader.head,"dehE",4) == 0) {
            this->meta_address = 0x4BF10000;
            header = demoHeader.asStandardHeader();
        }
    }
    this->SetName(header.mapName);
    this->SetBuild(header.mapBuild);
    this->type = header.mapType;
    this->game = header.mapGame;
    
    const char *tagData = cache_file_c + header.tagDataOffset;
    const char *baseData = tagData - this->meta_address;
    
    HaloCacheFileTagDataHeader *index = (HaloCacheFileTagDataHeader *)(tagData);
    HaloCacheFileTagArrayIndex *tagArray = (HaloCacheFileTagArrayIndex *)(baseData + index->tagArrayAddress);
    this->principal_tag = index->principalScenarioTag.tag_index;
    
    const char *vertex_offset = cache_file_c + index->modelDataOffset;
    const char *index_offset = cache_file_c + index->modelDataOffset + index->vertexSize;
    
    for(uint32_t t=0;t<index->tagCount;t++) {
        std::unique_ptr<ProtonTag> tag;
        char tagClasses[12];
        memcpy(tagClasses,tagArray + t,12);
        const char *tagNamePtr = baseData + tagArray[t].nameAddress;
        char tagName[256] = {};
        if(tagNamePtr < tagData || tagNamePtr >= (tagData + header.tagDataSize)) {
            sprintf(tagName,"Unknown Tag (%u)",t);
        }
        else {
            strncpy(tagName, tagNamePtr, 255);
        }
        const char *tagData = baseData + tagArray[t].dataAddress;
        uint32_t tagDataLength = 0;
        
        if(tagArray[t].notInMap == 0) {
            tagDataLength = header.tagDataSize - (tagArray[t].dataAddress - this->meta_address);
            for(uint32_t i=0;i<index->tagCount;i++) {
                if(tagArray[i].notInMap != 0 || tagArray[i].dataAddress == tagArray[t].dataAddress) continue;
                uint32_t difference = tagArray[i].dataAddress - tagArray[t].dataAddress;
                if(tagDataLength > difference) {
                    tagDataLength = difference;
                }
            }
        }
        
        if(tagArray[t].notInMap == 1) {
            tag = std::unique_ptr<ProtonTag>(new ProtonTag(tagName,tagClasses,tagArray[t].dataAddress));
        }
        else if(memcmp(tagArray[t].tagClassA,"psbs",4) == 0) {
            if(index->principalScenarioTag.tag_index == NULLED_TAG_ID) {
                // No scenario tag - SBSP tag is invalid.
                tag = std::unique_ptr<ProtonTag>(new ProtonTag(tagName,tagClasses,NULL,0,0,NULL,0,NULL));
            }
            else {
                const char *scenarioData = baseData + tagArray[index->principalScenarioTag.tag_index].dataAddress;
                HaloTagReflexive *sbsps = (HaloTagReflexive *)(scenarioData + 0x5A4);
                HaloScnrBSPIndex *sbspIndices = (HaloScnrBSPIndex *)(baseData + sbsps->address);
                for(uint32_t b=0;b<sbsps->count;b++) {
                    if(sbspIndices[b].sbsp.tag_id.tag_index != t) continue;
                    tag = std::unique_ptr<ProtonTag>(new ProtonTag(tagName,tagClasses,(const char *)(cache_file_c + sbspIndices[b].sbspFileOffset),sbspIndices[b].sbspSize,sbspIndices[b].sbspAddress,NULL,0,NULL));
                    break;
                }
            }
        }
        else if(memcmp(tagArray[t].tagClassA,"2dom",4) == 0) {
            std::unique_ptr<char> newTagData(new char[tagDataLength]);
            memcpy(newTagData.get(),baseData + tagArray[t].dataAddress,tagDataLength);
            HaloMapMod2 *mod2 = (HaloMapMod2 *)newTagData.get();
            HaloMapMod2Geo *geos = (HaloMapMod2Geo *)(newTagData.get() + mod2->geos.address - tagArray[t].dataAddress);
            
            uint32_t resource_length = 0;
            for(uint32_t g=0;g<mod2->geos.count;g++) {
                HaloMapMod2GeoPart *parts = (HaloMapMod2GeoPart *)(newTagData.get() + geos[g].parts.address - tagArray[t].dataAddress);
                for(uint32_t p=0;p<geos[g].parts.count;p++) {
                    resource_length += parts[p].vertex.count * 0x44;
                    resource_length += parts[p].index.count * 2 + 4;
                }
            }
            
            std::unique_ptr<char> resourceData(new char[resource_length]);
            
            uint32_t offset = 0;
            for(uint32_t g=0;g<mod2->geos.count;g++) {
                HaloMapMod2GeoPart *parts = (HaloMapMod2GeoPart *)(newTagData.get() + geos[g].parts.address - tagArray[t].dataAddress);
                for(uint32_t p=0;p<geos[g].parts.count;p++) {
                    const char *vertices = vertex_offset + parts[p].vertex.vertex_offset;
                    memcpy(resourceData.get() + offset, vertices, parts[p].vertex.count * 0x44);
                    parts[p].vertex.vertex_offset = offset;
                    offset += parts[p].vertex.count * 0x44;
                    
                    const char *indices = index_offset + parts[p].index.index_offsetA;
                    memcpy(resourceData.get() + offset, indices, parts[p].index.count * 2 + 4);
                    parts[p].index.index_offsetA = offset;
                    parts[p].index.index_offsetB = offset;
                    offset += parts[p].index.count * 2 + 4;
                }
            }
            
            tag = std::unique_ptr<ProtonTag>(new ProtonTag(tagName,tagClasses,newTagData.get(),tagDataLength,tagArray[t].dataAddress,resourceData.get(),resource_length,NULL));
            
        }
        else if(memcmp(tagArray[t].tagClassA,"mtib",4) == 0) {
            std::unique_ptr<char> newTagData(new char[tagDataLength]);
            memcpy(newTagData.get(),baseData + tagArray[t].dataAddress,tagDataLength);
            
            HaloMapBitmTag *bitm = (HaloMapBitmTag *)(newTagData.get());
            HaloMapBitmBitmaps *bitmaps = (HaloMapBitmBitmaps *)(newTagData.get() + bitm->bitmaps.address - tagArray[t].dataAddress);
            
            uint32_t resource_length = 0;
            for(uint32_t b=0;b<bitm->bitmaps.count;b++) {
                if(bitmaps[b].not_internalized) continue;
                resource_length += bitmaps[b].length;
            }
            
            std::unique_ptr<char> newResourceData(new char[resource_length]);
            uint32_t resource_offset = 0;
            
            for(uint32_t b=0;b<bitm->bitmaps.count;b++) {
                if(bitmaps[b].not_internalized) continue;
                memcpy(newResourceData.get() + resource_offset, cache_file_c + bitmaps[b].data_offset, bitmaps[b].length);
                bitmaps[b].data_offset = resource_offset;
                resource_offset += bitmaps[b].length;
            }
            
            tag = std::unique_ptr<ProtonTag>(new ProtonTag(tagName,tagClasses,newTagData.get(),tagDataLength,tagArray[t].dataAddress,newResourceData.get(),resource_length,NULL));
            
        }
        else if(memcmp(tagArray[t].tagClassA,"!dns",4) == 0) {
            std::unique_ptr<char> newTagData(new char[tagDataLength]);
            memcpy(newTagData.get(),baseData + tagArray[t].dataAddress,tagDataLength);
            
            HaloMapSndTag *snd = (HaloMapSndTag *)(newTagData.get());
            HaloMapSndTagRange *ranges = (HaloMapSndTagRange *)(newTagData.get() + snd->ranges.address - tagArray[t].dataAddress);
            
            uint32_t resource_length = 0;
            for(uint32_t s=0;s<snd->ranges.count;s++) {
                HaloMapSndTagPermutation *perms = (HaloMapSndTagPermutation *)(newTagData.get() + ranges[s].permutations.address - tagArray[t].dataAddress);
                for(uint32_t p=0;p<ranges[s].permutations.count;p++) {
                    if(perms[p].not_internalized) continue;
                    resource_length += perms[p].length;
                }
            }
            
            std::unique_ptr<char> newResourceData(new char[resource_length]);
            uint32_t resource_offset = 0;
            
            for(uint32_t s=0;s<snd->ranges.count;s++) {
                HaloMapSndTagPermutation *perms = (HaloMapSndTagPermutation *)(newTagData.get() + ranges[s].permutations.address - tagArray[t].dataAddress);
                for(uint32_t p=0;p<ranges[s].permutations.count;p++) {
                    if(perms[p].not_internalized) continue;
                    memcpy(newResourceData.get() + resource_offset, cache_file_c + perms[p].data_offset, perms[p].length);
                    perms[p].data_offset = resource_offset;
                    resource_offset += perms[p].length;
                }
            }
            
            tag = std::unique_ptr<ProtonTag>(new ProtonTag(tagName,tagClasses,newTagData.get(),tagDataLength,tagArray[t].dataAddress,newResourceData.get(),resource_length,NULL));
        }
        else {
            tag = std::unique_ptr<ProtonTag>(new ProtonTag(tagName,tagClasses,tagData,tagDataLength,tagArray[t].dataAddress,NULL,0,NULL));
        }
        this->tags.push_back(std::move(tag));
    }
    
    for(uint32_t t=0;t<index->tagCount;t++) {
        this->tags.at(t).get()->ScanDependencies(this);
    }
}

ProtonCacheFile ProtonMap::ToCacheFile() const {
    ProtonMap map = *this;
    
    // Do the header. Just do it man.
    HaloCacheFileHeader header;
    header.mapGame = map.game;
    header.mapType = map.type;
    strcpy(header.mapName, map.Name());
    strcpy(header.mapBuild, map.Build());
    
    
    // Initialize tag data header.
    HaloCacheFileTagDataHeader metaHeader;
    metaHeader.tagCount = (uint32_t)map.tags.size();
    HaloTagID scenarioID(map.principal_tag);
    metaHeader.principalScenarioTag = scenarioID;
    
    uint32_t tagDataLength = 0;
    uint32_t tagNameLength = 0;
    
    uint32_t vertexLength = 0;
    uint32_t vertexCount = 0;
    uint32_t indexLength = 0;
    uint32_t indexCount = 0;
    uint32_t rawDataLength = 0;
    
    uint32_t sbspLength = 0;
    
    
    
    // Count resource data length and write info for SBSP data.
    for (std::vector<int>::size_type i=0;i<map.tags.size();i++) {
        ProtonTag *tag = map.tags.at(i).get();
        if(tag->resource_index == NO_RESOURCE_INDEX && memcmp(tag->tag_classes,"psbs",4) != 0) {
            tagDataLength += tag->DataLength();
        }
        if(memcmp(tag->tag_classes, "psbs", 4) == 0) {
            if(map.principal_tag == NULLED_TAG_ID) continue; // No scenario tag. Cannot save sbsp tag.
            const char *scenarioData = map.tags.at(map.principal_tag).get()->Data();
            uint32_t scenarioMagic = map.tags.at(map.principal_tag).get()->tag_magic;
            const HaloTagReflexive *sbsps = (HaloTagReflexive *)(scenarioData + 0x5A4);
            const HaloScnrBSPIndex *sbspIndices = (HaloScnrBSPIndex *)(scenarioData + sbsps->address - scenarioMagic);
            for(uint32_t b=0;b<sbsps->count;b++) {
                if(sbspIndices[b].sbsp.tag_id.tag_index != i) continue;
                sbspLength += tag->DataLength();
            }
        }
        if(memcmp(tag->tag_classes,"2dom",4) == 0) {
            char *tagData = tag->Data();
            HaloMapMod2 *mod2 = (HaloMapMod2 *)tag->Data();
            HaloMapMod2Geo *geos = (HaloMapMod2Geo *)(tagData + mod2->geos.address - tag->tag_magic);
            
            for(uint32_t g=0;g<mod2->geos.count;g++) {
                HaloMapMod2GeoPart *parts = (HaloMapMod2GeoPart *)(tagData + geos[g].parts.address - tag->tag_magic);
                for(uint32_t p=0;p<geos[g].parts.count;p++) {
                    vertexLength += parts[p].vertex.count * 0x44;
                    vertexCount += parts[p].vertex.count;
                    
                    indexLength += parts[p].index.count * 2 + 4;
                    indexCount += parts[p].index.count;
                }
            }
        }
        if(memcmp(tag->tag_classes,"!dns",4) == 0 || memcmp(tag->tag_classes,"mtib",4) == 0) rawDataLength += tag->ResourcesDataLength();
        tagNameLength += strlen(tag->Name()) + 1;
    }
    if((rawDataLength % 4) > 0)
        rawDataLength += 4 - (rawDataLength % 4);
    if((indexLength % 4) > 0)
        indexLength += 4 - (indexLength % 4);
    if((tagNameLength % 4) > 0)
        tagNameLength += 4 - (tagNameLength % 4);
    if((sbspLength % 4) > 0)
        sbspLength += 4 - (sbspLength % 4);
    
    
    // Tag name and data allocation
    std::unique_ptr<char []> tagNamesPtr(new char[tagNameLength]);
    char *tagNames = tagNamesPtr.get();
    memset(tagNames,0,tagNameLength);
    
    std::unique_ptr<char []> rawDataPtr(new char[rawDataLength]);
    char *rawData = rawDataPtr.get();
    
    std::unique_ptr<char []> tagsDataPtr(new char[tagDataLength]);
    char *tagsData = tagsDataPtr.get();
    
    std::unique_ptr<char []> sbspDataPtr(new char[sbspLength]);
    char *sbspData = sbspDataPtr.get();
    
    
    
    // Tag array allocation
    metaHeader.tagArrayAddress = map.meta_address + sizeof(metaHeader);
    std::unique_ptr<HaloMapTagIndex []> tagArrayPtr(new HaloMapTagIndex [metaHeader.tagCount]);
    HaloMapTagIndex *tagArray = tagArrayPtr.get();
    
    uint32_t tagNameAddress = metaHeader.tagArrayAddress + metaHeader.tagCount * sizeof(HaloMapTagIndex);
    uint32_t tagNameOffset = 0;
    //Tag name copying
    for(uint32_t i=0;i<metaHeader.tagCount; i++) {
        ProtonTag *tag = map.tags.at(i).get();
        memcpy(tagArray[i].tag_class_a, tag->tag_classes + 0, 4);
        memcpy(tagArray[i].tag_class_b, tag->tag_classes + 4, 4);
        memcpy(tagArray[i].tag_class_c, tag->tag_classes + 8, 4);
        tagArray[i].tag_id = HaloTagID(i);
        
        tagArray[i].name_offset = tagNameAddress + tagNameOffset;
        strcpy(tagNames + tagNameOffset, tag->Name());
        tagNameOffset += strlen(tag->Name()) + 1;
    }
    
    // Tag dependency resolving
    for(uint32_t i=0;i<metaHeader.tagCount; i++) {
        ProtonTag *tag = map.tags.at(i).get();
        if(tag->resource_index != NO_RESOURCE_INDEX) continue;
        for(std::vector<int>::size_type d=0; d<tag->dependencies.size(); d++) {
            ProtonTagDependency *dependency = tag->dependencies.at(d).get();
            if(dependency->type == PROTON_TAG_DEPENDENCY_TAGID) {
                HaloTagID *tagID = (HaloTagID *)(tag->Data() + dependency->offset);
                *tagID = HaloTagID(dependency->tag);
            }
            else if(dependency->type == PROTON_TAG_DEPENDENCY_DEPENDENCY) {
                HaloTagDependency *dep = (HaloTagDependency *)(tag->Data() + dependency->offset);
                dep->tag_id = HaloTagID(dependency->tag);
                if(dependency->tag == NULLED_TAG_ID) {
                    dep->name_address = 0;
                }
                else {
                    dep->name_address = tagArray[dependency->tag].name_offset;
                    memcpy(dep->tag_class, tagArray[dependency->tag].tag_class_a, 4);
                }
            }
        }
    }
    
    // Model data and raw data
    uint32_t vertexOffset = 0;
    uint32_t indexOffset = 0;
    uint32_t rawDataOffset = 0;
    
    uint32_t modelSize = vertexLength + indexLength;
    if((modelSize % 4) > 0) {
        modelSize += 4 - (modelSize % 4);
    }
    std::unique_ptr<char []> modelDataPtr(new char[modelSize]);
    char *modelData = modelDataPtr.get();
    uint32_t modelCount = 0;
    
    for(uint32_t i=0;i<metaHeader.tagCount;i++) {
        ProtonTag *tag = map.tags.at(i).get();
        if(tag->resource_index != NO_RESOURCE_INDEX) continue;
        if(memcmp(tag->tag_classes,"2dom",4) == 0) {
            char *tagData = tag->Data();
            HaloMapMod2 *mod2 = (HaloMapMod2 *)tag->Data();
            HaloMapMod2Geo *geos = (HaloMapMod2Geo *)(tagData + mod2->geos.address - tag->tag_magic);
            
            for(uint32_t g=0;g<mod2->geos.count;g++) {
                HaloMapMod2GeoPart *parts = (HaloMapMod2GeoPart *)(tagData + geos[g].parts.address - tag->tag_magic);
                for(uint32_t p=0;p<geos[g].parts.count;p++) {
                    uint32_t vertex_size = parts[p].vertex.count * 0x44;
                    memcpy(modelData + vertexOffset, tag->ResourcesData() + parts[p].vertex.vertex_offset, vertex_size);
                    parts[p].vertex.vertex_offset = vertexOffset;
                    vertexOffset += vertex_size;
                    
                    uint32_t index_size = parts[p].index.count * 2 + 4;
                    memcpy(modelData + indexOffset + vertexLength, tag->ResourcesData() + parts[p].index.index_offsetA, index_size);
                    parts[p].index.index_offsetA = indexOffset;
                    parts[p].index.index_offsetB = indexOffset;
                    indexOffset += index_size;
                    
                    modelCount++;
                }
            }
        }
        else if(memcmp(tag->tag_classes,"mtib",4) == 0) {
            HaloMapBitmTag *bitm = (HaloMapBitmTag *)(tag->Data());
            HaloMapBitmBitmaps *bitmaps = (HaloMapBitmBitmaps *)(tag->Data() + bitm->bitmaps.address - tag->tag_magic);
            
            for(uint32_t b=0;b<bitm->bitmaps.count;b++) {
                if(bitmaps[b].not_internalized) continue;
                memcpy(rawData + rawDataOffset, tag->ResourcesData() + bitmaps[b].data_offset, bitmaps[b].length);
                bitmaps[b].data_offset = rawDataOffset + sbspLength + sizeof(HaloCacheFileHeader);
                rawDataOffset += bitmaps[b].length;
            }
        }
        else if(memcmp(tag->tag_classes,"!dns",4) == 0) {
            HaloMapSndTag *snd = (HaloMapSndTag *)(tag->Data());
            HaloMapSndTagRange *ranges = (HaloMapSndTagRange *)(tag->Data() + snd->ranges.address - tag->tag_magic);
            
            for(uint32_t s=0;s<snd->ranges.count;s++) {
                HaloMapSndTagPermutation *perms = (HaloMapSndTagPermutation *)(tag->Data() + ranges[s].permutations.address - tag->tag_magic);
                for(uint32_t p=0;p<ranges[s].permutations.count;p++) {
                    if(perms[p].not_internalized) continue;
                    memcpy(rawData + rawDataOffset, tag->ResourcesData() + perms[p].data_offset, perms[p].length);
                    perms[p].data_offset = rawDataOffset + sbspLength + sizeof(HaloCacheFileHeader);
                    rawDataOffset += perms[p].length;
                }
            }
        }
    }
    
    
    // SBSP data copying
    if(map.principal_tag != NULLED_TAG_ID) {
        uint32_t sbspOffset = 0;
        for(uint32_t i=0;i<metaHeader.tagCount;i++) {
            ProtonTag *tag = map.tags.at(i).get();
            if(memcmp(tag->tag_classes,"psbs",4) == 0) {
                char *scenarioData = map.tags.at(map.principal_tag).get()->Data();
                uint32_t scenarioMagic = map.tags.at(map.principal_tag).get()->tag_magic;
                HaloTagReflexive *sbsps = (HaloTagReflexive *)(scenarioData + 0x5A4);
                HaloScnrBSPIndex *sbspIndices = (HaloScnrBSPIndex *)(scenarioData + sbsps->address - scenarioMagic);
                for(uint32_t b=0;b<sbsps->count;b++) {
                    if(sbspIndices[b].sbsp.tag_id.tag_index != i) continue;
                    sbspIndices[b].sbspSize = (uint32_t)tag->DataLength();
                    memcpy(sbspData + sbspOffset, tag->Data(), tag->DataLength());
                    sbspIndices[b].sbspFileOffset = sizeof(HaloCacheFileHeader) + sbspOffset;
                    sbspOffset += sbspIndices[b].sbspSize;
                }
            }
        }
    }
    
    // Tag data copying
    uint32_t tagDataAddress = tagNameAddress + tagNameLength;
    uint32_t tagDataOffset = 0;
    for(uint32_t i=0;i<metaHeader.tagCount;i++) {
        ProtonTag *tag = map.tags.at(i).get();
        tagArray[i].not_in_map = tag->resource_index == NO_RESOURCE_INDEX ? 0 : 1;
        if(tagArray[i].not_in_map) {
            tagArray[i].data_offset = tag->resource_index;
        }
        else if(memcmp(tagArray[i].tag_class_a,"psbs",4) == 0) {
            tagArray[i].data_offset = 0;
        }
        else {
            tagArray[i].data_offset = tagDataAddress + tagDataOffset;
            tag->AlignDataToAddress(tagArray[i].data_offset);
            memcpy(tagsData + tagDataOffset, tag->Data(), tag->DataLength());
            tagDataOffset += tag->DataLength();
        }
    }
    uint32_t tagArrayLength = (uint32_t)(sizeof(HaloCacheFileTagArrayIndex) * map.tags.size());
    
    // Let's put it all together.
    uint32_t final_tag_data_length = sizeof(HaloCacheFileTagDataHeader) + tagNameLength + tagDataLength + tagArrayLength;
    if(final_tag_data_length % 4 > 0)
        final_tag_data_length += 4 - (final_tag_data_length % 4);
    
    uint32_t final_size = sizeof(HaloCacheFileHeader) + sbspLength + rawDataLength + modelSize + final_tag_data_length;
    
    std::unique_ptr<char []> finalDataPtr(new char[final_size]);
    char *finalData = finalDataPtr.get();
    header.tagDataOffset = final_size - final_tag_data_length;
    header.fileSize = final_size;
    header.tagDataSize = final_tag_data_length;
    
    // Halo Demo uses its own special header.
    uint32_t writeOffset = sizeof(HaloCacheFileHeader);
    if(map.game == HALO_MAP_GAME_DEMO) {
        HaloCacheFileHeaderDemo demoHeader = header.asDemoHeader();
        memcpy(finalData,&demoHeader,sizeof(HaloCacheFileHeaderDemo));
    }
    else {
        memcpy(finalData,&header,sizeof(HaloCacheFileHeader));
    }
    
    
    memcpy(finalData + writeOffset,sbspData,sbspLength);
    writeOffset += sbspLength;
    
    memcpy(finalData + writeOffset,rawData,rawDataLength);
    writeOffset += rawDataLength;
    
    metaHeader.modelDataOffset = writeOffset;
    metaHeader.modelSize = modelSize;
    metaHeader.vertexSize = vertexLength;
    metaHeader.partCountA = modelCount;
    metaHeader.partCountB = modelCount;
    
    memcpy(finalData + writeOffset,modelData,modelSize);
    writeOffset += modelSize;
    
    memcpy(finalData + writeOffset,&metaHeader,sizeof(HaloCacheFileTagDataHeader));
    writeOffset += sizeof(HaloCacheFileTagDataHeader);
    
    memcpy(finalData + writeOffset,tagArray,tagArrayLength);
    writeOffset += tagArrayLength;
    
    memcpy(finalData + writeOffset,tagNames,tagNameLength);
    writeOffset += tagNameLength;
    
    memcpy(finalData + writeOffset,tagsData,tagDataLength);
    writeOffset += tagDataLength;
    
    
    
    ProtonCacheFile cacheFile(finalData,final_size);
    return cacheFile;
}

ProtonMap::ProtonMap() {
    
}

ProtonMap& ProtonMap::operator=(const ProtonMap &map) {
    if(&map == this) return *this;
    memcpy(this->name,map.name,sizeof(ProtonMap::name));
    memcpy(this->build,map.build,sizeof(ProtonMap::build));
    this->type = map.type;
    this->game = map.game;
    this->principal_tag = map.principal_tag;
    this->meta_address = map.meta_address;
    
    for(std::vector<int>::size_type i=0;i<map.tags.size();i++) {
        std::unique_ptr<ProtonTag> tag(new ProtonTag);
        *(tag.get()) = *(map.tags.at(i).get());
        this->tags.push_back(std::move(tag));
    }
    
    return *this;
}


ProtonMap::ProtonMap(const ProtonMap& map) {
    this->operator=(map);
}

const char *ProtonMap::Name() const {
    return this->name;
}
void ProtonMap::SetName(const char *name) {
    strncpy(this->name,name,sizeof(this->name)-1);
}

const char *ProtonMap::Build() const {
    return this->build;
}
void ProtonMap::SetBuild(const char *build) {
    strncpy(this->build,build,sizeof(this->build)-1);
}