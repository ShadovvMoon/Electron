//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef __V3D__
#define __V3D__

#include "defines.h"

class vector3d {
public:
    GLfloat x, y, z;
    vector3d (float sx, float sy, float sz);
    vector3d* add(vector3d* vector);
    vector3d* sub(vector3d* vector);
    vector3d* set(vector3d* vector);
    vector3d* cross(vector3d* vector);
    vector3d* norm();
    vector3d* mul(float delta);
};

#endif /* defined(__V3D__) */
