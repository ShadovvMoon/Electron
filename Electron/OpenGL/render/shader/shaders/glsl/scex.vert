#version 120

attribute vec2 texCoord_buffer_light;
attribute vec2 texCoord_buffer;
attribute vec3 normal_buffer;
attribute vec3 binormals_buffer;
attribute vec3 tangents_buffer;

varying vec2 tex_coord;
varying vec3 normals;

void main(void) {

    // Texture coords
    normals      = normal_buffer;
    tex_coord    = texCoord_buffer;
    gl_Position  = ftransform();
}