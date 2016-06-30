#pragma once
#include "ofMain.h"
#include "ofxPBR.h"
#include "ofxImGui.h"
#include "ofxJSON.h"
#include "ofxPBRFiles.h"
#include "ofxPBRImage.h"

class ofxPBRHelper {
public:
	void setup(ofxPBR* pbr, string folderPath, bool enableOtherGui = false);
	void drawGui();
	void addLight(ofxPBRLight* light, string name);
	void addMaterial(ofxPBRMaterial* material, string name);
	void addCubeMap(ofxPBRCubeMap * cubeMap, string name);

private:
	void loadJsonFiles();
	void saveJson(string fileName);
	void setMaterialsFromJson(string materialName);
	void setLightsFromJson(string lightName);
	void setCubeMapsFromJson(string cubeMapName);
	void setPBRFromJson();

	ofxImGui gui;
    bool enableOtherGui;
	ofxJSONElement settings;
	ofxPBR* pbr;
	ofxPBRFiles* files;
	string folderPath;

	struct PBRParams {
        bool enableCubeMap = true;
		string cubeMapName = "";
		bool enableEnvironment = true;
		int shadowMapRes = 1024;
	};
	PBRParams pbrParams;
	int shadowResIndex = 3;

	int currentJsonIndex = -1;
	vector<string> jsonFiles;

	struct CubeMapParams {
		int resolution = 512;
		float exposure = 1.0;
		float rotation = 0.0;
		string url = "";
		float envronmentLevel = 0.0;
	};
	map<string, pair<ofxPBRCubeMap*, CubeMapParams>> cubeMaps;
	vector<string> cubeMapKeys;
	string currentCubeMapKey;
	string selectedCubeMapKey;
	bool panoramaLoaded = false;
	ofFile currentPanoramaFile;
	bool panoramaErase = false;
	string erasePanoramaName;
	bool showPanoramaWindow = false;
	int currentCubeMapIndex = -1;
	int selectedCubeMap = 0;
	int selectedCubeMapRes = 2;

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
	map<string, pair<ofxPBRLight*, LightParams>> lights;
	string currentLightKey;
	int selectedLight = 0;

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
	map<string, pair<ofxPBRMaterial*, MaterialParams>> materials;
	string currentMaterialKey;
	bool textureLoaded = false;
	ofFile currentFile;
	bool texErase = false;
	string eraseTexName;
	bool showTextureWindow = false;
	int currentId = 0;
	int selectedMaterial = 0;

	ofFbo shadowMap;
	ImTextureID depthMapId;

	bool Combo(const char* label, int* current_item, const std::vector<std::string>& items, int height_in_items = -1)
	{
		return ImGui::Combo(
			label, 
			current_item, 
			[](void* data, int idx, const char** out_text) { *out_text = ((const std::vector<std::string>*)data)->at(idx).c_str(); return true; },
			(void*)&items,
			items.size(), 
			height_in_items);
	};
};