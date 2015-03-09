//
//  ProtonTagDependency.cpp
//  Proton
//
//  Created by Paul Whitcomb on 2/21/15.
//  Copyright (c) 2015 Paul Whitcomb. All rights reserved.
//

#include "ProtonTagDependency.h"

ProtonTagDependency::ProtonTagDependency(unsigned long offset, uint16_t tag, ProtonTagDependencyType type) {
    this->offset = offset;
    this->tag = tag;
    this->type = type;
}
ProtonTagDependency::ProtonTagDependency(unsigned long offset, HaloTagDependency dependency) {
    this->offset = offset;
    this->tag = dependency.tag_id.tag_index;
    this->type = PROTON_TAG_DEPENDENCY_DEPENDENCY;
}
ProtonTagDependency::ProtonTagDependency(unsigned long offset, HaloTagID tag_id) {
    this->offset = offset;
    this->tag = tag_id.tag_index;
    this->type = PROTON_TAG_DEPENDENCY_TAGID;
}


ProtonTagDependency::ProtonTagDependency() {}