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

#include <types/ofPoint.h>
#include <graphics/ofPath.h>
#include "ofxARCore.h"
#include "ofxAndroidUtils.h"

#ifdef TARGET_ANDROID

ofxARCore::ofxARCore(){
    if(!ofGetJavaVMPtr()){
        ofLogNotice("ofxARCore") << "couldn't find java virtual machine";
        return;
    }

    JNIEnv *env;
    if (ofGetJavaVMPtr()->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        ofLogNotice("ofxARCore") << "failed to get environment using GetEnv()";
        return;
    }

    jclass localClass = env->FindClass("cc/ofxarcorelib/ofxARCoreLib");
    javaClass = (jclass) env->NewGlobalRef(localClass);

    if(!javaClass){
        ofLogError("ofxARCore") << "constructor: couldn't get java class for ofxARCore";
        return;
    }

    jmethodID constructor = env->GetMethodID(javaClass,"<init>","()V");
    if(!constructor){
        ofLogError("ofxARCore") << "constructor: couldn't get java constructor for ofxARCore";
        return;
    }

    javaTango = env->NewObject(javaClass,constructor);
    if(!javaTango){
        ofLogError("ofxARCore") << "constructor: couldn't create java ofxARCore";
        return;
    }

    javaTango = (jobject)env->NewGlobalRef(javaTango);

    ofAddListener(ofxAndroidEvents().pause,this,&ofxARCore::pauseApp);
    ofAddListener(ofxAndroidEvents().resume,this,&ofxARCore::resumeApp);
}

ofxARCore::~ofxARCore(){
    ofRemoveListener(ofxAndroidEvents().pause,this,&ofxARCore::pauseApp);
    ofRemoveListener(ofxAndroidEvents().resume,this,&ofxARCore::resumeApp);
}

void ofxARCore::setup(bool enableAugmentedImages){
    _sessionInitialized = false;
    this->enableAugmentedImages = enableAugmentedImages;


    std::string path = "AugmentedImageDatabase";
    arcoreImagesDir = new ofDirectory(path);

    ofxAndroidRequestPermission(OFX_ANDROID_PERMISSION_CAMERA);
    // Request camera permissions
    if(ofxAndroidCheckPermission(OFX_ANDROID_PERMISSION_CAMERA)) {
        setupSession();
    }

    // Prepare texture mesh
    quad.getVertices().resize(4);
    quad.getTexCoords().resize(4);
    quad.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
    quad.getVertices()[0] = {0, 0, 0};
    quad.getVertices()[1] = {ofGetWidth(), 0, 0};
    quad.getVertices()[2] = {ofGetWidth(), ofGetHeight(), 0};
    quad.getVertices()[3] = {0, ofGetHeight(), 0};

    quad.getTexCoords()[0] = {0, 0};
    quad.getTexCoords()[1] = {1,0};
    quad.getTexCoords()[2] = {1,1};
    quad.getTexCoords()[3] = {0,1};
}

void ofxARCore::setupSession(){
    ofLogVerbose("ofxARCore") << "Initializing ARCore";
    GLuint texId = setupTexture();

    if (!javaTango) {
        ofLogError("ofxARCore") << "setup(): java ofxARCore not loaded";
        return;
    }

    JNIEnv *env = ofGetJNIEnv();
    jmethodID javaSetupMethod = env->GetMethodID(javaClass, "setup", "(IIII)V");
    if (!javaSetupMethod) {
        ofLogError("ofxARCore") << "setup(): couldn't get java setup for ofxARCore";
        return;
    }

    env->CallVoidMethod(javaTango, javaSetupMethod, texId, ofGetWidth(), ofGetHeight(), this->enableAugmentedImages);

    _sessionInitialized = true;
}

GLuint ofxARCore::setupTexture(){

    GLuint texId[1];
    glGenTextures(1, texId);

    glEnable(GL_TEXTURE_EXTERNAL_OES);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texId[0]);

    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (!ofIsGLProgrammableRenderer()) {
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }

    glDisable(GL_TEXTURE_EXTERNAL_OES);

    // Set the externally created texture reference
    texture.setUseExternalTextureID(texId[0]);
    texture.texData.width = 1;
    texture.texData.height = 1;
    texture.texData.tex_w = 1;
    texture.texData.tex_h = 1;
    texture.texData.tex_t = 1; // Hack!
    texture.texData.tex_u = 1;
    texture.texData.textureTarget = GL_TEXTURE_EXTERNAL_OES;
    texture.texData.glInternalFormat = GL_RGBA;
    return texId[0];
}

