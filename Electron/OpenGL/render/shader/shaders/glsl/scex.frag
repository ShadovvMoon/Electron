#version 120
varying vec2 tex_coord;
varying vec3 normals;

uniform vec2 baseMapUV;
uniform vec4 useMap;
uniform sampler2D mapTexture0;
uniform sampler2D mapTexture1;
uniform sampler2D mapTexture2;
uniform sampler2D mapTexture3;
uniform vec4 uScale;
uniform vec4 vScale;
uniform vec4 uOffset;
uniform vec4 vOffset;
uniform ivec4 colorFunction;
uniform ivec4 alphaFunction;
uniform int mapCount;
void main(void) {
    vec2 coords  = vec2(tex_coord[0]*baseMapUV[0], tex_coord[1]*baseMapUV[1]);
    vec4 colour  = vec4(0.0,0.0,0.0,1.0);
    
    int cF = 0;
    int aF = 0;
    for (int i = 0; i < mapCount; i++) {
        vec2 mapCoords = vec2(coords.x * uScale[i] + uOffset[i], coords.y * vScale[i] + vOffset[i]);
        vec4 tex;
        if (i == 0) {
            tex = texture2D(mapTexture0, mapCoords);
        } else if (i == 1) {
            tex = texture2D(mapTexture1, mapCoords);
        } else if (i == 2) {
            tex = texture2D(mapTexture2, mapCoords);
        } else if (i == 3) {
            tex = texture2D(mapTexture3, mapCoords);
        }
        
        if (cF == 0) {
            colour.rgb = tex.rgb; // set
        } else if (cF == 1) {
            // skip
        } else if (cF == 2) {
            colour.rgb *= tex.rgb; // multiply
        } else if (cF == 3) {
            colour.rgb = colour.rgb * tex.rgb * tex.rgb; // double multiply
        } else if (cF == 4) {
            colour.rgb += tex.rgb; // add
        }
        
        if (aF == 0) {
            
        } else if (aF == 1) {
            // skip
        } else if (aF == 2) {
            colour.a *= (tex.r + tex.g + tex.b)/3.0 * 2.0; // multiply
        } else if (aF == 3) {
            colour.a = colour.a * tex.r * tex.r; // double multiply
        } else if (aF == 4) {
            colour.a += tex.r; // add
        } else if (aF == 7) {
            colour.a -= tex.r; // subtract current
        } else if (aF == 9) {
            colour.a = mix(colour.a, tex.a, 0.5);
        } else if (aF == 10) {
            colour.a = mix(colour.a, (1.0 - tex.a), 0.5);
        }
        
        cF = colorFunction[i];
        aF = alphaFunction[i];
    }
    gl_FragColor = colour;
    //gl_FragColor.a = 1.0;
}