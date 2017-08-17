#include "ofxARCore.h"
#ifdef TARGET_ANDROID
#include "ofxAndroidUtils.h"


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
}

ofxARCore::~ofxARCore(){
//    toAnalyze.close();
}

//bool ofxARCore::setThreaded(bool _threaded){
//    threaded = _threaded;
//}

void ofxARCore::setup(){
    _textureInitialized = false;
//    ofTextureData td;
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


    ofxAndroidRequestCameraPermission();
    if(ofxAndroidCheckCameraPermission()) {
        ofLog() << "Initializing tango";
        if (!javaTango) {
            ofLogError("ofxARCore") << "setup(): java ofxARCore not loaded";
            return;
        }

        JNIEnv *env = ofGetJNIEnv();
        jmethodID javaSetupMethod = env->GetMethodID(javaClass, "setup", "(I)V");
        if (!javaSetupMethod) {
            ofLogError("ofxARCore") << "setup(): couldn't get java setup for ofxARCore";
            return;
        }

        env->CallVoidMethod(javaTango, javaSetupMethod, texId[0]);
    }

    // Setup drawing mesh
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

void ofxARCore::update(){
    JNIEnv *env = ofGetJNIEnv();
    jmethodID method = env->GetMethodID(javaClass, "update", "()V");
    env->CallVoidMethod(javaTango, method);

    if(isInitialized() && !_textureInitialized){
        _textureInitialized = true;
        JNIEnv *env = ofGetJNIEnv();
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
    if(!javaTango){
        ofLogError("ofxARCore") << "isInitialized(): java ofxARCore not loaded";
        return false;
    }

    JNIEnv *env = ofGetJNIEnv();
    jmethodID javaSetupMethod = env->GetMethodID(javaClass,"isInitialized","()Z");
    if(!javaSetupMethod){
        ofLogError("ofxARCore") << "isInitialized(): couldn't get java for ofxARCore";
        return false;
    }

    return env->CallBooleanMethod(javaTango,javaSetupMethod);
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

ofMatrix4x4 ofxARCore::getProjectionMatrix(){
    JNIEnv *env = ofGetJNIEnv();
    jfloatArray data = (jfloatArray) env->CallObjectMethod(javaTango,env->GetMethodID(javaClass,"getProjectionMatrix","()[F"));

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
//
//void ofxAndroidMobileVision::setTrackProminentFaceOnly(bool prominentOnly){
//    if(!javaMobileVision) return;
//    JNIEnv *env = ofGetJNIEnv();
//    env->CallVoidMethod(javaMobileVision,env->GetMethodID(javaClass,"setProminentFaceOnly","(Z)V"), prominentOnly);
//}
//
//void ofxAndroidMobileVision::setMinFaceSize(float minFaceSize){
//    if(!javaMobileVision) return;
//    JNIEnv *env = ofGetJNIEnv();
//    env->CallVoidMethod(javaMobileVision,env->GetMethodID(javaClass,"setMinFaceSize","(F)V"), minFaceSize);
//}
//
//void ofxAndroidMobileVision::update(ofPixels &pixels){
//    if(threaded) {
//        if (toAnalyze.empty()) {
//            ToAnalyzeData d = ToAnalyzeData();
//            d.pixels = pixels;
//            toAnalyze.send(d);
//        }
//    } else {
//        process(pixels);
//    }
//
//    fromAnalyze.tryReceive(faces);
//
//}
//
//vector<ofxAndroidMobileVisionFace> & ofxAndroidMobileVision::getFaces(){
//    return faces;
//}
//
//void ofxAndroidMobileVision::process(ofPixels &pixels){
//    if(!javaMobileVision){
//        ofLogError("ofxAndroidMobileVision") << "update(): java not loaded";
//        return;
//    }
//
//    JNIEnv *env = ofGetJNIEnv();
//    jmethodID javaMethod = env->GetMethodID(javaClass,"update","([BII)I");
//    if(!javaMethod ){
//        ofLogError("ofxAndroidMobileVision") << "update(): couldn't get java update for MobileVision";
//        return;
//    }
//
//    jbyteArray arr = env->NewByteArray(pixels.size());
//    env->SetByteArrayRegion( arr, 0, pixels.size(), (const signed char*) pixels.getData());
//    int numFaces = env->CallIntMethod(javaMobileVision, javaMethod, arr, pixels.getWidth(), pixels.getHeight());
//    env->DeleteLocalRef(arr);
//
//    vector<ofxAndroidMobileVisionFace> analyzedfaces;
//    for(int i=0;i<numFaces;i++) {
//        // Get data
//        auto method = env->GetMethodID(javaClass, "getData", "(I)[F");
//        jfloatArray data = (jfloatArray) env->CallObjectMethod(javaMobileVision, method, 0);
//
//        jboolean isCopy;
//        jfloat *body =  env->GetFloatArrayElements(data, &isCopy);
//
//        ofxAndroidMobileVisionFace face;
//        face.smileProbability = body[0];
//        face.leftEyeOpenProbability = body[1];
//        face.rightEyeOpenProbability = body[2];
//        for(int j=0;j<12;j++){
//            ofVec2f p;
//            p.x = body[j*2+3];
//            p.y = body[j*2+4];
//            face.landmarks.push_back(p);
//        }
//        analyzedfaces.push_back(face);
//
//        env->DeleteLocalRef(data);
//    }
//
//    fromAnalyze.send(analyzedfaces);
//}
//
//void ofxAndroidMobileVision::threadedFunction(){
//    ToAnalyzeData d;
//    while(toAnalyze.receive(d)){
//        process(d.pixels);
//    }
//}
#endif