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

void main(void) {
    
    // Create the Texture Space Matrix
    TBNMatrix = mat3(tangents_buffer, binormals_buffer, normal_buffer);
    vec3 position = vec3(gl_ModelViewMatrix * gl_Vertex);
    
    // Compute the Eye Vector
    eyeVec  = (vec3(0.0) - position);
    eyeVec *= TBNMatrix;
    
    // Compute the Light Vector
    lightVec  = gl_LightSource[0].position.xyz - position;
    lightVec *= TBNMatrix;
    
    // Texture coords
    normals         = normal_buffer;
    tex_coord    = texCoord_buffer;
    gl_Position  = ftransform();
    
}