#pragma once
#include "ofConstants.h"
#include "ofThread.h"
#include "ofThreadChannel.h"

#ifdef TARGET_ANDROID

#include <jni.h>
#include "ofBaseTypes.h"
#include "ofMatrix4x4.h"
#include "ofTexture.h"

//struct ofxAndroidMobileVisionFace {
//	float smileProbability;
//	float leftEyeOpenProbability;
//	float rightEyeOpenProbability;
//
//	vector<ofVec2f> landmarks;
//};
//
//struct ToAnalyzeData{
//	ofPixels pixels;
//};

class ofxARCore : ofThread{
public:

	ofxARCore();
    ~ofxARCore();

    void setup();

    bool isInitialized();
    void update();
    void draw();

    ofMatrix4x4 getViewMatrix();
    ofMatrix4x4 getProjectionMatrix();

    ofTexture texture;

private:
    ofMesh quad;
    jclass javaClass;
    jobject javaTango;

    bool _textureInitialized;

//	bool threaded;

//	ofThreadChannel<ToAnalyzeData> toAnalyze;
//	ofThreadChannel<vector<ofxAndroidMobileVisionFace> > fromAnalyze;
//
//	vector<ofxAndroidMobileVisionFace> faces;

//	void threadedFunction();
//	void process(ofPixels &pixels);
};

#endif