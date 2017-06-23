#version 410 core
in vec2 tex_coord;
in vec3 normals;

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
uniform vec4 fog;
uniform vec2 fogSettings;
out vec4 FragColor;

void main(void) {
    vec2 coords  = vec2(tex_coord[0]*baseMapUV[0], tex_coord[1]*baseMapUV[1]);
    vec4 colour  = vec4(0.0,0.0,0.0,1.0);
    
    int cF = 0;
    int aF  = 0;
    for (int i = 0; i < mapCount; i++) {
        vec2 mapCoords = vec2(coords.x * uScale[i] + uOffset[i], coords.y * vScale[i] + vOffset[i]);
        vec4 tex;
        if (i == 0) {
            tex = texture(mapTexture0, mapCoords);
        } else if (i == 1) {
            tex = texture(mapTexture1, mapCoords);
        } else if (i == 2) {
            tex = texture(mapTexture2, mapCoords);
        } else if (i == 3) {
            tex = texture(mapTexture3, mapCoords);
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
            colour.a = tex.a; //set
        } else if (aF == 1) {
            // skip
        } else if (aF == 2) {
            colour.a *= tex.a; // multiply
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
    FragColor = colour;

    //gl_FragColor.a = 1.0;
    
    // FOGGING
    float z = (gl_FragCoord.z / gl_FragCoord.w);
    float fogFactor = (z/fogSettings[0]);
    fogFactor = clamp(fogFactor, 0.0, fogSettings[1]);
    FragColor = mix(FragColor, vec4(fog[0],fog[1],fog[2],1.0), fogFactor);
}