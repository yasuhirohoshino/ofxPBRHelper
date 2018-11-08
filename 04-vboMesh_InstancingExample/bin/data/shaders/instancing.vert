#version 410

const int MAX_LIGHTS = 8;

const int MODE_PBR = 0;
const int MODE_DIRECTIONALSHADOW = 1;
const int MODE_SPOTSHADOW = 2;
const int MODE_OMNISHADOW = 3;

uniform int renderMode;

// default uniforms
uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform vec4 globalColor;
uniform mat4 viewMatrix;

// depth camera's view projection matrix
uniform mat4 lightsViewProjectionMatrix;

// in
in vec4 position;
in vec3 normal;
in vec2 texcoord;
in vec4 color;

// out
out vec4 m_positionVarying;
out vec3 normalVarying;
out vec2 texCoordVarying;
out vec4 colorVarying;

out vec3 mv_normalVarying;
out vec4 mv_positionVarying;

uniform samplerBuffer posTex;

void main() {
    vec3 pos = texelFetch(posTex, gl_InstanceID).xyz;
    vec4 v = vec4(position.xyz + pos.xyz, 1.0);

    if(renderMode == MODE_PBR){
		// render pass
		m_positionVarying = inverse(viewMatrix) * modelViewMatrix * v;
        mat4 normalMatrix = inverse(transpose(modelViewMatrix));
        mv_positionVarying = modelViewMatrix * v;
        mv_normalVarying = vec3(mat3(normalMatrix) * normal);
        texCoordVarying = texcoord;
        colorVarying = color;
        gl_Position = modelViewProjectionMatrix * v;
	}else{
		// depth map pass
        m_positionVarying = inverse(viewMatrix) * modelViewMatrix * v;
        gl_Position = lightsViewProjectionMatrix * m_positionVarying;
	}
}