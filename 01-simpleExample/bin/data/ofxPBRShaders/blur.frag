#version 150

uniform sampler2DArray depthMap;
uniform int index;
uniform bool horizontal;
uniform vec2 resolution;
                                   
in vec2 texCoordVarying;
out vec4 fragColor;
                                   
const int numKurnel = 5;
float weight[5] = float[] (0.20236, 0.179044, 0.124009, 0.067234, 0.028532);

void main(){
    vec2 st = texCoordVarying;
    vec2 tex_offset = 1.0 / resolution; // gets size of single texel
	float rawColor = texture(depthMap, vec3(st, index)).r;
    float result = rawColor * weight[0]; // current fragment's contribution
    if(horizontal){
        for(int i = 1; i < numKurnel; ++i){
            result += texture(depthMap, vec3(st + vec2(tex_offset.x * i, 0.0), index)).r * weight[i];
            result += texture(depthMap, vec3(st - vec2(tex_offset.x * i, 0.0), index)).r * weight[i];
        }
    } else {
        for(int i = 1; i < numKurnel; ++i){
            result += texture(depthMap, vec3(st + vec2(0.0, tex_offset.y * i), index)).r * weight[i];
            result += texture(depthMap, vec3(st - vec2(0.0, tex_offset.y * i), index)).r * weight[i];
        }
    }
    gl_FragDepth = rawColor;//result;
	fragColor = vec4(result, 0.0, 0.0, 1.0);
}