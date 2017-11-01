package com.example.anothereality.testcamerarender;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Point;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.opengl.GLES30;
import android.opengl.GLSurfaceView.Renderer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import android.hardware.camera2.*;
import android.os.HandlerThread;
import android.util.Log;
import android.util.Size;
import android.view.Surface;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;
import android.os.Handler;


/**
 * Created by HF on 07/10/17.
 */

public class CameraRenderer implements SurfaceTexture.OnFrameAvailableListener
{
    private static final String TAG = "";
    private HandlerThread mBackgroundThread;

    public native long createRenderer();
    public native int getGLCameraTextureID(long ndkRenderer);
    public native int decodeAndEnqueueCameraTexture(long ndkRenderer, long timestamp);
    public native void onSurfaceRender(long ndkRenderer, Surface surface);
    public native void setCameraTransform(long ndkRenderer, float [] matrix);
    public native void onSurfaceCreated(long ndkRenderer, Surface surface, AssetManager manager, Activity ActivityObject, int VRRenderSurfaceWidth, int VRRenderSurfaceHeight);
    public native void onSurfaceChanged(long ndkRenderer);
    public native void onResume(long ndkRenderer);
    public native void onPause(long ndkRenderer);
    public native long deleteRenderer(long ndkRenderer);

    static {
        System.loadLibrary("camera_textureview");
    }

    private Surface CurrentView;
    private String CurrentCameraID;
    private Size CamSize;
    private Semaphore mCameraOpenCloseLock = new Semaphore(1);
    private CameraDevice CurrentCameraDevice;
    private Handler BackgroundHandler;
    private SurfaceTexture CurrentSurfaceTexture;
    private boolean SurfaceTextureAvailable;
    private CaptureRequest.Builder CurrentCaptureBuilder;
    private CameraCaptureSession CurrentCaptureSession;
    private Surface CurrentCaptureSurface;
    private float CurrentUVMatrix[];
    private boolean CameraClosed = false;
    int ScreenWidth;
    int ScreenHeight;
    Context CurrentContext;
    boolean StopRendering = false;
    private Activity OwnerActivity;

    public CameraRenderer(Context NewContext, Surface NewSurface, int Width,  int Height, Activity Owner)
    {
        CurrentView = NewSurface;
        CurrentContext = NewContext;
        ScreenWidth = Width;
        ScreenHeight = Height;
        CurrentUVMatrix = new float[16];
        OwnerActivity = Owner;

        CurrentUVMatrix[0] = 1;
        CurrentUVMatrix[5] =  1;
        CurrentUVMatrix[10] =  1;
        CurrentUVMatrix[15] =  1;
    }

    //@Override
    public void onSurfaceCreated()
    {
        //if we already have a renderer kill it
        if(ndkRenderer != 0)
        {
            deleteRenderer(ndkRenderer);
            ndkRenderer = createRenderer();
        }
        else
        {
            ndkRenderer = createRenderer();
        }

        //Point ss = new Point();
        //CurrentView.getDisplay().getRealSize(ss);

        AssetManager manager = CurrentContext.getAssets();
        ChooseCamera(ScreenWidth, ScreenHeight);

        onSurfaceCreated(ndkRenderer, CurrentView, manager, OwnerActivity, CamSize.getWidth(), CamSize.getHeight());
        CurrentSurfaceTexture = new SurfaceTexture(getGLCameraTextureID(ndkRenderer));
        CurrentSurfaceTexture.setOnFrameAvailableListener(this);
        CurrentCaptureSurface = new Surface(CurrentSurfaceTexture);



        //GLES30.glClearColor(0.0f, 1.0f, 0.0f, 1.0f);

    }

    public synchronized void onFrameAvailable ( SurfaceTexture st )
    {
        SurfaceTextureAvailable = true;
        //mView.requestRender();
    }

    private synchronized void onCameraClosed (  )
    {
        CameraClosed = true;
        //mView.requestRender();
    }


    //@Override
    public void onSurfaceChanged(int width, int height)
    {
        GLES30.glViewport(0, 0, width, height);

        ScreenWidth = width;
        ScreenHeight = height;

        onSurfaceChanged(ndkRenderer);

        synchronized(this)
        {
            if (CameraClosed)
            {
                CameraClosed = false;
                openCamera();
            }
        }
    }

