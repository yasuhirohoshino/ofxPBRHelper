#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofDisableArbTex();

    geomShader.setGeometryInputType(GL_TRIANGLES);
    geomShader.setGeometryOutputCount(9);
    geomShader.setGeometryOutputType(GL_TRIANGLES);
    geomShader.load("shaders/geometryPBR.vert", "shaders/geometryPBR.frag", "shaders/geometryPBR.geom");
	pbr.setup(scene, &cam, 1024);
	
	gui.setup();

    cam.setupPerspective(false, 60, 1, 12000);
    
    scene = bind(&ofApp::renderScene, this);
	pbr.setup(scene, &cam, 2048);
    
    ofxPBRFiles::getInstance()->setup("ofxPBRAssets");
    pbrHelper.setup(&pbr, ofxPBRFiles::getInstance()->getPath() + "/settings", true);
    pbrHelper.addLight(&pbrLight1, "light1");
    pbrHelper.addMaterial(&floorMaterial, "floorMaterial");
    pbrHelper.addMaterial(&sphereMaterial, "sphereMaterial");
    pbrHelper.addCubeMap(&cubeMap1, "cubeMap1");
    
    ofSetSphereResolution(32);
    ofSetConeResolution(32, 1);
    ofSetBoxResolution(1);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	pbr.updateDepthMaps();
    
    ofDisableAlphaBlending();
    
    cam.begin();
    pbr.drawEnvironment();
	pbr.renderScene();
    cam.end();
    
    ofEnableAlphaBlending();
    
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
            pbrHelper.drawGui();
            ImGui::End();
        }
    }
    gui.end();
}

//--------------------------------------------------------------
void ofApp::renderScene(){
    ofEnableDepthTest();
    
    pbr.beginCustomRenderer(&geomShader);
    sphereMaterial.begin(&pbr);
    
    ofSetIcoSphereResolution(0);
    ofDrawIcoSphere(-750 * 2, 0, 0, 300);
    
    ofSetIcoSphereResolution(1);
    ofDrawIcoSphere(-750, 0, 0, 300);
    
    ofSetIcoSphereResolution(2);
    ofDrawIcoSphere(0, 0, 0, 300);
    
    ofSetIcoSphereResolution(3);
    ofDrawIcoSphere(750, 0, 0, 300);
    
    ofSetIcoSphereResolution(4);
    ofDrawIcoSphere(750 * 2, 0, 0, 300);
    
    sphereMaterial.end();
    pbr.endCustomRenderer();
    
    pbr.beginDefaultRenderer();
    floorMaterial.begin(&pbr);
    ofDrawBox(0, -450, 0, 5000, 10, 5000);
    floorMaterial.end();
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

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
