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