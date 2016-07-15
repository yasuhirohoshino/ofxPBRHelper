#version 330

#define MAX_LIGHTS 8

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform vec4 globalColor;

layout(triangles) in;
layout(triangle_strip, max_vertices= 9) out;

in vec4 positionForGeom[3];
in vec3 normalForGeom[3];
in vec2 texCoordForGeom[3];
in vec4 colorForGeom[3];

out vec2 texCoordVarying;
out vec3 normalVarying;
out vec4 positionVarying;
out vec4 colorVarying;

out mat4 normalMatrix;

vec4 position[3];
vec4 centerPos;
vec2 centerTexCoord;

void emitVertex(vec4 vertex, vec3 normal, vec2 texCoord, int index){
    
    normalMatrix = inverse(transpose(modelViewMatrix));
    normalVarying = normal;
    positionVarying = vertex;
    texCoordVarying = texCoord;
    colorVarying = colorForGeom[index];
    gl_Position = modelViewProjectionMatrix * vertex;
    
    EmitVertex();
}

void emitMesh(int index1, int index2){
    vec3 v0 = normalize(position[index1].xyz - centerPos.xyz);
    vec3 v1 = normalize(position[index2].xyz - centerPos.xyz);
    vec3 normal = -normalize(cross(v0,v1));
    
    emitVertex(centerPos, normal, centerTexCoord, 0);
    emitVertex(position[index1], normal, texCoordForGeom[index1], 1);
    emitVertex(position[index2], normal, texCoordForGeom[index2], 2);
    
    EndPrimitive();
}

void main() {
    position[0] = gl_in[0].gl_Position;
    position[1] = gl_in[1].gl_Position;
    position[2] = gl_in[2].gl_Position;
    
    vec3 v0 = normalize(position[2].xyz - position[0].xyz);
    vec3 v1 = normalize(position[1].xyz - position[0].xyz);
    vec3 n = -normalize(cross(v0, v1));
    centerPos = (position[0].xyzw + position[1].xyzw + position[2].xyzw)/vec4(3.0);
    centerPos -= vec4(n * 0.2, 0.0);
    
    centerTexCoord = (texCoordForGeom[0] + texCoordForGeom[1] + texCoordForGeom[2]) / vec2(3.0);
    
    emitMesh(0, 1);
    emitMesh(1, 2);
    emitMesh(2, 0);
    
}