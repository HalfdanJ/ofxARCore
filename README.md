# ofxARCore
Experimental addon for openFrameworks to use [ARCore](https://developers.google.com/ar) on Android devices. 

![preview](preview.gif)

## About
This openFrameworks addon implements some of the basic features from the ARCore API to be used in openFrameworks. This addon is not feature complete yet. Currently it only supports the basic camera tracking, rendering the camera image, besides basic anchor support. Even though its not feature complete, it should give a good starting point to work with AR in openFrameworks.

This is not an official Google product.

## Developer guide
To use the addon, you need a special version of openFrameworks from [github](https://github.com/halfdanj/openframeworks/tree/android-camera-permissions). It should hopefully merged into the official openFrameworks soon. Follow the [Android Studio guide](https://github.com/openframeworks/openFrameworks/blob/master/docs/android_studio.md) to learn how to get started with openFrameworks and Android. 

To add the addon, add `ofxARCore` to `addons.make` in your project, or through the project generator. Additionally you will need to add the following two lines to the end of `settings.gradle` of your project:

```
include ':arcore_client'
project(':arcore_client').projectDir = new File(ofRoot + 'addons/ofxARCore/ofxARCoreLib/arcore_client')
```

To see basic usage of the addon, see the [example code](exampleBasic/src/ofApp.cpp)

## Examples
### [Basic example](exampleBasic/)
Shows how to use the addon in the most basic way, just rendering a grid centered around origin. 

### [Drawing example](exampleDrawing/)
A simple demo that lets you draw in the air. 

The app calculates the world coordinates from touch events on the screen, and draws a continuous white line through the air. Read more on [AR Experiments site](https://experiments.withgoogle.com/ar).




