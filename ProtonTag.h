//
//  ProtonTag.h
//  Proton
//
//  Created by Paul Whitcomb on 2/11/15.
//  Copyright (c) 2015 Paul Whitcomb. All rights reserved.
//


#ifndef __Proton__ProtonTag__
#define __Proton__ProtonTag__

#define NO_RESOURCE_INDEX 0xFFFFFFFF

#include <cstdlib>
#include <vector>
#include <memory>
#include <string.h>

#include "ProtonTagDependency.h"


class ProtonTagArray;

class ProtonTag {
public:
    ProtonTag(const char *name, const char tag_classes[12], uint32_t resource_index);
    ProtonTag(const char *name, const char tag_classes[12], const char *tag_data, uint32_t tag_data_length, uint32_t tag_magic, const char *resources_data, uint32_t resources_data_length, ProtonTagArray *parent_tag_array);
    
    
    char tag_classes[12] = {};
    uint32_t tag_magic; // This is the base address of what it would be when open in Halo.
    std::vector<std::unique_ptr<ProtonTagDependency>> dependencies;
    uint32_t resource_index = NO_RESOURCE_INDEX;
    void ScanDependencies(ProtonTagArray *parent_tag_array);
    
    void OffsetData(uint32_t offset, uint32_t size);
    void AppendData(uint32_t offset, uint32_t size);
    void DeleteData(uint32_t offset, uint32_t size);
    void InsertData(uint32_t offset, const char *data, uint32_t size);
    
    void SetName(const char *name);
    const char *Name() const;
    
    char *Data();
    uint32_t DataLength();
    void SetData(const char *data, uint32_t length);
    
    void AlignDataToAddress(uint32_t new_address);
    
    char *ResourcesData();
    uint32_t ResourcesDataLength();
    void SetResourcesData(const char *data, uint32_t length);
    
    ProtonTag();
    ProtonTag(const ProtonTag& tag);
    ProtonTag& operator=(const ProtonTag& tag);
    
    
private:
    char name[256] = {};
    std::unique_ptr<char[]> tag_data;
    uint32_t tag_data_length = 0;
    std::unique_ptr<char[]> resources_data;
    uint32_t resources_data_length = 0;
};

#endif /* defined(__Proton__ProtonTag__) */
