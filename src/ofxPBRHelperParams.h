#pragma once
#include "ofMain.h"

struct PBRParams {
    bool enableCubeMap = true;
    string cubeMapName = "";
    bool enableEnvironment = true;
    int shadowMapRes = 1024;
};

struct CubeMapParams {
    int resolution = 512;
    float exposure = 1.0;
    float rotation = 0.0;
    string url = "";
    float envronmentLevel = 0.0;
};

struct LightParams {
    bool enable = true;
    int lightType = 0;
    int shadowType = 0;
    ofVec3f pos = ofVec3f(1000, 1000, 1000);
    ofVec3f target = ofVec3f::zero();
    ofVec2f skyLightCoord = ofVec2f(0, PI / 2);
    float skyLightRadius = 3000;
    float nearClip = 1.0;
    float farClip = 6000;
    float scale = 1.0;
    int depthMapRes;
    ofFloatColor color = ofFloatColor(1.0);
    float intensity = 1.0;
    float radius = 2000;
    float cutoff = 45.0;
    float spotFactor = 0.0;
    float shadowBias = 0.001;
    float softShadowExponent = 75.0;
};

struct MaterialParams {
    string baseColorTex = "";
    string roughnessTex = "";
    string metallicTex = "";
    string normalTex = "";
    string occlusionTex = "";
    string emissionTex = "";
    string detailBaseColorTex = "";
    string detailNormalTex = "";
};