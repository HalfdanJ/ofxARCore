// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include "ofConstants.h"
#include "ofThread.h"
#include "ofThreadChannel.h"
#include "ofLog.h"
#include "ofMesh.h"
#include "ofEventUtils.h"
#ifdef TARGET_ANDROID

#include <jni.h>
#include "ofBaseTypes.h"
#include "ofMatrix4x4.h"
#include "ofTexture.h"

typedef struct ofAugmentedImage {
	ofMatrix4x4 matrix;
	float width;
	float height;
	int index;
	bool is_tracking;
	std::string name;
} ofAugmentedImage;

typedef struct ofHitPose {
	ofMatrix4x4 matrix;
	float distance;
} ofHitPose;

typedef struct ofARPlane {
	ofMesh mesh;
    ofPath path;
	ofMatrix4x4 center;
} ofARPlane;

class ofxARCore : ofThread{

public:

    ofxARCore();
    ~ofxARCore();

    void setup(bool enableAugmentedImages);

    bool isInitialized();
	bool isTracking();

    // boehm-e
	float getCameraFOV();
    float getDpi();

    void update();
    void draw();

    void pauseApp();
    void resumeApp();
	void reloadTexture();

    void addAnchor();
    ofMatrix4x4 getAnchor(int i=0);

    ofMatrix4x4 getViewMatrix();
	std::vector<ofAugmentedImage*> getImageMatrices();
	std::vector<ofARPlane *> getPlanes();
	ofHitPose *getHitPose(int x, int y);
    ofMatrix4x4 getProjectionMatrix(float near=0.1f, float far=100.0f);

    std::vector<float> getPointCloud();     /* get point cloud arcore */

    ofTexture texture;

private:
    void setupSession();
    GLuint setupTexture();

    ofMesh quad;
    jclass javaClass;
    jobject javaTango;
    ofDirectory *arcoreImagesDir;

    bool _sessionInitialized;
	bool enableAugmentedImages;
};

#endif