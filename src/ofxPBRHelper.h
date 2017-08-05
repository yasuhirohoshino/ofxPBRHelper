#pragma once
#include "ofMain.h"
#include "ofxPBR.h"
#include "ofxImGui.h"
#include "ofxJSON.h"
#include "ofxPBRFiles.h"
#include "ofxPBRImage.h"
#include "ofxPBRHelperParams.h"

class ofxPBRHelper {
public:
	~ofxPBRHelper();
	void setup(ofxPBR* pbr, string folderPath, bool enableOtherGui = false);
	void drawGui(ofCamera* cam);
    void drawLights();
    
	void addLight(ofxPBRLight* light, string name);
	void addMaterial(ofxPBRMaterial* material, string name);
	void addCubeMap(ofxPBRCubeMap* cubeMap, string name);
    
    void addSharedMaterial(map<string, pair<ofxPBRMaterial*, MaterialParams*>> sharedMaterials);
    void addSharedCubeMap(map<string, pair<ofxPBRCubeMap*, CubeMapParams*>> sharedCubeMaps);

private:
    void drawGeneralGui();
    void drawCubeMapsGui();
    void drawLightsGui();
    void drawMaterialsGui();
    
    void drawPanoramasGui();
    void drawTexturesGui();
    
	void loadJsonFiles();
	void saveJson(string fileName);
	void setMaterialsFromJson(string materialName);
	void setLightsFromJson(string lightName);
	void setCubeMapsFromJson(string cubeMapName);
	void setPBRFromJson();

	ofxImGui::Gui gui;
    bool usingOtherGui;
	ofxJSONElement settings;
	ofxPBR* pbr;
	ofxPBRFiles* files;
	string folderPath;

    // PBR
	PBRParams pbrParams;
	int shadowResIndex = 3;
    
	int currentJsonIndex = -1;
	vector<string> jsonFiles;
	vector<ofxJSONElement> jsons;
    
    // CubeMaps
	map<string, pair<ofxPBRCubeMap*, CubeMapParams*>> cubeMaps;
	ofFile currentPanoramaFile;
	vector<string> cubeMapKeys;
	string currentCubeMapKey;
	string selectedCubeMapKey;
	string erasePanoramaName;
	bool panoramaLoaded = false;
	bool panoramaErase = false;
	bool showPanoramaWindow = false;
	int currentCubeMapIndex = -1;
	int selectedCubeMap = 0;
	int selectedCubeMapRes = 2;
    
    // Lights
	map<string, ofxPBRLight*> lights;
	string currentLightKey;
	int selectedLight = 0;

    // Materials
	map<string, pair<ofxPBRMaterial*, MaterialParams*>> materials;
	ofFile currentFile;
	string currentMaterialKey;
	string eraseTexName;
	bool textureLoaded = false;
	bool texErase = false;
	bool showTextureWindow = false;
	int currentId = 0;
	int selectedMaterial = 0;

	// ImGui extention

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

    ofCamera * cam;
};