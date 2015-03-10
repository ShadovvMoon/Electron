//
//  ProtonTagArray.cpp
//  Proton
//
//  Created by Paul Whitcomb on 2/11/15.
//  Copyright (c) 2015 Paul Whitcomb. All rights reserved.
//

#include "ProtonTagArray.h"


// Recursive importation...
uint32_t ProtonTagArray::ImportTag(uint32_t tag, ProtonTagArray *tag_array, bool allow_duplicates) {
    uint32_t tag_indices[tag_array->tags.size()];
    for(std::vector<int>::size_type i=0;i<tag_array->tags.size();i++) {
        tag_indices[i] = 0xFFFFFFFF;
    }
    return this->ProtonTagArray::RecursiveTagImport(tag, tag_array, allow_duplicates, tag_indices);
}

// Imports tag from tag_array. Returns the tag ID of the newly imported tag. If allow_duplicates is false and it finds a tag with the same name, it returns that tag instead and does not import the tag.
uint32_t ProtonTagArray::RecursiveTagImport(uint32_t tag, ProtonTagArray *tag_array, bool allow_duplicates, uint32_t *tag_indices) {
    if(tag_indices[tag] != 0xFFFFFFFF) return tag_indices[tag];
    ProtonTag *original_tag = tag_array->tags.at(tag).get();
    uint32_t duplicates = 0;
    for(std::vector<int>::size_type i=0;i<this->tags.size();i++) {
        ProtonTag *against = this->tags.at(i).get();
        if(memcmp(against->tag_classes,original_tag->tag_classes,4) == 0 && strcmp(original_tag->Name(), against->Name()) == 0) {
            if(allow_duplicates) duplicates++;
            else return (uint32_t)i;
        }
    }
    std::unique_ptr<ProtonTag> importedTagPtr(new ProtonTag());
    ProtonTag *importedTag = importedTagPtr.get();
    *importedTag = *original_tag;
    this->tags.push_back(std::move(importedTagPtr));
    tag_indices[tag] = (uint32_t)this->tags.size() - 1;
    if(duplicates > 0) {
        char newname[512] = {};
        sprintf(newname, "%s duplicate %u",importedTag->Name(),duplicates);
        importedTag->SetName(newname);
    }
    for(std::vector<int>::size_type i=0;i<original_tag->dependencies.size();i++) {
        ProtonTagDependency *dependency = importedTag->dependencies.at(i).get();
        uint32_t newTag = this->RecursiveTagImport(dependency->tag, tag_array, allow_duplicates,tag_indices);
        importedTag->dependencies.at(i).get()->tag = newTag;
    }
    return tag_indices[tag];
}

void ProtonTagArray::DeleteTag(unsigned int tag) {
    #warning TODO: Tag deletion isn't added/finished yet, neither is recursive deletion.
    
}


void ProtonTagArray::InsertTag(ProtonTag tag) {
    
}