#include "ofxPBRHelper.h"

ofxPBRHelper::~ofxPBRHelper()
{
	gui.close();
}

void ofxPBRHelper::setup(ofxPBR * pbr, string folderPath, bool usingOtherGui)
{
    this->usingOtherGui = usingOtherGui;
	if(!usingOtherGui) gui.setup();
	this->pbr = pbr;
	this->folderPath = folderPath;

	ofDisableArbTex();

	files = ofxPBRFiles::getInstance();

	loadJsonFiles();
	if (jsonFiles.size() != 0) {
		jsons.clear();
		for (auto & j : jsonFiles) {
			ofxJSONElement json;
			json.openLocal(folderPath + "/" + j + ".json");
			jsons.push_back(json);
		}
		settings = jsons[0];
		currentJsonIndex = 0;
		setPBRFromJson();
	}
	else {
		currentJsonIndex = -1;
    }
	textureLoaded = false;

}

void ofxPBRHelper::drawGui(ofCamera* cam)
{
    this->cam = cam;
    if(!usingOtherGui){
        gui.begin();
        ImGui::Begin("ofxPBRHelper");
    }
    
    drawGeneralGui();
    drawCubeMapsGui();
    drawLightsGui();
    drawMaterialsGui();
    drawTexturesGui();
    
    if(!usingOtherGui){
        ImGui::End();
        gui.end();
    }
}

