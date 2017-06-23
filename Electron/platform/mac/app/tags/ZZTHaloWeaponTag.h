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
//  ZZTHaloWeaponTag.h
//  Apocalyptic
//
//  Created by Paul Whitcomb on 2/4/14.
//  Copyright (c) 2014 Zero2. All rights reserved.
//

#include "ZZTHaloObjectTag.h"

#ifndef _ZZTHaloWeaponTag_h
#define _ZZTHaloWeaponTag_h


enum itemBitmaskTypes {
    ITEM_BITMASK_ALWAYS_MAINTAINS_Z_UP = 15,
    ITEM_BITMASK_DESTROYED_BY_EXPLOSIONS = 14,
    ITEM_BITMASK_UNAFFECTED_BY_GRAVITY = 13
};

enum weaponSecondaryTriggerModes : uint16_t {
    WEAPON_SECONDARY_TRIGGER_NORMAL = 0,
    WEAPON_SECONDARY_TRIGGER_SLAVED_TO_PRIMARY = 1,
    WEAPON_SECONDARY_TRIGGER_INHIBITS_PRIMARY = 2,
    WEAPON_SECONDARY_TRIGGER_LOADS_ALTERNATE_AMMUNITION = 3,
    WEAPON_SECONDARY_TRIGGER_LOADS_MULTIPLE_PRIMARY_AMMUNITION = 4
};

enum weaponBitmaskTypes {
    WEAPON_BITMASK_VERTICAL_HEAT_DISPLAY = 31,
    WEAPON_BITMASK_MUTUALLY_EXCLUSIVE_TRIGGERS = 30,
    WEAPON_BITMASK_ATTACKS_AUTOMATICALLY_ON_BUMP = 29,
    WEAPON_BITMASK_MUST_BE_READIED = 28,
    WEAPON_BITMASK_DOESNT_COUNT_TOWARDS_MAXIMUM = 27,
    WEAPON_BITMASK_AIM_ASSISTS_ONLY_WHEN_ZOOMED = 26,
    WEAPON_BITMASK_PREVENTS_GRENADE_THROWING = 25,
    WEAPON_BITMASK_MUST_BE_PICKED_UP = 24,
    WEAPON_BITMASK_HOLDS_TRIGGER_WHEN_DROPPED = 23,
    WEAPON_BITMASK_PREVENTS_MELEE_ATTACK = 22,
    WEAPON_BITMASK_DETONATES_WHEN_DROPPED = 21,
    WEAPON_BITMASK_CANNOT_FIRE_AT_MAXIMUM_AGE = 20,
    WEAPON_BITMASK_SECONDARY_TRIGGER_OVERRIDES_GRENADES = 19,
    WEAPON_BITMASK_DOES_NOT_DEPOWER_ACTIVE_CAMO_IN_MULTIPLAYER = 18,
    WEAPON_BITMASK_ENABLES_INTEGRATED_NIGHT_VISION = 17,
    WEAPON_BITMASK_AIS_USE_WEAPON_MELEE_DAMAGE = 16
};

enum weaponFunctions : uint16_t {
    WEAPON_FUNCTION_NONE = 0,
    WEAPON_FUNCTION_HEAT = 1,
    WEAPON_FUNCTION_PRIMARY_AMMUNITION = 2,
    WEAPON_FUNCTION_SECONDARY_AMMUNITION = 3,
    WEAPON_FUNCTION_PRIMARY_RATE_OF_FIRE = 4,
    WEAPON_FUNCTION_SECONDARY_RATE_OF_FIRE = 5,
    WEAPON_FUNCTION_READY = 6,
    WEAPON_FUNCTION_PRIMARY_EJECTION_PORT = 7,
    WEAPON_FUNCTION_SECONDARY_EJECTION_PORT = 8,
    WEAPON_FUNCTION_OVERHEATED = 9,
    WEAPON_FUNCTION_PRIMARY_CHARGED = 10,
    WEAPON_FUNCTION_SECONDARY_CHARGED = 11,
    WEAPON_FUNCTION_ILLUMINATION = 12,
    WEAPON_FUNCTION_AGE = 13,
    WEAPON_FUNCTION_INTEGRATED_LIGHT = 14,
    WEAPON_FUNCTION_PRIMARY_FIRING = 15,
    WEAPON_FUNCTION_SECONDARY_FIRING = 16,
    WEAPON_FUNCTION_PRIMARY_FIRING_ON = 17,
    WEAPON_FUNCTION_SECONDARY_FIRING_ON = 18
};

