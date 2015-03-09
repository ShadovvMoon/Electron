//
//  ProtonTag.cpp
//  Proton
//
//  Created by Paul Whitcomb on 2/11/15.
//  Copyright (c) 2015 Paul Whitcomb. All rights reserved.
//

#include <new>
#include <string>
#include <iostream>
#include "ProtonTag.h"
#include "HaloData.h"
#include "ProtonTagArray.h"


// This function clones the tag. Cloning may be illegal in some countries, so know the law before you use this function.
ProtonTag::ProtonTag(ProtonTag *original_tag, ProtonTagArray *parent_tag_array) {
    
    memcpy(this->tag_classes,original_tag->tag_classes,sizeof(this->tag_classes));
    
    if(original_tag->resources_data_length > 0) {
        this->resources_data_length = original_tag->resources_data_length;
        this->resources_data = std::unique_ptr<char []>(new char[this->resources_data_length]);
        memcpy(this->resources_data.get(),original_tag->resources_data.get(),this->resources_data_length);
    }
    
    this->resource_index = original_tag->resource_index;
    
    if(original_tag->tag_data_length > 0) {
        this->tag_magic = original_tag->tag_magic;
        this->tag_data_length = original_tag->tag_data_length;
        this->tag_data = std::unique_ptr<char []>(new char[this->tag_data_length]);
        memcpy(this->tag_data.get(),original_tag->tag_data.get(),this->tag_data_length);
        this->ScanDependencies(parent_tag_array);
    }
}


// This function initializes an indexed tag. Only Halo CE has these kinds of tags. The variable resource_index is the index for a tag in bitmaps.map, sounds.map, or loc.map, depending on class.
ProtonTag::ProtonTag(const char *name, const char tag_classes[12], uint32_t resource_index) {
    
    this->SetName(name);
    
    memcpy(this->tag_classes,tag_classes,sizeof(this->tag_classes));
    
    this->resource_index = resource_index;
}


// This function creates a tag from existing tag data. If you aren't going to specify resource data, make sure resources_data_length is 0.
ProtonTag::ProtonTag(const char *name, const char tag_classes[12], const char *tag_data, uint32_t tag_data_length, uint32_t tag_magic, const char *resources_data, uint32_t resources_data_length, ProtonTagArray *parent_tag_array) {
    
    this->SetName(name);
    
    memcpy(this->tag_classes,tag_classes,sizeof(this->tag_classes));
    this->resource_index = NO_RESOURCE_INDEX;
    
    if(resources_data_length > 0) {
        this->resources_data_length = resources_data_length;
        this->resources_data = std::unique_ptr<char []>(new char[this->resources_data_length]);
        memcpy(this->resources_data.get(),resources_data,this->resources_data_length);
    }
    
    if(tag_data_length > 0) {
        this->tag_magic = tag_magic;
        this->tag_data_length = tag_data_length;
        this->tag_data = std::unique_ptr<char []>(new char[this->tag_data_length]);
        memcpy(this->tag_data.get(),tag_data,this->tag_data_length);
        this->ScanDependencies(parent_tag_array);
    }
}

// Setters and getters for the tag name. The maximum length for the name is 255 characters (then a null character).
void ProtonTag::SetName(const char *name) {
    strncpy(this->name, name, 255);
}
const char *ProtonTag::Name() const {
    return this->name;
}


// I'd hate to write this a zillion times for the scenario tag.
static inline void addDependencyFromReflexive(uint32_t reflexive_offset,uint32_t dependency_offset,uint32_t size_of_chunk, ProtonTag *tag) {
    HaloTagReflexive *reflexive = (HaloTagReflexive *)(tag->Data() + reflexive_offset);
    for(uint32_t i=0;i<reflexive->count;i++) {
        uint32_t offset = (uint32_t)(reflexive->address - tag->tag_magic + i * size_of_chunk + dependency_offset);
        HaloTagDependency *dependency = (HaloTagDependency *)(tag->Data() + offset);
        if(dependency->tag_id.tag_index == 0xFFFF) continue;
        std::unique_ptr<ProtonTagDependency> newDependency(new ProtonTagDependency(offset,*dependency));
        tag->dependencies.push_back(std::move(newDependency));
    }
}

