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
    printf("camera setup\n");
    //position = new vector3d(0.0, 0.0, 0.0);
    //view     = new vector3d(0.0, 0.0, -0.5);
    //up       = new vector3d(0.0, 0.0, 1.0);
    
    
    //position = new vector3d(76.658791, -113.148872, 7.970234);
    //view     = new vector3d(71.658791, -118.148872, 2.970234);
    //up       = new vector3d(0.0, 0.0, 1.0);
    
    position = new vector3d(131.793106, -194.661621, 29.700350);
    view     = new vector3d(126.509056, -188.770096, 26.183468);
    up       = new vector3d(0.0, 0.0, 1.0);
    vstrafe  = new vector3d(0.0, 0.0, 0.0);
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
    //delete newView
}

void Camera::drag(float dx, float dy) {
    float angleY = 0.0f;							// This is the direction for looking up or down
    float angleZ = 0.0f;							// This will be the value we need to rotate around the Y axis (Left and Right)
    static float currentRotX = 0.0f;

    // Get the direction the mouse moved in, but bring the number down to a reasonable amount
    angleY = (float)( (dx) ) / 200.0;
    angleZ = (float)( (dy) ) / 200.0f;

    currentRotX -= angleZ;
    
    vector3d *vVector = new vector3d(0,0,0);
    vVector->set(view);
    vVector->sub(position);
    vVector->cross(up);
    vVector->norm();
    this->rotate(angleZ, vVector->x, vVector->y, vVector->z);
    this->rotate(-1*angleY, 0, 0, 1);
    // delete vVector
}

void Camera::move(float delta) {
    vector3d *vVector = new vector3d(0,0,0);
    vVector->add(view);
    vVector->sub(position);
    vVector->norm();
    vVector->mul(delta);
    position->add(vVector);
    view->add(vVector);
    //delete
}

void Camera::strafe(float delta) {
    vector3d *vVector = new vector3d(0,0,0);
    vVector->set(vstrafe);
    vVector->mul(delta);
    position->add(vVector);
    vVector->z = 0.0;
    view->add(vVector);
    //delete
}

void Camera::look(shader_options *options) {
    vstrafe->set(view);
    vstrafe->sub(position);
    vstrafe->cross(up);
    vstrafe->norm();

    #ifdef RENDER_CORE_32
    vector3d *f = new vector3d(view);
    f->sub(position);
    f->norm();
    vector3d *s = new vector3d(f);
    up->norm();
    s->cross(up);
    s->norm();
    vector3d *u = new vector3d(s);
    u->cross(f);
    
    // modelview is an identity here so glMultMatrixf will just be M
    float M[4][4] = {{ s->x,  s->y,  s->z, 0.0f},
                     { u->x,  u->y,  u->z, 0.0f},
                     {-f->x, -f->y, -f->z, 0.0f},
                     {  0.0,   0.0,   0.0, 1.0f}};
    
    float T[4][4] = {{ 1.0,  0.0, 0.0, -position->x},
                     { 0.0,  1.0, 0.0, -position->y},
                     { 0.0,  0.0, 1.0, -position->z},
                     { 0.0,  0.0, 0.0, 1.0f}};
    
    for (int i = 0; i < 16; i++) {
        options->modelview[i] = 0.0;
    }
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            for (int inner = 0; inner < 4; inner++) {
                options->modelview[row*4+col] += M[col][inner] * T[inner][row];
            }
        }
    }
    
    delete f;
    delete s;
    delete u;
    #else
    gluLookAt(position->x, position->y, position->z,
              view->x,	   view->y,     view->z,
              up->x,       up->y,       up->z);
    #endif
    
    options->camera[0] = position->x;
    options->camera[1] = position->y;
    options->camera[2] = position->z;
}