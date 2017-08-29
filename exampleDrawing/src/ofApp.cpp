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


ofEvent<void> clearDrawingEvent;

//--------------------------------------------------------------
void ofApp::setup() {
    ofBackground(0);
    ofSetOrientation(OF_ORIENTATION_DEFAULT, false);
    font.load("Roboto-Regular.ttf", 40);

    ofAddListener(clearDrawingEvent, this, &ofApp::clearDrawing);

    initialized = false;

    // Setup ARCore
    arcore.setup();
}

//--------------------------------------------------------------
void ofApp::update(){
    arcore.update();

    if(!initialized && arcore.isInitialized()){
        // Load projection matrix once when ARCore is initialized
        initialized = true;
        projectionMatrix = arcore.getProjectionMatrix(0.01f,100.0);
    }

    if(initialized && _touchDown){
        addRibbonPoint(mouseX, mouseY,  0.002);
    }
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofSetColor(255);
    if(initialized) {
        // Draw the camera image
        arcore.draw();

        if(arcore.isTracking()) {
            // Load the projection and view matrix from ARCore
            ofSetMatrixMode(OF_MATRIX_PROJECTION);
            ofLoadMatrix(projectionMatrix);
            ofSetMatrixMode(OF_MATRIX_MODELVIEW);
            ofLoadMatrix(arcore.getViewMatrix());

            // Draw the lines
            for (auto line : lineMeshes) {
                line.draw(OF_MESH_FILL);
            }
        } else {
            drawMessage("Not tracking");
        }
    } else {
        drawMessage("Initializing ARCore");
    }
}

//--------------------------------------------------------------
void ofApp::drawMessage(string text){
    ofPushStyle();
    ofSetColor(0,0,0, 100);
    ofDrawRectangle(0, ofGetHeight() - 200, ofGetWidth(), 300);

    ofSetColor(255);
    float w = font.stringWidth(text);
    font.drawString(text, ofGetWidth()/2-w/2, ofGetHeight() - 110);
    ofPopStyle();
}


//--------------------------------------------------------------
// Calculate the world coordinate from screen coordinate
ofVec3f ofApp::screenToWorld(float x, float y, float z){
    ofVec2f s = ofVec3f(
            (x - ofGetWidth()/2.f)/ofGetWidth(),
            (y - ofGetHeight()/2.f)/ofGetHeight());

    float zFull = z * 100.f;
    float zNearToFar = ofLerp(0.01, 100.0, z);
    float alpha = zFull / zNearToFar;

    auto m  = arcore.getViewMatrix() * projectionMatrix;
    auto p4 = ofVec4f(2.f*s.x,-2.f*s.y,alpha,1.0) * m.getInverse();

    return ofVec3f(p4.x, p4.y, p4.z) / p4.w;
}

//--------------------------------------------------------------
void ofApp::addRibbonPoint(float x, float y, float z){
    ofVec3f p = screenToWorld(x, y, z);
    lines[lines.size()-1].curveTo(p);

    // Resample line
    ofPolyline l = lines[lines.size()-1].getResampledBySpacing(0.01);

    // Update mesh
    updateLineMesh(lines[lines.size()-1], lineMeshes[lineMeshes.size()-1]);

    // When line is long, split it up (for optimization)
    if(l.size() > 100){
        createNewLine();
        lines[lines.size()-1].addVertex(l.getVertices()[l.size()-2]);
        lines[lines.size()-1].addVertex(l.getVertices()[l.size()-1]);
    }
}

//--------------------------------------------------------------
// Create line mesh based on polyline
void ofApp::updateLineMesh(ofPolyline & line, ofVboMesh & mesh, int offset){
    auto viewmatrix = arcore.getViewMatrix().getInverse();

    float thickness = 0.005;
    mesh.clear();

    int n = line.size();
    for(int i=1;i<line.size(); i++) {
        int i0 = (i - 1 + offset) % n;
        int i1 = (i + offset) % n;
        ofVec3f thisPoint = line[i0];
        ofVec3f nextPoint = line[i1];
        ofVec3f direction = (nextPoint - thisPoint);
        ofVec3f unitDirection = direction.getNormalized();

        ofVec3f screenDir = ofVec3f(0, 0, 0) * viewmatrix - nextPoint;
        ofVec3f perp = screenDir.cross(unitDirection);

        ofVec3f toTheLeft = perp;
        ofVec3f toTheRight = -perp;

        ofVec3f leftPoint = thisPoint + toTheLeft * thickness;
        ofVec3f rightPoint = thisPoint + toTheRight * thickness;

        mesh.addVertex(ofVec3f(leftPoint.x, leftPoint.y, leftPoint.z));
        mesh.addVertex(ofVec3f(rightPoint.x, rightPoint.y, rightPoint.z));
    }
}


//--------------------------------------------------------------
void ofApp::createNewLine(){
    lines.push_back(ofPolyline());

    ofVboMesh vm;
    vm.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
    lineMeshes.push_back(vm);
    lineOffsets.push_back(0);
}


//--------------------------------------------------------------
extern "C"
JNIEXPORT void JNICALL
Java_cc_openframeworks_ardrawing_OFActivity_clearDrawing(JNIEnv *env, jclass type) {
    ofNotifyEvent(clearDrawingEvent);
}

//--------------------------------------------------------------
void ofApp::clearDrawing(){
    lines.clear();
    lineOffsets.clear();
    lineMeshes.clear();
}

//--------------------------------------------------------------
void ofApp::touchDown(int x, int y, int id){
    _touchDown = true;
    createNewLine();
}

//------------------------------------o--------------------------
void ofApp::touchMoved(int x, int y, int id){

}

//--------------------------------------------------------------
void ofApp::touchUp(int x, int y, int id){
    _touchDown = false;
}

