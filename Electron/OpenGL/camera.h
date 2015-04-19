//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef __CAMERA__
#define __CAMERA__

#include "defines.h"
#include "vector3d.h"

class Camera {
public:
    vector3d *position, *view, *up, *strafe;
    Camera ();
    void rotate(float angle, float x, float y, float z);
    void look();
};

#endif /* defined(____render__) */
