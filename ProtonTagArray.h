//
//  ProtonTagArray.h
//  Proton
//
//  Created by Paul Whitcomb on 2/11/15.
//  Copyright (c) 2015 Paul Whitcomb. All rights reserved.
//


#ifndef __Proton__ProtonTagArray__
#define __Proton__ProtonTagArray__

#include <stdio.h>
#include <memory.h>
#include "ProtonTag.h"

class ProtonTagArray {
public:
    uint32_t ImportTag(uint32_t tag, ProtonTagArray *tag_array, bool allow_duplicates);
    void DeleteTag(unsigned int tag);
    std::vector<std::unique_ptr<ProtonTag>> tags;
    uint16_t principal_tag = NULLED_TAG_ID;
    
private:
    void InsertTag(ProtonTag tag);
    uint32_t RecursiveTagImport(uint32_t tag, ProtonTagArray *tag_array, bool allow_duplicates, uint32_t *tag_indices);
};

#endif /* defined(__Proton__ProtonTagArray__) */