    //@Override
    public void onDrawFrame()
    {
        //GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT);

        if(StopRendering)
        {
            return;
        }

        synchronized(this) {
            if ( SurfaceTextureAvailable ) {
                CurrentSurfaceTexture.updateTexImage();
                //CurrentSurfaceTexture.getTransformMatrix(CurrentUVMatrix);
                decodeAndEnqueueCameraTexture(ndkRenderer, CurrentSurfaceTexture.getTimestamp());
                SurfaceTextureAvailable = false;
            }
        }

        setCameraTransform(ndkRenderer, CurrentUVMatrix);
        onSurfaceRender(ndkRenderer, CurrentView);
    }

    public void onResume()
    {
        if(BackgroundHandler == null)
        {
            startBackgroundThread();
        }

        openCamera();
        StopRendering = false;
        onResume(ndkRenderer);
    }

    public void onPause()
    {
        if(!StopRendering)
        {
            StopRendering = true;
            onPause(ndkRenderer);
            closeCamera();
            stopBackgroundThread();
        }
    }

    public void DestroyNativeResources()
    {
        deleteRenderer(ndkRenderer);
        ndkRenderer = 0;
    }


    void ChooseCamera( final int width, final int height ) {
        CameraManager manager = (CameraManager)CurrentContext.getSystemService(Context.CAMERA_SERVICE);
        try {
            for (String cameraID : manager.getCameraIdList()) {
                CameraCharacteristics characteristics = manager.getCameraCharacteristics(cameraID);
                if (characteristics.get(CameraCharacteristics.LENS_FACING) == CameraCharacteristics.LENS_FACING_FRONT)
                    continue;

                CurrentCameraID = cameraID;

                boolean foundConfig = false;
                StreamConfigurationMap map = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);

                //boolean supp = map.isOutputSupportedFor(CurrentCaptureSurface);

                for ( Size psize : map.getOutputSizes(SurfaceTexture.class)) {
                    if ( width == psize.getWidth() && height == psize.getHeight() )
                    {
                        foundConfig = true;
                        CamSize = psize;
                        break;
                    }
                }

                if(!foundConfig)
                {
                    CamSize = map.getOutputSizes(SurfaceTexture.class)[0];
                }

                break;
            }
        } catch ( CameraAccessException e ) {
            Log.e("mr", "cacPreviewSize - Camera Access Exception");
        } catch ( IllegalArgumentException e ) {
            Log.e("mr", "cacPreviewSize - Illegal Argument Exception");
        } catch ( SecurityException e ) {
            Log.e("mr", "cacPreviewSize - Security Exception");
        }
    }

    void openCamera() {
        CameraManager manager = (CameraManager)CurrentContext.getSystemService(Context.CAMERA_SERVICE);
        try {
            CameraCharacteristics characteristics = manager.getCameraCharacteristics(CurrentCameraID);
            if (!mCameraOpenCloseLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
                throw new RuntimeException("Time out waiting to lock camera opening.");
            }

            manager.openCamera(CurrentCameraID, StateCallback , BackgroundHandler);

        } catch ( CameraAccessException e ) {
            Log.e("mr", "OpenCamera - Camera Access Exception");
        } catch ( IllegalArgumentException e ) {
            Log.e("mr", "OpenCamera - Illegal Argument Exception");
        } catch ( SecurityException e ) {
            Log.e("mr", "OpenCamera - Security Exception");
        } catch ( InterruptedException e ) {
            Log.e("mr", "OpenCamera - Interrupted Exception");
        }
    }

    private void createCameraPreviewSession() {
        try {

            CurrentSurfaceTexture.setDefaultBufferSize(CamSize.getWidth(), CamSize.getHeight());
            CurrentCaptureBuilder = CurrentCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            CurrentCaptureBuilder.addTarget(CurrentCaptureSurface);

            CurrentCameraDevice.createCaptureSession(Arrays.asList(CurrentCaptureSurface),
                    new CameraCaptureSession.StateCallback() {
                        @Override
                        public void onConfigured(CameraCaptureSession cameraCaptureSession) {
                            if (null == CurrentCameraDevice)
                                return;

                            CurrentCaptureSession = cameraCaptureSession;
                            try {
                                //CurrentCaptureBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
                                //CurrentCaptureBuilder.set(CaptureRequest.CONTROL_AE_MODE, CaptureRequest.CONTROL_AE_MODE_ON_AUTO_FLASH);

                                CurrentCaptureBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AE_MODE_OFF);
                                CurrentCaptureBuilder.set(CaptureRequest.FLASH_MODE,CameraMetadata.FLASH_MODE_OFF);

                                CurrentCaptureSession.setRepeatingRequest(CurrentCaptureBuilder.build(), CameraCaptureSessionCallback, BackgroundHandler);
                            } catch (CameraAccessException e) {
                                Log.e("mr", "createCaptureSession");
                            }
                        }
                        @Override
                        public void onConfigureFailed(CameraCaptureSession cameraCaptureSession) {
                        }
                    }, null
            );
        } catch (CameraAccessException e) {
            Log.e("mr", "createCameraPreviewSession");
        }
    }

    private final CameraCaptureSession.CaptureCallback CameraCaptureSessionCallback = new CameraCaptureSession.CaptureCallback()
    {
        @Override
        public void onCaptureStarted(CameraCaptureSession session, CaptureRequest request, long timestamp, long frameNumber) {
            //Log.e("mr", "onCaptureStarted");
        }
        @Override
        public void onCaptureProgressed(CameraCaptureSession session, CaptureRequest request, CaptureResult partialResult) {
            //Log.e("mr", "onCaptureProgressed");
        }
        @Override
        public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request, TotalCaptureResult result) {
            //Log.e("mr", "onCaptureCompleted");
        }
        @Override
        public void onCaptureFailed(CameraCaptureSession session, CaptureRequest request, CaptureFailure failure) {
            Log.e("mr", "onCaptureFailed");
        }
        @Override
        public void onCaptureSequenceCompleted(CameraCaptureSession session, int sequenceId, long frameNumber) {
            //Log.e("mr", "onCaptureSequenceCompleted");
        }
        @Override
        public void onCaptureSequenceAborted(CameraCaptureSession session, int sequenceId) {
            Log.e("mr", "onCaptureSequenceAborted");
        }
        @Override
        public void onCaptureBufferLost(CameraCaptureSession session, CaptureRequest request, Surface target, long frameNumber) {
            Log.e("mr", "onCaptureBufferLost");
        }
    };
    private final CameraDevice.StateCallback StateCallback = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(CameraDevice cameraDevice) {
            mCameraOpenCloseLock.release();
            CurrentCameraDevice = cameraDevice;
            createCameraPreviewSession();
        }

        @Override
        public void onDisconnected(CameraDevice cameraDevice) {
            mCameraOpenCloseLock.release();
            cameraDevice.close();
            CurrentCameraDevice = null;
            onCameraClosed();
        }

        @Override
        public void onError(CameraDevice cameraDevice, int error) {
            mCameraOpenCloseLock.release();
            cameraDevice.close();
            CurrentCameraDevice = null;
            onCameraClosed();
        }

    };

    private static Size chooseOptimalSize(Size[] choices, int width, int height, Size aspectRatio) {
        // Collect the supported resolutions that are at least as big as the preview Surface
        List<Size> bigEnough = new ArrayList<>();
        int w = aspectRatio.getWidth();
        int h = aspectRatio.getHeight();
        for (Size option : choices) {
            if (option.getHeight() == option.getWidth() * h / w &&
                    option.getWidth() >= width && option.getHeight() >= height) {
                bigEnough.add(option);
            }
        }

        // Pick the smallest of those, assuming we found any
        if (bigEnough.size() > 0) {
            return Collections.min(bigEnough, new CompareSizesByArea());
        } else {
            Log.e(TAG, "Couldn't find any suitable preview size");
            return choices[0];
        }
    }

    static class CompareSizesByArea implements Comparator<Size> {

        @Override
        public int compare(Size lhs, Size rhs) {
            // We cast here to ensure the multiplications won't overflow
            return Long.signum((long) lhs.getWidth() * lhs.getHeight() -
                    (long) rhs.getWidth() * rhs.getHeight());
        }

    }

    private void startBackgroundThread() {
        mBackgroundThread = new HandlerThread("CameraBackground");
        mBackgroundThread.start();
        BackgroundHandler = new Handler(mBackgroundThread.getLooper());
    }

    private void stopBackgroundThread() {
        mBackgroundThread.quitSafely();
        try {
            mBackgroundThread.join();
            mBackgroundThread = null;
            BackgroundHandler = null;
        } catch (InterruptedException e) {
            Log.e("mr", "stopBackgroundThread");
        }
    }

    private void closeCamera() {
        try {
            mCameraOpenCloseLock.acquire();
            if (null != CurrentCaptureSession) {
                CurrentCaptureSession.close();
                CurrentCaptureSession = null;
            }
            if (null != CurrentCameraDevice) {
                CurrentCameraDevice.close();
                CurrentCameraDevice = null;
            }
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera closing.", e);
        } finally {
            mCameraOpenCloseLock.release();
        }
    }

    private long ndkRenderer = 0;
}