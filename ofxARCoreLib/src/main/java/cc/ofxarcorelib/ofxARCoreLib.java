package cc.ofxarcorelib;

//import com.google.android.gms.vision.Frame;
//import android.graphics.ImageFormat;
import android.app.Activity;
import android.content.Context;
import android.hardware.display.DisplayManager;
import android.opengl.Matrix;
import android.os.Looper;
import android.os.MessageQueue;
import android.util.Log;
import android.view.Display;
import android.widget.Toast;
//
//import java.nio.ByteBuffer;
//
import com.google.ar.core.Config;
import com.google.ar.core.Frame;
import com.google.ar.core.Session;
import com.google.ar.core.exceptions.CameraException;

import cc.openframeworks.OFActivity;
import cc.openframeworks.OFAndroid;


import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.logging.Handler;


public class ofxARCoreLib {
	private static final String TAG = "ofxARCoreLib";


	private Config mDefaultConfig;
	private Session mSession;

	private boolean mIsReady = false;
	private int mTexId;
	private AtomicBoolean mIsFrameAvailableTangoThread = new AtomicBoolean(false);
	private int mDisplayRotation = 0;
//	private TangoSupport.MatrixTransformData matrixTransformData;
//	private final float[] textureCoords0 =
//			new float[]{1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
//	float[] textureUV = new float[8];

	FloatBuffer textureUV = FloatBuffer.allocate(8);
	FloatBuffer textureUVTransformed = FloatBuffer.allocate(8);


	float[] mViewMatrix = new float[16];
	float[] mProjectionMatrix = new float[16];

//	public ofxARCoreLib(){
//
//		Context context = OFAndroid.getContext();
//		mSession = new Session((Activity) context);
//
//	}



	public void setup(int texId){
		Log.d(TAG, "SETUP");
		Context context = OFAndroid.getContext();

		((Activity)context).runOnUiThread(new Runnable() {
			@Override
			public void run() {
				Context context = OFAndroid.getContext();

//		DisplayManager displayManager = (DisplayManager)
//				context.getSystemService(Context.DISPLAY_SERVICE);
//		Log.i(TAG, String.valueOf(displayManager));

				mSession = new Session((Activity) context);

				// Create default config, check is supported, create session from that config.
				mDefaultConfig = Config.createDefaultConfig();
				if (!mSession.isSupported(mDefaultConfig)) {
					Toast.makeText(context, "This device does not support AR", Toast.LENGTH_LONG).show();
					return;
				}
				mSession.resume(mDefaultConfig);


				mSession.setCameraTextureName(mTexId);
				mSession.setDisplayGeometry(1080, 1920);

				ByteBuffer bbTexCoords = ByteBuffer.allocateDirect(4  * 2 * 4);
				bbTexCoords.order(ByteOrder.nativeOrder());
				textureUV = bbTexCoords.asFloatBuffer();
				textureUV.put(QUAD_TEXCOORDS);
				textureUV.position(0);


				ByteBuffer bbTexCoordsTransformed = ByteBuffer.allocateDirect(4  * 2 * 4);
				bbTexCoordsTransformed.order(ByteOrder.nativeOrder());
				textureUVTransformed = bbTexCoordsTransformed.asFloatBuffer();


//				mIsReady = true;
			}//public void run() {
		});


		mTexId = texId;


//		mTango = new Tango(context, new Runnable() {
//			// Pass in a Runnable to be called from UI thread when Tango is ready; this Runnable
//			// will be running on a new thread.
//			// When Tango is ready, we can call Tango functions safely here only when there are no
//			// UI thread changes involved.
//			@Override
//			public void run() {
//				synchronized (ofxARCoreLib.this) {
//					try {
//						mConfig = setupTangoConfig(mTango);
//						mTango.connect(mConfig);
//						startupTango();
//					} catch (TangoOutOfDateException e) {
//						Log.e(TAG, "Out of date", e);
////						showsToastAndFinishOnUiThread(R.string.exception_out_of_date);
//					} catch (TangoErrorException e) {
//						Log.e(TAG, "Tango error", e);
////						showsToastAndFinishOnUiThread(R.string.exception_tango_error);
//					} catch (TangoInvalidException e) {
//						Log.e(TAG, "Tango invalid exception", e);
////						showsToastAndFinishOnUiThread(R.string.exception_tango_invalid);
//					}
//				}
//			}
//		});
	}

