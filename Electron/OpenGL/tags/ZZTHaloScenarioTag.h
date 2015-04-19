/*
 
 Copyright (c) 2015, Samuel Colbran
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
//  ZZTHaloScenarioTag.h
//  Electron
//
//  Created by Samuel Colbran on 19/4/15.
//  Copyright (c) 2015 Samuco. All rights reserved.
//

#include "ZZTHaloDataTypes.h"

#ifndef _ZZTHaloScenarioTag_h
#define _ZZTHaloScenarioTag_h

typedef struct {
    uint8_t unk0[0x30];     //0x0
    Reflexive skyBox;       //0x30
    uint32_t unk1;          //0x3C
    Reflexive children;     //0x40
    uint8_t unk2[0xA0];     //0x4C
    Reflexive resources;    //0xEC
    Reflexive functions;    //0xF8
    uint32_t unk4[2];       //0x104
    Reflexive unknown;      //0x110
    Reflexive comments;     //0x118
    uint8_t unk5[0xE0];     //0x124
    Reflexive objectNames;  //0x204
    Reflexive scen;
    Reflexive scenRef;
    Reflexive bipd;
    Reflexive bipdRef;
    Reflexive vehi;
    Reflexive vehiRef;
    Reflexive eqip;
    Reflexive eqipRef;
    Reflexive weap;
    Reflexive weapRef;
    Reflexive deviceGroups;
    Reflexive mach;
    Reflexive machRef;
    Reflexive cont;
    Reflexive contRef;
    Reflexive light;
    Reflexive lightRef;
    Reflexive sound;
    Reflexive soundRef;
    Reflexive unk6;
    Reflexive unk7;
    Reflexive unk8;
    Reflexive unk9;
    Reflexive unk10;
    Reflexive unk11;
    Reflexive unk12;
    Reflexive playerStartingProfile;
    Reflexive playerSpawn;
    Reflexive triggerVolumes;
    Reflexive animations;
    Reflexive multiplayerFlags;
    Reflexive mpEquip;
    Reflexive startEquip;
    Reflexive bspSwitchTriggers;
    Reflexive decal;
    Reflexive decalRef;
    Reflexive dobcRef;
    Reflexive unk13;
    Reflexive unk14;
    Reflexive unk15;
    Reflexive unk16;
    Reflexive unk17;
    Reflexive unk18;
    Reflexive unk19;
    Reflexive actorRef;
    Reflexive encounters;
    Reflexive commandlists;
    Reflexive unk20;
    Reflexive AiStartingLocations;
    Reflexive AiPlatoons;
    Reflexive AiConversations;
    uint32_t scriptDataSize; //0x474
    uint32_t unk21;          //0x478
    Reflexive scripts;       //0x47C
    uint32_t unk22[2];       //0x488
    Reflexive commands;      //0x490
    Reflexive points;        //0x49C
    Reflexive AiAnimationRef;//0x4A8
    Reflexive globals;       //0x4B4
    uint32_t unk23[9];       //0x4C0
    Reflexive AiRecordingRef;//0x4E4
    Reflexive unk24;         //0x4F0
    Reflexive participants;  //0x4FC
    uint32_t unk25[9];       //0x508
    Reflexive lines;         //0x52C
    uint8_t unk26[0x6C];     //0x538
    Reflexive bsp;           //0x5A4
} __attribute__((packed)) HaloScenarioTag;

#endif
