#version 120

attribute vec2 vertex_buffer;
attribute vec2 texCoord_buffer_light;
attribute vec2 texCoord_buffer;
attribute vec3 normal_buffer;
attribute vec3 binormals_buffer;
attribute vec3 tangents_buffer;

varying vec2 tex_coord;
varying vec2 tex_coord_light;
varying vec3 normals;
varying vec3 lightVec, eyeVec;
varying mat3 TBNMatrix;
varying vec4 position;

void main(void) {
    
    // Create the Texture Space Matrix
    TBNMatrix = mat3(tangents_buffer, binormals_buffer, normal_buffer);
    position = gl_ModelViewMatrix * gl_Vertex;
    vec3 position3 = vec3(position);
    
    // Compute the Eye Vector
    eyeVec  = (vec3(0.0) - position3);
    eyeVec *= TBNMatrix;
    
    // Compute the Light Vector
    lightVec  = gl_LightSource[0].position.xyz - position3;
    lightVec *= TBNMatrix;
    
    // Texture coords
    normals         = normal_buffer;
    tex_coord    = texCoord_buffer;
    gl_Position  = ftransform();
}