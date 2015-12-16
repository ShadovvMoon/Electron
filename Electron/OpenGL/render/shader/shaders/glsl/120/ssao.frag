#version 120
uniform sampler2D tDiffuse;
uniform sampler2D tPosition;
uniform sampler2D tNormals;
uniform sampler2D tDepth;
uniform vec3 cameraPosition;

vec3 normal_from_depth(float depth, vec2 texcoords) {
    
    const vec2 offset1 = vec2(0.0,0.001);
    const vec2 offset2 = vec2(0.001,0.0);
    
    float depth1 = texture2D(tDepth, gl_TexCoord[0].xy + offset1).r;
    float depth2 = texture2D(tDepth, gl_TexCoord[0].xy + offset2).r;
    
    vec3 p1 = vec3(offset1, depth1 - depth);
    vec3 p2 = vec3(offset2, depth2 - depth);
    
    vec3 normal = cross(p1, p2);
    normal.z = -normal.z;
    
    return normalize(normal);
}

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(1233.9898,78.233))) * 43758.5453);
}

void main( void )
{
    vec4 image = texture2D( tDiffuse, gl_TexCoord[0].xy );
    vec4 position = texture2D( tPosition, gl_TexCoord[0].xy );
    vec4 normal = texture2D( tNormals, gl_TexCoord[0].xy );

    // SSAO
    const float total_strength = 1.0;
    const float base = 0.5;
    const float area = 0.2;
    const float falloff = 0.000001;
    const float radius = 0.00250;
    const int samples = 4;
    vec3 sample_sphere[16] = vec3[16](
        vec3( 0.5381, 0.1856,-0.4319), vec3( 0.1379, 0.2486, 0.4430),
        vec3( 0.3371, 0.5679,-0.0057), vec3(-0.6999,-0.0451,-0.0019),
        vec3( 0.0689,-0.1598,-0.8547), vec3( 0.0560, 0.0069,-0.1843),
        vec3(-0.0146, 0.1402, 0.0762), vec3( 0.0100,-0.1924,-0.0344),
        vec3(-0.3577,-0.5301,-0.4358), vec3(-0.3169, 0.1063, 0.0158),
        vec3( 0.0103,-0.5869, 0.0046), vec3(-0.0897,-0.4940, 0.3287),
        vec3( 0.7119,-0.0154,-0.0918), vec3(-0.0533, 0.0596,-0.5411),
        vec3( 0.0352,-0.0631, 0.5460), vec3(-0.4776, 0.2847,-0.0271)
    );
    
    float r1 = rand(gl_TexCoord[0].xy);
    vec3 random = vec3(r1,r1,r1);
    float depth = texture2D(tDepth, gl_TexCoord[0].xy).r;
    vec3 eposition = vec3(gl_TexCoord[0].xy, depth);
    vec3 enormal = normal_from_depth(depth, gl_TexCoord[0].xy);
    
    float radius_depth = radius/depth;
    float occlusion = 0.0;
    for(int i=0; i < samples; i++) {
        
        vec3 ray = radius_depth * reflect(sample_sphere[i], random);
        vec3 hemi_ray = eposition + sign(dot(ray,enormal)) * ray;
        
        float occ_depth = texture2D(tDepth, clamp(hemi_ray.xy, 0.0, 1.0)).r;
        float difference = depth - occ_depth;
        
        occlusion += step(falloff, difference) * (1.0-smoothstep(falloff, area, difference));
    }
    
    float ao = 1.0 - total_strength * occlusion * (1.0 / samples);
    ao = clamp(ao + base, 0.0, 1.0);
    gl_FragColor = vec4(vec3(ao), 1.0);
}
