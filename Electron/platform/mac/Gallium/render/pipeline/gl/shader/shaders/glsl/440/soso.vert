#version 410 core

in vec3 vertex_buffer;
in vec2 texCoord_buffer_light;
in vec2 texCoord_buffer;
in vec3 normal_buffer;
in vec3 binormals_buffer;
in vec3 tangents_buffer;

out vec2 tex_coord;
out vec2 tex_coord_light;
out vec3 normals;
out vec3 lightVec, eyeVec;
out mat3 TBNMatrix;

uniform mat4 ProjectionMatrix;
uniform mat4 ModelViewMatrix;
uniform vec3 Position;
void main(void) {
    
    // Create the Texture Space Matrix
    vec4 vertex = vec4(vertex_buffer, 1.0);
    //vertex += vec4(Position, 0.0);
    
    TBNMatrix = mat3(tangents_buffer, binormals_buffer, normal_buffer);
    vec3 position = vec3(ModelViewMatrix * vertex);
    
    // Compute the Eye Vector
    eyeVec  = (vec3(0.0) - position);
    eyeVec *= TBNMatrix;
    
    // Compute the Light Vector
    lightVec  = vec3(0.0,0.0,0.0) - position;
    lightVec *= TBNMatrix;
    
    // Texture coords
    normals         = normal_buffer;
    tex_coord    = texCoord_buffer;
    gl_Position   = ProjectionMatrix * ModelViewMatrix * vertex;
}