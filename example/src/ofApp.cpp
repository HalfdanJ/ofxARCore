#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	tango.setup();
    initialized = false;
//    grabber.setup(1280, 960);

    ofBackground(0);

    ofSetOrientation(OF_ORIENTATION_DEFAULT, false);
}

//--------------------------------------------------------------
void ofApp::update(){
    tango.update();
    if(!initialized && tango.isInitialized()){
        initialized = true;
//        projectionMatrix = tango.getCameraIntrinsics();

    }

}

//--------------------------------------------------------------
void ofApp::draw(){
//grabber.draw(0,0);
    ofSetColor(255);

    if(initialized) {

        tango.draw();

        ofSetMatrixMode(OF_MATRIX_PROJECTION);
        ofLoadMatrix(tango.getProjectionMatrix());
        ofSetMatrixMode(OF_MATRIX_MODELVIEW);

        ofLoadMatrix(tango.getViewMatrix());

//        ofSetColor(255, 0, 0);
        ofDrawBox(0.1);

        ofDrawGrid(0.1, 30, true, true, true, true);
    }

}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){ 
	
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){ 
	
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::touchDown(int x, int y, int id){

}

//--------------------------------------------------------------
void ofApp::touchMoved(int x, int y, int id){

}

//--------------------------------------------------------------
void ofApp::touchUp(int x, int y, int id){

}

//--------------------------------------------------------------
void ofApp::touchDoubleTap(int x, int y, int id){

}

//--------------------------------------------------------------
void ofApp::touchCancelled(int x, int y, int id){

}

//--------------------------------------------------------------
void ofApp::swipe(ofxAndroidSwipeDir swipeDir, int id){

}

//--------------------------------------------------------------
void ofApp::pause(){

}

//--------------------------------------------------------------
void ofApp::stop(){

}

//--------------------------------------------------------------
void ofApp::resume(){

}

//--------------------------------------------------------------
void ofApp::reloadTextures(){

}

//--------------------------------------------------------------
bool ofApp::backPressed(){
	return false;
}

//--------------------------------------------------------------
void ofApp::okPressed(){

}

//--------------------------------------------------------------
void ofApp::cancelPressed(){

}
