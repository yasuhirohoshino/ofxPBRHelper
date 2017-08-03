#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofDisableArbTex();

	renderShader.load("ofxPBRShaders/default2.vert", "ofxPBRShaders/default2.frag");
	cam.setupPerspective(false, 60, 1, 8000);
	scene = bind(&ofApp::renderScene, this);
	pbr.setup(scene, &cam, 2048);
    
    gui.setup();
    
    ofxPBRFiles::getInstance()->setup("ofxPBRAssets");
	pbrHelper.setup(&pbr, ofxPBRFiles::getInstance()->getPath() + "/settings", true);
	pbrHelper.addLight(&pbrLight1, "light1");
	pbrHelper.addLight(&pbrLight2, "light2");
	pbrHelper.addMaterial(&floorMaterial, "floor");
	pbrHelper.addMaterial(&cubeMaterial, "cube");
	pbrHelper.addMaterial(&sphereMaterial, "sphere");
	pbrHelper.addMaterial(&coneMaterial, "cone");
	pbrHelper.addMaterial(&cylinderMaterial, "cylinder");
	pbrHelper.addMaterial(&planeMaterial, "plane");
	pbrHelper.addCubeMap(&cubeMap1, "cubeMap1");

	shader.load("ofxPBRShaders/default");
    
    ofSetSphereResolution(32);
    ofSetConeResolution(32, 1);
    ofSetBoxResolution(1);
    ofSetCylinderResolution(32, 2);
}

//--------------------------------------------------------------
void ofApp::update(){
	ofSetWindowTitle(ofToString(ofGetFrameRate()));
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofDisableAlphaBlending();
    ofEnableDepthTest();
    
	pbr.updateDepthMaps();

	//pbr.renderScene();
    cam.begin();
	pbr.renderScene();
    cam.end();
    
    ofDisableDepthTest();
    ofEnableAlphaBlending();
    
    cam.begin();
    pbrHelper.drawLights();
    cam.end();

    gui.begin();
    {
		//if (ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
		//	cam.enableMouseInput();
		//}
		//else {
		//	cam.disableMouseInput();
		//}
		//if (ImGui::IsKeyDown(GLFW_KEY_LEFT_CONTROL)) {
		//	if (ImGui::GetIO().MouseDown[1] != 0.0) {
		//		cam.truck(-ImGui::GetIO().MouseDelta.x * 2);
		//		cam.boom(ImGui::GetIO().MouseDelta.y * 2);
		//	}
		//	if (ImGui::GetIO().MouseWheel != 0.0) {
		//		cam.dolly(ImGui::GetIO().MouseWheel * 10.0);
		//	}
		//}

		ImGui::Begin("control panel");
		{
			ImGui::Checkbox("show gui", &showGui);
			if (ImGui::Checkbox("enable camera control", &mouseInput)) {
				if (mouseInput) {
					cam.enableMouseInput();
				}
				else {
					cam.disableMouseInput();
				}
			}
		}
		ImGui::End();
        if(showGui){
            ImGui::Begin("ofxPBR");
            pbrHelper.drawGui(&cam);
            ImGui::End();
        }
    }
    gui.end();
}

//--------------------------------------------------------------
void ofApp::exit() {
	ofxPBRFiles::getInstance()->close();
}

//--------------------------------------------------------------
void ofApp::renderScene(){
    ofEnableDepthTest();
	ofEnableAlphaBlending();

	pbr.beginCustomRenderer(&renderShader);

	planeMaterial.begin(&pbr);
	ofDrawPlane(-350 * 2, 150, 0, 300, 300);
	planeMaterial.end();

	sphereMaterial.begin(&pbr);
	ofDrawSphere(-350, 150, 0, 150);
	sphereMaterial.end();

	coneMaterial.begin(&pbr);
	ofPushMatrix();
	ofTranslate(0, 150, 0);
	ofRotateX(180);
	ofDrawCone(0, 0, 0, 150, 300);
	ofPopMatrix();
	coneMaterial.end();

	cubeMaterial.begin(&pbr);
	ofDrawBox(350, 150, 0, 300, 300, 300);
	cubeMaterial.end();

	cylinderMaterial.begin(&pbr);
	ofDrawCylinder(350 * 2, 150, 0, 150, 300);
	cylinderMaterial.end();

	floorMaterial.begin(&pbr);
	ofDrawBox(0, -5, 0, 5000, 10, 5000);
	floorMaterial.end();

	pbr.endCustomRenderer();

    ofDisableDepthTest();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	if (key == ' ') {
		renderShader.load("ofxPBRShaders/default2.vert", "ofxPBRShaders/default2.frag");
	}
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