	/**
	 * Sets up the tango configuration object. Make sure mTango object is initialized before
	 * making this call.
	 */
//	private TangoConfig setupTangoConfig(Tango tango) {
//		// Create a new Tango Configuration and enable the HelloMotionTrackingActivity API.
//		TangoConfig config = tango.getConfig(TangoConfig.CONFIG_TYPE_DEFAULT);
//		config.putBoolean(TangoConfig.KEY_BOOLEAN_MOTIONTRACKING, true);
//
//		config.putBoolean(TangoConfig.KEY_BOOLEAN_LOWLATENCYIMUINTEGRATION, true);
//
//		// Tango Service should automatically attempt to recover when it enters an invalid state.
//		config.putBoolean(TangoConfig.KEY_BOOLEAN_AUTORECOVERY, true);
//
//		config.putBoolean(TangoConfig.KEY_BOOLEAN_DRIFT_CORRECTION, true);
//
//		return config;
//	}

	public boolean isInitialized(){
//		if(mTango == null){
//			return false;
//		}
		return mIsReady;
	}

	/**
	 * Set up the callback listeners for the Tango Service and obtain other parameters required
	 * after Tango connection.
	 * Listen to new Pose data.
	 */
	private void startupTango() {
//		// Lock configuration and connect to Tango.
//		// Select coordinate frame pair.
//		final ArrayList<TangoCoordinateFramePair> framePairs =
//				new ArrayList<TangoCoordinateFramePair>();
//		framePairs.add(new TangoCoordinateFramePair(
//				TangoPoseData.COORDINATE_FRAME_START_OF_SERVICE,
//				TangoPoseData.COORDINATE_FRAME_DEVICE));
//
//		// Listen for new Tango data.
//		mTango.connectListener(framePairs, new Tango.TangoUpdateCallback() {
//			@Override
//			public void onPoseAvailable(final TangoPoseData pose) {
////				logPose(pose);
//			}
//
//			@Override
//			public void onXyzIjAvailable(TangoXyzIjData xyzIj) {
//				// We are not using onXyzIjAvailable for this app.
//			}
//
//			@Override
//			public void onPointCloudAvailable(TangoPointCloudData pointCloud) {
//				// We are not using onPointCloudAvailable for this app.
//			}
//
//			@Override
//			public void onTangoEvent(final TangoEvent event) {
//				// Ignoring TangoEvents.
//			}
//
//			@Override
//			public void onFrameAvailable(int cameraId) {
//				if (cameraId == TangoCameraIntrinsics.TANGO_CAMERA_COLOR) {
//					mIsFrameAvailableTangoThread.set(true);
//
//				}
//			}
//		});
//
//		TangoSupport.initialize(mTango);
//		updateTexture();
//		mIsReady = true;

	}


//
//	/**
//	 * Log the Position and Orientation of the given pose in the Logcat as information.
//	 *
//	 * @param pose the pose to log.
//	 */
//	private void logPose(TangoPoseData pose) {
//		StringBuilder stringBuilder = new StringBuilder();
//
//		float translation[] = pose.getTranslationAsFloats();
//		stringBuilder.append("Position: " +
//				translation[0] + ", " + translation[1] + ", " + translation[2]);
//
//		float orientation[] = pose.getRotationAsFloats();
//		stringBuilder.append(". Orientation: " +
//				orientation[0] + ", " + orientation[1] + ", " +
//				orientation[2] + ", " + orientation[3]);
//
//		Log.i(TAG, stringBuilder.toString());
//	}