// Same, man. I feel your pain.
static inline void addPredictedResources(uint32_t reflexive_offset, ProtonTag *tag) {
    HaloTagReflexive *reflexive = (HaloTagReflexive *)(tag->Data() + reflexive_offset);
    for(uint32_t i=0;i<reflexive->count;i++) {
        uint32_t offset = (uint32_t)(reflexive->address - tag->tag_magic + i * 8 + 4);
        HaloTagID *dependency = (HaloTagID *)(tag->Data() + offset);
        if(dependency->tag_index == 0xFFFF) continue;
        std::unique_ptr<ProtonTagDependency> newDependency(new ProtonTagDependency(offset,*dependency));
        tag->dependencies.push_back(std::move(newDependency));
    }
}

// Index the dependencies based on a parent tag array.
void ProtonTag::ScanDependencies(ProtonTagArray *parent_tag_array) {
    if(parent_tag_array == NULL) return;
    std::vector<int>::size_type array_size = parent_tag_array->tags.size();
    this->dependencies.clear();
    const char *data = this->Data();
    if(data == NULL || this->DataLength() < sizeof(HaloTagDependency))
        return;
    if(strncmp(this->tag_classes, "rncs", 4) == 0) {
        addDependencyFromReflexive(0x30, 0, 16, this);
        addDependencyFromReflexive(0x40, 0, 32, this);
        addPredictedResources(0xEC, this);
        addDependencyFromReflexive(0x21C, 0, 48, this);
        addDependencyFromReflexive(0x234, 0, 48, this);
        addDependencyFromReflexive(0x24C, 0, 48, this);
        addDependencyFromReflexive(0x264, 0, 48, this);
        addDependencyFromReflexive(0x27C, 0, 48, this);
        addDependencyFromReflexive(0x2A0, 0, 48, this);
        addDependencyFromReflexive(0x2B8, 0, 48, this);
        addDependencyFromReflexive(0x2D0, 0, 48, this);
        addDependencyFromReflexive(0x2E8, 0, 48, this);
        
        addDependencyFromReflexive(0x348, 0x28, 104, this);
        addDependencyFromReflexive(0x348, 0x3C, 104, this);
        
        addDependencyFromReflexive(0x378, 0x14, 148, this);
        addDependencyFromReflexive(0x384, 0x50, 144, this);
        
        addDependencyFromReflexive(0x390, 0x3C, 204, this);
        addDependencyFromReflexive(0x390, 0x4C, 204, this);
        addDependencyFromReflexive(0x390, 0x5C, 204, this);
        addDependencyFromReflexive(0x390, 0x6C, 204, this);
        addDependencyFromReflexive(0x390, 0x7C, 204, this);
        addDependencyFromReflexive(0x390, 0x8C, 204, this);
        
        addDependencyFromReflexive(0x3B4, 0, 16, this);
        addDependencyFromReflexive(0x3C0, 0, 48, this);
        addDependencyFromReflexive(0x420, 0, 16, this);
        addDependencyFromReflexive(0x444, 0x20, 60, this);
        
        HaloTagReflexive *aiConversations = (HaloTagReflexive *)(data + 0x468);
        for(uint32_t i=0;i<aiConversations->count;i++) {
            uint32_t offset = aiConversations->address - this->tag_magic + 0x5C + i * 116;
            addDependencyFromReflexive(offset, 0x1C, 124, this);
            addDependencyFromReflexive(offset, 0x2C, 124, this);
            addDependencyFromReflexive(offset, 0x3C, 124, this);
            addDependencyFromReflexive(offset, 0x4C, 124, this);
            addDependencyFromReflexive(offset, 0x5C, 124, this);
            addDependencyFromReflexive(offset, 0x6C, 124, this);
        }
        
        addDependencyFromReflexive(0x4B4, 0x18, 40, this);
        addDependencyFromReflexive(0x5A4, 0x10, 32, this);
    }
    else if(strncmp(this->tag_classes, "psbs", 4) == 0) {
        HaloTagDependency *lightmaps = (HaloTagDependency *)(data + 0x18);
        std::unique_ptr<ProtonTagDependency> lightmaps_ptr(new ProtonTagDependency(0x18, *lightmaps));
        this->dependencies.push_back(std::move(lightmaps_ptr));
        addDependencyFromReflexive(0xBC, 0, 20, this);
        
        HaloTagReflexive *lightmaps_reflexive = (HaloTagReflexive *)(data + 0x11C);
        for(uint32_t i=0;i<lightmaps_reflexive->count;i++) {
            uint32_t offset = lightmaps_reflexive->address - this->tag_magic + 0x14 + i * 32;
            addDependencyFromReflexive(offset, 0, 256, this);
        }
        addDependencyFromReflexive(0x134, 0, 16, this);
        
        HaloTagReflexive *clusters = (HaloTagReflexive *)(data + 0x14C);
        for(uint32_t i=0;i<clusters->count;i++) {
            uint32_t offset = clusters->address - this->tag_magic + i * 104;
            addDependencyFromReflexive(offset + 0x50, 0x24, 64, this);
        }
        
        addDependencyFromReflexive(0x1A8, 0x20, 136, this);
        addDependencyFromReflexive(0x1CC, 0x20, 240, this);
        addDependencyFromReflexive(0x1CC, 0x80, 240, this);
        addDependencyFromReflexive(0x214, 0x20, 116, this);
        addDependencyFromReflexive(0x220, 0x20, 80, this);
    }
    else if(strncmp(this->tag_classes, "mtib", 4) == 0) {
        HaloTagReflexive *bitmaps_ref = (HaloTagReflexive *)(data + 0x60);
        for(uint32_t i=0;i<bitmaps_ref->count;i++) {
            uint32_t data_offset = bitmaps_ref->address - this->tag_magic + i * 0x30 + 0x20;
            HaloTagID *bitmSelfReference = (HaloTagID *)(data + data_offset);
            std::unique_ptr<ProtonTagDependency> new_dependency(new ProtonTagDependency(data_offset, *bitmSelfReference));
            this->dependencies.push_back(std::move(new_dependency));
        }
    }
    else if(strncmp(this->tag_classes, "!dns", 4) == 0) {
        const HaloTagDependency *promo_sound = (HaloTagDependency *)(data + 0x70);
        if(promo_sound->tag_id.tag_index != 0xFFFF) {
            std::unique_ptr<ProtonTagDependency> new_dependency(new ProtonTagDependency(0x70, *promo_sound));
            this->dependencies.push_back(std::move(new_dependency));
        }
        const char *baseData = data - this->tag_magic;
        const HaloTagReflexive *ranges = (HaloTagReflexive *)(data + 0x98);
        for(uint32_t r=0;r<ranges->count;r++) {
            const char *range = baseData + ranges->address + r * 0x48;
            const HaloTagReflexive *permutations = (HaloTagReflexive *)(range + 0x3C);
            for(uint32_t p=0;p<permutations->count;p++) {
                const char *permutation = baseData + permutations->address + p * 0x7C;
                uint32_t offset = (uint32_t)(permutation - data);
                HaloTagID *tagID1 = (HaloTagID *)(permutation + 0x34);
                std::unique_ptr<ProtonTagDependency> new_dependency1(new ProtonTagDependency(offset + 0x34, *tagID1));
                HaloTagID *tagID2 = (HaloTagID *)(permutation + 0x3C);
                std::unique_ptr<ProtonTagDependency> new_dependency2(new ProtonTagDependency(offset + 0x3C, *tagID2));
                this->dependencies.push_back(std::move(new_dependency1));
                this->dependencies.push_back(std::move(new_dependency2));
            }
        }
    }
    else if(strncmp(this->tag_classes, "rtna", 4) == 0) {
        addDependencyFromReflexive(0x54, 0, 20, this);
    }
    else {
        const uint32_t iterate = 2;
        for(uint32_t i=0;i < this->DataLength() - sizeof(HaloTagDependency) + iterate;i += iterate) {
            const HaloTagDependency *tag_dependency = (const HaloTagDependency *)(data + i);
            uint16_t index = tag_dependency->tag_id.tag_index;
            uint16_t expected_id = index + 0xE174;
            if(tag_dependency->tag_id.tag_index < array_size && tag_dependency->reserved_data == 0 && tag_dependency->tag_id.table_id == expected_id) {
                std::unique_ptr<ProtonTagDependency> new_dependency(new ProtonTagDependency(i, *tag_dependency));
                this->dependencies.push_back(std::move(new_dependency));
            }
        }
    }
    for(uint32_t i=0;i<3;i++) {
        if(memcmp(this->tag_classes + 4 * i, "ejbo", 4) == 0) {
            addPredictedResources(0x170, this);
            break;
        }
    }
}


