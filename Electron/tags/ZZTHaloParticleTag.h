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
//  ZZTHaloParticleTag.h
//  Apocalyptic
//
//  Created by Paul Whitcomb on 2/12/14.
//  Copyright (c) 2014 Zero2. All rights reserved.
//

#include "ZZTHaloDataTypes.h"

#ifndef _ZZTHaloParticleTag_h
#define _ZZTHaloParticleTag_h

enum particleBitmaskTypes {
    PARTICLE_BITMASK_CAN_ANIMATE_BACKWARDS = 31,
    PARTICLE_BITMASK_ANIMATION_STOPS_AT_REST = 30,
    PARTICLE_BITMASK_ANIMATION_STOPS_AT_RANDOM_FRAME = 29,
    PARTICLE_BITMASK_ANIMATE_ONCE_PER_FRAME = 28,
    PARTICLE_BITMASK_DIES_AT_REST = 27,
    PARTICLE_BITMASK_DIES_ON_CONTACT_WITH_STRUCTURE = 26,
    PARTICLE_BITMASK_TINT_FROM_DIFFUSE_TEXTURE = 25,
    PARTICLE_BITMASK_DIES_ON_CONTACT_WITH_WATER = 24,
    PARTICLE_BITMASK_DIES_ON_CONTACT_WITH_AIR = 23,
    PARTICLE_BITMASK_SELF_ILLUMINATED = 22,
    PARTICLE_BITMASK_RANDOM_HORIZONTAL_MIRRORING = 21,
    PARTICLE_BITMASK_RANDOM_VERTICLE_MIRRORING = 20
};

enum particleOrientation : uint16_t {
    PARTICLE_ORIENTATION_SCREEN_FACING = 0,
    PARTICLE_ORIENTATION_PARALLEL_TO_DIRECTION = 1,
    PARTICLE_ORIENTATION_PERPENDICULAR_TO_DIRECTION = 2
};

enum particleShaderFlagsTypes {
    PARTICLE_SHADER_FLAG_SORT_BIAS = 15,
    PARTICLE_SHADER_FLAG_NONLINEAR_TINT = 14,
    PARTICLE_SHADER_FLAG_DONT_OVERDRAW_FIRST_PERSON_WEAPON = 13
};

enum particleFramebufferBlendFunction : uint16_t {
    PARTICLE_FRAMEBUFFER_BLEND_ALPHA_BLEND = 0,
    PARTICLE_FRAMEBUFFER_BLEND_MULTIPLY = 1,
    PARTICLE_FRAMEBUFFER_BLEND_DOUBLE_MULTIPLY = 2,
    PARTICLE_FRAMEBUFFER_BLEND_ADD = 3,
    PARTICLE_FRAMEBUFFER_BLEND_SUBTRACT = 4,
    PARTICLE_FRAMEBUFFER_BLEND_COMPONENT_MINIMUM = 5,
    PARTICLE_FRAMEBUFFER_BLEND_COMPONENT_MAXIMUM = 6,
    PARTICLE_FRAMEBUFFER_BLEND_ALPHA_MULTIPLY_ADD = 7
};

enum particleFramebufferFadeMode : uint16_t {
    PARTICLE_FRAMEBUFFER_FADE_NONE = 0,
    PARTICLE_FRAMEBUFFER_FADE_FADE_WHEN_PERPENDICULAR = 1,
    PARTICLE_FRAMEBUFFER_FADE_FADE_WHEN_PARALLEL = 2
};

enum particleMapFlagsTypes {
    PARTICLE_MAP_FLAG_UNFILTERED = 7
};

enum particleSecondaryMapFlagsTypes {
    PARTICLE_SECONDARY_MAP_FLAG_UNFILTERED = 31
};

typedef struct {
    bitmask32 particleBitmask; //0x0
    TagDependency bitmap; //0x4
    TagDependency physics; //0x14
    TagDependency materialEffects; //0x24
    float unknown; //0x34
    float lifespanFrom; //0x38
    float lifespanTo; //0x3C
    float fadeInTime; //0x40
    float fadeOutTime; //0x44
    TagDependency collisionEffect; //0x48
    TagDependency deathEffect; //0x58
    float minimumSize; //0x68
    float unknown1; //0x6C
    float unknown2; //0x70
    float radiusAnimationFrom; //0x74
    float radiusAnimationTo; //0x78
    float unknown3; //0x7C
    float animationRateFrom; //0x80
    float animationRateTo; //0x84
    float contactDeteriorationRate; //0x88
    float fadeStartSize; //0x8C
    float fadeEndSize; //0x90
    char unknown4[0x4]; //0x94
    uint16_t firstSequenceIndex; //0x98
    uint16_t intitialSequenceCount; //0x9A
    uint16_t loopingSequenceCount; //0x9C
    uint16_t finalSequenceCount; //0x9E
    char unknown5[0x8]; //0xA0
    float unknown6; //0xA8
    enum particleOrientation orientation; //0xAC
    char unknown7[0x26]; //0xAE
    bitmask16 shaderFlags; //0xD8
    enum particleFramebufferBlendFunction framebufferBlendFunction; //0xDA
    enum particleFramebufferFadeMode framebufferFadeMode; //0xDC
    bitmask8 mapFlags; //0xDE
    char unknown8[0x1D]; //0xDF
    TagDependency secondaryMapBitmap; //0xFC
} __attribute__((packed)) HaloParticleData; //incomplete


#endif