	public float[] getViewMatrix(){
		return mViewMatrix;
	}

	public float[] getProjectionMatrix(){
		return mProjectionMatrix;
	}

	private void updateTexture(){
		mSession.setCameraTextureName(mTexId);
//		textureUV = TangoSupport.getVideoOverlayUVBasedOnDisplayRotation(textureCoords0, mDisplayRotation);\
	}

	public float[] getTextureUv(){
//		Log.i(TAG, "Get texture "+mIsReady);
//		return new float[16];
		float[] ret = new float[8];
		textureUVTransformed.position(0);
		textureUVTransformed.get(ret);
		return ret;
//		return textureUVTransformed.array();
	}

//	/**
//	 * Use Tango camera intrinsics to calculate the projection Matrix for the OpenGL scene.
//	 *
//	 * @param intrinsics camera instrinsics for computing the project matrix.
//	 */
//	private static float[] projectionMatrixFromCameraIntrinsics(TangoCameraIntrinsics intrinsics) {
//		float cx = (float) intrinsics.cx;
//		float cy = (float) intrinsics.cy;
//		float width = (float) intrinsics.width;
//		float height = (float) intrinsics.height;
//		float fx = (float) intrinsics.fx;
//		float fy = (float) intrinsics.fy;
//
//		// Uses frustumM to create a projection matrix, taking into account calibrated camera
//		// intrinsic parameter.
//		// Reference: http://ksimek.github.io/2013/06/03/calibrated_cameras_in_opengl/
//		float near = 0.1f;
//		float far = 100;
//
//		float xScale = near / fx;
//		float yScale = near / fy;
//		float xOffset = (cx - (width / 2.0f)) * xScale;
//		// Color camera's coordinates has y pointing downwards so we negate this term.
//		float yOffset = -(cy - (height / 2.0f)) * yScale;
//
//		float m[] = new float[16];
//		Matrix.frustumM(m, 0,
//				xScale * (float) -width / 2.0f - xOffset,
//				xScale * (float) width / 2.0f - xOffset,
//				yScale * (float) -height / 2.0f - yOffset,
//				yScale * (float) height / 2.0f - yOffset,
//				near, far);
//		return m;
//	}



	public void update(){
//		if(!mIsReady) return;

		Frame frame = null;
		try {
			frame = mSession.update();
			if (frame.isDisplayRotationChanged()) {
				frame.transformDisplayUvCoords(textureUV, textureUVTransformed);
			}

			// If not tracking, don't draw 3d objects.
			if (frame.getTrackingState() == Frame.TrackingState.NOT_TRACKING) {
				return;
			}

			frame.getViewMatrix(mViewMatrix, 0);

			mSession.getProjectionMatrix(mProjectionMatrix, 0, 0.1f, 100.0f);
			mIsReady = true;

//			final float lightIntensity = frame.getLightEstimate().getPixelIntensity();


		} catch (Throwable e) {
			e.printStackTrace();
		}



//		if (mIsFrameAvailableTangoThread.compareAndSet(true, false)) {

//			double mRgbTimestampGlThread = mTango.updateTexture(TangoCameraIntrinsics.
//					TANGO_CAMERA_COLOR);
//
//			// Get the transform from color camera to Start of Service
//			// at the timestamp of the RGB image in OpenGL coordinates.
//			matrixTransformData =
//					TangoSupport.getMatrixTransformAtTime(
//							mRgbTimestampGlThread,
//							TangoPoseData
//									.COORDINATE_FRAME_START_OF_SERVICE,
//							TangoPoseData.COORDINATE_FRAME_CAMERA_COLOR,
//							TangoSupport.ENGINE_OPENGL,
//							TangoSupport.ENGINE_OPENGL,
//							mDisplayRotation);
//		}
	}

	public static final float[] QUAD_TEXCOORDS = new float[]{
			0.0f, 1.0f,
			0.0f, 0.0f,
			1.0f, 1.0f,
			1.0f, 0.0f,
	};
}
