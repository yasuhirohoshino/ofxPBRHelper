#pragma once
#include "ofMain.h"
class ofxPBRImage {
public:
	void load(string path) {
		ofFile file;
		file.open(path);

		if (file.getExtension() == "hdr" || file.getExtension() == "exr" || file.getExtension() == "dds" || file.getExtension() == "tga") {
			fImg.load(file.getAbsolutePath());
			isFloat = true;
		}
		else {
			img.load(file.getAbsolutePath());
			isFloat = false;
		}
	}

	void fetchPixels(ofPixels* pixel) {
		pixel = &img.getPixels();
	}

	void fetchPixels(ofFloatPixels* pixel) {
		pixel = &fImg.getPixels();
	}

	ofTexture getTexture() {
		if (isFloat) {
			return fImg.getTexture();
		}
		else {
			return img.getTexture();
		}
	}

	void setImage(ofImage * img) {
		this->img = *img;
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

	ofImage* getImage() {
		return &img;
	}

	void resize(int width, int height) {
		if (isFloat) {
			fImg.resize(width, height);
		}
		else {
			img.resize(width, height);
		}
	}

	void saveImage(string path) {
		if (isFloat) {
			fImg.save(path);
		}
		else {
			img.save(path);
		}
	}

	ofFloatColor getColor(int x, int y) {
		if (isFloat) {
			return fImg.getColor(x, y);
		}
		else {
			return img.getColor(x, y);
		}
	}

	int getWidth() {
		if (isFloat) {
			return fImg.getWidth();
		}
		else {
			return img.getWidth();
		}
	}

	int getHeight() {
		if (isFloat) {
			return fImg.getHeight();
		}
		else {
			return img.getHeight();
		}
	}

	ofFloatImage fImg;
	ofImage img;

private:
	bool isFloat = false;
};
