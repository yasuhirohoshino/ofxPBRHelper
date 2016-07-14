#pragma once

#include "ofMain.h"
#include "ofxImGui.h"

#include "ofxPBR.h"
#include "ofxPBRHelper.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
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
    
    ofxPBRCubeMap cubeMap1;
    ofxPBRMaterial floorMaterial, sphereMaterial;
    ofxPBRLight pbrLight1;
    ofxPBR pbr;
    ofxPBRHelper pbrHelper;
    
    ofShader geomShader;
    ofEasyCam cam;
    
    bool mouseInput = true;
    bool showGui = true;
    ofxImGui gui;
};
