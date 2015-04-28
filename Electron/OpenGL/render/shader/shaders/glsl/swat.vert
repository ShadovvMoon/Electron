#version 120
attribute vec2 texCoord_buffer_light;
attribute vec2 texCoord_buffer;
attribute vec3 normal_buffer;
attribute vec3 binormals_buffer;
attribute vec3 tangents_buffer;

varying vec2 tex_coord;
varying vec2 tex_coord_light;
varying vec3 normals;

varying vec4  fragPos;
varying vec4  EyePos;
varying vec3 lightVec, eyeVec;
varying mat3 TBNMatrix;
uniform vec2 timerOffset;

void main(void) {
    // Create the Texture Space Matrix
    TBNMatrix = mat3(tangents_buffer, binormals_buffer, normal_buffer);
    vec3 position = vec3(gl_ModelViewMatrix * gl_Vertex);
    
    // Compute the Eye Vector
    eyeVec  = (vec3(0.0) - position);
    eyeVec *= TBNMatrix;
    EyePos	= gl_ModelViewProjectionMatrix * gl_Vertex;
    fragPos = gl_ModelViewMatrix * gl_Vertex;
    
    // Compute the Light Vector
    lightVec  = gl_LightSource[0].position.xyz - position;
    lightVec *= TBNMatrix;
    
    vec4 modelPos = vec4(gl_ModelViewMatrix[0][3], gl_ModelViewMatrix[1][3], gl_ModelViewMatrix[2][3], 0);
    vec4 vertex = gl_Vertex + modelPos;
    float scaled = 1.0;
    float time = timerOffset.x;
    gl_TexCoord[0].x = 2.5*scaled * vertex.x + time * 0.2;
    gl_TexCoord[0].y = 2.5*scaled * vertex.y + time * 0.23;
    gl_TexCoord[1].x = -3.4*scaled * vertex.y + time * 0.3;
    gl_TexCoord[1].y = 3.4*scaled * vertex.x + time * 0.35;
    gl_TexCoord[2].x = (gl_Position.x / gl_Position.w * 0.5 + 0.5);
    gl_TexCoord[2].y = (gl_Position.y / gl_Position.w * 0.5 + 0.5);
    
    // Texture coordinates
    normals         = normal_buffer;
    tex_coord       = texCoord_buffer;
    tex_coord_light = texCoord_buffer_light;
    gl_Position  = ftransform();
}