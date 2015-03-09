//
//  ProtonTagDependency.h
//  Proton
//
//  Created by Paul Whitcomb on 2/21/15.
//  Copyright (c) 2015 Paul Whitcomb. All rights reserved.
//

#ifndef __Proton__ProtonTagDependency__
#define __Proton__ProtonTagDependency__

#include <cstdlib>
#include <string>
#include <stdio.h>
#include "HaloData.h"


typedef enum ProtonTagDependencyType {
    PROTON_TAG_DEPENDENCY_DEPENDENCY,
    PROTON_TAG_DEPENDENCY_TAGID
} ProtonTagDependencyType;

struct ProtonTagDependency {
public:
    ProtonTagDependency(unsigned long offset, uint16_t tag, ProtonTagDependencyType type);
    ProtonTagDependency(unsigned long offset, HaloTagDependency dependency);
    ProtonTagDependency(unsigned long offset, HaloTagID tag_id);
    unsigned long offset;
    uint16_t tag;
    ProtonTagDependencyType type;
    ProtonTagDependency();
} ;

#endif /* defined(__Proton__ProtonTagDependency__) */
