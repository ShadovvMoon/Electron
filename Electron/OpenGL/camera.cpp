//
//  render.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "camera.h"
#include "math.h"

Camera::Camera () {
    //position = new vector3d(0.0, 0.0, 0.0);
    //view     = new vector3d(0.0, 0.0, -0.5);
    //up       = new vector3d(0.0, 0.0, 1.0);
    
    
    //position = new vector3d(76.658791, -113.148872, 7.970234);
    //view     = new vector3d(71.658791, -118.148872, 2.970234);
    //up       = new vector3d(0.0, 0.0, 1.0);
    
    position = new vector3d(131.793106, -194.661621, 29.700350);
    view     = new vector3d(126.509056, -188.770096, 26.183468);
    up       = new vector3d(0.0, 0.0, 1.0);
}

void Camera::rotate(float angle, float x, float y, float z) {
    view = view->sub(position);
    float cosTheta = (float)cos(angle);
    float sinTheta = (float)sin(angle);
    
    vector3d *newView = new vector3d(0,0,0);
    newView->x = (cosTheta + (1 - cosTheta) * x * x) * view->x;
    newView->x += ((1 - cosTheta) * x * y - z * sinTheta) * view->y;
    newView->x += ((1 - cosTheta) * x * z + y * sinTheta) * view->z;
    
    // Find the new y position for the new rotated point
    newView->y  = ((1 - cosTheta) * x * y + z * sinTheta)	* view->x;
    newView->y += (cosTheta + (1 - cosTheta) * y * y)		* view->y;
    newView->y += ((1 - cosTheta) * y * z - x * sinTheta)	* view->z;
    
    // Find the new z position for the new rotated point
    newView->z  = ((1 - cosTheta) * x * z - y * sinTheta)	* view->x;
    newView->z += ((1 - cosTheta) * y * z + x * sinTheta)	* view->y;
    newView->z += (cosTheta + (1 - cosTheta) * z * z)		* view->z;
    
    // Now we just add the newly rotated vector to our position to set
    // our new rotated view of our camera.
    view->set(position);
    view->add(newView);
}

void Camera::look() {
    gluLookAt(position->x, position->y, position->z,
              view->x,	   view->y,     view->z,
              up->x,       up->y,       up->z);
}