// Swapping and getting data has never been more... yeah.
void ProtonTag::SetData(const char *data, uint32_t length) {
    this->tag_data_length = length;
    if(this->tag_data_length > 0) {
        this->tag_data = std::unique_ptr<char []>(new char[this->tag_data_length]);
        memcpy(this->tag_data.get(), data, this->tag_data_length);
    }
}
char *ProtonTag::Data() {
    return this->tag_data.get();
}
uint32_t ProtonTag::DataLength() {
    return this->tag_data_length;
}

// This function is used to align data to a new base address value.
void ProtonTag::AlignDataToAddress(uint32_t new_address) {
    if(this->resource_index != NO_RESOURCE_INDEX || this->Data() == NULL || this->DataLength() < sizeof(HaloTagReflexive) || this->tag_magic == new_address) return;
    if(memcmp(this->tag_classes, "rocs", 4) == 0) {
        #warning TODO: Scenario tag's reflexives are not mapped out yet. Due to the complexity of this tag, using the brute-force algorithm might break the tag on some maps.
    }
    else if(memcmp(this->tag_classes,"psbs",4) == 0) {
        // SBSP tag must use mapped out reflexives, or else holes might occur in the map.
        std::cout << "ProtonTag::AlignDataToAddress WARNING: This function does not support sbsp tags yet.\n";
        return;
    }
    else {
        // Default brute force algorithm.
        uint32_t iterate = 2;
        uint32_t min_address = this->tag_magic;
        uint32_t max_address = this->tag_magic + (uint32_t)this->tag_data_length;
        uint32_t difference = new_address - this->tag_magic;
        for(uint32_t i=0;i<this->tag_data_length - sizeof(HaloTagReflexive) + iterate; i+= iterate) {
            HaloTagReflexive *reflexive = (HaloTagReflexive *)(this->Data() + i);
            if(reflexive->address < min_address || reflexive->address >= max_address) continue;
            if(reflexive->reserved_data != 0) continue;
            reflexive->address += difference;
        }
        this->tag_magic = new_address;
    }
}


