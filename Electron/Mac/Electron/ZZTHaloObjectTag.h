/*
 
 Copyright (c) 2014, Paul Whitcomb
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * Neither the name of Paul Whitcomb nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 */
//
//  ZZTHaloObjectTag.h
//  Apocalyptic
//
//  Created by Paul Whitcomb on 2/7/14.
//  Copyright (c) 2014 Zero2. All rights reserved.
//

#include "ZZTHaloDataTypes.h"

#ifndef _ZZTHaloObjectTag_h
#define _ZZTHaloObjectTag_h

typedef struct {
    float red;
    float green;
    float blue;
} ColorRGB;

enum objectType : uint16_t {
    OBJECT_BIPD = 0x0,
    OBJECT_VEHI = 0x1,
    OBJECT_WEAP = 0x2,
    OBJECT_EQIP = 0x3,
    OBJECT_GARB = 0x4,
    OBJECT_PROJ = 0x5,
    OBJECT_SCEN = 0x6,
    OBJECT_MACH = 0x7,
    OBJECT_CTRL = 0x8,
    OBJECT_LIFI = 0x9,
    OBJECT_PLAC = 0xA,
    OBJECT_SSCE = 0xB,
};

enum haloFunctionType : uint16_t {
    FUNCTION_TYPE_ONE = 0,
    FUNCTION_TYPE_ZERO = 1,
    FUNCTION_TYPE_COSINE = 2,
    FUNCTION_TYPE_COSINE_VARIABLE = 3,
    FUNCTION_TYPE_DIAGONAL_WAVE = 4,
    FUNCTION_TYPE_DIAGONAL_WAVE_VARIABLE = 5,
    FUNCTION_TYPE_SLIDE = 6,
    FUNCTION_TYPE_SLIDE_VARIABLE = 7,
    FUNCTION_TYPE_NOISE = 8,
    FUNCTION_TYPE_JITTER = 9,
    FUNCTION_TYPE_WANDER = 10,
    FUNCTION_TYPE_SPARK = 11
};

enum haloFunctions : uint16_t {
    FUNCTION_NONE = 0,
    FUNCTION_A_IN = 1,
    FUNCTION_B_IN = 2,
    FUNCTION_C_IN = 3,
    FUNCTION_D_IN = 4,
    FUNCTION_A_OUT = 5,
    FUNCTION_B_OUT = 6,
    FUNCTION_C_OUT = 7,
    FUNCTION_D_OUT = 8
};

enum haloMapTo : uint16_t {
    MAP_TO_LINEAR = 0,
    MAP_TO_EARLY = 1,
    MAP_TO_VERY_EARLY = 2,
    MAP_TO_LATE = 3,
    MAP_TO_VERY_LATE = 4,
    MAP_TO_COSINE = 5
};

enum objectAttachmentsFunctionsScale : uint16_t {
    OBJECT_ATTACHMENT_SCALE_BY_NONE = 0,
    OBJECT_ATTACHMENT_SCALE_BY_A = 1,
    OBJECT_ATTACHMENT_SCALE_BY_B = 2,
    OBJECT_ATTACHMENT_SCALE_BY_C = 3,
    OBJECT_ATTACHMENT_SCALE_BY_D = 4
};

enum objectBitmaskTypes {
    OBJECT_BITMASK_DOES_NOT_CAST_SHADOW = 15,
    OBJECT_BITMASK_TRANSPARENT_SELF_OCCLUSION = 14,
    OBJECT_BITMASK_BRIGHTER_THAN_IT_SHOULD_BE = 13,
    OBJECT_BITMASK_NOT_A_PATHFINDING_OBSTICAL = 12
};

enum objectFunctionBitmaskTypes {
    OBJECT_FUNCTION_INVERT = 31,
    OBJECT_FUNCTION_ADDITIVE = 30,
    OBJECT_FUNCTION_ALWAYS_ACTIVE = 29
};

enum objectFunctionBoundsMode : uint16_t {
    OBJECT_FUNCTION_BOUNDS_MODE_CLIP = 0,
    OBJECT_FUNCTION_BOUNDS_MODE_CLIP_AND_NORMALIZE = 1,
    OBJECT_FUNCTION_BOUNDS_MODE_SCALE_TO_FIT = 2
};

