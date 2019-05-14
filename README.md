<a href="https://play.google.com/store/apps/details?id=cc.openframeworks.ardrawing">
<img src=".img/dl.png" width="200"/>
</a>

# ofxARCore
Experimental addon for openFrameworks to use [ARCore](https://developers.google.com/ar) on Android devices.

<div style="position: relative; width: 90%; height: 100%; margin-left:5%; margin-right: 5%;display: flex; justify-content: center; float: left;">
<img align="left" src=".img/demo.gif" style="padding-left:1%; padding-right:1%" width="45%" />
<img align="left" src=".img/demo.webp" style="padding-left:1%; padding-right:1%" width="50%" />
</div>





## About
This addon is based on the work of [HalfdanJ](https://github.com/HalfdanJ/).
This is not an official Google product.

## Developer guide
To use the addon, you need the development branch of openFrameworks from [github](http://github.com/openFrameworks/openFrameworks).  Follow the [Android Studio guide](https://github.com/openframeworks/openFrameworks/blob/master/docs/android_studio.md) to learn how to get started with openFrameworks and Android.

To add the addon, add `ofxARCore` to `addons.make` in your project, or through the project generator. Additionally you will need to add the following two lines to the end of `settings.gradle` of your project:

## What is implemented

### Anchor

```h
// ofApp.h

ofxARCore arcore;
vector<ofMatrix4x4> anchors;
```

```cpp
// ofApp.cpp

void ofApp::setup() {
  arcore.setup();
}

void ofApp::update() {
  anchors.push_back(arcore.getViewMatrix().getInverse());
}

void RafalleApp::draw() {

  for (int i = 0; i < anchors.size(); i++) {

    ofMatrix4x4 anchor = anchors[i];
    ofPushMatrix();
    ofMultMatrix(anchor);

    ofDrawBox(ofVec3f(0,0,0), 0.05); // Draws a 5cm box at anchor location

    ofPopMatrix();
  }


  ofDisableDepthTest();

}


```

### Point Cloud

```h
// ofApp.h

vector<float> point_cloud;
ofVbo vbo_pointcloud;
```

```cpp
// ofApp.cpp

void ofApp::setup() {
  for(int i = 0; i < point_cloud.size(); i++) {
    point_color.push_back(ofColor::red);
  }
  vbo_pointcloud.setVertexData(&point_cloud[0], 3, 1000, GL_DYNAMIC_DRAW);
  vbo_pointcloud.setColorData(&point_color[0], 1000, GL_DYNAMIC_DRAW);
  point_color.clear();
}

void ofApp::update() {
  point_cloud = arcore->getPointCloud();
  if (point_cloud.size() > 1) {
    for (int i = 0; i < point_cloud.size(); i++)
    point_color.push_back(ofColor::red);

    vbo_pointcloud.updateVertexData(&point_cloud[0], (int) point_cloud.size() * 2);
    vbo_pointcloud.updateColorData(&point_color[0], (int) point_color.size() * 2);
  }

  for (int i = 0; i < point_cloud.size(); i+=3) {
    ofVec3f pos(point_cloud[i], point_cloud[i+1], point_cloud[i+2]);
    points.push_back(pos);
  }
}

for (int i = 0; i < points.size(); i+=3) {
  ofDrawBox(points[i], 0.01); // Draws a 1cm box at each point of point cloud
}

```

### Augmented Images
```cpp
// ofApp.cpp

void ofApp::draw() {

  vector<ofAugmentedImage*> augmented_images = arcore->getImageMatrices();

  // draw a box above each detected image
  for (int i = 0; i < augmented_images.size(); i++) {

    if (augmented_images[i]->is_tracking == true) {

      // get AugmentedImage position
      ofMatrix4x4 anchor = augmented_images[i]->matrix;
      ofPushMatrix();
      // translate to AugmentedImage position
      ofMultMatrix(anchor);

      ofBoxPrimitive box;

      // set box dimentions according to arcore image width estimation
      box.set(augmented_images[i]->width, 0.01, augmented_images[i]->height);
      box.setPosition(0,0,0);

      // draw box above the image
      box.draw();


      ofPopMatrix();
    }
  }

}
```

### Hit testing

```cpp

void ofApp::touchDown(int x, int y, int id) {

  ofHitPose *hitPose = arcore->getHitPose(x, y);

  if (pose != NULL) {
    ofMatrix4x4 pose = hitPose->pose;
    float distance   = hitPose->distance;

    // translate to the hit location
    ofPushMatrix();
    ofMultMatrix(pose);

    // draw a box at the hit location
    ofDrawBox(0,0,0, 0.1);

  }

}
```


### Planes

```cpp

void ofApp::draw() {

    vector<ofARPlane*> planes = arcore->getPlanes();

    // for each plane
    for (int i = 0; i < planes.size(); i++) {

        // translate to it's center
        ofARPlane *plane= planes[i];
        ofPushMatrix();
        ofMultMatrix(plane->center);

        // draw a red box on it's center
        ofSetColor(255,0,0,100);
        ofDrawBox(0,0.025,0, 0.2, 0.05, 0.1);

        // draw the plane
        ofSetColor(0,255,0,100);
        plane->mesh.draw();

        // draw the path (contours)
        ofSetColor(0,0,255,100);
        plane->path.draw();

        ofPopMatrix();
    }
```




### Utils
#### Camera FOV
```cpp
// ofApp.cpp

arcore.getCameraFOV();
```
#### Screen DPI
```cpp
// ofApp.cpp

arcore.getDpi();
```