enum weaponMovementPenalized : uint16_t {
    WEAPON_MOVEMENT_PENALIZED_ALWAYS = 0,
    WEAPON_MOVEMENT_PENALIZED_WHEN_ZOOMED = 1,
    WEAPON_MOVEMENT_PENALIZED_WHEN_ZOOMED_OR_RELOADING = 2
};

enum weaponType : uint16_t {
    WEAPON_TYPE_UNDEFINED = 0,
    WEAPON_TYPE_SHOTGUN = 1,
    WEAPON_TYPE_NEEDLER = 2,
    WEAPON_TYPE_PLASMA_PISTOL = 3,
    WEAPON_TYPE_PLASMA_RIFLE = 4
};

enum magazineBitmaskTypes {
    WEAPON_MAGAZINE_WASTES_ROUND_WHEN_RELOADED = 31,
    WEAPON_MAGAZINE_EVERY_ROUND_MUST_BE_CHAMBERED = 30
};

typedef struct {
    uint16_t rounds; //0x0
    char padding[0xA]; //0x2
    TagDependency equipment; //0xC
} __attribute__((packed)) HaloWeaponMagazineMagazine; //0x1C

typedef struct {
    bitmask32 flags; //0x0
    uint16_t roundsRecharged; //0x4
    uint16_t roundsTotalInitial; //0x6
    uint16_t roundsTotalMaximum; //0x8
    uint16_t roundsLoadedMaximum; //0xA
    char padding[0x8]; //0xC
    float reloadTime; //0x14
    uint16_t roundsReloaded; //0x18
    char padding1[0x2]; //0x1A
    float chamberTime; //0x1C
    char padding2[0x18]; //0x20
    TagDependency reloadingEffect; //0x38
    TagDependency chamberingEffect; //0x48
    char padding3[0xC]; //0x58
    Reflexive magazines; //0x64
} __attribute__((packed)) HaloWeaponMagazine; //0x70

enum weaponTriggerBitmaskTypes {
    WEAPON_TRIGGER_TRACKS_FIRED_PROJECTILE = 31,
    WEAPON_TRIGGER_RANDOM_FIRING_EFFECTS = 30,
    WEAPON_TRIGGER_CAN_FIRE_WITH_PARTIAL_AMMO = 29,
    WEAPON_TRIGGER_DOES_NOT_REPEAT_AUTOMATICALLY = 28,
    WEAPON_TRIGGER_LOCKS_IN_ON_OFF_STATE = 27,
    WEAPON_TRIGGER_PROJECTILE_USE_WEAPON_ORIGIN = 26,
    WEAPON_TRIGGER_STICKS_WHEN_DROPPED = 25,
    WEAPON_TRIGGER_EJECTS_DURING_CHAMBER = 24,
    WEAPON_TRIGGER_DISCHARGING_SPEWS = 23,
    WEAPON_TRIGGER_ANALOG_RATE_OF_FIRE = 22,
    WEAPON_TRIGGER_USE_ERROR_WHEN_UNZOOMED = 21,
    WEAPON_TRIGGER_PROJECTILE_VECTOR_CANNOT_BE_ADJUSTED = 20,
    WEAPON_TRIGGER_PROJECTILE_HAVE_IDENTICAL_ERROR = 19,
    WEAPON_TRIGGER_PROJECTILE_IS_CLIENT_SIDE_ONLY = 18
};

