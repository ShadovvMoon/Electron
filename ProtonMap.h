//
//  ProtonMap.h
//  Proton
//
//  Created by Paul Whitcomb on 2/20/15.
//  Copyright (c) 2015 Paul Whitcomb. All rights reserved.
//

#ifndef __Proton__ProtonMap__
#define __Proton__ProtonMap__

#include <cstdlib>
#include <vector>
#include <iostream>
#include <memory>
#include <string.h>

#include "ProtonTagArray.h"
#include "ProtonCacheFile.h"
#include "HaloData.h"

class ProtonMap : public ProtonTagArray {
public:
    ProtonMap(const void *cache_file_data);
    ProtonCacheFile ToCacheFile() const;
    
    HaloMapGame game = HALO_MAP_GAME_CUSTOM_PROTON_TAG_ARRAY;
    HaloMapType type = HALO_MAP_TYPE_CUSTOM_PROTON_TAG_ARRAY;
    
    uint32_t meta_address = 0x40440000;
    
    const char *Name() const;
    void SetName(const char *name);
    
    const char *Build() const;
    void SetBuild(const char *build);
    
    
    ProtonMap();
    ProtonMap(const ProtonMap& map);
    ProtonMap& operator=(const ProtonMap& map);
private:
    char name[32] = {};
    char build[32] = {};
};

#endif /* defined(__Proton__ProtonMap__) */