void ofxARCore::reloadTexture() {
    GLuint texId = setupTexture();
    JNIEnv *env = ofGetJNIEnv();
    jmethodID javaSetupMethod = env->GetMethodID(javaClass, "setup2", "(I)V");
    if (!javaSetupMethod) {
        ofLogError("ofxARCore") << "setup(): couldn't get java setup for ofxARCore";
        return;
    }
    env->CallVoidMethod(javaTango, javaSetupMethod, texId);
}

void ofxARCore::pauseApp(){
    JNIEnv *env = ofGetJNIEnv();
    env->CallVoidMethod(javaTango, env->GetMethodID(javaClass, "appPause", "()V"));
}

void ofxARCore::resumeApp(){

    setupSession();
    reloadTexture();

    JNIEnv *env = ofGetJNIEnv();
    env->CallVoidMethod(javaTango, env->GetMethodID(javaClass, "appResume", "()V"));

    // Check if permissions have been granted, and the session isn't setup yet
    if(!_sessionInitialized){
        if(ofxAndroidCheckPermission(OFX_ANDROID_PERMISSION_CAMERA)) {
            setupSession();
        }
    }
}

void ofxARCore::update(){
    JNIEnv *env = ofGetJNIEnv();
    jmethodID method = env->GetMethodID(javaClass, "update", "()V");
    env->CallVoidMethod(javaTango, method);


    // Check if textureUVs are marked dirty
    bool textureUvDirty =  env->CallBooleanMethod(javaTango,
                                                  env->GetMethodID(javaClass, "textureUvDirty", "()Z"));

    if(textureUvDirty){
        jmethodID method = env->GetMethodID(javaClass, "getTextureUv", "()[F");
        jfloatArray data = (jfloatArray) env->CallObjectMethod(javaTango, method);

        jboolean isCopy;
        jfloat *body =  env->GetFloatArrayElements(data, &isCopy);

        // Set texture UVs
        quad.getTexCoords()[0] = {body[2*2],1-body[2*2+1]};
        quad.getTexCoords()[1] = {body[0*2],1-body[0*2+1]};
        quad.getTexCoords()[2] = {body[1*2],1-body[1*2+1]};
        quad.getTexCoords()[3] = {body[3*2],1-body[3*2+1]};
    }
}

bool ofxARCore::isInitialized(){
    JNIEnv *env = ofGetJNIEnv();
    jmethodID method = env->GetMethodID(javaClass,"isInitialized","()Z");
    return env->CallBooleanMethod(javaTango, method);
}

float ofxARCore::getCameraFOV() {

    JNIEnv *env = ofGetJNIEnv();
    jmethodID javaGetFovMethod = env->GetMethodID(javaClass,"getFOV","()F");
    if(!javaGetFovMethod){
        ofLogError("javaGetFovMethod") << "javaGetFovMethod(): couldn't get java";
        return false;
    }
    jfloat fov = env->CallFloatMethod (javaTango, javaGetFovMethod);
    ofLog() << "FOVVV " << fov;

    return fov;
}

/* point cloud and plane */
std::vector<float> ofxARCore::getPointCloud() {
    JNIEnv *env = ofGetJNIEnv();
    jfloatArray data = (jfloatArray) env->CallObjectMethod(javaTango,
                                                           env->GetMethodID(javaClass,"getPointCloud","()[F"));

    jsize size = env->GetArrayLength(data);
    std::vector<float> result;
    result.resize((int)size);
    env->GetFloatArrayRegion(data, 0, size, (jfloat*)&result[0]);
    return result;
}


float ofxARCore::getDpi() {

    JNIEnv *env = ofGetJNIEnv();
    jmethodID javaGetDpiMethod = env->GetMethodID(javaClass,"getDPI","()F");
    if(!javaGetDpiMethod){
        ofLogError("javaGetDpiMethod ") << "javaGetDpiMethod (): couldn't get java";
        return false;
    }
    jfloat dpi = env->CallFloatMethod (javaTango, javaGetDpiMethod);
    ofLog() << "DPI " << dpi;

    return dpi;

}

bool ofxARCore::isTracking(){
    JNIEnv *env = ofGetJNIEnv();
    jmethodID method = env->GetMethodID(javaClass,"isTracking","()Z");
    return env->CallBooleanMethod(javaTango, method);
}

ofMatrix4x4 ofxARCore::getViewMatrix(){
    JNIEnv *env = ofGetJNIEnv();
    jfloatArray data = (jfloatArray) env->CallObjectMethod(javaTango,env->GetMethodID(javaClass,"getViewMatrix","()[F"));

    jboolean isCopy;
    jfloat *body =  env->GetFloatArrayElements(data, &isCopy);

    ofMatrix4x4 m;
    m.set(body);
    return m;
}


