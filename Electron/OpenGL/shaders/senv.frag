#version 120
uniform sampler2D baseTexture;
varying vec2 tex_coord;
void main(void) {
    vec4 texel0 = texture2D(baseTexture, tex_coord);
    gl_FragColor = texel0;
}