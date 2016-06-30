#version 150

uniform sampler2D image;
uniform float threthould;

in vec2 texCoordVarying;

out vec4 fragColor;

vec3 GetBloomColor(vec3 color, float threthould, float intensity) {
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > threthould){
        return color * min(brightness - threthould, 1.0) * intensity;
    }else{
        return vec3(0.0, 0.0, 0.0);
    }
}
void main() {
    vec4 color = texture(image, texCoordVarying);
    fragColor = vec4(GetBloomColor(color.rgb, threthould, 1.0), color.a);
}