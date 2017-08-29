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
import android.util.Log;
import android.widget.Toast;

import com.google.ar.core.Anchor;
import com.google.ar.core.Config;
import com.google.ar.core.Frame;
import com.google.ar.core.Pose;
import com.google.ar.core.Session;
import com.google.ar.core.exceptions.NotTrackingException;

import cc.openframeworks.OFAndroid;
import cc.openframeworks.OFAndroidObject;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.util.ArrayList;

public class ofxARCoreLib extends OFAndroidObject {
	private static final String TAG = "ofxARCoreLib";

	private Config mDefaultConfig;
	private Session mSession;

	private boolean mIsReady = false;
	private int mTexId;

	private FloatBuffer mTextureUV = FloatBuffer.allocate(8);
	private FloatBuffer mTextureUVTransformed = FloatBuffer.allocate(8);
	private boolean mTextureUVDirty;

	private float[] mViewMatrix = new float[16];
	private float[] mProjectionMatrix = new float[16];
	private float[] mAnchorMatrix = new float[16];

	private Frame.TrackingState mTrackingState = Frame.TrackingState.NOT_TRACKING;

	private Pose mPose;
	private ArrayList<Anchor> mAnchors = new ArrayList<>();

	public void setup(int texId, final int width, final int height){
		Context context = OFAndroid.getContext();

		mTexId = texId;

		((Activity)context).runOnUiThread(new Runnable() {
			@Override
			public void run() {
				Context context = OFAndroid.getContext();

				mSession = new Session((Activity) context);

				// Create default config, check is supported, create session from that config.
				mDefaultConfig = Config.createDefaultConfig();
				if (!mSession.isSupported(mDefaultConfig)) {
					Toast.makeText(context, "This device does not support AR", Toast.LENGTH_LONG).show();
					return;
				}
				mSession.resume(mDefaultConfig);

				mSession.setCameraTextureName(mTexId);
				mSession.setDisplayGeometry(width, height);

				// Allocate UV coordinate buffers
				ByteBuffer bbTexCoords = ByteBuffer.allocateDirect(4  * 2 * 4);
				bbTexCoords.order(ByteOrder.nativeOrder());
				mTextureUV = bbTexCoords.asFloatBuffer();
				mTextureUV.put(QUAD_TEXCOORDS);
				mTextureUV.position(0);

				ByteBuffer bbTexCoordsTransformed = ByteBuffer.allocateDirect(4  * 2 * 4);
				bbTexCoordsTransformed.order(ByteOrder.nativeOrder());
				mTextureUVTransformed = bbTexCoordsTransformed.asFloatBuffer();
			}
		});
	}

	public boolean isInitialized(){
		return mIsReady;
	}

	public boolean isTracking(){
		return mTrackingState == Frame.TrackingState.TRACKING;
	}

	public float[] getViewMatrix(){
		return mViewMatrix;
	}

	public float[] getProjectionMatrix(float near, float far){
		if(mIsReady) mSession.getProjectionMatrix(mProjectionMatrix, 0, near, far);
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
			Anchor a = mSession.addAnchor(mPose);
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

	public void update(){
		if(mSession == null) return;

		Frame frame = null;
		try {
			frame = mSession.update();
			if (frame.isDisplayRotationChanged()) {
				mTextureUVTransformed.position(0);
				frame.transformDisplayUvCoords(mTextureUV, mTextureUVTransformed);
				mTextureUVDirty = true;
			}

			// If not tracking, return
			mTrackingState = frame.getTrackingState();
			if (mTrackingState == Frame.TrackingState.NOT_TRACKING) {
				return;
			}

			mPose = frame.getPose();
			frame.getViewMatrix(mViewMatrix, 0);

			mIsReady = true;

//			final float lightIntensity = frame.getLightEstimate().getPixelIntensity();

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

	@Override
	protected void appPause() {
		if(mSession == null) return;
		mSession.pause();
	}

	@Override
	protected void appResume() {
		if(mSession != null)
			mSession.resume(mDefaultConfig);
	}

	@Override
	protected void appStop() {
	}
}