enum weaponFiringNoise : uint16_t {
    WEAPON_FIRING_SILENT = 0,
    WEAPON_FIRING_MEDUM = 1,
    WEAPON_FIRING_LOUD = 2,
    WEAPON_FIRING_SHOUT = 3,
    WEAPON_FIRING_QUIET = 4
};

enum weaponFiringOverchargeAction : uint16_t {
    WEAPON_FIRING_OVERCHARGE_ACTION_NONE = 0,
    WEAPON_FIRING_OVERCHARGE_ACTION_EXPLODE = 1,
    WEAPON_FIRING_OVERCHARGE_ACTION_DISCHARGE = 2
};

enum weaponFiringDistributionFunction : uint16_t {
    WEAPON_FIRING_DISTRIBUTION_FUNCTION_POINT = 0,
    WEAPON_FIRING_DISTRIBUTION_FUNCTION_HORIZONTAL_FAN = 1
};

typedef struct {
    uint16_t shotCountLowerBound; //0x0
    uint16_t shotCountUpperBound; //0x2
    char padding[0x20]; //0x4
    TagDependency firingEffect; //0x24
    TagDependency misfireEffect; //0x34
    TagDependency emptyEffect; //0x44
    TagDependency firingdamage; //0x54
    TagDependency misfireDamage; //0x64
    TagDependency emptyDamage; //0x74
} __attribute__((packed)) HaloWeaponFiringEffects;

typedef struct {
    bitmask32 flags; //0x0
    float rateOfFireFrom; //0x4
    float rateOfFireTo; //0x8
    float firingAccelerationTime; //0xC
    float firingDecelerationTime; //0x10
    float firingBlurredRateOfFire; //0x14
    char padding[0x8]; //0x18
    refIndex magazine; //0x20
    uint16_t roundsPerShot; //0x22
    uint16_t minimumRounds; //0x24
    uint16_t roundsBetweenTracers; //0x26
    char padding1[0x6]; //0x28
    enum weaponFiringNoise noise; //0x2E
    float errorFrom; //0x30
    float errorTo; //0x34
    float errorAccelerationTime; //0x38
    float errorDecelerationTime; //0x3C
    char padding2[0x8];
    float chargingTime; //0x48
    float chargedTime; //0x4C
    enum weaponFiringOverchargeAction overchargeAction; //0x50
    char padding3[0x2]; //0x52
    float chargedIllumination; //0x54
    float overchargeSpewTime; //0x58
    TagDependency chargingEffect; //0x5C
    enum weaponFiringDistributionFunction projectileDistributionFunction; //0x6C
    uint16_t projectilesPerShot; //0x6E
    float projectileDistributionAngle; //0x70
    char padding4[0x4]; //0x74
    float projectileMinimumError; //0x78
    float projectileErrorAngleFrom; //0x7C
    float projectileErrorAngleTo; //0x80
    Vector firstPersonOffset; //0x84;
    char padding5[0x4]; //0x90
    TagDependency projectile; //0x94
    float ejectionPortRecoveryTime; //0xA4
    float illuminationRecoveryTime; //0xA8
    char padding6[0xC]; //0xAC
    float heatGeneratedPerRound; //0xB8
    float ageGeneratedPerRound; //0xBC
    char padding7[0x4]; //0xC0
    float overloadTime; //0xC4
    char padding8[0x40]; //0xC8
    Reflexive firingEffects; //0x108
} __attribute__((packed)) HaloWeaponTrigger;

typedef struct {
    HaloObjectTagData objectData; //0x0
    bitmask16 flags; //0x17C
    char padding5[0x2]; //0x17E
    uint16_t messageIndex; //0x180
    uint16_t messageSortOrder; //0x182
    float messageScale; //0x184
    uint16_t messageHudValueScale; //0x188
    enum objectFunction messageAin; //0x18A
    char padding[0x2D]; //0x18C
    enum objectFunction messageBin; //0x1B9
    char padding1[0x2D]; //0x1BB
    enum objectFunction messageCin; //0x1E8
    char padding2[0x2D]; //0x1EA
    enum objectFunction messageDin; //0x217
    char padding3[0x2F]; //0x219
    TagDependency materialEffects; //0x248
    TagDependency collisionSound; //0x258
    char padding4[0x78]; //0x268
    float detonationDelayFrom; //0x2E0
    float detonationDelayTo; //0x2E4
    TagDependency detonatingEffect; //0x2E8
    TagDependency detonationEffect; //0x2F8
} __attribute__((packed)) HaloItemTagData;  //0x308

