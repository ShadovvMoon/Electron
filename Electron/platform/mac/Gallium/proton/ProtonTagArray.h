//
//  ProtonTagArray.h
//  Proton
//
//  Created by Paul Whitcomb on 2/11/15.
//  Copyright (c) 2015 Paul Whitcomb. All rights reserved.
//


#ifndef __Proton__ProtonTagArray__
#define __Proton__ProtonTagArray__

#include <cstdlib>
#include <vector>
#include <memory>
#include "ProtonTag.h"

class ProtonTagArray {
public:
    uint32_t ImportTag(uint32_t tag, const ProtonTagArray &parent_tag_array, bool allow_duplicates);
    uint32_t DuplicateTag(uint32_t tag);
    void DeleteTag(uint32_t tag, bool recursive_deletion);
    std::vector<std::unique_ptr<ProtonTag>> tags;
    uint16_t principal_tag = NULLED_TAG_ID;
    ProtonTagArray();
    ProtonTagArray& operator=(const ProtonTagArray &tagarray);
    ProtonTagArray(const ProtonTagArray& tagarray);
    
private:
    uint32_t RecursiveTagImport(uint32_t tag, const ProtonTagArray &parent_tag_array, bool allow_duplicates, uint32_t *tag_indices);
};

#endif /* defined(__Proton__ProtonTagArray__) */
