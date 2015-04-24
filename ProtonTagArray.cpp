//
//  ProtonTagArray.cpp
//  Proton
//
//  Created by Paul Whitcomb on 2/11/15.
//  Copyright (c) 2015 Paul Whitcomb. All rights reserved.
//

#include "ProtonTagArray.h"
#include <memory>

// Recursive importation...
uint32_t ProtonTagArray::ImportTag(uint32_t tag, const ProtonTagArray &parent_tag_array, bool allow_duplicates) {
    auto tag_indices = std::make_unique<uint32_t[]>(parent_tag_array.tags.size());
    for(std::vector<int>::size_type i=0;i<parent_tag_array.tags.size();i++) {
        tag_indices[i] = 0xFFFFFFFF;
    }
    return this->RecursiveTagImport(tag, parent_tag_array, allow_duplicates, tag_indices.get());
}

// Imports tag from tag_array. Returns the tag ID of the newly imported tag. If allow_duplicates is false and it finds a tag with the same name, it returns that tag instead and does not import the tag.
uint32_t ProtonTagArray::RecursiveTagImport(uint32_t tag, const ProtonTagArray &parent_tag_array, bool allow_duplicates, uint32_t *tag_indices) {
    if(tag_indices[tag] != 0xFFFFFFFF) return tag_indices[tag];
    const ProtonTag *original_tag = parent_tag_array.tags.at(tag).get();
    uint32_t duplicates = 0;
    for(std::vector<int>::size_type i=0;i<this->tags.size();i++) {
        ProtonTag *against = this->tags[i].get();
        if(memcmp(against->tag_classes,original_tag->tag_classes,4 /* todo: 4 and not 12? */) == 0 && strcmp(original_tag->Name(), against->Name()) == 0) {
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
        ProtonTagDependency *dependency = importedTag->dependencies[i].get();
        uint32_t newTag = this->RecursiveTagImport(dependency->tag, parent_tag_array, allow_duplicates,tag_indices);
        importedTag->dependencies[i].get()->tag = newTag;
    }
    return tag_indices[tag];
}

// Duplicates a single tag. The tag dependencies and tag data are copied as well. This function also returns the tag index of the new tag.
uint32_t ProtonTagArray::DuplicateTag(uint32_t tag) {
    std::unique_ptr<ProtonTag> duplicateTagPtr(new ProtonTag());
    *duplicateTagPtr.get() = *this->tags.at(tag).get();
    this->tags.push_back(std::move(duplicateTagPtr));
    return (uint32_t)this->tags.size() - 1;
}

// Deletes the tag. If recursive_delete is true, then any tags that this tag references will also be deleted unless they are referenced by additional tags. Know that since dependencies aren't mapped out, deleting stock tags might break the map.
void ProtonTagArray::DeleteTag(uint32_t tag, bool recursive_deletion) {
    if(tag >= this->tags.size()) return;
    
    // Decrease all of the tags after it by 1 and also remove all references to this tag.
    for(std::vector<int>::size_type i=0;i<this->tags.size();i++) {
        ProtonTag *tag_dep = this->tags[i].get();
        for(std::vector<int>::size_type d=0;d<tag_dep->dependencies.size();d++) {
            ProtonTagDependency *dependency = tag_dep->dependencies.at(d).get();
            
            if(dependency->tag == tag) {
                dependency->tag = NULLED_TAG_ID;
            }
            else if(dependency->tag > tag && dependency->tag != NULLED_TAG_ID) {
                dependency->tag --;
            }
            
            if(dependency->type == PROTON_TAG_DEPENDENCY_DEPENDENCY) {
                ((HaloTagDependency *)(tag_dep->Data() + dependency->offset))->tag_id = HaloTagID(dependency->tag);
            }
            else if(dependency->type == PROTON_TAG_DEPENDENCY_TAGID) {
                *((HaloTagID *)(tag_dep->Data() + dependency->offset)) = HaloTagID(dependency->tag);
            }
        }
        
    }
    ProtonTag copiedTag = *(this->tags.at(tag).get());
    
    this->tags.erase(this->tags.begin() + tag);
    
    if(this->principal_tag == tag) this->principal_tag = NULLED_TAG_ID;
    else if(this->principal_tag > tag) this->principal_tag --;
    
    
    if(!recursive_deletion) return;
    for(std::vector<int>::size_type e=0;e<copiedTag.dependencies.size();e++) {
        ProtonTagDependency *dependency = copiedTag.dependencies.at(e).get();
        bool recursive_remove = true;
        
        for(std::vector<int>::size_type i=0;i<this->tags.size();i++) {
            
            if(i==dependency->tag) continue;
            
            ProtonTag *tag_dep = this->tags[i].get();
            for(std::vector<int>::size_type d=0;d<tag_dep->dependencies.size();d++) {
                if(tag_dep->dependencies.at(d).get()->tag == dependency->tag)
                    recursive_remove = false;
            }
            if(!recursive_remove) break;
        }
        
        if(recursive_remove) {
            this->DeleteTag(dependency->tag, true);
        }
    }
}


ProtonTagArray& ProtonTagArray::operator=(const ProtonTagArray &tagarray) {
    if(&tagarray == this) return *this;
    this->principal_tag = tagarray.principal_tag;
    
    for(std::vector<int>::size_type i=0;i<tagarray.tags.size();i++) {
        std::unique_ptr<ProtonTag> tag(new ProtonTag);
        *(tag.get()) = *(tagarray.tags[i].get());
        this->tags.push_back(std::move(tag));
    }
    
    return *this;
}


ProtonTagArray::ProtonTagArray(const ProtonTagArray& tagarray) {
    *this = tagarray;
}

ProtonTagArray::ProtonTagArray() {}