// Data! Data! ALL OF THE DATA IS MIIIIINE! MWHAHAHA!
void ProtonTag::SetResourcesData(const char *data, uint32_t length) {
    this->resources_data_length = length;
    if(this->tag_data_length > 0) {
        this->resources_data = std::unique_ptr<char []>(new char[this->resources_data_length]);
        memcpy(this->resources_data.get(), data, this->resources_data_length);
    }
}
char *ProtonTag::ResourcesData() {
    return this->resources_data.get();
}
uint32_t ProtonTag::ResourcesDataLength() {
    return this->resources_data_length;
}


ProtonTag::ProtonTag() {}



ProtonTag& ProtonTag:: operator=(const ProtonTag& tag) {
    if(&tag == this) return *this;
    this->SetName(tag.Name());
    this->SetData(tag.tag_data.get(), tag.tag_data_length);
    this->SetResourcesData(tag.resources_data.get(), tag.resources_data_length);
    this->resource_index = tag.resource_index;
    memcpy(this->tag_classes,tag.tag_classes,sizeof(ProtonTag::tag_classes));
    this->tag_magic = tag.tag_magic;
    this->dependencies.clear();
    for(std::vector<int>::size_type i=0;i<tag.dependencies.size();i++) {
        const ProtonTagDependency *old_dependency = tag.dependencies.at(i).get();
        std::unique_ptr<ProtonTagDependency> newDependency(new ProtonTagDependency);
        *newDependency.get() = *old_dependency;
        this->dependencies.push_back(std::move(newDependency));
    }
    return *this;
}

ProtonTag::ProtonTag(const ProtonTag& tag) {
    this->operator=(tag);
}