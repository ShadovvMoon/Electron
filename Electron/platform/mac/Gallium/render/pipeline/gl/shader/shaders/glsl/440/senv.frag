#version 410 core
uniform sampler2D baseTexture;
uniform sampler2D primaryDetailMap;
uniform sampler2D secondaryDetailMap;
uniform sampler2D lightMap;
uniform sampler2D bumpMap;
uniform samplerCube cubeTextureMap;

uniform vec2 baseMapUV;
uniform vec4 maps; //0 = hasPrimary, 1 = primaryScale, 2 = hasSecondary, 3 = secondaryScale
uniform vec4 maps2;  //0 = useLight, //1 = blend
uniform float maps3;  //0 = useLight, //1 = blend

uniform vec2 reflectionScale;
uniform vec2 scale;
in vec2 tex_coord;
in vec2 tex_coord_light;
in vec3 normals;

// Cubemapping
in vec3 lightVec, eyeVec;
in mat3 TBNMatrix;

uniform int togglebump; // false/true
uniform int textureon; // false/true
uniform float normalweight = 1.0;

// Fog
uniform vec4 fog;
uniform vec2 fogSettings;
out vec4 FragColor;
void main(void) {
    vec3 LightSource = vec3(0.0,0.0,0.0);
    
    vec2 coords  = vec2(tex_coord[0]*scale[0], tex_coord[1]*scale[1]);
    vec4 texel0  = texture(baseTexture, coords);
    vec4 texel1  = texture(primaryDetailMap, coords*maps[1]);
    vec4 texel2  = texture(secondaryDetailMap, coords*maps[3]);
    vec4 blight  = texture(lightMap, tex_coord_light);
    vec4 texel3  = texture(cubeTextureMap, normals);

    vec4 white   = vec4(1.0,1.0,1.0,1.0);
    vec4 detail1 = mix(white, texel1 * 2.0, maps[0]);
    vec4 detail2 = mix(white, texel2 * 2.0, maps[2]);
    vec4 detail3 = mix(white, blight, maps2[0]);
    vec4 detail  = mix(detail2, detail1, texel0.a);
    //vec4 cube    = mix(white, texel3*2, maps2[2]);
    //vec4 cubea   = mix(white, cube, texel0.a);
    
    FragColor = texel0 * detail * detail3;
    FragColor.a = 1.0;
    
    // CUBEMAPPING
    if ( maps2[3] > 0 || maps2[2] > 0 ) {
        vec4 reflex = vec4(0.0, 0.0, 0.0, 1.0);
        vec3 eye = normalize(eyeVec);
        //eye = vec3(eye.x * 0.5, eye.y * 0.5, eye.z);
        vec2 offsetdir = vec2( eye.x, eye.y );
        float dist = length(lightVec);
        vec3 light = normalize(lightVec);

        // Query the Maps
        vec3 color = vec3(0.0,0.0,0.0);
        vec3 norm = normalize(normals);
        if ( maps2[3] > 0 ) {
            norm = texture(bumpMap, coords*maps3).rgb - 0.5;
            norm = vec3(norm.x * normalweight, norm.y * normalweight, norm.z);
        }
        
        vec3 refl = reflect(norm, eye);  // in tangent space !
        vec3 reflw = vec3( 1.0, -1.0, 1.0) * (TBNMatrix * refl);
        reflex = texture(cubeTextureMap, reflw);
        
        float oneMinusDot = 1.0 - dot(normalize(normals), normalize(eye));
        float scale = mix(reflectionScale[0], reflectionScale[1], oneMinusDot);
        //gl_FragColor = mix(gl_FragColor, reflex, min(texel0.a, scale));
        FragColor = FragColor * mix(white, reflex*2, min(texel0.a, scale));
    }
    
    // FOGGING
    float z = (gl_FragCoord.z / gl_FragCoord.w);
    float fogFactor = (z/fogSettings[0]);
    fogFactor = clamp(fogFactor, 0.0, fogSettings[1]);
    FragColor = mix(FragColor, vec4(fog[0],fog[1],fog[2],1.0), fogFactor);
}