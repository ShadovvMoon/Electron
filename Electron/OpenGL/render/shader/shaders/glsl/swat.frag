#version 120
uniform sampler2D baseTexture;
uniform sampler2D bumpMap;

uniform vec2 reflectionScale;
uniform vec2 scale;
varying vec2 tex_coord;
varying vec2 tex_coord_light;
varying vec3 normals;
uniform float bumpScales;
uniform vec2 timerOffset;

// Cubemapping
varying vec4  EyePos;
varying vec4 fragPos;
varying vec3 lightVec, eyeVec;
uniform int togglebump; // false/true
uniform int textureon; // false/true
uniform float normalweight = 1.0;
varying mat3 TBNMatrix;

// need to scale our framebuffer - it has a fixed width/height of 2048
uniform vec4 frameSize;

const vec3 Xunitvec = vec3 (1.0, 0.0, 0.0);
const vec3 Yunitvec = vec3 (0.0, 1.0, 0.0);
void main(void) {
    float FrameWidth = frameSize[0];
    float FrameHeight = frameSize[1];
    float textureWidth = frameSize[2];
    float textureHeight = frameSize[3];
    
    vec2 mapCoords = vec2(tex_coord.x * bumpScales + timerOffset.x * 0.2, tex_coord.y * bumpScales + timerOffset.y * 0.23);
    vec2 mapCoords2 = vec2(tex_coord.x * bumpScales - timerOffset.x * 0.3, tex_coord.y * bumpScales + timerOffset.y * 0.25);
    //vec3 n = texture2D(bumpMap, mapCoords).rgb;
    //vec3 n = texture2D(bumpMap, mapCoords).rgb;
    
    vec3 wnTex0 = vec3(1.0,1.0,1.0) - (normalize(texture2D(bumpMap, mapCoords).xyz - vec3(0.5, 0.5, 0.5)));
    vec3 wnTex1 = vec3(1.0,1.0,1.0) - (normalize(texture2D(bumpMap, mapCoords2).xyz - vec3(0.5, 0.5, 0.5)));
    
    // Scale the normal vector based on angle with the horizontal vector
    // calc refraction
    float distance = sqrt(pow(fragPos.z - EyePos.z,2)) / 20.0;
    if (distance > 1000)
        distance = 1000;
    else if (distance < 2)
        distance = 2;
    float scaled = 1/distance;
    vec3 n = normalize(wnTex0 + wnTex1) * min(0.3, scaled);
    
    // perform the div by w
    float recipW = 1.0 / EyePos.w;
    vec2 eye = EyePos.xy * vec2(recipW);
    
    // calc the refraction lookup
    vec2 index;
    index.s = eye.x;
    index.t = eye.y;
    
    // scale and shift so we're in the range 0-1
    index.s = index.s / 2.0 + 0.5;
    index.t = index.t / 2.0 + 0.5;
    
    // as we're looking at the framebuffer, we want it clamping at the edge of the rendered scene, not the edge of the texture,
    // so we clamp before scaling to fit
    float recipTextureWidth = 1.0 / textureWidth;
    float recipTextureHeight = 1.0 / textureHeight;
    index.s = clamp(index.s, 0.0, 1.0 - recipTextureWidth);
    index.t = clamp(index.t, 0.0, 1.0 - recipTextureHeight);
    
    // scale the texture so we just see the rendered framebuffer
    index.s = index.s * FrameWidth * recipTextureWidth;
    index.t = index.t * FrameHeight * recipTextureHeight;
    
    //Clip edges
    vec3 MirrorColor = vec3 (texture2D(baseTexture, index));
    index.st += 0.05 * n.xy;
    
    // refraction
    vec3 RefractionColor = vec3 (texture2D(baseTexture, index));
    vec3 light_position = vec3(0,0,0);
    vec3 L = normalize(light_position.xyz - EyePos.xyz);
    vec3 E = normalize(-EyePos.xyz); // we are in Eye Coordinates, so EyePos is (0,0,0)
    vec3 R = normalize(-reflect(L,n));
    float shiny = dot(vec3(0.0, 1.0, 0.0), L);
    float cosTheta = clamp( dot( L, n ), 0,1 );

    vec3 color = RefractionColor;
    vec4 waterColour = vec4(30/255.0, 50/255.0, 48/255.0, 0.8)*2;
    gl_FragColor = mix(waterColour, waterColour*vec4(color.x, color.y, color.z, 1.0), 0.5);
}