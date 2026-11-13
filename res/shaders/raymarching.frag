#version 460

layout(location = 0) in int type;
layout(location = 1) in mat4 model;
layout(location = 2) in float scale;
layout(location = 3) in vec4 color;

uniform float br;
uniform float fov;
uniform vec2 res;

float sd_sphere(vec3 p, vec3 s, float r){
    return sqrt((s[0] - p[0])*(s[0] - p[0]) + (s[1] - p[1])*(s[1] - p[1]) + (s[2] - p[2])*(s[2] - p[2])) - r;
}

void main(){
    vec3 rayDir;
    vec3 ray;
    float
    while( ){
        for(int i =0; i <sphereCount; i++){

        }
    }
}