#version 120

attribute vec2 texCoord_buffer_light;
attribute vec2 texCoord_buffer;
attribute vec3 normal_buffer;

varying vec2 tex_coord;
varying vec2 tex_coord_light;
varying vec3 normals;


void main(void) {
    vec3 Normal         = gl_NormalMatrix * normal_buffer;
    vec4 CameraPosition = gl_ModelViewProjectionMatrix * gl_Vertex;
    vec4 Position       = gl_ModelViewMatrix * gl_Vertex;
    vec3 Reflection     = reflect(Position.xyz - CameraPosition.xyz, normalize(Normal));
    normals = vec3(Reflection.x, -Reflection.yz);
    
    tex_coord       = texCoord_buffer;
    tex_coord_light = texCoord_buffer_light;
    gl_Position  = ftransform();
    normals = gl_Position.xyz;
}