#version 120
uniform sampler2D baseTexture;

uniform vec2 baseMapUV;
uniform vec4 maps; //0 = hasPrimary, 1 = primaryScale, 2 = hasSecondary, 3 = secondaryScale
varying vec2 tex_coord;
void main(void) {
    vec4 texel0 = texture2D(baseTexture, tex_coord);
    gl_FragColor = texel0;
}