typedef struct {
    HaloItemTagData itemData; //0x0
    bitmask32 weaponFlags; //0x308
    char weaponLabel[32]; //0x30C
    enum weaponSecondaryTriggerModes secondaryTriggerMode; //0x32C
    char padding[0x2]; //0x32E
    enum weaponFunctions exportAin; //0x330
    enum weaponFunctions exportBin; //0x332
    enum weaponFunctions exportCin; //0x334
    enum weaponFunctions exportDin; //0x336
    float exportReadyTime; //0x338
    TagDependency readyEffect; //0x33C
    float heatRecoveryThreshold; //0x34C
    float heatOverheatedThreshold; //0x350
    float heatDetonationThreshold; //0x354
    float heatDetonationFraction; //0x358
    float heatLossPerSecond; //0x35C
    float heatIllumination; //0x360
    char padding1[0x10]; //0x364
    TagDependency heatOverheatedEffect; //0x374
    TagDependency heatDetonation; //0x384
    TagDependency meleeDamage; //0x394
    TagDependency meleeResponse; //0x3A4
    char padding2[0x8]; //0x3B4
    TagDependency actorFiringParameters; //0x3BC
    float reticleNearReticleRange; //0x3CC
    float reticleFarReticleRange; //0x3D0
    float reticleIntersectionReticleRange; //0x3D4
    char padding3[0x2]; //0x3D8
    uint16_t zoomLevels; //0x3DA
    float zoomFrom; //0x3DC
    float zoomTo; //0x3E0
    float aimAssistAutoaimAngle; //0x3E4
    float aimAssistAutoaimRange; //0x3E8
    float aimAssistMagnetismAngle; //0x3EC
    float aimAssistMagnetismRange; //0x3F0
    float aimAssistMagnetismDeviationAngle; //0x3F4
    char padding4[0x4]; //0x3F8
    enum weaponMovementPenalized movementMovementPenalized; //0x3FC
    char padding5[0x2]; //0x3FE
    float weaponMovementForwardPenalty; //0x400
    float weaponMovementSidewaysPenalty; //0x404
    char padding6[0x4]; //0x408
    float aiTargetingParametersMinimumTargetRange; //0x40C
    float aiTargetingParametersLookingTimeModifier; //0x410
    char padding7[0x4]; //0x414
    float lightPowerOnTime; //0x418
    float lightPowerOffTime; //0x41C
    TagDependency lightPowerOnEffect; //0x420
    TagDependency lightPowerOffEffect; //0x430
    float ageHeatRecoveryPenalty; //0x440
    float ageRateOfFirePenalty; //0x444
    float ageMisfireStart; //0x448
    float ageMisfireChance; //0x44C
    char padding8[0xC]; //0x450
    TagDependency firstPersonModel; //0x45C
    TagDependency firstPersonAnimation; //0x46C
    char padding9[0x4];
    TagDependency hudInterface; //0x480
    TagDependency pickupSound; //0x490
    TagDependency zoomInSound; //0x4A0
    TagDependency zoomOutSound; //0x4B0
    char padding10[0xC]; //0x4C0
    float activeCamoDing; //0x4CC
    float activeCamoRegrowthRate; //0x4D0
    char padding11[0xE]; //0x4D4
    enum weaponType type; //0x4E2
    char padding12[0xC]; //0x4E4
    Reflexive magazines; //0x4F0
    Reflexive triggers; //0x4FC
} __attribute__((packed)) HaloWeaponData; //0x508 complete

typedef struct {
    uint32_t junk[0xB9];
    Reflexive seats; //0x4FC
} __attribute__((packed)) HaloVehicleData; //0x508 complete


#endif
