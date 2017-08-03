#include "ofxPBRFiles.h"

ofxPBRFiles * ofxPBRFiles::instance = NULL;

ofxPBRFiles * ofxPBRFiles::getInstance() {
	if (!instance) {
		instance = new ofxPBRFiles();
	}
	return instance;
}

void ofxPBRFiles::setup(string folderPath)
{
	this->folderPath = folderPath;
	ofDirectory dir;
	dir.open(folderPath + "/textures/");
	textures.clear();
	for (auto file : dir.getFiles()) {
		ofDisableArbTex();
		ofTexture* texture = new ofTexture();
		ofxPBRImage img;
		img.load(file.getAbsolutePath());
		*texture = img.getTexture();
		textures.insert(map<string, ofTexture*>::value_type(file.getFileName(), texture));
		ofEnableArbTex();
	}

	dir.open(folderPath + "/panoramas_small/");
	panoramas.clear();
	for (auto file : dir.getFiles()) {
		ofDisableArbTex();
		ofxPBRImage img;
		img.load(file.getAbsolutePath());
		ofTexture* texture = new ofTexture();
		*texture = img.getTexture();
		panoramas.insert(map<string, ofTexture*>::value_type(file.getFileName(), texture));
		ofEnableArbTex();
	}
}

void ofxPBRFiles::close()
{
	for (auto &t : textures) {
		t.second->clear();
		delete t.second;
	}

	for (auto &p : panoramas) {
		p.second->clear();
		delete p.second;
	}
}

string ofxPBRFiles::getPath()
{
	return folderPath;
}



ofxPBRFiles::ofxPBRFiles(){}
ofxPBRFiles::~ofxPBRFiles(){}