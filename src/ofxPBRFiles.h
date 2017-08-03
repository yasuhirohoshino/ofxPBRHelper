#pragma once
#include "ofMain.h"
#include "ofxPBRImage.h"

class ofxPBRFiles {
public:
	static ofxPBRFiles * getInstance();

	void setup(string folderPath);
	void close();

	map<string, ofTexture*> textures;
	map<string, ofTexture*> panoramas;
	string getPath();

private:
	ofxPBRFiles();
	~ofxPBRFiles();
	ofxPBRFiles(const ofxPBRFiles&);
	static ofxPBRFiles* instance;
	string folderPath;
};