void ofxPBRHelper::drawGeneralGui(){
    if(ImGui::CollapsingHeader("General", 0, true, true)){

		// save json button
        if (ImGui::Button("save")) {
            if (settings.isNull() == true && currentJsonIndex == -1) {
                ImGui::OpenPopup("Save As ...");
            }else{
                saveJson(jsonFiles[currentJsonIndex]);
            }
        }
        ImGui::SameLine();

		// save new json button
        if (ImGui::Button("save as...")) {
            ImGui::OpenPopup("Save As ...");
        }

        if (ImGui::BeginPopupModal("Save As ...", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char newJsonName[64] = ""; ImGui::InputText("file name", newJsonName, 64);
            if (ImGui::Button("save", ImVec2(120, 0))) {
                saveJson(newJsonName);
                ImGui::CloseCurrentPopup();
                loadJsonFiles();
            }
            ImGui::SameLine();
            if (ImGui::Button("cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }

        // load json button
        if (Combo("load", &currentJsonIndex, jsonFiles)) {
			settings = jsons[currentJsonIndex];
            for (auto m : materials) {
                setMaterialsFromJson(m.first);
            }
            for (auto l : lights) {
                setLightsFromJson(l.first);
            }
			for (auto c : cubeMaps) {
				setCubeMapsFromJson(c.first);
			}
            setPBRFromJson();
        }

        // enable / disable cubemap
        if (ImGui::Checkbox("enable cubeMap", &pbrParams.enableCubeMap)) {
            pbr->enableCubeMap(pbrParams.enableCubeMap);
        }

        // select cubemap
        if (Combo("cubemap", &currentCubeMapIndex, cubeMapKeys)) {
            currentCubeMapKey = cubeMapKeys[currentCubeMapIndex];
            pbrParams.cubeMapName = cubeMapKeys[currentCubeMapIndex];
            pbr->setCubeMap(cubeMaps[currentCubeMapKey].first);
        }

        // select shadow map resolution
		ImGui::InputInt("shadowMap res", &pbrParams.shadowMapRes);
		if (ImGui::Button("apply shadowMap res")) {
			pbr->resizeDepthMap(pbrParams.shadowMapRes);
		}
    }
}

// cubemap gui
void ofxPBRHelper::drawCubeMapsGui(){
    if(ImGui::CollapsingHeader("Cube Map", 0, true, true)){

		// loaded cubemap list
        ImGui::BeginChild("cubeMap list", ImVec2(150, 200), true);
        int cubeMapIndex = 0;
        for (auto elm : cubeMaps) {
            char label[128];
            string name = elm.first;
            sprintf(label, name.c_str());
            if (ImGui::Selectable(label, selectedCubeMap == cubeMapIndex)) {
                selectedCubeMap = cubeMapIndex;
                selectedCubeMapKey = elm.first;
            }
            cubeMapIndex++;
        }
        ImGui::EndChild();
        ImGui::SameLine();
        
        ImGui::PushItemWidth(200);
        ImGui::BeginGroup();
        if (cubeMaps.find(selectedCubeMapKey) != cubeMaps.end()) {

			// open panorama assets window
            if (ImGui::Button("set panorama")) {
                showPanoramaWindow = !showPanoramaWindow;
            }
            
			// load panorama
            if (panoramaLoaded) {
                ofDisableArbTex();
                ofxPBRImage img;
                img.load(currentPanoramaFile.getAbsolutePath());
                img.resize(512, 256);
                img.saveImage(files->getPath() + "/panoramas_small/" + currentPanoramaFile.getFileName());
                ofTexture* texture = new ofTexture();
                *texture = img.getTexture();
                files->panoramas.insert(map<string, ofTexture*>::value_type(currentPanoramaFile.getFileName(), texture));
                
                ofEnableArbTex();
                ofFile::copyFromTo(currentPanoramaFile.getAbsolutePath(), files->getPath() + "/panoramas/" + currentPanoramaFile.getFileName());
                panoramaLoaded = false;
            }
            
			// erase panorama
            if (panoramaErase) {
                files->panoramas[erasePanoramaName]->clear();
                files->panoramas[erasePanoramaName] = nullptr;
                files->panoramas.erase(erasePanoramaName);
                panoramaErase = false;
            }
            
			// show current panorama
            if (cubeMaps[selectedCubeMapKey].first->isAllocated()) {
                ImTextureID env = (ImTextureID)(uintptr_t)cubeMaps[selectedCubeMapKey].first->getPanoramaTexture()->getTextureData().textureID;
                ImGui::Image(env, ImVec2(200, 100));
            }
            
            ofxPBRCubeMap* cubeMap = cubeMaps[selectedCubeMapKey].first;
            CubeMapParams* params = cubeMaps[selectedCubeMapKey].second;
            
			// change cubemap exposure
            if (ImGui::DragFloat("exposure", &params->exposure, 0.1)) {
                params->exposure = fmaxf(params->exposure, 0.0);
                cubeMap->setExposure(params->exposure);
            }
            
			// change cubemap rotation
            if (ImGui::DragFloat("rotation", &params->rotation, 0.005, 0.0, 2 * PI)) {
                cubeMap->setRotation(params->rotation);
            }
            
			//// change cubemap level
   //         if (ImGui::SliderFloat("env level", &params->envronmentLevel, 0.0, 1.0)) {
   //             cubeMap->setEnvLevel(params->envronmentLevel);
   //         }
            
            drawPanoramasGui();
        }
        ImGui::EndGroup();
        ImGui::PopItemWidth();
    }
}

// panorama gui
void ofxPBRHelper::drawPanoramasGui(){
    if (showPanoramaWindow) {
        ImGui::SetNextWindowSize(ofVec2f(600, 300), ImGuiSetCond_FirstUseEver);
        ImGui::Begin("Panorama", &showPanoramaWindow);

		// load panorama image  button
        if (ImGui::Button("load image")) {
            ofFileDialogResult openFileResult = ofSystemLoadDialog("Select a image");
            if (openFileResult.bSuccess) {
                currentPanoramaFile.open(openFileResult.getPath());
                panoramaLoaded = true;
            }
        }
        ImGui::SameLine();

		// select cubemap's face resolution
        const char* res[] = { "128", "256", "512", "1024", "2048" };
        if (ImGui::Combo("resolution", &selectedCubeMapRes, res, 5)) {
            CubeMapParams* p = cubeMaps[selectedCubeMapKey].second;
            switch (selectedCubeMapRes)
            {
                case 0:
                    p->resolution = 128;
                    break;
                case 1:
                    p->resolution = 256;
                    break;
                case 2:
                    p->resolution = 512;
                    break;
                case 3:
                    p->resolution = 1024;
                    break;
                case 4:
                    p->resolution = 2048;
                    break;
                default:
                    break;
            }
        }

		// show all panorama assets
        int index = 0;
        int totalWidth = 0;
        for (auto p : files->panoramas) {
            ofVec2f btnRes = ofVec2f(200, 100);
            totalWidth += btnRes.x;
            totalWidth += ImGui::GetStyle().ItemSpacing.x;
            if (index != 0 && ImGui::GetWindowWidth() > totalWidth) {
                ImGui::SameLine();
            }
            else if (index != 0) {
                totalWidth = btnRes.x + ImGui::GetStyle().ItemSpacing.x;
            }
            
            ImGui::BeginChild(index, ImVec2(200, 125), false);
            ImGui::GetStyle().ItemInnerSpacing = ImVec2(0, 0);

			// show panorama image & set cubemap button
            ImTextureID textureID = (ImTextureID)(uintptr_t)p.second->getTextureData().textureID;
            if (ImGui::ImageButton(textureID, ImVec2(200, 100), ImVec2(0, 0), ImVec2(1, 1), 1.0)) {
                cubeMaps[selectedCubeMapKey].second->url = files->getPath() + "/panoramas/" + p.first;
                cubeMaps[selectedCubeMapKey].first->load(files->getPath() + "/panoramas/" + p.first, cubeMaps[selectedCubeMapKey].second->resolution, true, files->getPath() + "/cubemapCache/");
                showPanoramaWindow = false;
            }

			// right click to popup details
            ImGui::PushID(index);
            if (ImGui::BeginPopupContextItem("detail"))
            {
				// show filename
                string s = p.first;
                ImGui::Text(s.c_str());
                
				// delete panorama button
                if (ImGui::Button("Delete")) {
                    ImGui::CloseCurrentPopup();
                    erasePanoramaName = p.first;
                    panoramaErase = true;
                    ofFile::removeFile(files->getPath() + "/panoramas/" + erasePanoramaName);
                    ofFile::removeFile(files->getPath() + "/panoramas_small/" + erasePanoramaName);
                }
                ImGui::EndPopup();
            }
            ImGui::PopID();

			// show filename
            string s = p.first;
            ImGui::Text(s.c_str());
            ImGui::EndChild();
            index++;
        }
        ImGui::End();
    }
}

void ofxPBRHelper::drawLightsGui(){
    if(ImGui::CollapsingHeader("Lights", 0, true, true)){

		// select lights
        ImGui::BeginChild("light list", ImVec2(150, 600), true);
        int lightIndex = 0;
        for (auto elm : lights) {
            char label[128];
            string name = elm.first;
            sprintf(label, name.c_str());
            if (ImGui::Selectable(label, selectedLight == lightIndex)) {
                selectedLight = lightIndex;
                currentLightKey = elm.first;
            }
            lightIndex++;
        }
        ImGui::EndChild();
        ImGui::SameLine();
        
		// show light parameters
        ImGui::PushItemWidth(200);
        ImGui::BeginGroup();
        if (lights.find(currentLightKey) != lights.end()) {
            ofxPBRLight* light = lights[currentLightKey];
            ofxPBRLightData* lightParam = &lights[currentLightKey]->getParameters();
            
            ImGui::Text(currentLightKey.c_str());

			// set light enable / disable
            if (ImGui::Checkbox("enable", &lightParam->enable)) {
                light->setEnable(lightParam->enable);
            }

			// select light type
            const char* lightType[] = { "directional", "spot", "point", "sky" };
			int type = static_cast<int>(lightParam->lightType);
            if (ImGui::Combo("light type", &type, lightType, 4)) {
                switch (type) {
                    case 0:
                        light->setLightType(LightType_Directional);
                        break;
                    case 1:
                        light->setLightType(LightType_Spot);
                        break;
                    case 2:
                        light->setLightType(LightType_Point);
                        break;
                    case 3:
                        light->setLightType(LightType_Sky);
                        break;
                    default:
                        break;
                }
            }
            
			// light parameters
            if (light->getLightType() != LightType_Sky) {
                
				ofVec3f position = light->getPosition();
				if (ImGui::DragFloat3("position", &position[0])) {
					ofMatrix4x4 transMat = light->getGlobalTransformMatrix();
					transMat.setTranslation(position);
					light->setTransformMatrix(transMat);
				}

				ofVec3f rotation = light->getOrientationQuat().getEuler();
				if (ImGui::DragFloat2("rotate", &rotation[0])) {
					ofQuaternion quatX, quatY;
					quatX.makeRotate(rotation.x, ofVec3f(1.0, 0.0, 0.0));
					quatY.makeRotate(rotation.y, ofVec3f(0.0, 1.0, 0.0));
					ofMatrix4x4 transMat = light->getGlobalTransformMatrix();
					transMat.setRotate(quatX * quatY);
					light->setTransformMatrix(transMat);
				}
                
				// set color
				ofFloatColor color = lightParam->color;
                if (ImGui::ColorEdit3("color", &color[0])) {
                    light->setColor(color);
                }
                
            }
            else {

				// sky light parameters
                if (cubeMaps.find(currentCubeMapKey) != cubeMaps.end() && cubeMaps[currentCubeMapKey].first->isAllocated()) {
                    ofxPBRCubeMap* cubeMap = cubeMaps[currentCubeMapKey].first;

					// set sky light position & color
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
                    ImVec2 canvas_size = ImVec2(256, 128);
                    ImTextureID env = (ImTextureID)(uintptr_t)cubeMap->getPanoramaTexture()->getTextureData().textureID;
                    draw_list->AddImage(env, ImVec2(canvas_pos.x, canvas_pos.y), ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y));
                    draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), ImColor(255, 255, 255));
                    ImGui::InvisibleButton("canvas", canvas_size);
                    if (ImGui::IsItemHovered())
                    {
                        if (ImGui::IsMouseClicked(0) || ImGui::IsMouseDragging(0))
                        {
                            ImVec2 mouse_pos_in_canvas = ImVec2(ImGui::GetIO().MousePos.x - canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y);
                            ofFloatColor c = cubeMap->getColor(cubeMap->getPanoramaTexture()->getWidth() * (mouse_pos_in_canvas.x / canvas_size.x), cubeMap->getPanoramaTexture()->getHeight() * (mouse_pos_in_canvas.y / canvas_size.y));
                            lightParam->color = c;
                            light->setColor(c);
							light->setSkyLightCoordinate(
								-PI / 2 + (mouse_pos_in_canvas.x / canvas_size.x) * 2 * PI,
								(mouse_pos_in_canvas.y / canvas_size.y) * PI);
                        }
                    }
                    
                    ImVec2 circlePos = ImVec2(((PI / 2 + light->getSkyLightLongitude()) / TWO_PI) * canvas_size.x + canvas_pos.x, 
						light->getSkyLightLatitude() / PI * canvas_size.y + canvas_pos.y);
                    draw_list->AddCircleFilled(circlePos, 5, ImColor(255, 255, 255));
                    draw_list->AddCircle(circlePos, 5, ImColor(0, 0, 0));
                    
					// set color
					ofFloatColor color = lightParam->color;
					if (ImGui::DragFloat3("color", &color[0], 0.01)) {
						light->setColor(color);
					}
                    
                }else{
                    ImGui::Text("cubeMap texture is not loaded.");
                }
            }
            
			// set light intensity
			float intensity = lightParam->intensity;
            if (ImGui::DragFloat("intensity", &intensity, 0.1, 0.0, 100.0)) {
                light->setIntensity(intensity);
            }
            
			// set point light radius
            if (light->getLightType() == LightType_Point) {
				float radius = lightParam->pointLightRadius;
                if (ImGui::DragFloat("radius", &radius)) {
                    light->setPointLightRadius(radius);
                }
            }

			// set spot light parameters
            if (light->getLightType() == LightType_Spot) {
				float distance = lightParam->spotLightDistance;
				// set distance
                if (ImGui::DragFloat("distance", &distance)) {
                    light->setSpotLightDistance(distance);
                }

				// set spot light cutoff
				float cutoff = lightParam->spotLightCutoff;
                if (ImGui::DragFloat("cutoff", &cutoff, 0.1, 0.0, 90.0)) {
                    light->setSpotLightCutoff(cutoff);
                }

				// set spot light gradient
				float gradient = lightParam->spotLightGradient;
                if (ImGui::DragFloat("spotFactor", &gradient, 0.1, 0.0, 10.0)) {
                    light->setSpotLightGradient(gradient);
                }
            }
            ImGui::Spacing();

            ImGui::Text("shadow");
            
			// select shadow type
            const char* shadowType[] = { "none", "hard shadow", "soft shadow" };
			int shadow = static_cast<int>(lightParam->shadowType);
            if (ImGui::Combo("shadow type", &shadow, shadowType, 3)) {
                switch (shadow) {
                    case 0:
                        light->setShadowType(ShadowType_None);
                        break;
                    case 1:
                        light->setShadowType(ShadowType_Hard);
                        break;
                    case 2:
                        light->setShadowType(ShadowType_Soft);
                        break;
                    default:
                        break;
                }
            }
            
            if (light->getShadowType() != ShadowType_None) {
                
				// set shadow bias
				float shadowBias = lightParam->shadowBias;
                if (ImGui::DragFloat("shadowBias", &shadowBias, 0.0001, 0.0, 1.0, "%.4f")) {
                    light->setShadowBias(shadowBias);
                }
                
				// set shadow strength
                float shadowStrength = lightParam->shadowStrength;
                if (ImGui::DragFloat("shadowStrength", &shadowStrength, 0.01, 0.000, 1.000, "%.2f")) {
                    light->setShadowStrength(shadowStrength);
                }
            }
            
        }
        
        ImGui::EndGroup();
        ImGui::PopItemWidth();
    }
}

void ofxPBRHelper::drawMaterialDetailGui(ImTextureID imTexId, int index, function<void()> deleteFunc, function<void()> parameterFunc) {
	ImGui::PushID(index);

	if (ImGui::ImageButton(imTexId, ImVec2(40, 40), ImVec2(0, 0), ImVec2(1, 1), 1.0)) {
		showTextureWindow = true;
		currentId = index;
	}

	if (ImGui::BeginPopupContextItem("delete"))
	{
		if (ImGui::Button("Delete")) {
			ImGui::CloseCurrentPopup();
			deleteFunc();
		}
		ImGui::EndPopup();
	}

	ImGui::SameLine();
	ImGui::BeginGroup();
	parameterFunc();
	ImGui::EndGroup();
	ImGui::PopID();
}

void ofxPBRHelper::drawMaterialsGui(){
    if(ImGui::CollapsingHeader("Materials", 0, true, true)){
        ImGui::BeginChild("material list", ImVec2(150, 600), true);
        
        int materialIndex = 0;
        for (auto elm : materials) {
            char label[128];
            string name = elm.first;
            sprintf(label, name.c_str(), materialIndex);
            if (ImGui::Selectable(label, selectedMaterial == materialIndex)) {
                selectedMaterial = materialIndex;
                currentMaterialKey = elm.first;
            }
            materialIndex++;
        }
        
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::PushItemWidth(200);
        ImGui::BeginGroup();

		if (materials.find(currentMaterialKey) != materials.end()) {
            ofxPBRMaterial* material = materials[currentMaterialKey].first;
            MaterialParams* params = materials[currentMaterialKey].second;
            
            // base color
            ImGui::Text("base color");

            ImTextureID baseColorId;
            if (material->baseColorMap != nullptr && material->baseColorMap->isAllocated()) {
                baseColorId = (ImTextureID)(uintptr_t)material->baseColorMap->getTextureData().textureID;
            }else{
                baseColorId = 0x0;
                material->enableBaseColorMap = false;
            }

			drawMaterialDetailGui(
				baseColorId, 
				0,
				[&]() {
					material->enableBaseColorMap = false;
					material->baseColorMap = nullptr;
					params->baseColorTex = "";
				},
				[&]() {
					ImGui::ColorEdit4("", &material->baseColor[0]);
					ImGui::Checkbox("enable texture", &material->enableBaseColorMap);
				});

            ImGui::Spacing();
            
            // roughness
            ImGui::Text("roughness");

            ImTextureID roughnessId;
            if (material->roughnessMap != nullptr && material->roughnessMap->isAllocated()) {
                roughnessId = (ImTextureID)(uintptr_t)material->roughnessMap->getTextureData().textureID;
            }
            else {
                roughnessId = 0x0;
                material->enableRoughnessMap = false;
            }

			drawMaterialDetailGui(
				roughnessId,
				1,
				[&]() {
				material->enableRoughnessMap = false;
				material->roughnessMap = nullptr;
				params->roughnessTex = "";
			},
				[&]() {
				ImGui::SliderFloat("", &material->roughness, 0.0, 1.0);
				ImGui::Checkbox("enable texture", &material->enableRoughnessMap);
			});
            
            // metallic
            ImGui::Text("metallic");

            ImTextureID metallicId;
            if (material->metallicMap != nullptr && material->metallicMap->isAllocated()) {
                metallicId = (ImTextureID)(uintptr_t)material->metallicMap->getTextureData().textureID;
            }
            else {
                metallicId = 0x0;
                material->enableMetallicMap = false;
            }

			drawMaterialDetailGui(
				metallicId,
				2,
				[&]() {
				material->enableMetallicMap = false;
				material->metallicMap = nullptr;
				params->metallicTex = "";
			},
				[&]() {
				ImGui::SliderFloat("", &material->metallic, 0.0, 1.0);
				ImGui::Checkbox("enable texture", &material->enableMetallicMap);
			});
            
            // normal
            ImGui::Text("normal");

            ImTextureID normalId;
            if (material->normalMap != nullptr && material->normalMap->isAllocated()) {
                normalId = (ImTextureID)(uintptr_t)material->normalMap->getTextureData().textureID;
            }
            else {
                normalId = 0x0;
                material->enableNormalMap = false;
            }

			drawMaterialDetailGui(
				normalId,
				3,
				[&]() {
				material->enableNormalMap = false;
				material->normalMap = nullptr;
				params->normalTex = "";
			},
				[&]() {
				ImGui::Checkbox("enable texture", &material->enableNormalMap);
				ImGui::SliderFloat("", &material->normalVal, 0.0, 1.0);
			});
            
            // occlusion
            ImGui::Text("occlusion");

            ImTextureID occlusionId;
            if (material->occlusionMap != nullptr && material->occlusionMap->isAllocated()) {
                occlusionId = (ImTextureID)(uintptr_t)material->occlusionMap->getTextureData().textureID;
            }
            else {
                occlusionId = 0x0;
                material->enableOcclusionMap = false;
            }

			drawMaterialDetailGui(
				occlusionId,
				4,
				[&]() {
				material->enableOcclusionMap = false;
				material->occlusionMap = nullptr;
				params->occlusionTex = "";
			},
				[&]() {
				ImGui::Checkbox("enable texture", &material->enableOcclusionMap);
			});
            
            // emission
            ImGui::Text("emission");

            ImTextureID emissionId;
            if (material->emissionMap != nullptr && material->emissionMap->isAllocated()) {
                emissionId = (ImTextureID)(uintptr_t)material->emissionMap->getTextureData().textureID;
            }
            else {
                emissionId = 0x0;
                material->enableEmissionMap = false;
            }

			drawMaterialDetailGui(
				emissionId,
				5,
				[&]() {
				material->enableEmissionMap = false;
				material->emissionMap = nullptr;
				params->emissionTex = "";
			},
				[&]() {
				ImGui::Checkbox("enable texture", &material->enableEmissionMap);
			});

            // detail base color
            ImGui::Text("detail base color");

            ImTextureID detailBaseColorId;
            if (material->detailBaseColorMap != nullptr && material->detailBaseColorMap->isAllocated()) {
                detailBaseColorId = (ImTextureID)(uintptr_t)material->detailBaseColorMap->getTextureData().textureID;
            }
            else {
                detailBaseColorId = 0x0;
                material->enableDetailBaseColorMap = false;
            }

			drawMaterialDetailGui(
				detailBaseColorId,
				6,
				[&]() {
				material->enableDetailBaseColorMap = false;
				material->detailBaseColorMap = nullptr;
				params->detailBaseColorTex = "";
			},
				[&]() {
				ImGui::Checkbox("enable texture", &material->enableDetailBaseColorMap);
			});
            
            // detail normal
            ImGui::Text("detail normal");

            ImTextureID detailNormalId;
            if (material->detailNormalMap != nullptr && material->detailNormalMap->isAllocated()) {
                detailNormalId = (ImTextureID)(uintptr_t)material->detailNormalMap->getTextureData().textureID;
            }
            else {
                detailNormalId = 0x0;
                material->enableDetailNormalMap = false;
            }

			drawMaterialDetailGui(
				detailNormalId,
				7,
				[&]() {
				material->enableDetailNormalMap = false;
				material->detailNormalMap = nullptr;
				params->detailNormalTex = "";
			},
				[&]() {
				ImGui::Checkbox("enable texture", &material->enableDetailNormalMap);
			});
            
            ImGui::Text("texture repeat");
            ImGui::PushID(8);
            ImGui::DragFloat2("", &material->textureRepeat[0]);
            ImGui::PopID();
            
            ImGui::Text("detail texture repeat");
            ImGui::PushID(9);
            ImGui::DragFloat2("", &material->detailTextureRepeat[0]);
            ImGui::PopID();
        }

		ImGui::EndGroup();
        ImGui::PopItemWidth();
    }
    
    if (textureLoaded) {
        ofDisableArbTex();
        ofTexture* texture = new ofTexture();
        files->textures.insert(map<string, ofTexture*>::value_type(currentFile.getFileName(), texture));
        ofxPBRImage img;
        img.load(currentFile.getAbsolutePath());
        *files->textures[currentFile.getFileName()] = img.getTexture();
        ofEnableArbTex();
        
        textureLoaded = false;
    }
    
    if (texErase) {
        files->textures[eraseTexName]->clear();
        files->textures[eraseTexName] = nullptr;
        files->textures.erase(eraseTexName);
        texErase = false;
    }
}

void ofxPBRHelper::drawTexturesGui(){
	if (showTextureWindow) {
		if (materials.find(currentMaterialKey) != materials.end()) {
			ofxPBRMaterial* material = materials[currentMaterialKey].first;
			MaterialParams* params = materials[currentMaterialKey].second;

			ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("Textures", &showTextureWindow);
			if (ImGui::Button("load texture")) {
				ofFileDialogResult openFileResult = ofSystemLoadDialog("Select a image");
				if (openFileResult.bSuccess) {
					if (files->textures.find(openFileResult.getName()) == files->textures.end()) {
						currentFile.open(openFileResult.getPath());
						ofFile::copyFromTo(currentFile.getAbsolutePath(), files->getPath() + "/textures/" + currentFile.getFileName());
						textureLoaded = true;
					}
				}
			}

			int totalWidth = -ImGui::GetStyle().ItemSpacing.x;
			int index = 0;

			ImGui::BeginChild("textures");
			for (auto t : files->textures)
			{
				ofVec2f btnRes = ofVec2f(100, 100);
				totalWidth += btnRes.x;
				totalWidth += ImGui::GetStyle().ItemSpacing.x;
				if (index != 0 && ImGui::GetWindowWidth() > totalWidth) {
					ImGui::SameLine();
				}
				else if (index != 0) {
					totalWidth = btnRes.x;
				}

				ImTextureID textureID = (ImTextureID)(uintptr_t)t.second->getTextureData().textureID;
				ImGui::BeginChild(index, ImVec2(100, 125), false);
				//ImGui::GetStyle().ItemInnerSpacing = ImVec2(0, 0);
				if (ImGui::ImageButton(textureID, ImVec2(btnRes.x, btnRes.y), ImVec2(0, 0), ImVec2(1, 1), 1.0)) {

					switch (currentId) {
					case 0:
						material->baseColorMap = t.second;
						params->baseColorTex = t.first;
						material->enableBaseColorMap = true;
						break;

					case 1:
						material->roughnessMap = t.second;
						params->roughnessTex = t.first;
						material->enableRoughnessMap = true;
						break;

					case 2:
						material->metallicMap = t.second;
						params->metallicTex = t.first;
						material->enableMetallicMap = true;
						break;

					case 3:
						material->normalMap = t.second;
						params->normalTex = t.first;
						material->enableNormalMap = true;
						break;

					case 4:
						material->occlusionMap = t.second;
						params->occlusionTex = t.first;
						material->enableOcclusionMap = true;
						break;

					case 5:
						material->emissionMap = t.second;
						params->emissionTex = t.first;
						material->enableEmissionMap = true;
						break;

					case 6:
						material->detailBaseColorMap = t.second;
						params->detailBaseColorTex = t.first;
						material->enableDetailBaseColorMap = true;
						break;

					case 7:
						material->detailNormalMap = t.second;
						params->detailNormalTex = t.first;
						material->enableDetailNormalMap = true;
						break;

					default:
						break;
					}
					showTextureWindow = false;
				}
				ImGui::PushID(index);

				if (ImGui::BeginPopupContextItem("detail"))
				{
					string s = t.first;
					ImGui::Text(s.c_str());

					string size = ofToString(t.second->getWidth()) + " x " + ofToString(t.second->getHeight());
					ImGui::Text(size.c_str());

					if (ImGui::Button("Delete")) {
						ImGui::CloseCurrentPopup();
						eraseTexName = t.first;
						texErase = true;
						ofFile::removeFile(files->getPath() + "/textures/" + eraseTexName);
					}
					ImGui::EndPopup();
				}
				ImGui::PopID();

				ImGui::EndChild();
				index++;
			}
			ImGui::EndChild();
			ImGui::End();
		}
	}
}

void ofxPBRHelper::drawLights(){
    for(auto l : lights){
        ofxPBRLight* light = l.second;
        float radius = 0.0;
		ofPushStyle();
		ofSetColor(255, 100);
        switch (light->getLightType()) {
            case LightType_Sky:
            case LightType_Directional:
                ofPushStyle();
                ofNoFill();
                ofPushMatrix();
                ofMultMatrix(light->getGlobalTransformMatrix());
                ofDrawBox(0, 0, -light->getFarClip() / 2, pbr->getDepthMapResolution(), pbr->getDepthMapResolution(), light->getFarClip());
                radius = pbr->getDepthMapResolution() / sqrt(2);
                for(int i=0;i<4;i++){
                    ofDrawLine(0, 0, 0, radius * sin(PI / 4 + i * PI / 2), radius * cos(PI / 4 + i * PI / 2), 0);
                }
                ofPopMatrix();
                ofPopStyle();
                break;
                
            case LightType_Point:
                ofPushStyle();
                ofNoFill();
                ofPushMatrix();
                ofSetCircleResolution(64);
                ofTranslate(light->getGlobalTransformMatrix().getTranslation());
                ofPushMatrix();
                ofDrawCircle(0, 0, 0, light->getPointLightRadius());
                ofRotateX(90);
                ofDrawCircle(0, 0, 0, light->getPointLightRadius());
                ofRotateY(90);
                ofDrawCircle(0, 0, 0, light->getPointLightRadius());
                ofPopMatrix();
                ofPopMatrix();
                ofPopStyle();
                break;
                
            case LightType_Spot:
                ofPushStyle();
                ofNoFill();
                ofPushMatrix();
                ofMultMatrix(light->getGlobalTransformMatrix());
                ofPushMatrix();
                radius = light->getSpotLightDistance() * tan(ofDegToRad(light->getSpotLightCutoff()));
                for(int i=0;i<4;i++){
                    ofDrawLine(0, 0, 0, radius * sin(PI / 4 + i * PI / 2), radius * cos(PI / 4 + i * PI / 2), -light->getSpotLightDistance());
                    ofDrawLine(radius * sin(PI / 4 + i * PI / 2), radius * cos(PI / 4 + i * PI / 2), -light->getSpotLightDistance(),
                               radius * sin(PI / 4 + ((i + 1) % 4) * PI / 2), radius * cos(PI / 4 + ((i + 1) % 4) * PI / 2), -light->getSpotLightDistance());
                }
                ofPopMatrix();
                ofPopMatrix();
                ofPopStyle();
                break;
                
            default:
                break;
        }
		ofPopStyle();
    }
}

void ofxPBRHelper::addLight(ofxPBRLight * light, string name)
{
    lights.insert(map<string, ofxPBRLight*>::value_type(name, light));
    pbr->addLight(light);
	light->setup();
    setLightsFromJson(name);
}

void ofxPBRHelper::addMaterial(ofxPBRMaterial * material, string name)
{
    MaterialParams* params = new MaterialParams;
    materials.insert(map<string, pair<ofxPBRMaterial*, MaterialParams*>>::value_type(name, pair<ofxPBRMaterial*, MaterialParams*>(material, params)));
	setMaterialsFromJson(name);
}

void ofxPBRHelper::addCubeMap(ofxPBRCubeMap * cubeMap, string name)
{
    CubeMapParams* params = new CubeMapParams;
	cubeMaps.insert(map<string, pair<ofxPBRCubeMap*, CubeMapParams*>>::value_type(name, pair<ofxPBRCubeMap*, CubeMapParams*>(cubeMap, params)));
	cubeMapKeys.push_back(name);
	setCubeMapsFromJson(name);
}

void ofxPBRHelper::addSharedMaterial(map<string, pair<ofxPBRMaterial*, MaterialParams*>> sharedMaterials)
{
    for(auto material : sharedMaterials){
        materials.insert(map<string, pair<ofxPBRMaterial*, MaterialParams*>>::value_type(material.first, pair<ofxPBRMaterial*, MaterialParams*>(material.second.first, material.second.second)));
        setMaterialsFromJson(material.first);
    }
}

void ofxPBRHelper::addSharedCubeMap(map<string, pair<ofxPBRCubeMap*, CubeMapParams*>> sharedCubeMaps)
{
    for(auto cubeMap : sharedCubeMaps){
        cubeMaps.insert(map<string, pair<ofxPBRCubeMap*, CubeMapParams*>>::value_type(cubeMap.first, pair<ofxPBRCubeMap*, CubeMapParams*>(cubeMap.second.first, cubeMap.second.second)));
        cubeMapKeys.push_back(cubeMap.first);
        setCubeMapsFromJson(cubeMap.first);
    }
}

// JSON

void ofxPBRHelper::loadJsonFiles()
{
	ofDirectory dir;
	dir.open(folderPath);
	jsonFiles.clear();
	for (auto file : dir.getFiles()) {
		if (file.getExtension() == "json") {
			jsonFiles.push_back(file.getBaseName());
		}
	}
}

void ofxPBRHelper::saveJson(string fileName)
{
	settings["pbr"]["cubeMapName"] = pbrParams.cubeMapName;
    settings["pbr"]["enableCubeMap"] = pbrParams.enableCubeMap;
	settings["pbr"]["enableEnvironment"] = pbrParams.enableEnvironment;
	settings["pbr"]["shadowMapRes"] = pbrParams.shadowMapRes;

	Json::Value cubeMapJson;
	for (auto cubeMap : cubeMaps) {
		ofxPBRCubeMap* c = cubeMap.second.first;
		CubeMapParams* p = cubeMap.second.second;
		cubeMapJson[cubeMap.first]["resolution"] = p->resolution;
		cubeMapJson[cubeMap.first]["exposure"] = p->exposure;
		cubeMapJson[cubeMap.first]["rotation"] = p->rotation;
		cubeMapJson[cubeMap.first]["url"] = p->url;
		cubeMapJson[cubeMap.first]["envronmentLevel"] = p->envronmentLevel;

	}
	settings["cubeMap"] = cubeMapJson;

	Json::Value materialJson;
	for (auto material : materials) {
		ofxPBRMaterial* m = material.second.first;
		MaterialParams* p = material.second.second;
		materialJson[material.first]["baseColor"]["r"] = m->baseColor.r;
		materialJson[material.first]["baseColor"]["g"] = m->baseColor.g;
		materialJson[material.first]["baseColor"]["b"] = m->baseColor.b;
		materialJson[material.first]["baseColor"]["a"] = m->baseColor.a;
		materialJson[material.first]["baseColorTex"] = p->baseColorTex;
		materialJson[material.first]["enableBaseColorMap"] = m->enableBaseColorMap;

		materialJson[material.first]["roughnessTex"] = p->roughnessTex;
		materialJson[material.first]["enableRoughnessMap"] = m->enableRoughnessMap;
		materialJson[material.first]["roughness"] = m->roughness;

		materialJson[material.first]["metallicTex"] = p->metallicTex;
		materialJson[material.first]["enableMetallicMap"] = m->enableMetallicMap;
		materialJson[material.first]["metallic"] = m->metallic;

		materialJson[material.first]["normalTex"] = p->normalTex;
		materialJson[material.first]["enableNormalMap"] = m->enableNormalMap;
		materialJson[material.first]["normalVal"] = m->normalVal;

		materialJson[material.first]["occlusionTex"] = p->occlusionTex;
		materialJson[material.first]["enableOcclusionMap"] = m->enableOcclusionMap;

		materialJson[material.first]["emissionTex"] = p->emissionTex;
		materialJson[material.first]["enableEmissionMap"] = m->enableEmissionMap;

		materialJson[material.first]["detailBaseColorTex"] = p->detailBaseColorTex;
		materialJson[material.first]["enableDetailBaseColorMap"] = m->enableDetailBaseColorMap;

		materialJson[material.first]["detailNormalTex"] = p->detailNormalTex;
		materialJson[material.first]["enableDetailNormalMap"] = m->enableDetailNormalMap;

		materialJson[material.first]["textureRepeat"]["x"] = m->textureRepeat.x;
		materialJson[material.first]["textureRepeat"]["y"] = m->textureRepeat.y;

		materialJson[material.first]["detailTextureRepeat"]["x"] = m->detailTextureRepeat.x;
		materialJson[material.first]["detailTextureRepeat"]["y"] = m->detailTextureRepeat.y;
	}
	settings["material"] = materialJson;

	Json::Value lightJson;
	for (auto light : lights) {
		ofxPBRLight* l = light.second;
		ofxPBRLightData* p = &l->getParameters();
		Json::Value json;

		json["enable"] = p->enable;
		json["lightType"] = p->lightType;
		json["intensity"] = p->intensity;

		for (int i = 0; i < 4; i++) {
			json["matrix"][0 + i * 4] = l->getGlobalTransformMatrix().getRowAsVec4f(i).x;
			json["matrix"][1 + i * 4] = l->getGlobalTransformMatrix().getRowAsVec4f(i).y;
			json["matrix"][2 + i * 4] = l->getGlobalTransformMatrix().getRowAsVec4f(i).z;
			json["matrix"][3 + i * 4] = l->getGlobalTransformMatrix().getRowAsVec4f(i).w;
		}

		json["color"][0] = p->color.r;
		json["color"][1] = p->color.g;
		json["color"][2] = p->color.b;

		json["shadowType"] = p->shadowType;
		json["shadowBias"] = p->shadowBias;
		json["shadowStrength"] = p->shadowStrength;

		json["spotLightGradient"] = p->spotLightGradient;
		json["spotLightCutoff"] = p->spotLightCutoff;

		json["skyLightLatitude"] = p->skyLightLatitude;
		json["skyLightLongitude"] = p->skyLightLongitude;
		json["skyLightAngle"] = p->skyLightAngle;

		lightJson[light.first] = json;
	}
	settings["light"] = lightJson;
	jsons.push_back(settings);
	settings.save(folderPath + "/" + fileName + ".json");
}

void ofxPBRHelper::setMaterialsFromJson(string materialName)
{
	ofxPBRMaterial* material = materials[materialName].first;
	MaterialParams* params = materials[materialName].second;
	if (settings.isNull() == false && settings["material"][materialName].isNull() == false) {
		Json::Value m = settings["material"][materialName];
		material->baseColor = ofFloatColor(
			m["baseColor"]["r"].asFloat(),
			m["baseColor"]["g"].asFloat(),
			m["baseColor"]["b"].asFloat(),
			m["baseColor"]["a"].asFloat()
		);
		string baseColorTex = m["baseColorTex"].asString();
		if (files->textures.find(baseColorTex) != files->textures.end()) {
			material->baseColorMap = files->textures[baseColorTex];
			params->baseColorTex = baseColorTex;
		}
		material->enableBaseColorMap = m["enableBaseColorMap"].asBool();

		string roughnessTex = m["roughnessTex"].asString();
		if (files->textures.find(roughnessTex) != files->textures.end()) {
			material->roughnessMap = files->textures[roughnessTex];
			params->roughnessTex = roughnessTex;
		}
		material->enableRoughnessMap = m["enableRoughnessMap"].asBool();
		material->roughness = m["roughness"].asFloat();

		string metallicTex = m["metallicTex"].asString();
		if (files->textures.find(metallicTex) != files->textures.end()) {
			material->metallicMap = files->textures[metallicTex];
			params->metallicTex = metallicTex;
		}
		material->enableMetallicMap = m["enableMetallicMap"].asBool();
		material->metallic = m["metallic"].asFloat();

		string normalTex = m["normalTex"].asString();
		if (files->textures.find(normalTex) != files->textures.end()) {
			material->normalMap = files->textures[normalTex];
			params->normalTex = normalTex;
		}
		material->enableNormalMap = m["enableNormalMap"].asBool();
		material->normalVal = m["normalVal"].asFloat();

		string occlusionTex = m["occlusionTex"].asString();
		if (files->textures.find(occlusionTex) != files->textures.end()) {
			material->occlusionMap = files->textures[occlusionTex];
			params->occlusionTex = occlusionTex;
		}
		material->enableOcclusionMap = m["enableOcclusionMap"].asBool();

		string emissionTex = m["emissionTex"].asString();
		if (files->textures.find(emissionTex) != files->textures.end()) {
			material->emissionMap = files->textures[emissionTex];
			params->emissionTex = emissionTex;
		}
		material->enableEmissionMap = m["enableEmissionMap"].asBool();

		string detailBaseColorTex = m["detailBaseColorTex"].asString();
		if (files->textures.find(detailBaseColorTex) != files->textures.end()) {
			material->detailBaseColorMap = files->textures[detailBaseColorTex];
			params->detailBaseColorTex = detailBaseColorTex;
		}
		material->enableDetailBaseColorMap = m["enableDetailBaseColorMap"].asBool();

		string detailNormalTex = m["detailNormalTex"].asString();
		if (files->textures.find(detailNormalTex) != files->textures.end()) {
			material->detailNormalMap = files->textures[detailNormalTex];
			params->detailNormalTex = detailNormalTex;
		}
		material->enableDetailNormalMap = m["enableDetailNormalMap"].asBool();

		material->textureRepeat = ofVec2f(
			m["textureRepeat"]["x"].asFloat(),
			m["textureRepeat"]["y"].asFloat());

		material->detailTextureRepeat = ofVec2f(
			m["detailTextureRepeat"]["x"].asFloat(),
			m["detailTextureRepeat"]["y"].asFloat());
	}
}

void ofxPBRHelper::setLightsFromJson(string lightName)
{
	if (lights.find(lightName) != lights.end()) {
		ofxPBRLight* light = lights[lightName];

		if (settings.isNull() == false && settings["light"][lightName].isNull() == false) {
			light->setup();

			Json::Value p = settings["light"][lightName];
			ofxPBRLightData params;
			params.enable = p["enable"].asBool();
			params.intensity = p["intensity"].asFloat();
			params.lightType = static_cast<LightType>(p["lightType"].asInt());

			params.color.r = p["color"][0].asFloat();
			params.color.g = p["color"][1].asFloat();
			params.color.b = p["color"][2].asFloat();

			float matValue[16];;
			for (int i = 0; i < 16; i++) {
				matValue[i] = p["matrix"][i].asFloat();
			}
			ofMatrix4x4 transformMat;
			transformMat.set(&matValue[0]);
			light->setTransformMatrix(transformMat);

			params.shadowType = static_cast<ShadowType>(p["shadowType"].asInt());
			params.shadowBias = p["shadowBias"].asFloat();
			params.shadowStrength = p["shadowStrength"].asFloat();

			params.spotLightGradient = p["spotLightGradient"].asFloat();
			params.spotLightCutoff = p["spotLightCutoff"].asFloat();

			params.skyLightLatitude = p["skyLightLatitude"].asFloat();
			params.skyLightLongitude = p["skyLightLongitude"].asFloat();
			params.skyLightAngle = p["skyLightAngle"].asFloat();

			light->setParameters(params);
		}
	}
}

void ofxPBRHelper::setCubeMapsFromJson(string cubeMapName)
{
	ofxPBRCubeMap* cubeMap = cubeMaps[cubeMapName].first;
	CubeMapParams* params = cubeMaps[cubeMapName].second;
	if (settings.isNull() == false && settings["cubeMap"][cubeMapName].isNull() == false) {
		Json::Value c = settings["cubeMap"][cubeMapName];
		params->resolution = c["resolution"].asInt();
		params->url = c["url"].asString();
		if (params->url != "") {
			cubeMap->load(c["url"].asString(), params->resolution, true, files->getPath() + "/cubemapCache/");
			if (pbrParams.cubeMapName == cubeMapName && cubeMaps.find(pbrParams.cubeMapName) != cubeMaps.end()) {
				currentCubeMapKey = pbrParams.cubeMapName;
				pbr->setCubeMap(cubeMaps[cubeMapName].first);
				for (int i = 0; i < cubeMapKeys.size(); i++) {
					if (cubeMapKeys[i] == currentCubeMapKey) currentCubeMapIndex = i;
				}
			}
		}
		params->exposure = c["exposure"].asFloat();
		params->rotation = c["rotation"].asFloat();
		params->envronmentLevel = c["envronmentLevel"].asFloat();
	}
	cubeMap->setExposure(params->exposure);
	cubeMap->setRotation(params->rotation);
	cubeMap->setEnvLevel(params->envronmentLevel);
}

void ofxPBRHelper::setPBRFromJson()
{
	if (settings.isNull() == false && settings["pbr"].isNull() == false) {
		Json::Value p = settings["pbr"];
		pbrParams.cubeMapName = p["cubeMapName"].asString();
		if (pbrParams.cubeMapName != "" && cubeMaps.find(pbrParams.cubeMapName) != cubeMaps.end()) {
			currentCubeMapKey = pbrParams.cubeMapName;
			pbr->setCubeMap(cubeMaps[pbrParams.cubeMapName].first);
			for (int i = 0; i < cubeMapKeys.size(); i++) {
				if (cubeMapKeys[i] == currentCubeMapKey) currentCubeMapIndex = i;
			}
		}
        pbrParams.enableCubeMap = p["enableCubeMap"].asBool();
		pbrParams.shadowMapRes = p["shadowMapRes"].asInt();
		pbrParams.enableEnvironment = p["enableEnvironment"].asBool();
	}

    pbr->enableCubeMap(pbrParams.enableCubeMap);
	pbr->resizeDepthMap(pbrParams.shadowMapRes);
}