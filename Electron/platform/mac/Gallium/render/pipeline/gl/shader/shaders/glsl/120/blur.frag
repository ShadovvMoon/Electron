#version 120
uniform sampler2D tDiffuse;
uniform sampler2D tPosition;
uniform sampler2D tNormals;
uniform sampler2D tDepth;
uniform vec3 cameraPosition;



float nrand( vec2 n ) {
    return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}

vec2 rot2d( vec2 p, float a ) {
    vec2 sc = vec2(sin(a),cos(a));
    return vec2( dot( p, vec2(sc.y, -sc.x) ), dot( p, sc.xy ) );
}



#define SIGMA 10.0
#define BSIGMA 0.1
#define MSIZE 15
float getDepth(vec2 sample) {
    return 0.5;
}

void main( void )
{
    vec4 image    = texture2D( tDiffuse, gl_TexCoord[0].xy );
    vec4 position = texture2D( tPosition, gl_TexCoord[0].xy );
    vec3 normal   = texture2D( tNormals, gl_TexCoord[0].xy ).xyz;
    vec3 ssao     = texture2D( tDepth, gl_TexCoord[0].xy ).xyz;
    
    vec3 light = vec3(0,0,50);
    vec3 lightDir = light - position.xyz ;
    
    normal = normalize(normal);
    lightDir = normalize(lightDir);
    
    vec3 eyeDir = normalize(cameraPosition-position.xyz);
    vec3 vHalfVector = normalize(lightDir.xyz+eyeDir);
    
    
    vec2 resolution = vec2(4096.0, 2048.0);
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    
    const float yblur = 1024;
    const float blursize = 1.0/yblur;
    vec2 vTexCoord = uv;//vec2(gl_TexCoord[0].x, gl_TexCoord[0].y);
    vec4 sum = vec4(0.0);
    sum += texture2D(tDepth, vec2(vTexCoord.x, vTexCoord.y - 4.0*blursize)) * 0.05;
    sum += texture2D(tDepth, vec2(vTexCoord.x, vTexCoord.y - 3.0*blursize)) * 0.09;
    sum += texture2D(tDepth, vec2(vTexCoord.x, vTexCoord.y - 2.0*blursize)) * 0.12;
    sum += texture2D(tDepth, vec2(vTexCoord.x, vTexCoord.y - blursize)) * 0.15;
    sum += texture2D(tDepth, vec2(vTexCoord.x, vTexCoord.y)) * 0.16;
    sum += texture2D(tDepth, vec2(vTexCoord.x, vTexCoord.y + blursize)) * 0.15;
    sum += texture2D(tDepth, vec2(vTexCoord.x, vTexCoord.y + 2.0*blursize)) * 0.12;
    sum += texture2D(tDepth, vec2(vTexCoord.x, vTexCoord.y + 3.0*blursize)) * 0.09;
    sum += texture2D(tDepth, vec2(vTexCoord.x, vTexCoord.y + 4.0*blursize)) * 0.05;
    gl_FragColor = sum;
    
    
    
    
    
    
    
    /*
    const int NUM_TAPS = 12;
    float max_siz = 20.0;// * (0.5+0.5*sin(iGlobalTime));
    
    vec2 fTaps_Poisson[NUM_TAPS];
    fTaps_Poisson[0]  = vec2(-.326,-.406);
    fTaps_Poisson[1]  = vec2(-.840,-.074);
    fTaps_Poisson[2]  = vec2(-.696, .457);
    fTaps_Poisson[3]  = vec2(-.203, .621);
    fTaps_Poisson[4]  = vec2( .962,-.195);
    fTaps_Poisson[5]  = vec2( .473,-.480);
    fTaps_Poisson[6]  = vec2( .519, .767);
    fTaps_Poisson[7]  = vec2( .185,-.893);
    fTaps_Poisson[8]  = vec2( .507, .064);
    fTaps_Poisson[9]  = vec2( .896, .412);
    fTaps_Poisson[10] = vec2(-.322,-.933);
    fTaps_Poisson[11] = vec2(-.792,-.598);
    
    vec2 resolution = vec2(4096.0, 2048.0);
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    //uv.x += 0.05*iGlobalTime;
    vec4 sum = vec4(0);
    float rnd = 6.28 * nrand( uv);
    
    vec4 basis = vec4( rot2d(vec2(1,0),rnd), rot2d(vec2(0,1),rnd) );
    for (int i=0; i < NUM_TAPS; i++)
    {
        vec2 ofs = fTaps_Poisson[i]; ofs = vec2(dot(ofs,basis.xz),dot(ofs,basis.yw) );
        //vec2 ofs = rot2d( fTaps_Poisson[i], rnd );
        vec2 texcoord = uv + max_siz * ofs / resolution.xy;
        sum += texture2D(tDepth, texcoord, -10.0);
    }
    
    float ao = (sum / vec4(NUM_TAPS)).x;
    gl_FragColor = mix(image, vec4(ao), 0.8);
    */
    
    
    
    
    // // //vec4(vec3(ao), 1.0); //
    
    
    
    //gl_FragColor = image * vec4(ssao, 1.0); //vec4(1.0-final_colour, 1.0);
    
    /*
    const float zNear = 0.1;
    const float zFar = 4000000.0;
    float z_b = texture2D( tDepth, gl_TexCoord[0].xy ).x;
    float z_n = 2.0 * z_b - 1.0;
    float z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
    
    gl_FragColor = vec4(z_n, z_n, z_n, 1.0); //mix(image, vec4(1.0,1.0,1.0,1.0), pow(depth.x, 5.0) / 5.0);
    */
    
    // max(dot(normal,lightDir),0.0) *  + pow(max(dot(normal,vHalfVector),0.0), 100.0) * 1.0;
}
