#include "ofxPBRHelper.h"

void ofxPBRHelper::setup(ofxPBR * pbr, string folderPath, bool enableOtherGui)
{
    this->enableOtherGui = enableOtherGui;
	if(!enableOtherGui) gui.setup();
	this->pbr = pbr;
	this->folderPath = folderPath;

	ofDisableArbTex();
	depthMapId = 0;

	files = ofxPBRFiles::getInstance();

	loadJsonFiles();
	if (jsonFiles.size() != 0) {
		settings.openLocal(folderPath + "/" + jsonFiles[0] + ".json");
		currentJsonIndex = 0;
		setPBRFromJson();
    }else{
        currentJsonIndex = -1;
    }
	textureLoaded = false;
}

void ofxPBRHelper::drawGui()
{
    if(!enableOtherGui){
        gui.begin();
        ImGui::Begin("ofxPBRHelper");
    }
    
    drawGeneralGui();
    drawCubeMapsGui();
    drawLightsGui();
    drawMaterialsGui();
    
    if(!enableOtherGui){
        ImGui::End();
        gui.end();
    }
}

void ofxPBRHelper::drawGeneralGui(){
    if(ImGui::CollapsingHeader("General")){
        if (ImGui::Button("save")) {
            if (settings.isNull() == true && currentJsonIndex == -1) {
                ImGui::OpenPopup("Save As ...");
            }else{
                saveJson(jsonFiles[currentJsonIndex]);
            }
        }
        ImGui::SameLine();
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
        
        if (Combo("load", &currentJsonIndex, jsonFiles)) {
            settings.openLocal(folderPath + "/" + jsonFiles[currentJsonIndex] + ".json");
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
        
        if (ImGui::Checkbox("enable cubeMap", &pbrParams.enableCubeMap)) {
            pbr->enableCubeMap(pbrParams.enableCubeMap);
        }
        
        if (Combo("cubemap", &currentCubeMapIndex, cubeMapKeys)) {
            currentCubeMapKey = cubeMapKeys[currentCubeMapIndex];
            pbrParams.cubeMapName = cubeMapKeys[currentCubeMapIndex];
            pbr->setCubeMap(cubeMaps[currentCubeMapKey].first);
        }
        
        const char* shadowRes[] = { "128", "256", "512", "1024", "2048", "4096" };
        if (ImGui::Combo("shadowMap Res", &shadowResIndex, shadowRes, 6)) {
            switch (shadowResIndex)
            {
                case 0:
                    pbrParams.shadowMapRes = 128;
                    break;
                case 1:
                    pbrParams.shadowMapRes = 256;
                    break;
                case 2:
                    pbrParams.shadowMapRes = 512;
                    break;
                case 3:
                    pbrParams.shadowMapRes = 1024;
                    break;
                case 4:
                    pbrParams.shadowMapRes = 2048;
                    break;
                case 5:
                    pbrParams.shadowMapRes = 4096;
                    break;
                    
                default:
                    break;
            }
            pbr->resizeDepthMap(pbrParams.shadowMapRes);
        }
    }
}

void ofxPBRHelper::drawCubeMapsGui(){
    if(ImGui::CollapsingHeader("Cube Map")){
        
        //            int button = 0;
        //            ImGui::RadioButton("unique", &button, 0); ImGui::SameLine();
        //            ImGui::RadioButton("common", &button, 1);
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
        
        ImGui::BeginGroup();
        ImGui::BeginChild("cubeMap params", ImVec2(0, 200));
        if (cubeMaps.find(selectedCubeMapKey) != cubeMaps.end()) {
            if (ImGui::Button("set panorama")) {
                showPanoramaWindow = !showPanoramaWindow;
            }
            
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
            
            if (panoramaErase) {
                files->panoramas[erasePanoramaName]->clear();
                files->panoramas[erasePanoramaName] = nullptr;
                files->panoramas.erase(erasePanoramaName);
                panoramaErase = false;
            }
            
            if (showPanoramaWindow) {
                ImGui::SetNextWindowSize(ofVec2f(600, 300), ImGuiSetCond_FirstUseEver);
                ImGui::Begin("Panorama", &showPanoramaWindow);
                if (ImGui::Button("load image")) {
                    ofFileDialogResult openFileResult = ofSystemLoadDialog("Select a image");
                    if (openFileResult.bSuccess) {
                        currentPanoramaFile.open(openFileResult.getPath());
                        panoramaLoaded = true;
                    }
                }
                ImGui::SameLine();
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
                    ImTextureID textureID = (ImTextureID)(uintptr_t)p.second->getTextureData().textureID;
                    if (ImGui::ImageButton(textureID, ImVec2(200, 100), ImVec2(0, 0), ImVec2(1, 1), 1.0)) {
                        cubeMaps[selectedCubeMapKey].second->url = files->getPath() + "/panoramas/" + p.first;
                        cubeMaps[selectedCubeMapKey].first->load(files->getPath() + "/panoramas/" + p.first, cubeMaps[selectedCubeMapKey].second->resolution, true, files->getPath() + "/cubemapCache/");
                        showPanoramaWindow = false;
                    }
                    ImGui::PushID(index);
                    if (ImGui::BeginPopupContextItem("detail"))
                    {
                        string s = p.first;
                        ImGui::Text(s.c_str());
                        
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
                    string s = p.first;
                    ImGui::Text(s.c_str());
                    ImGui::EndChild();
                    index++;
                }
                ImGui::End();
            }
            
            if (cubeMaps[selectedCubeMapKey].first->isAllocated()) {
                ImTextureID env = (ImTextureID)(uintptr_t)cubeMaps[selectedCubeMapKey].first->getPanoramaTexture()->getTextureData().textureID;
                ImGui::Image(env, ImVec2(200, 100));
            }
            
            ofxPBRCubeMap* cubeMap = cubeMaps[selectedCubeMapKey].first;
            CubeMapParams* params = cubeMaps[selectedCubeMapKey].second;
            
            if (ImGui::DragFloat("exposure", &params->exposure, 0.1)) {
                params->exposure = fmaxf(params->exposure, 0.0);
                cubeMap->setExposure(params->exposure);
            }
            
            if (ImGui::DragFloat("rotation", &params->rotation, 0.005, 0.0, 2 * PI)) {
                cubeMap->setRotation(params->rotation);
            }
            
            if (ImGui::SliderFloat("env level", &params->envronmentLevel, 0.0, 1.0)) {
                cubeMap->setEnvLevel(params->envronmentLevel);
            }
        }
        ImGui::EndChild();
        ImGui::EndGroup();
    }
}

void ofxPBRHelper::drawLightsGui(){
    if(ImGui::CollapsingHeader("Lights")){
        ImGui::BeginChild("light list", ImVec2(150, 400), true);
        
        int lightIndex = 0;
        
        for (auto elm : lights) {
            char label[128];
            string name = elm.first;
            sprintf(label, name.c_str());
            if (ImGui::Selectable(label, selectedLight == lightIndex)) {
                selectedLight = lightIndex;
                currentLightKey = elm.first;
                depthMapId = (ImTextureID)(uintptr_t)pbr->getDepthMap(lightIndex)->getTextureData().textureID;
            }
            lightIndex++;
        }
        depthMapId = (ImTextureID)(uintptr_t)pbr->getDepthMap(selectedLight)->getTextureData().textureID;
        ImGui::EndChild();
        ImGui::SameLine();
        
        ImGui::BeginGroup();
        ImGui::BeginChild("light params", ImVec2(0, 400));
        if (lights.find(currentLightKey) != lights.end()) {
            ofxPBRLight* light = lights[currentLightKey].first;
            LightParams* lightParam = &lights[currentLightKey].second;
            
            ImGui::Text(currentLightKey.c_str());
            if (ImGui::Checkbox("enable", &lightParam->enable)) {
                light->enable(lightParam->enable);
            }
            const char* lightType[] = { "directional", "spot", "point", "sky" };
            if (ImGui::Combo("light type", &lightParam->lightType, lightType, 4)) {
                switch (lightParam->lightType) {
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
            
            if (light->getLightType() != LightType_Sky) {
                
                if (ImGui::DragFloat3("position", &lightParam->pos[0])) {
                    light->setPosition(lightParam->pos);
                    light->lookAt(lightParam->target);
                }
                
                if (ImGui::DragFloat3("target", &lightParam->target[0])) {
                    light->lookAt(lightParam->target);
                }
                
                if (ImGui::ColorEdit3("color", &lightParam->color[0])) {
                    light->setColor(lightParam->color);
                }
                
            }
            else {
                if (cubeMaps.find(currentCubeMapKey) != cubeMaps.end() && cubeMaps[currentCubeMapKey].first->isAllocated()) {
                    ofxPBRCubeMap* cubeMap = cubeMaps[currentCubeMapKey].first;
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
                            lightParam->skyLightCoord = mouse_pos_in_canvas;
                            ofFloatColor c = cubeMap->getColor(cubeMap->getPanoramaTexture()->getWidth() * (lightParam->skyLightCoord.x / canvas_size.x), cubeMap->getPanoramaTexture()->getHeight() * (lightParam->skyLightCoord.y / canvas_size.y));
                            lightParam->color = c;
                            light->setColor(c);
                            light->setSkyLightCoordinate(-PI / 2 + (lightParam->skyLightCoord.x / canvas_size.x) * 2 * PI, (lightParam->skyLightCoord.y / canvas_size.y) * PI, lightParam->skyLightRadius);
                            lightParam->pos = light->getPosition();
                            light->lookAt(ofVec3f(0, 0, 0));
                        }
                    }
                    
                    ImVec2 circlePos = ImVec2(lightParam->skyLightCoord.x + canvas_pos.x, lightParam->skyLightCoord.y + canvas_pos.y);
                    draw_list->AddCircleFilled(circlePos, 5, ImColor(255, 255, 255));
                    draw_list->AddCircle(circlePos, 5, ImColor(0, 0, 0));
                    
                    if (ImGui::DragFloat("skyLightDistance", &lightParam->skyLightRadius, 1)) {
                        light->setSkyLightCoordinate(-PI / 2 + (lightParam->skyLightCoord.x / canvas_size.x) * 2 * PI, (lightParam->skyLightCoord.y / canvas_size.y) * PI, lightParam->skyLightRadius);
                    }
                    
                    if (ImGui::DragFloat3("color", &lightParam->color[0])) {
                        light->setColor(lightParam->color);
                    }
                }else{
                    ImGui::Text("cubeMap texture is not loaded.");
                }
            }
            
            if (ImGui::DragFloat("intensity", &lightParam->intensity, 0.1, 0.0, 100.0)) {
                light->setIntensity(lightParam->intensity);
            }
            
            if (light->getLightType() == LightType_Point) {
                if (ImGui::DragFloat("radius", &lightParam->radius)) {
                    light->setPointLightRadius(lightParam->radius);
                }
            }
            
            if (light->getLightType() == LightType_Spot) {
                if (ImGui::DragFloat("distance", &lightParam->radius)) {
                    light->setSpotLightDistance(lightParam->radius);
                }
                if (ImGui::DragFloat("cutoff", &lightParam->cutoff, 0.1, 0.0, 90.0)) {
                    light->setSpotLightCutoff(lightParam->cutoff);
                }
                if (ImGui::DragFloat("spotFactor", &lightParam->spotFactor, 0.1, 0.0, 10.0)) {
                    light->setSpotLightFactor(lightParam->spotFactor);
                }
            }
            ImGui::Spacing();
            ImGui::Text("shadow");
            
            const char* shadowType[] = { "none", "hard shadow", "soft shadow" };
            if (ImGui::Combo("shadow type", &lightParam->shadowType, shadowType, 3)) {
                switch (lightParam->shadowType) {
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
                if (ImGui::DragFloat("nearClip", &lightParam->nearClip)) {
                    light->setNearClip(lightParam->nearClip);
                }
                
                if (ImGui::DragFloat("farClip", &lightParam->farClip)) {
                    light->setFarClip(lightParam->farClip);
                }
                
                if (ImGui::DragFloat("scale", &lightParam->scale, 0.1, 0.0, 100.0, "%.2f")) {
                    light->setScale(lightParam->scale);
                }
            }
            
            if (light->getShadowType() == ShadowType_Hard) {
                if (ImGui::DragFloat("shadowBias", &lightParam->shadowBias, 0.0001, 0.0, 1.0, "%.4f")) {
                    light->setShadowBias(lightParam->shadowBias);
                }
            }
            
            if (light->getShadowType() == ShadowType_Soft) {
                //if (ImGui::DragFloat("softShadowExponent", &lightParam->softShadowExponent, 0.1, 0.0, 200.0, "%.2f")) {
                //    light->setSoftShadowExponent(lightParam->softShadowExponent);
                //}
            }
            
            ImGui::Image(depthMapId, ImVec2(256, 256));
        }
        
        ImGui::EndChild();
        ImGui::EndGroup();
    }
}

void ofxPBRHelper::drawMaterialsGui(){
    if(ImGui::CollapsingHeader("Materials")){
        ImGui::BeginChild("material list", ImVec2(150, 400), true);
        
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
        ImGui::BeginGroup();
        ImGui::BeginChild("material params", ImVec2(0, 400));
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
            ImGui::PushID(0);
            if (ImGui::ImageButton(baseColorId, ImVec2(40, 40), ImVec2(0, 0), ImVec2(1, 1), 1.0)) {
                showTextureWindow = true;
                currentId = 0;
            }
            if (ImGui::BeginPopupContextItem("delete"))
            {
                if (ImGui::Button("Delete")) {
                    ImGui::CloseCurrentPopup();
                    material->enableBaseColorMap = false;
                    material->baseColorMap = nullptr;
                    params->baseColorTex = "";
                }
                ImGui::EndPopup();
            }
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::ColorEdit4("", &material->baseColor[0]);
            ImGui::Checkbox("enable texture", &material->enableBaseColorMap);
            ImGui::EndGroup();
            ImGui::PopID();
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
            ImGui::PushID(1);
            if (ImGui::ImageButton(roughnessId, ImVec2(40, 40), ImVec2(0, 0), ImVec2(1, 1), 1.0)) {
                showTextureWindow = true;
                currentId = 1;
            }
            if (ImGui::BeginPopupContextItem("delete"))
            {
                if (ImGui::Button("Delete")) {
                    ImGui::CloseCurrentPopup();
                    material->enableRoughnessMap = false;
                    material->roughnessMap = nullptr;
                    params->roughnessTex = "";
                }
                ImGui::EndPopup();
            }
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::SliderFloat("", &material->roughness, 0.0, 1.0);
            ImGui::Checkbox("enable texture", &material->enableRoughnessMap);
            ImGui::EndGroup();
            ImGui::PopID();
            ImGui::Spacing();
            
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
            ImGui::PushID(2);
            if (ImGui::ImageButton(metallicId, ImVec2(40, 40), ImVec2(0, 0), ImVec2(1, 1), 1.0)) {
                showTextureWindow = true;
                currentId = 2;
            }
            if (ImGui::BeginPopupContextItem("delete"))
            {
                if (ImGui::Button("Delete")) {
                    ImGui::CloseCurrentPopup();
                    material->enableMetallicMap = false;
                    material->metallicMap = nullptr;
                    params->metallicTex = "";
                }
                ImGui::EndPopup();
            }
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::SliderFloat("", &material->metallic, 0.0, 1.0);
            ImGui::Checkbox("enable texture", &material->enableMetallicMap);
            ImGui::EndGroup();
            ImGui::PopID();
            ImGui::Spacing();
            
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
            ImGui::PushID(3);
            if (ImGui::ImageButton(normalId, ImVec2(40, 40), ImVec2(0, 0), ImVec2(1, 1), 1.0)) {
                showTextureWindow = true;
                currentId = 3;
            }
            if (ImGui::BeginPopupContextItem("delete"))
            {
                if (ImGui::Button("Delete")) {
                    ImGui::CloseCurrentPopup();
                    material->enableNormalMap = false;
                    material->normalMap = nullptr;
                    params->normalTex = "";
                }
                ImGui::EndPopup();
            }
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Checkbox("enable texture", &material->enableNormalMap);
            ImGui::SliderFloat("", &material->normalVal, 0.0, 1.0);
            ImGui::PopID();
            ImGui::EndGroup();
            
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
            ImGui::PushID(4);
            if (ImGui::ImageButton(occlusionId, ImVec2(40, 40), ImVec2(0, 0), ImVec2(1, 1), 1.0)) {
                showTextureWindow = true;
                currentId = 4;
            }
            if (ImGui::BeginPopupContextItem("delete"))
            {
                if (ImGui::Button("Delete")) {
                    ImGui::CloseCurrentPopup();
                    material->enableOcclusionMap = false;
                    material->occlusionMap = nullptr;
                    params->occlusionTex = "";
                }
                ImGui::EndPopup();
            }
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Checkbox("enable texture", &material->enableOcclusionMap);
            ImGui::EndGroup();
            ImGui::PopID();
            
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
            ImGui::PushID(5);
            if (ImGui::ImageButton(emissionId, ImVec2(40, 40), ImVec2(0, 0), ImVec2(1, 1), 1.0)) {
                showTextureWindow = true;
                currentId = 5;
            }
            if (ImGui::BeginPopupContextItem("delete"))
            {
                if (ImGui::Button("Delete")) {
                    ImGui::CloseCurrentPopup();
                    material->enableEmissionMap = false;
                    material->emissionMap = nullptr;
                    params->emissionTex = "";
                }
                ImGui::EndPopup();
            }
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Checkbox("enable texture", &material->enableEmissionMap);
            ImGui::EndGroup();
            ImGui::PopID();
            
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
            ImGui::PushID(6);
            if (ImGui::ImageButton(detailBaseColorId, ImVec2(40, 40), ImVec2(0, 0), ImVec2(1, 1), 1.0)) {
                showTextureWindow = true;
                currentId = 6;
            }
            if (ImGui::BeginPopupContextItem("delete"))
            {
                if (ImGui::Button("Delete")) {
                    ImGui::CloseCurrentPopup();
                    material->enableDetailBaseColorMap = false;
                    material->detailBaseColorMap = nullptr;
                    params->detailBaseColorTex = "";
                }
                ImGui::EndPopup();
            }
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Checkbox("enable texture", &material->enableDetailBaseColorMap);
            ImGui::EndGroup();
            ImGui::PopID();
            
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
            ImGui::PushID(7);
            if (ImGui::ImageButton(detailNormalId, ImVec2(40, 40), ImVec2(0, 0), ImVec2(1, 1), 1.0)) {
                showTextureWindow = true;
                currentId = 7;
            }
            if (ImGui::BeginPopupContextItem("delete"))
            {
                if (ImGui::Button("Delete")) {
                    ImGui::CloseCurrentPopup();
                    material->enableDetailNormalMap = false;
                    material->detailNormalMap = nullptr;
                    params->detailNormalTex = "";
                }
                ImGui::EndPopup();
            }
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Checkbox("enable texture", &material->enableDetailNormalMap);
            ImGui::EndGroup();
            ImGui::PopID();
            
            ImGui::Text("texture repeat");
            ImGui::PushID(8);
            ImGui::DragFloat2("", &material->textureRepeat[0]);
            ImGui::PopID();
            
            ImGui::Text("detail texture repeat");
            ImGui::PushID(9);
            ImGui::DragFloat2("", &material->detailTextureRepeat[0]);
            ImGui::PopID();
        }
        ImGui::EndChild();
        ImGui::EndGroup();
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
                ImGui::GetStyle().ItemInnerSpacing = ImVec2(0, 0);
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
                
                string s = t.first;
                ImGui::Text(s.c_str());
                
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
        ofxPBRLight* light = l.second.first;
        switch (light->getLightType()) {
            case LightType_Sky:
            case LightType_Directional:
                ofPushStyle();
                ofNoFill();
                ofPushMatrix();
                ofMultMatrix(light->getGlobalTransformMatrix());
                ofSetSphereResolution(16);
                ofDrawSphere(0, 0, 0, 100);
                ofDrawBox(0, 0, -light->getFarClip() / 2, pbr->getDepthMapResolution(), pbr->getDepthMapResolution(), light->getFarClip());
                ofPopMatrix();
                ofPopStyle();
                break;
                
            case LightType_Point:
                ofPushStyle();
                ofNoFill();
                ofPushMatrix();
                ofSetCircleResolution(64);
                ofTranslate(light->getGlobalTransformMatrix().getTranslation());
                ofSetSphereResolution(16);
                ofDrawSphere(0, 0, 0, 100);
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
                ofSetSphereResolution(16);
                ofDrawSphere(0, 0, 0, 100);
                ofPushMatrix();
                ofRotateX(-90);
                ofRotateY(45);
                ofSetConeResolution(4, 0);
                ofDrawCone(0, light->getFarClip() / 2, 0, light->getFarClip() * tan(ofDegToRad(light->getSpotLightCutoff())), light->getFarClip());
                ofPopMatrix();
                ofPopMatrix();
                ofPopStyle();
                break;
                
            default:
                break;
        }
    }
}

void ofxPBRHelper::addLight(ofxPBRLight * light, string name)
{
    lights.insert(map<string, pair<ofxPBRLight*, LightParams>>::value_type(name, pair<ofxPBRLight*, LightParams>(light, LightParams())));
    pbr->addLight(light);
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
		ofxPBRLight* l = light.second.first;
		LightParams* p = &light.second.second;

		lightJson[light.first]["enable"] = p->enable;

		lightJson[light.first]["lightType"] = p->lightType;

		lightJson[light.first]["position"]["x"] = p->pos.x;
		lightJson[light.first]["position"]["y"] = p->pos.y;
		lightJson[light.first]["position"]["z"] = p->pos.z;

		lightJson[light.first]["target"]["x"] = p->target.x;
		lightJson[light.first]["target"]["y"] = p->target.y;
		lightJson[light.first]["target"]["z"] = p->target.z;

		lightJson[light.first]["skyLightCoord"]["x"] = p->skyLightCoord.x;
		lightJson[light.first]["skyLightCoord"]["y"] = p->skyLightCoord.y;

		lightJson[light.first]["color"]["r"] = p->color.r;
		lightJson[light.first]["color"]["g"] = p->color.g;
		lightJson[light.first]["color"]["b"] = p->color.b;
		lightJson[light.first]["color"]["a"] = p->color.a;

		lightJson[light.first]["intensity"] = p->intensity;

		lightJson[light.first]["radius"] = p->radius;
		lightJson[light.first]["cutoff"] = p->cutoff;
		lightJson[light.first]["spotFactor"] = p->spotFactor;

		lightJson[light.first]["shadowType"] = p->shadowType;
		lightJson[light.first]["nearClip"] = p->nearClip;
		lightJson[light.first]["farClip"] = p->farClip;
		lightJson[light.first]["scale"] = p->scale;
		lightJson[light.first]["shadowBias"] = p->shadowBias;
		lightJson[light.first]["softShadowExponent"] = p->softShadowExponent;
	}
	settings["light"] = lightJson;

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
		ofxPBRLight* light = lights[lightName].first;
		LightParams* params = &lights[lightName].second;
		if (settings.isNull() == false && settings["light"][lightName].isNull() == false) {
			Json::Value lightParams = settings["light"][lightName];

			params->enable = lightParams["enable"].asBool();

			params->lightType = lightParams["lightType"].asInt();

			params->pos.x = lightParams["position"]["x"].asFloat();
			params->pos.y = lightParams["position"]["y"].asFloat();
			params->pos.z = lightParams["position"]["z"].asFloat();

			params->target.x = lightParams["target"]["x"].asFloat();
			params->target.y = lightParams["target"]["y"].asFloat();
			params->target.z = lightParams["target"]["z"].asFloat();

			params->skyLightCoord.x = lightParams["skyLightCoord"]["x"].asFloat();
			params->skyLightCoord.y = lightParams["skyLightCoord"]["y"].asFloat();

			params->color.r = lightParams["color"]["r"].asFloat();
			params->color.g = lightParams["color"]["g"].asFloat();
			params->color.b = lightParams["color"]["b"].asFloat();
			params->color.a = lightParams["color"]["a"].asFloat();

			params->intensity = lightParams["intensity"].asFloat();

			params->radius = lightParams["radius"].asFloat();
			params->cutoff = lightParams["cutoff"].asFloat();
			params->spotFactor = lightParams["spotFactor"].asFloat();

			params->shadowType = lightParams["shadowType"].asInt();
			params->nearClip = lightParams["nearClip"].asFloat();
			params->farClip = lightParams["farClip"].asFloat();
			params->scale = lightParams["scale"].asFloat();
			params->shadowBias = lightParams["shadowBias"].asFloat();
			params->softShadowExponent = lightParams["softShadowExponent"].asFloat();
		}

		light->enable(params->enable);
		light->setPosition(params->pos);
		light->lookAt(params->target);

		switch (params->lightType) {
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
			light->setSkyLightCoordinate(-PI / 2 + (params->skyLightCoord.x / 256) * 2 * PI, (params->skyLightCoord.y / 128) * PI, 4000);
			break;
		default:
			break;
		}

		light->setColor(params->color);
		light->setIntensity(params->intensity);
		light->setPointLightRadius(params->radius);
		light->setSpotLightCutoff(params->cutoff);
		light->setSpotLightFactor(params->spotFactor);

		switch (params->shadowType) {
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

		light->setNearClip(params->nearClip);
		light->setFarClip(params->farClip);
		light->setScale(params->scale);
		light->setShadowBias(params->shadowBias);
		//light->setSoftShadowExponent(params->softShadowExponent);
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