enum objectFunction : uint16_t {
    OBJECT_FUNCTION_NONE = 0,
    OBJECT_FUNCTION_BODY_VITALITY = 1,
    OBJECT_FUNCTION_SHIELD_VITALITY = 2,
    OBJECT_FUNCTION_BODY_DAMAGE = 3,
    OBJECT_FUNCTION_SHIELD_DAMAGE = 4,
    OBJECT_FUNCTION_RANDOM_CONSTANT = 5,
    OBJECT_FUNCTION_UMBRELLA_SHIELD_VITALITY = 6,
    OBJECT_FUNCTION_SHIELD_STUN = 7,
    OBJECT_FUNCTION_RECENT_UMBRELLA_SHIELD_VITALITY = 8,
    OBJECT_FUNCTION_UMBRELLA_SHIELD_STUN = 9,
    OBJECT_FUNCTION_REGION_0_DAMAGE = 10,
    OBJECT_FUNCTION_REGION_1_DAMAGE = 11,
    OBJECT_FUNCTION_REGION_2_DAMAGE = 12,
    OBJECT_FUNCTION_REGION_3_DAMAGE = 13,
    OBJECT_FUNCTION_REGION_4_DAMAGE = 14,
    OBJECT_FUNCTION_REGION_5_DAMAGE = 15,
    OBJECT_FUNCTION_REGION_6_DAMAGE = 16,
    OBJECT_FUNCTION_REGION_7_DAMAGE = 17,
    OBJECT_FUNCTION_ALIVE = 18,
    OBJECT_FUNCTION_COMPASS = 19
};

typedef struct {
    bitmask32 flags; //0x0
    float period; //0x4
    enum haloFunctions scalePeriod; //0x8
    enum haloFunctionType function; //0xA
    enum haloFunctions scaleFunction; //0xC
    enum haloFunctionType wobbleFunction; //0xE
    float wobblePeriod; //0x10
    float wobbleMagnitude; //0x14
    float squareWaveThreshold; //0x18
    uint16_t stepCount; //0x1C
    enum haloMapTo mapTo; //0x1E
    uint16_t sawtoothCount; //0x20
    enum haloFunctions add; //0x22
    enum haloFunctions scaleResultBy; //0x24
    float boundsFrom; //0x28
    float boundsTo; //0x2C
    char padding[0x8]; //0x2E
    refIndex turnOffWith; //0x36
    float scaleBy; //0x38
    char padding1[0xFC]; //0x3C
    float functionAccelerationScale; //0x138
    char padding2[0xC]; //0x13C
    char usage[32]; //0x148
} __attribute__((packed)) HaloObjectFunctions;

typedef struct {
    TagDependency reference;
    char padding[0x10];
} __attribute__((packed)) HaloObjectWidgets; //0x20

typedef struct {
    float weight;
    ColorRGB colorLowerBound;
    ColorRGB colorUpperBound;
} __attribute__((packed)) HaloObjectChangeColorsPermutations;

typedef struct {
    enum haloFunctions darkenBy; //0x0
    enum haloFunctions scaleBy; //0x2
    bitmask32 scaleFlags; //0x4
    ColorRGB colorUpperBound; //0x8
    ColorRGB colorLowerBound; //0x14
    Reflexive permutations; //0x20
} __attribute__((packed)) HaloObjectChangeColors;

typedef struct {
    TagDependency type; //0x0
    char marker[32]; //0x10
    enum objectAttachmentsFunctionsScale primaryScale; //0x30
    enum objectAttachmentsFunctionsScale secondaryScale; //0x32
    enum objectAttachmentsFunctionsScale changeColor; //0x34
    char padding[0x12]; //0x36
} __attribute__((packed)) HaloObjectAttachments; //0x48

enum ObjectResourceType : uint16_t {
    RESOURCE_BITMAP = 0,
    RESOURCE_SOUND = 1
};

typedef struct {
    enum ObjectResourceType type; //0x0
    char padding[0x2];
    TagID name;
} __attribute__((packed)) HaloObjectResources;

typedef struct {
    enum objectType type; //0x0
    bitmask16 objectBitmask; //0x2
    float boundingRadius; //0x4
    Vector boundingOffset; //0x8
    Vector originOffset; //0x14
    float accelerationScale; //0x20
    uint32_t unknown; //0x24
    TagDependency model; //0x28
    TagDependency animation; //0x38
    char padding[0x28]; //0x48
    TagDependency collision; //0x70
    TagDependency physics; //0x80
    TagDependency shader; //0x90
    TagDependency effect; //0xA0
    char padding1[0x54]; //0xB0
    float renderBoundingRadius; //0x104
    enum objectFunction functionA_in; //0x108
    enum objectFunction functionB_in; //0x10A
    enum objectFunction functionC_in; //0x10C
    enum objectFunction functionD_in; //0x10E
    char padding2[0x2C]; //0x110
    uint16_t HUDtextMessageIndex; //0x13C
    uint16_t shaderPermutation; //0x13E
    Reflexive attachments; //0x140
    Reflexive widgets; //0x14C
    char padding3[0xC]; //0x158
    Reflexive changeColors; //0x164
    Reflexive resources;
} __attribute__((packed)) HaloObjectTagData; //0x17C


#endif
