#pragma once

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"
#include "ofxImGui.h"

#include "ofxPBR.h"
#include "ofxPBRHelper.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void resizeFbos();
        void renderScene();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
    function<void()> scene;
    
    ofxPBRCubeMap cubemap[2];
    ofxPBRMaterial material1, material2;
    ofxPBRLight pbrLight;
    ofxPBR pbr;
    ofxPBRHelper pbrHelper;
    
    ofFbo firstPass, secondPass;
    ofFbo::Settings defaultFboSettings;
    
    ofEasyCam cam;
    
    ofxAssimpModelLoader model;
    ofVec3f modelScale;
    ofVboMesh modelMesh;
        
    ofShader tonemap, fxaa;
    ofxImGui::Gui gui;
    float exposure = 1.0;
    float gamma = 2.2;
    
    bool mouseInput = true;
    bool showGui = true;
};
