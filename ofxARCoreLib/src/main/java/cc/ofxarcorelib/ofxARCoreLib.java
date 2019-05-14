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

package cc.ofxarcorelib;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.SizeF;
import android.util.SparseArray;
import android.view.Surface;
import android.widget.Toast;

import com.google.ar.core.Anchor;
import com.google.ar.core.AugmentedImage;
import com.google.ar.core.AugmentedImageDatabase;
import com.google.ar.core.Config;
import com.google.ar.core.Frame;
import com.google.ar.core.HitResult;
import com.google.ar.core.Plane;
import com.google.ar.core.Pose;
import com.google.ar.core.Session;
import com.google.ar.core.TrackingState;
import com.google.ar.core.PointCloud;
import com.google.ar.core.exceptions.CameraNotAvailableException;
import com.google.ar.core.exceptions.NotTrackingException;
import com.google.ar.core.exceptions.UnavailableApkTooOldException;
import com.google.ar.core.exceptions.UnavailableArcoreNotInstalledException;
import com.google.ar.core.exceptions.UnavailableDeviceNotCompatibleException;
import com.google.ar.core.exceptions.UnavailableSdkTooOldException;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import cc.openframeworks.OFAndroid;
import cc.openframeworks.OFAndroidObject;

import java.io.IOException;
import java.io.InputStream;
import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

public class ofxARCoreLib extends OFAndroidObject {
    private static final String TAG = "ofxARCoreLib";

    private Config mDefaultConfig;
    private Session mSession;
    private SparseArray<Plane> _detectedPlanes = new SparseArray();
    private boolean mIsReady = false;
    private int mTexId;

    private FloatBuffer mTextureUV = FloatBuffer.allocate(8);
    private FloatBuffer mTextureUVTransformed = FloatBuffer.allocate(8);
    private boolean mTextureUVDirty;

    private float[] mViewMatrix = new float[16];
    private float[] mProjectionMatrix = new float[16];
    private float[] mAnchorMatrix = new float[16];
    private ArrayList<float[]>  augImg_matrices = new ArrayList<float[]>();// = new float[16];

    private TrackingState mTrackingState = TrackingState.STOPPED;

    private Pose mPose;
    private ArrayList<Anchor> mAnchors = new ArrayList<>();
    private Frame frame = null;


    // boehm-e
    public float horizontalAngle;
    public float verticalAngle;
    public float screenDpi;

    // boehm-e point cloud
    private PointCloud pointcloud_data = null;
    private FloatBuffer pcloud_buffer;
    private float[] pcloud_array;

    // boehm-e plane
    private ArrayList<float[]>  planes = new ArrayList<float[]>();// = new float[16];
    float[] _planeMatrix = new float[16];
    float[] _planeVertices = new float[12];

    private boolean enableAugmentedImages;


