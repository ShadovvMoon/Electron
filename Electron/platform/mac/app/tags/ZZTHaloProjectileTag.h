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
//  ZZTHaloProjectileTag.h
//  Apocalyptic
//
//  Created by Paul Whitcomb on 2/7/14.
//  Copyright (c) 2014 Zero2. All rights reserved.
//

#include "ZZTHaloObjectTag.h"

#ifndef _ZZTHaloProjectileTag_h
#define _ZZTHaloProjectileTag_h

enum projectileBitmaksTypes {
    PROJECTILE_BITMASK_ORIENTED_ALONG_VELOCITY = 15,
    PROJECTILE_BITMASK_AI_MUST_USE_BALLISTIC_AIMING = 14,
    PROJECTILE_BITMASK_DETONATION_MAX_TIME_IF_ATTACHED = 13,
    PROJECTILE_BITMASK_HAS_SUPER_COMBINING_EXPLOSION = 12,
    PROJECTILE_BITMASK_COMBINE_INITIAL_VELOCITY_WITH_PARENT_VELOCITY = 11,
    PROJECTILE_BITMASK_RANDOM_ATTACHED_DETONATION_TIME = 10,
    PROJECTILE_BITMASK_MINIMUM_ATTACHED_DETONATION_TIME = 9
};

enum detonationTimerStarts : uint16_t {
    PROJECTILE_DETONATION_TIME_STARTS_IMMEDIATELY = 0,
    PROJECTILE_DETONATION_TIME_STARTS_AFTER_FIRST_BOUNCE = 1,
    PROJECTILE_DETONATION_TIME_STARTS_WHEN_AT_REST = 2
};

enum projectileNoise : uint16_t {
    PROJECTILE_NOISE_SILENT = 0,
    PROJECTILE_NOISE_MEDIUM = 1,
    PROJECTILE_NOISE_LOUD = 2,
    PROJECTILE_NOISE_SHOUT = 3,
    PROJECTILE_NOISE_QUIET = 4
};

enum projectileFunction : uint16_t {
    PROJECTILE_FUNCTION_NONE = 0,
    PROJECTILE_FUNCTION_RANGE_REMAINING = 1,
    PROJECTILE_FUNCTION_TIME_REMAINING = 2,
    PROJECTILE_FUNCTION_TRACER = 3
};

enum projectileMaterialReponseFlags {
    PROJECTILE_MATERIAL_RESPONSE_BITMASK_CANNOT_BE_OVER_PENETRATED = 7
};
enum projectileMaterialReponsePotentialFlags {
    PROJECTILE_MATERIAL_RESPONSE_POTENTIAL_BITMASK_ONLY_AGAINST_UNITS = 7
};

enum projectileMaterialResponse : uint16_t {
    PROJECTILE_MATERIAL_RESPONSE_DISAPPEAR = 0,
    PROJECTILE_MATERIAL_RESPONSE_DETONATE = 1,
    PROJECTILE_MATERIAL_RESPONSE_REFLECT = 2,
    PROJECTILE_MATERIAL_RESPONSE_OVERPENETRATE = 3,
    PROJECTILE_MATERIAL_RESPONSE_ATTACH = 4
};

enum projectileMaterialResponseScaleEffectsBy : uint16_t {
    PROJECTILE_MATERIAL_RESPONSE_SCALE_EFFECTS_BY_DAMAGE = 0,
    PROJECTILE_MATERIAL_RESPONSE_SCALE_EFFECTS_BY_ANGLE = 1
};

typedef struct {
    bitmask8 flags; //0x0
    char padding[0x1]; //0x1
    enum projectileMaterialResponse defaultResult; //0x2
    TagDependency defaultResultEffect; //0x4
    char padding2[0x10]; //0x14
    enum projectileMaterialResponse potentialResponse; //0x24
    bitmask8 potentialResultFlags; //0x26
    char padding3[0x1]; //0x27
    float potentialResultSkipFraction; //0x28
    float potentialResultBetweenFrom; //0x2C
    float potentialResultBetweenTo; //0x30
    float potentialResultAndFrom; //0x34
    float potentialResultAndTo; //0x38
    TagDependency potentialResultEffect; //0x3C
    char padding4[0x10]; //0x4C
    enum projectileMaterialResponseScaleEffectsBy scaleEffectsBy; //0x5C
    char padding5[0x2]; //0x5E
    float angularNoise; //0x60
    float velocityNoise; //0x64
    TagDependency detonationEffect; //0x68
    char padding6[0x18]; //0x78
    float penetrationInitialFriction; //0x90
    float penetrationMaximumDistance; //0x94
    float penetrationParallelFriction; //0x98
    float penetrationPerpendicularFriction; //0x9C
}  __attribute__((packed)) HaloProjectileMaterialResponses;



typedef struct {
    HaloObjectTagData objectData; //0x0
    bitmask16 projectileFlags; //0x17C
    char padding[0x2]; //0x17E
    enum detonationTimerStarts detonationTimerStarts; //0x180
    enum projectileNoise impactNoise; //0x182
    enum projectileFunction functionAin; //0x184
    enum projectileFunction functionBin; //0x186
    enum projectileFunction functionCin; //0x188
    enum projectileFunction functionDin; //0x18A
    TagDependency superDetonation; //0x18C
    float AIPerceptionRadius; //0x19C
    float collisionRadius; //0x1A0
    float armingTime; //0x1A4
    float dangerRadius; //0x1A8
    TagDependency detonation; //0x1AC
    float detonationFrom; //0x1BC
    float detonationTo; //0x1C0
    float detonationMinimumVelocity; //0x1C4
    float detonationMaximumRange; //0x1C8
    float physicsAirGravityScale; //0x1CC
    float physicsAirDamageRangeFrom; //0x1D0
    float physicsAirDamageRangeTo; //0x1D4
    float physicsWaterGravityScale; //0x1D8
    float physicsWaterDamageFrom; //0x1DC
    float physicsWaterDamageTo; //0x1E0
    float physicsInitialVelocity; //0x1E4
    float physicsFinalVelocity; //0x1E8
    float physicsGuidedAngularVelocity; //0x1EC
    enum projectileNoise physicsDetonationNoise; //0x1F0
    char padding1[0x2]; //0x1F2
    TagDependency detonationStartedEffect; //0x1F4
    TagDependency flybySound; //0x204
    TagDependency attachedDetonationDamage; //0x214
    TagDependency impactDamage; //0x224
    char padding2[0xC]; //0x234
    Reflexive materialResponses; //0x240
} __attribute__((packed)) HaloProjectileTag; //0x24C - complete

#endif
