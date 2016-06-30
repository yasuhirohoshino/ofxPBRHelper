#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofDisableArbTex();
    
    pos.resize(2500);
    
    for(auto &p : pos){
        p.set(ofRandom(-500, 500), ofRandom(-500, 500), ofRandom(-500, 500));
    }
    
    buffer.allocate();
    buffer.bind(GL_TEXTURE_BUFFER);
    buffer.setData(pos,GL_STREAM_DRAW);
    
    tex.allocateAsBufferTexture(buffer,GL_RGB32F);
    
    renderShader.load("shaders/instancing.vert", "shaders/ofxPBRShaders/default.frag");
    renderShader.begin();
    renderShader.setUniformTexture("posTex", tex, 13);
    renderShader.end();

    cam.setupPerspective(false, 60, 1, 12000);
    
    mesh = ofMesh::box(50,50,50,1,1,1);
    mesh.setUsage(GL_STATIC_DRAW);
    
    scene = bind(mem_fn(&ofApp::renderScene), this);
    
    pbr.setup(1024);
    ofxPBRFiles::getInstance()->setup("ofxPBRAssets");
    pbrHelper.setup(&pbr, ofxPBRFiles::getInstance()->getPath() + "/settings", true);
    pbrHelper.addLight(&pbrLight1, "light1");
    pbrHelper.addMaterial(&floorMaterial, "floorMaterial");
    pbrHelper.addMaterial(&cubeMaterial, "cubeMaterial");
    pbrHelper.addCubeMap(&cubeMap1, "cubeMap1");
    
    gui.setup();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    pbr.makeDepthMap(scene);
    
    cam.begin();
    pbr.drawEnvironment(&cam);
    scene();
    cam.end();
    
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
    pbr.begin(&cam, &renderShader);
    
    cubeMaterial.begin(pbr.getShader());
    mesh.drawInstanced(OF_MESH_FILL, pos.size());
    cubeMaterial.end();
    
    pbr.end();
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
