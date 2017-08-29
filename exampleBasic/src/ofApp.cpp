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


#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	arcore.setup();
    initialized = false;

    ofBackground(0);

    ofSetOrientation(OF_ORIENTATION_DEFAULT, false);
}

//--------------------------------------------------------------
void ofApp::update(){
    arcore.update();
    if(!initialized && arcore.isInitialized()){
        initialized = true;
        projectionMatrix = arcore.getProjectionMatrix(0.01f,100.0);
    }

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetColor(255);

    if(initialized) {
        arcore.draw();

        ofSetMatrixMode(OF_MATRIX_PROJECTION);
        ofLoadMatrix(projectionMatrix);
        ofSetMatrixMode(OF_MATRIX_MODELVIEW);
        ofLoadMatrix(arcore.getViewMatrix());

        ofDrawGrid(0.1, 30, true, true, true, true);
    }

}
