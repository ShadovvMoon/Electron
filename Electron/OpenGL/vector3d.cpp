//
//  render.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "vector3d.h"
vector3d::vector3d (float sx, float sy, float sz) {
    x = sx;
    y = sy;
    z = sz;
}
vector3d* vector3d::add(vector3d* vector) {
    x += vector->x;
    y += vector->y;
    z += vector->z;
    return this;
};
vector3d* vector3d::sub(vector3d* vector) {
    x -= vector->x;
    y -= vector->y;
    z -= vector->z;
    return this;
};
vector3d* vector3d::set(vector3d* vector) {
    x = vector->x;
    y = vector->y;
    z = vector->z;
    return this;
};

