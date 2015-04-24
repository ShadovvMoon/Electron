#version 120

attribute vec2 texCoord_buffer_light;
attribute vec2 texCoord_buffer;

varying vec2 tex_coord;
varying vec2 tex_coord_light;

void main(void) {
    tex_coord       = texCoord_buffer;
    tex_coord_light = texCoord_buffer_light;
    gl_Position  = ftransform();
}