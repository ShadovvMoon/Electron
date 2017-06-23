#version 410 core

in vec3 vertex_buffer;
in vec2 texCoord_buffer_light;
in vec2 texCoord_buffer;
in vec3 normal_buffer;
in vec3 binormals_buffer;
in vec3 tangents_buffer;
out vec2 tex_coord;
out vec3 normals;
uniform mat4 ProjectionMatrix;
uniform mat4 ModelViewMatrix;

void main(void) {

    // Texture coords
    vec4 vertex  = vec4(vertex_buffer, 1.0);
    normals      = normal_buffer;
    tex_coord    = texCoord_buffer;
    gl_Position  = ProjectionMatrix * ModelViewMatrix * vertex;
}