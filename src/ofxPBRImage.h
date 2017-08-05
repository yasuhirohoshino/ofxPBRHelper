#pragma once
#include "ofMain.h"
class ofxPBRImage {
public:
	void load(string path) {
		ofFile file;
		file.open(path);

		ofLogLevel logLevel = ofGetLogLevel();
		ofSetLogLevel(OF_LOG_SILENT);

		sImg.load(file.getAbsolutePath());
		if (sImg.getPixels().getBitsPerPixel() != 0) {
			isFloat = false;
		}
		else {
			fImg.load(file.getAbsolutePath());
			isFloat = true;
		}
		ofSetLogLevel(logLevel);
	}

	void fetchPixels(ofShortPixels* pixel) {
		pixel = &sImg.getPixels();
	}

	void fetchPixels(ofFloatPixels* pixel) {
		pixel = &fImg.getPixels();
	}

	ofTexture getTexture() {
		if (isFloat) {
			return fImg.getTexture();
		}
		else {
			return sImg.getTexture();
		}
	}

	void setImage(ofImage * img) {
		this->sImg = *img;
		isFloat = false;
	}

	void setImage(ofFloatImage * img) {
		this->fImg = *img;
		isFloat = true;
	}

	bool isFloatImage() {
		return isFloat;
	}

	ofFloatImage* getFloatImage() {
		return &fImg;
	}

	ofShortImage* getImage() {
		return &sImg;
	}

	void resize(int width, int height) {
		if (isFloat) {
			fImg.resize(width, height);
		}
		else {
			sImg.resize(width, height);
		}
	}

	void saveImage(string path) {
		if (isFloat) {
			fImg.save(path);
		}
		else {
			sImg.save(path);
		}
	}

	ofFloatColor getColor(int x, int y) {
		if (isFloat) {
			return fImg.getColor(x, y);
		}
		else {
			return sImg.getColor(x, y);
		}
	}

	int getWidth() {
		if (isFloat) {
			return fImg.getWidth();
		}
		else {
			return sImg.getWidth();
		}
	}

	int getHeight() {
		if (isFloat) {
			return fImg.getHeight();
		}
		else {
			return sImg.getHeight();
		}
	}

	ofFloatImage fImg;
	ofShortImage sImg;

private:
	bool isFloat = false;
};