    public void setup(int texId, final int width, final int height, int enableAugmentedImages){
        Context context = OFAndroid.getContext();

        if (enableAugmentedImages == 1) {
            this.enableAugmentedImages = true;
        } else {
            this.enableAugmentedImages = false;
        }


        // boehm-e
        // calculate camera fov

        CameraManager manager = (CameraManager) context.getSystemService(Context.CAMERA_SERVICE);
        calculateFOV(manager);

        // calculate dpi
        DisplayMetrics displaymetrics = new DisplayMetrics();

        activity.getWindowManager().getDefaultDisplay().getMetrics(displaymetrics);

        screenDpi = context.getResources().getDisplayMetrics().xdpi;



        mTexId = texId;

        final boolean _enableAugmentedImages = this.enableAugmentedImages;

        ((Activity)context).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Context context = OFAndroid.getContext();

                try {
                    mSession = new Session((Activity) context);

                } catch (UnavailableArcoreNotInstalledException e) {
                    Log.d("DEBUG ERWAN", "ERROR UnavailableArcoreNotInstalledException");
                    e.printStackTrace();
                } catch (UnavailableApkTooOldException e) {
                    Log.d("DEBUG ERWAN", "ERROR UnavailableApkTooOldException");
                    e.printStackTrace();
                } catch (UnavailableSdkTooOldException e) {
                    Log.d("DEBUG ERWAN", "ERROR UnavailableSdkTooOldException");
                    e.printStackTrace();
                } catch (UnavailableDeviceNotCompatibleException e) {
                    Log.d("DEBUG ERWAN", "ERROR UnavailableDeviceNotCompatibleException");
                    e.printStackTrace();
                }

                // Create default config, check is supported, create session from that config.
                mDefaultConfig =  new Config(mSession);
                if (!mSession.isSupported(mDefaultConfig)) {
                    Toast.makeText(context, "This device does not support AR", Toast.LENGTH_LONG).show();
                    return;
                }

                // boehm-e AugmentedImageDatabase
                if (_enableAugmentedImages == true) {
                    AugmentedImageDatabase db = createAugmentedImageDatabase();
                    for (int i=0; i<db.getNumImages(); i++) {
                        float[] new_matrix = new float[20];
                        new_matrix[18] = i;
                        new_matrix[19] = 0; // NOT TRACKING
                        augImg_matrices.add(new_matrix);
                    }
                    mDefaultConfig.setAugmentedImageDatabase(db);
                }

                // boehm-e Set Auto Focus
                mDefaultConfig.setFocusMode(Config.FocusMode.AUTO);

                mSession.configure(mDefaultConfig);

                mSession.setCameraTextureName(mTexId);
                mSession.setDisplayGeometry(Surface.ROTATION_0, width, height);

                // Allocate UV coordinate buffers
                ByteBuffer bbTexCoords = ByteBuffer.allocateDirect(4  * 2 * 4);
                bbTexCoords.order(ByteOrder.nativeOrder());
                mTextureUV = bbTexCoords.asFloatBuffer();
                mTextureUV.put(QUAD_TEXCOORDS);
                mTextureUV.position(0);

                ByteBuffer bbTexCoordsTransformed = ByteBuffer.allocateDirect(4  * 2 * 4);
                bbTexCoordsTransformed.order(ByteOrder.nativeOrder());
                mTextureUVTransformed = bbTexCoordsTransformed.asFloatBuffer();

                try {
                    mSession.resume();
                } catch (CameraNotAvailableException e) {
                    e.printStackTrace();
                }

            }
        });
    }

    public boolean isInitialized(){
        return mIsReady;
    }

    public boolean isTracking(){
        return mTrackingState == TrackingState.TRACKING;
    }

    public float[] getViewMatrix(){
        return mViewMatrix;
    }

    public float[][] getImageMatrices(){
        int size = augImg_matrices.size();
        float [][] matrices = new float[size][16];
        for (int i = 0; i<size; i++) {
            matrices[i] = augImg_matrices.get(i);
        }
        return matrices;
    }

    public float[][] getPlanes(){
        int size = planes.size();
        float [][] matrices = new float[size][];
        for (int i = 0; i<size; i++) {
            matrices[i] = planes.get(i);
        }
        return matrices;
    }


    /* point cloud @kashimAstro */
    public float[] getPointCloud()
    {
        return pcloud_array;
    }

    public float[] getProjectionMatrix(float near, float far){
        if(mIsReady) {
            try {
                mSession.update().getCamera().getProjectionMatrix(mProjectionMatrix, 0, near, far);
            } catch (CameraNotAvailableException e) {
                e.printStackTrace();
            }
        }
        return mProjectionMatrix;
    }

    public float[] getTextureUv(){
        float[] ret = new float[8];
        mTextureUVTransformed.position(0);
        mTextureUVTransformed.get(ret);
        return ret;
    }

    public void addAnchor(){
        if(!mIsReady) return;
        try {
            Anchor a = mSession.createAnchor(mPose);
            mAnchors.add(a);
        } catch (NotTrackingException e) {
            e.printStackTrace();
        }
    }

    public float[] getAnchorPoseMatrix(int index){
        if(mAnchors.size() <= index) return mAnchorMatrix;
        mAnchors.get(index).getPose().toMatrix(mAnchorMatrix, 0);
        return mAnchorMatrix;
    }

    public boolean textureUvDirty(){
        if(mTextureUVDirty){
            mTextureUVDirty = false;
            return true;
        }
        return false;
    }

    public void setup2(int texId){    // this could have a better name
        mTexId = texId;
        updateTexture();
    }

    private String toHex(String arg) {
        return String.format("%040x", new BigInteger(1, arg.getBytes(/*YOUR_CHARSET?*/)));
    }

    public float[] hitTest(int x, int y) {
        float[] hitPose = new float[18];
        hitPose[17] = 0; // no hit found

        if (frame == null ) {
            return hitPose;
        }

        List<HitResult> hits = frame.hitTest(x, y);
        float minDist = 1000;
        HitResult hit = null;
        for (int i=0; i<hits.size(); i++) {
            if (hits.get(i).getDistance() < minDist) {
                minDist = hits.get(i).getDistance();
                hit = hits.get(i);
            }
        }

        if (hit != null) {
            Log.d("DEBUG ANDROID", "DEBUG ANDROID " + String.valueOf(hit.getDistance()));
            hit.getHitPose().toMatrix(hitPose, 0);
            hitPose[16] = hit.getDistance(); // distance
            hitPose[17] = 1;                 // hit found
        } else {
            Log.d("DEBUG ANDROID", "DEBUG ANDROID NO STRIKE");

        }

        return hitPose;
    }

    static float[] merge_float_array(float[] first, float[] second) {
        float[] both = Arrays.copyOf(first, first.length+second.length);
        System.arraycopy(second, 0, both, first.length, second.length);
        return both;
    }

    // boehm-e update plane
    private void updatePlanes(Collection<Plane> planes) {
        if (!planes.isEmpty()) {
            for (Plane plane : planes) {
                plane.getCenterPose().toMatrix(this._planeMatrix, 0);
                if (plane.getSubsumedBy() == null && this._detectedPlanes.get(plane.hashCode()) == null) {
                    this._detectedPlanes.append(plane.hashCode(), plane);
                } else if (plane.getSubsumedBy() != null) {
                    this._detectedPlanes.delete(plane.hashCode());
                } else {
                }
            }
        }
    }

    private static void getPlaneVertices(Plane plane, float[] planeVertices) {
        planeVertices[0] = plane.getExtentX() / 2.0f;
        planeVertices[1] = 0.0f;
        planeVertices[2] = plane.getExtentZ() / 2.0f;
        planeVertices[3] = (-plane.getExtentX()) / 2.0f;
        planeVertices[4] = 0.0f;
        planeVertices[5] = plane.getExtentZ() / 2.0f;
        planeVertices[6] = (-plane.getExtentX()) / 2.0f;
        planeVertices[7] = 0.0f;
        planeVertices[8] = (-plane.getExtentZ()) / 2.0f;
        planeVertices[9] = plane.getExtentX() / 2.0f;
        planeVertices[10] = 0.0f;
        planeVertices[11] = (-plane.getExtentZ()) / 2.0f;
    }

    public void update(){
        if(mSession == null) return;

        try {
            frame = mSession.update();


            if (frame.hasDisplayGeometryChanged()) {
                mTextureUVTransformed.position(0);
                frame.transformDisplayUvCoords(mTextureUV, mTextureUVTransformed);
                mTextureUVDirty = true;
            }

            // If not tracking, return
            mTrackingState = frame.getCamera().getTrackingState();
            if (mTrackingState != TrackingState.TRACKING) {
                return;
            }

            mPose = frame.getCamera().getPose();
            frame.getCamera().getViewMatrix(mViewMatrix, 0);



            /* point cloud @kashimAstro */
            pointcloud_data = frame.acquirePointCloud();
            pcloud_buffer   = pointcloud_data.getPoints();
            pcloud_array 	=  new float[pcloud_buffer.limit()];
            pcloud_buffer.get(pcloud_array);

            mIsReady = true;

//			final float lightIntensity = frame.getLightEstimate().getPixelIntensity();


            // Plane detection boehm-e
            Collection<Plane> arcore_planes = frame.getUpdatedTrackables(Plane.class);

            updatePlanes(arcore_planes);
            planes.clear();

            for (int j=0; j<this._detectedPlanes.size(); j++) {
                Plane plane = this._detectedPlanes.valueAt(j);
                float[] mesh = plane.getPolygon().array();
                List<Float> transformed_mesh = new ArrayList<Float>();

                for (int i =0; i<mesh.length - 3; i+=1) {
                    float x = mesh[i];
                    float y = 0;
                    float z = mesh[i+1];
                    transformed_mesh.add(x);
                    transformed_mesh.add(y);
                    transformed_mesh.add(z);
                }

                float[] pose = new float[16];
                plane.getCenterPose().toMatrix(pose, 0);


                float[] floatArray = new float[transformed_mesh.size()];
                int i = 0;

                for (Float f : transformed_mesh) {
                    floatArray[i++] = (f != null ? f : 0); // Or whatever default you want.
                }

                planes.add(merge_float_array(pose, floatArray));
            }

            // AugmentedImageDatabase boehm-e
            if (this.enableAugmentedImages == true) {

                Collection<AugmentedImage> updatedAugmentedImages =
                        frame.getUpdatedTrackables(AugmentedImage.class);



                int index = 0;
                for (AugmentedImage img : updatedAugmentedImages) {
                    if (img.getTrackingState() == TrackingState.TRACKING && img.getTrackingMethod() == AugmentedImage.TrackingMethod.FULL_TRACKING) {
                        String name = img.getName();
                        Pose pose = img.getCenterPose();
                        float width = img.getExtentX();
                        float height = img.getExtentZ();
                        float[] new_matrix = new float[20];
                        pose.toMatrix(new_matrix, 0);
                        new_matrix[16] = width;
                        new_matrix[17] = height;
                        new_matrix[18] = img.getIndex();
                        new_matrix[19] = 1; // tracking
                        augImg_matrices.set(img.getIndex(), new_matrix);

                    } else {

                        float[] new_matrix = new float[20];
                        new_matrix[18] = img.getIndex();
                        new_matrix[19] = 0; // NOT TRACKING
                        augImg_matrices.set(img.getIndex(), new_matrix);
                    }
                    index++;
                }
            }


        } catch (Throwable e) {
            e.printStackTrace();
        }
    }

    private static final float[] QUAD_TEXCOORDS = new float[]{
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            1.0f, 0.0f,
    };

    private void updateTexture(){
        mSession.setCameraTextureName(mTexId);
    }





    //	boehm-e
    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    private void calculateFOV(CameraManager cManager) {
        try {
            for (final String cameraId : cManager.getCameraIdList()) {
                CameraCharacteristics characteristics = cManager.getCameraCharacteristics(cameraId);
                int cOrientation = characteristics.get(CameraCharacteristics.LENS_FACING);
                if (cOrientation == CameraCharacteristics.LENS_FACING_BACK) {
                    float[] maxFocus = characteristics.get(CameraCharacteristics.LENS_INFO_AVAILABLE_FOCAL_LENGTHS);
                    SizeF size = characteristics.get(CameraCharacteristics.SENSOR_INFO_PHYSICAL_SIZE);
                    float w = size.getWidth();
                    float h = size.getHeight();
                    horizontalAngle = w;
//					horizontalAngle = (float) (2*Math.atan(w/(maxFocus[0]*2)));
                    verticalAngle = (float) (2*Math.atan(h/(maxFocus[0]*2)));
                    return;
                }
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    public float getFOV() {
        Log.d("ofxARCoreLib.java", "HORIZONTAL FOV : " + horizontalAngle);
        float fov = (float) horizontalAngle;

        return fov;
//		return horizontalAngle;
    }

    public float getDPI() {
        Log.d("ofxARCoreLib.java", "SCREEN DPI : " + screenDpi);

        float dpi = (float) screenDpi;

        return dpi;
    }


    // boehm_e AugmentedImageDatabase
    public AugmentedImageDatabase createAugmentedImageDatabase() {
        AugmentedImageDatabase imageDatabase = new AugmentedImageDatabase(this.mSession);
        Bitmap bitmap;


        try {
            for(String imgName : OFAndroid.getContext().getAssets().list("AugmentedImageDatabase")){
                String imgPath = "AugmentedImageDatabase/"+imgName;
                InputStream inputStream = OFAndroid.getContext().getAssets().open(imgPath);
                bitmap = BitmapFactory.decodeStream(inputStream);
                int index = imageDatabase.addImage(imgName, bitmap);

                Log.d("DEBUG ERWAN", "ADDED IMAGE : "+imgPath);
            }
        } catch (IOException e) {
            Log.d("DEBUG ERWAN", "fail to load image");
            e.printStackTrace();
        }

        return imageDatabase;
    }





    @Override
    protected void appPause() {
        if(mSession == null) return;
        mSession.pause();
    }

    @Override
    protected void appResume() {
        if(mSession != null) {
            try {
                mSession.resume();
            } catch (CameraNotAvailableException e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    protected void appStop() {

    }
}
