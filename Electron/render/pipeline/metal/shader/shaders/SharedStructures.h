//
//  SharedStructures.h
//  ss
//
//  Created by Samuco on 23/6/16.
//  Copyright (c) 2016 Samuco. All rights reserved.
//

#ifndef SharedStructures_h
#define SharedStructures_h

#include <simd/simd.h>

typedef struct __attribute__((__aligned__(256)))
{
    matrix_float4x4 modelview_projection_matrix;
    matrix_float4x4 normal_matrix;
} uniforms_t;

typedef struct __attribute__((__aligned__(256)))
{
    float scaleU = 1.0;
    float scaleV = 1.0;
    
    float primaryScale = 1.0;
    float secondaryScale = 1.0;
    float bumpScale = 1.0;
    
    float mixPrimary = 0.0;
    float mixSecondary = 0.0;
    float mixLight = 0.0;
    float mixCube = 0.0;
    float mixBump = 0.0;
} senv_options;

typedef struct __attribute__((__aligned__(256)))
{
    uniforms_t shared;
    senv_options options;
} senv_uniform;

#endif /* SharedStructures_h */

