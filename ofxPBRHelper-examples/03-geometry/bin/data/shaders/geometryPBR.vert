#version 330

#define MAX_LIGHTS 8

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform vec4 globalColor;

in vec4 position;
in vec3 normal;
in vec2 texcoord;
in vec4 color;

out vec4 positionForGeom;
out vec3 normalForGeom;
out vec2 texCoordForGeom;
out vec4 colorForGeom;

void main() {
    positionForGeom = position;
    normalForGeom = normal;
    texCoordForGeom = texcoord;
    colorForGeom = color;
    gl_Position = position;
}