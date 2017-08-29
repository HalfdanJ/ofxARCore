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

#ifdef TARGET_ANDROID

#include <jni.h>
#include "ofBaseTypes.h"
#include "ofMatrix4x4.h"
#include "ofTexture.h"


class ofxARCore : ofThread{
public:

	ofxARCore();
    ~ofxARCore();

    void setup();

    bool isInitialized();
	bool isTracking();

    void update();
    void draw();

    void pauseApp();
    void resumeApp();

    void addAnchor();
    ofMatrix4x4 getAnchor(int i=0);

    ofMatrix4x4 getViewMatrix();
    ofMatrix4x4 getProjectionMatrix(float near=0.1f, float far=100.0f);

    ofTexture texture;

private:
    void setupSession();
    GLuint setupTexture();

    ofMesh quad;
    jclass javaClass;
    jobject javaTango;

    bool _sessionInitialized;
};

#endif