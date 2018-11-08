#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofDisableArbTex();
    model.loadModel("dragon.obj");
    for (int i = 0; i < model.getNumMeshes(); i++) {
        modelMesh.append(model.getMesh(i));
    }
    modelScale = model.getModelMatrix().getScale();
    
    defaultFboSettings.textureTarget = GL_TEXTURE_2D;
    defaultFboSettings.useDepth = true;
    defaultFboSettings.depthStencilAsTexture = true;
    defaultFboSettings.useStencil = true;
    defaultFboSettings.minFilter = GL_LINEAR;
    defaultFboSettings.maxFilter = GL_LINEAR;
    defaultFboSettings.wrapModeHorizontal = GL_CLAMP_TO_EDGE;
    defaultFboSettings.wrapModeVertical = GL_CLAMP_TO_EDGE;
    resizeFbos();
    
    cam.setupPerspective(false, 60, 1, 12000);
    scene = bind(&ofApp::renderScene, this);
	pbr.setup(scene, &cam, 2048);
    
	gui.setup();

    ofxPBRFiles::getInstance()->setup("ofxPBRAssets");
    pbrHelper.setup(&pbr, ofxPBRFiles::getInstance()->getPath() + "/settings", true);
    pbrHelper.addLight(&pbrLight, "light");
    pbrHelper.addMaterial(&material1, "material1");
    pbrHelper.addMaterial(&material2, "material2");
    pbrHelper.addCubeMap(&cubemap[0], "cubeMap1");
    pbrHelper.addCubeMap(&cubemap[1], "cubeMap2");
    
    string path = "shaders/postEffect/";
    detectBloomColor.load(path + "passThrough.vert", path + "detectBloomColor.frag");
    bloomBlur.load(path + "passThrough.vert", path + "blur.frag");
    tonemap.load(path + "passThrough.vert", path + "tonemap.frag");
    fxaa.load(path + "passThrough.vert", path + "fxaa.frag");
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofDisableAlphaBlending();
    ofEnableDepthTest();
    
	pbr.updateDepthMaps();

    firstPass.begin();
    ofClear(0);
    cam.begin();
    pbr.drawEnvironment();
    scene();
    cam.end();
    firstPass.end();
    
    ofDisableDepthTest();
    ofEnableAlphaBlending();
    
    // post effect
    
    bloomVPass.begin();
    ofClear(0);
    detectBloomColor.begin();
    detectBloomColor.setUniform1f("threthould", bloomThrethould);
    firstPass.draw(0, 0, bloomVPass.getWidth(), bloomVPass.getHeight());
    detectBloomColor.end();
    bloomVPass.end();
    
    for(int i=0;i<2;i++){
        bloomHPass.begin();
        ofClear(0);
        bloomBlur.begin();
        bloomBlur.setUniform1i("horizontal", true);
        bloomVPass.draw(0, 0);
        bloomBlur.end();
        bloomHPass.end();
        
        bloomVPass.begin();
        ofClear(0);
        bloomBlur.begin();
        bloomBlur.setUniform1i("horizontal", false);
        bloomHPass.draw(0, 0);
        bloomBlur.end();
        bloomVPass.end();
    }
    
    secondPass.begin();
    ofClear(0);
    tonemap.begin();
    tonemap.setUniformTexture("image", firstPass.getTexture(), 0);
    tonemap.setUniformTexture("bloom", bloomVPass.getTexture(), 1);
    tonemap.setUniform1f("exposure", exposure);
    tonemap.setUniform1f("gamma", gamma);
    firstPass.draw(0, 0);
    tonemap.end();
    secondPass.end();
    
    fxaa.begin();
    fxaa.setUniformTexture("image", secondPass.getTexture(), 0);
    fxaa.setUniform2f("texel", 1.0 / float(secondPass.getWidth()), 1.0 / float(secondPass.getHeight()));
    secondPass.draw(0, 0);
    fxaa.end();
    
    gui.begin();
    {
        ImGui::Begin("control panel");
        {
            ImGui::Checkbox("show gui", &showGui);
            if(ImGui::Checkbox("enable camera control", &mouseInput)){
                if (mouseInput){
                    cam.enableMouseInput();
                } else {
                    cam.disableMouseInput();
                }
            }
        }
        ImGui::End();
        if(showGui){
            ImGui::Begin("ofxPBRHelper");
            ImGui::DragFloat("exposure", &exposure, 0.1);
            ImGui::DragFloat("gamma", &gamma, 0.1);
            ImGui::DragFloat("bloomThrethould", &bloomThrethould, 0.01);
            pbrHelper.drawGui();
            ImGui::End();
        }
    }
    gui.end();
}

//--------------------------------------------------------------
void ofApp::resizeFbos(){
    ofFbo::Settings firstPassSettings;
    firstPassSettings = defaultFboSettings;
    firstPassSettings.width = ofGetWidth();
    firstPassSettings.height = ofGetHeight();
    firstPassSettings.internalformat = GL_RGBA32F;
    firstPassSettings.colorFormats.push_back(GL_RGBA32F);
    firstPass.allocate(firstPassSettings);
    
    ofFbo::Settings bloomPassSettings;
    bloomPassSettings = defaultFboSettings;
    bloomPassSettings.width = ofGetWidth() / 2;
    bloomPassSettings.height = ofGetHeight() / 2;
    bloomPassSettings.internalformat = GL_RGBA32F;
    bloomPassSettings.colorFormats.push_back(GL_RGBA32F);
    bloomHPass.allocate(bloomPassSettings);
    bloomVPass.allocate(bloomPassSettings);
    
    ofFbo::Settings secondPassSettings;
    secondPassSettings = defaultFboSettings;
    secondPassSettings.width = ofGetWidth();
    secondPassSettings.height = ofGetHeight();
    secondPassSettings.internalformat = GL_RGB;
    secondPassSettings.colorFormats.push_back(GL_RGB);
    secondPass.allocate(secondPassSettings);
}

//--------------------------------------------------------------
void ofApp::renderScene(){
    ofEnableDepthTest();
    
    pbr.beginDefaultRenderer();
    
    material1.begin(&pbr);
    ofPushMatrix();
    ofScale(modelScale.x, modelScale.y, modelScale.z);
    modelMesh.draw();
    ofPopMatrix();
    material1.end();
    
    material2.begin(&pbr);
    ofDrawBox(0, -5, 0, 5000, 10, 5000);
    material2.end();
    
    pbr.endDefaultRenderer();
    
    ofDisableDepthTest();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    resizeFbos();
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
