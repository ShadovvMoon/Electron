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
//  ZZTHaloDataTypes.h
//  
//
//  Created by Paul Whitcomb on 2/12/14.
//
//

#include <stdint.h>

typedef uint16_t unichar;
typedef uint8_t bitmask8;
typedef uint16_t bitmask16;
typedef uint16_t refIndex;
typedef uint32_t bitmask32;

#ifndef _ZZTHaloDataTypes_h
#define _ZZTHaloDataTypes_h

typedef struct {
    uint32_t count;
    void *pointer;
    uint32_t null;
} __attribute__((packed)) Reflexive;

typedef struct {
    uint16_t objectTableIndex;
    uint16_t objectIndex;
} __attribute__((packed)) ObjectID;

typedef struct {
    uint16_t tagTableIndex;
    uint16_t tagIndex;
} __attribute__((packed)) TagID;

typedef struct {
    uint16_t playerTableIndex;
    uint16_t playerIndex;
} __attribute__((packed)) PlayerID;

//TagID NullTagID;

typedef struct {
    char className[4];
    void *tagName;
    uint32_t zero;
    TagID identity;
} __attribute__((packed)) TagDependency;

typedef struct {
    float yaw;
    float pitch;
    float roll;
} __attribute__((packed)) Orientation;

#endif
