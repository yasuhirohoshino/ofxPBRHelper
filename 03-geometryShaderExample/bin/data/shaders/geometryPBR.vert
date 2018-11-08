#version 410

// in
in vec4 position;
in vec3 normal;
in vec2 texcoord;
in vec4 color;

// out
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