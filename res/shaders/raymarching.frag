#version 460

layout(location = 0) out vec4 outColor;

layout(std430, set = 0, binding = 0) readonly buffer SphereBuffer {
    vec4 spheres[]; // xyz -> center, w -> radius
};

layout(push_constant) uniform PushConstants {
    vec4  player;            // xyz = position, w = fov
    vec2  resolution;
    float boundingRadius;
    uint  sphereCount;
} pc;

float sd_sphere(vec3 p, vec4 s){
    return sqrt((s[0] - p[0])*(s[0] - p[0]) + (s[1] - p[1])*(s[1] - p[1]) + (s[2] - p[2])*(s[2] - p[2])) - s[3];

}

float sceneSDF(vec3 p) {
    float d = 1e20;
    for (uint i = 0; i < pc.sphereCount; i++)
        d = min(d, sd_sphere(p, spheres[i]));
    return d;
}
void main() {
    vec2 uv = (gl_FragCoord.xy - 0.5 * pc.resolution) / pc.resolution.y;
    vec3 rayDir = normalize(vec3(uv, 1.0 / tan(pc.player.w * 0.5)));
    vec3 ro = pc.player.xyz;

    float t = 0.0;
    for (int step = 0; step < 128 && t < pc.boundingRadius; step++) {
        float d = sceneSDF(ro + rayDir * t);
        if (d < 0.001) { outColor = vec4(1.0 - t / pc.boundingRadius); return; }
        t += d;
    }
    outColor = vec4(0.0);
}