// boehm-e | hit pose of
ofHitPose *ofxARCore::getHitPose(int x, int y) {
    JNIEnv *env = ofGetJNIEnv();
    jfloatArray data = (jfloatArray) env->CallObjectMethod(javaTango,
                                                           env->GetMethodID(javaClass,"hitTest","(II)[F"),
                                                           x, y);
    jboolean isCopy;
    jfloat *body =  env->GetFloatArrayElements(data, &isCopy);


    int hit_ok = body[17];
    bool has_hit = hit_ok == 1;

    if (has_hit == false) {
        return NULL;
    }
    ofMatrix4x4 m;
    m.set(body);
    ofHitPose *pose = new ofHitPose{m, body[16]};
    return pose;
}


// boehm-e | augmented images of
std::vector<ofAugmentedImage*> ofxARCore::getImageMatrices(){

    std::vector<ofAugmentedImage*> matrices;

    JNIEnv *env = ofGetJNIEnv();
    jobjectArray objArray = (jobjectArray) env->CallObjectMethod(javaTango, env->GetMethodID(javaClass, "getImageMatrices", "()[[F"));

    int arraySize = env->GetArrayLength(objArray);
    for (int i=0; i < arraySize; i++)
    {
        jboolean isCopy;
        jobject array = env->GetObjectArrayElement(objArray, i);

        jfloat *body =  env->GetFloatArrayElements((jfloatArray)array, &isCopy);
        ofMatrix4x4 m;
        m.set(body);

        int index = body[18];

        std::string name = ofSplitString(arcoreImagesDir->getSorted().getPath(index), "/")[1];

        int is_tracking = body[19];
        bool tracking = is_tracking == 1;
        matrices.push_back(new ofAugmentedImage{m, body[16], body[17], index, tracking, name});

    }
    return matrices;
}


// boehm-e | augmented images of
std::vector<ofARPlane *> ofxARCore::getPlanes(){

    std::vector<ofARPlane*> planes;

    JNIEnv *env = ofGetJNIEnv();
    jobjectArray objArray = (jobjectArray) env->CallObjectMethod(javaTango, env->GetMethodID(javaClass, "getPlanes", "()[[F"));

    int arraySize = env->GetArrayLength(objArray);
    for (int i=0; i < arraySize; i++)
    {
        jboolean isCopy;
        jobject array = env->GetObjectArrayElement(objArray, i);

        jfloat *body =  env->GetFloatArrayElements((jfloatArray)array, &isCopy);
        ofMatrix4x4 m;
        m.set(body);

        ofMesh mesh;
        ofPath path;

        ofLog() << "DEBUG CPP PLANE " << env->GetArrayLength((jfloatArray )array) << std::endl;
        for (int i=16; i<env->GetArrayLength((jfloatArray )array); i+=3) {
//            if (i==16) {
//                path.moveTo(body[i], body[i+1]);
//            } else {
//                path.lineTo(body[i], body[i+1]);
//            }
            mesh.addVertex(ofPoint(body[i], body[i+1], body[i+2]));
//            mesh.addVertex(ofPoint(body[i+2], 0.0, body[i+3]));
//            mesh.addVertex(m.getTranslation());
//            mesh.addIndex(i-16);
//            mesh.addIndex(i-15);
//            mesh.addIndex(i-14);
        }
        for (int j=0; j<mesh.getVertices().size()-3; j++) {
            mesh.addIndex(j);
            mesh.addIndex(j+1);
            mesh.addIndex(j+2);
        }

        planes.push_back(new ofARPlane{mesh, path,  m});

    }
    return planes;
}

ofMatrix4x4 ofxARCore::getProjectionMatrix(float near, float far){
    JNIEnv *env = ofGetJNIEnv();
    jfloatArray data = (jfloatArray) env->CallObjectMethod(javaTango,
                                                           env->GetMethodID(javaClass,"getProjectionMatrix","(FF)[F"),
                                                           near, far);
    jboolean isCopy;
    jfloat *body =  env->GetFloatArrayElements(data, &isCopy);
    ofMatrix4x4 m;
    m.set(body);
    return m;
}

void ofxARCore::draw(){
    texture.bind();
    quad.draw();
    texture.unbind();
}

void ofxARCore::addAnchor(){
    JNIEnv *env = ofGetJNIEnv();
    jmethodID method = env->GetMethodID(javaClass, "addAnchor", "()V");
    env->CallVoidMethod(javaTango, method);
}

ofMatrix4x4 ofxARCore::getAnchor(int i){
    JNIEnv *env = ofGetJNIEnv();
    jfloatArray data = (jfloatArray) env->CallObjectMethod(javaTango,
                                                           env->GetMethodID(javaClass,"getAnchorPoseMatrix","(I)[F"),
                                                           i);
    jboolean isCopy;
    jfloat *body =  env->GetFloatArrayElements(data, &isCopy);
    ofMatrix4x4 m;
    m.set(body);
    return m;
}




#endif