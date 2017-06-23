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
//  ZZTHaloDamageTag.h
//  Dynamic Blood
//
//  Created by Paul Whitcomb on 2/12/14.
//  Copyright (c) 2014 Zero2. All rights reserved.
//

#include "ZZTHaloDataTypes.h"

#ifndef _ZZTHaloDamageTag_h
#define _ZZTHaloDamageTag_h

enum FadeFunction : uint16_t {
    FADE_LINEAR = 0,
    FADE_EARLY = 1,
    FADE_VERY_EARLY = 2,
    FADE_LATE = 3,
    FADE_VERY_LATE = 4,
    FADE_COSINE = 5
};

enum HaloFunction : uint16_t {
    FUNCT_ONE = 0,
    FUNCT_ZERO = 1,
    FUNCT_COS = 2,
    FUNCT_COS_VARIABLE = 3,
    FUNCT_DIAGONAL_WAVE = 4,
    FUNCT_DIAGONAL_WAVE_VARIABLE = 5,
    FUNCT_SLIDE = 6,
    FUNCT_SLIDE_VARIABLE = 7,
    FUNCT_NOISE = 8,
    FUNCT_JITTER = 9,
    FUNCT_WANDER = 10,
    FUNCT_SPARK = 11
};

enum DamageCategory : uint16_t {
    DAMAGE_CATEGORY_NONE = 0,
    DAMAGE_CATEGORY_FALLING = 1,
    DAMAGE_CATEGORY_BULLET = 2,
    DAMAGE_CATEGORY_GRENADE = 3,
    DAMAGE_CATEGORY_HIGH_EXPLOSIVE = 4,
    DAMAGE_CATEGORY_SNIPER = 5,
    DAMAGE_CATEGORY_MELEE = 6,
    DAMAGE_CATEGORY_FLAME = 7,
    DAMAGE_CATEGORY_MOUNTED_WEAPON = 8,
    DAMAGE_CATEGORY_VEHICLE = 9,
    DAMAGE_CATEGORY_PLASMA = 10,
    DAMAGE_CATEGORY_NEEDLE = 11,
    DAMAGE_CATEGORY_SHOTGUN = 12
};


typedef struct {
    float radiusFrom;                           //0x0
    float radiusTo;                             //0x4
    float cutoffScale;                          //0x8
    uint16_t flagsBitmask;                      //0xC
    char unknown0[0x16];                        //0xE
    uint16_t type;                              //0x24
    uint16_t priority;                          //0x26
    char unknown10[0xC];                        //0x28
    float duration;                             //0x34
    enum FadeFunction fadeFunction;             //0x38
    char unknown11[0xA];                        //0x3A
    float intensity;                            //0x44
    uint32_t unknown12;                         //0x48
    ColorARGB color;                            //0x4C
    float lowFrequencyVibrateFrequency;         //0x5C
    float lowFrequencyVibrateDuration;          //0x60
    enum FadeFunction lowFrequencyFadeFunction; //0x64
    char unknown1[0xA];                         //0x66
    float highFrequencyVibrateFrequency;        //0x70
    float highFrequencyVibrateDuration;         //0x74
    enum FadeFunction highFrequencyFadeFunction;//0x78
    char unknown2[0x1E];                        //0x7A
    float tempCameraImpulseDuration;            //0x98
    enum FadeFunction tempCameraImpulseFadeFunction;//0x9C
    char unknown15[0x2];                        //0x9E
    float tempCameraImpulseRotation;            //0xA0
    float tempCameraImpulsePushback;            //0xA4
    float tempCameraImpulseJitterFrom;          //0xA8
    float tempCameraImpulseJitterTo;            //0xAC
    char unknown3[0x8];                         //0xB0
    float permanentImpulseCameraAngle;          //0xB8
    char unknown4[0x10];                        //0xBC
    float cameraShakeDuration;                  //0xCC
    enum FadeFunction cameraShakeFalloffFunction;//0xD0
    char unknown16[0x2];                        //0xD2
    float cameraShakeRandomTranslation;         //0xD4
    float cameraShakeRandomRotation;            //0xD8
    char unknown5[0xC];                         //0xDC
    enum HaloFunction cameraShakeWobbleFunction;//0xE8
    char unknown17[0x2];                        //0xEA
    float cameraShakeWobblePeriod;              //0xEC
    float cameraShakeWobbleWeight;              //0xF0
    char unknown6[0x20];                        //0xF4
    TagDependency sound;                        //0x114
    char unknown7[0x70];                        //0x124
    float breakingEffectForwardVelocity;        //0x194
    float breakingEffectForwardRadius;          //0x198
    float breakingEffectForwardExponent;        //0x19C
    char unknown8[0xC];                         //0x1A0
    float breakingEffectOutwardVelocity;        //0x1AC
    float breakingEffectOutwardRadius;          //0x1B0
    float breakingEffectOutwardExponent;        //0x1B4
    char unknown9[0xC];                         //0x1B8
    uint16_t damageSideEffect;                  //0x1C4
    enum DamageCategory damageCategory;         //0x1C6
    uint32_t damageFlags;                       //0x1C8
    float AOECoreRadius;                        //0x1CC
    float damageLowerBound;                     //0x1D0
    float damageUpperBoundFrom;                 //0x1D4
    float damageUpperBoundTo;                   //0x1D8
    float damageVehicleFraction;                //0x1DC
    float damageActiveCamouflageDamage;         //0x1E0
    float damageStun;                           //0x1E4
    float damageStunMax;                        //0x1E8
    float damageStunTime;                       //0x1EC
    uint32_t unknown13;                         //0x1F0
    float damageForce;                          //0x1F4
    char unknown14[0x8];                        //0x1F8
    float damageScaleDirt;                      //0x200, increase by 0x4
    float damageScaleSand;
    float damageScaleStone;
    float damageScaleSnow;
    float damageScaleWood;
    float damageScaleMetalHollow;
    float damageScaleMetalThin;
    float damageScaleMetalTick;
    float damageScaleRubber;
    float damageScaleGlass;
    float damageScaleForceField;
    float damageScaleGrunt;
    float damageScaleHunterArmor;
    float damageScaleHunterSkin;
    float damageScaleElite;
    float damageScaleJackal;
    float damageScaleJackalShield;
    float damageScaleEngineer;
    float damageScaleEngineerForceField;
    float damageScaleFloodCombatForm;
    float damageScaleFloodCarrierForm;
    float damageScaleCyborg;
    float damageScaleCyborgEnergyShield;
    float damageScaleArmoredHuman;
    float damageScaleHuman;
    float damageScaleSentinel;
    float damageScaleMonitor;
    float damageScalePlastic;
    float damageScaleWater;
    float damageScaleLeaves;
    float damageScaleEliteEnergyShield;
    float damageScaleIce;
    float damageScaleHunterShield;              //0x280
    char unknown[0x1C];                         //0x284
} __attribute__((packed)) HaloDamageTagData;    //0x2A0

#endif
