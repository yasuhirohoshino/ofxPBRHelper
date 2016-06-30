#version 330

uniform samplerBuffer posTex;

// default
uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform vec4 globalColor;

// passThrough
// in
in vec4 position;
in vec3 normal;
in vec2 texcoord;
in vec4 color;
// out
out vec4 positionVarying;
out vec3 normalVarying;
out vec2 texCoordVarying;
out vec4 colorVarying;

out mat4 normalMatrix;

void main() {
    vec3 pos = texelFetch(posTex, gl_InstanceID).xyz;
    vec4 v = vec4(position.xyz + pos.xyz, 1.0);
    normalMatrix = inverse(transpose(modelViewMatrix));
    
    normalVarying = normal;
    positionVarying = v;
    texCoordVarying = texcoord;
    colorVarying = color;
    gl_Position = modelViewProjectionMatrix * v;
}