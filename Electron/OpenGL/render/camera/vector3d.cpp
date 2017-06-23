//
//  render.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "vector3d.h"
#include <math.h>
vector3d::vector3d (float sx, float sy, float sz) {
    x = sx;
    y = sy;
    z = sz;
}
vector3d::vector3d (vector3d *old) {
    x = old->x;
    y = old->y;
    z = old->z;
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
vector3d* vector3d::cross(vector3d* vector) {
    float tx = ((y * vector->z) - (z * vector->y));
    float ty = ((z * vector->x) - (x * vector->z));
    float tz = ((x * vector->y) - (y * vector->x));
    x = tx;
    y = ty;
    z = tz;
    return this;
};
vector3d* vector3d::norm() {
    float val = sqrt(x*x+y*y+z*z);
    x /= val;
    y /= val;
    z /= val;
    return this;
};
vector3d* vector3d::mul(float delta) {
    x *= delta;
    y *= delta;
    z *= delta;
    return this;
}
float vector3d::distance(vector3d* vector) {
    return sqrtf(pow(vector->x - x, 2) + pow(vector->y - y, 2) + pow(vector->z - z, 2));
}
float vector3d::get(int index) {
    switch (index) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
    }
    return -1;
}

float vector3d::dot(vector3d *o) {
    return x * o->x + y * o->y + z * o->z;
}
float vector3d::mag() {
    return sqrtf(x*x+y*y+z*z);
}

