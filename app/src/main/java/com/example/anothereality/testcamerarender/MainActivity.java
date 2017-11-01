package com.example.anothereality.testcamerarender;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.WindowManager;


import junit.framework.Assert;

public class MainActivity extends Activity implements SurfaceHolder.Callback
{
    static
    {
        try
        {
            //System.loadLibrary("MGD");
            //System.loadLibrary("vrapi");
        }
        catch (UnsatisfiedLinkError e)
        {
            // Feel free to remove this log message.
            Log.e("[ MGD ]", "MGD not loaded: " + e.getMessage());
            Log.d("[ MGD ]", Log.getStackTraceString(e));
        }
    }
    public volatile boolean HasPermission = false;
    private SurfaceView CurrentSurfaceView;
    private SurfaceTexture CurrentSurfaceTexture;
    private Surface CurrentSurface;
    private TextureView CurrentView;
    private Handler CurrentHandler;
    private Runnable MainRunnable;
    private CameraRenderer Renderer;
    private long millisecLoop = 5;
    private boolean FromResume = false;
    private boolean SurfaceBeingCreated = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        CurrentHandler = new Handler();
        MainRunnable = new Runnable()
        {
            @Override
            public void run()
            {
                runOnUiThread(new Runnable()
                {
                    @Override
                    public void run()
                    {
                        RenderFrame();
                        CurrentHandler.postDelayed(this, millisecLoop);
                    }
                });
            }
        };


        RequestCamera();
    }

    @Override protected void onStart()
    {
        super.onStart();

    }

    @Override protected void onResume()
    {
        super.onResume();

        if(CurrentSurfaceView != null )
        {
            if(!SurfaceBeingCreated)
            {
                ResumeRenderer();
            }
            else
            {
                FromResume = true;
            }
        }
        else if(CurrentSurfaceView == null)
        {
            CreateSurface();
            FromResume = true;
        }


    }

    private void ResumeRenderer()
    {
        Renderer.onResume();
        CurrentHandler.post(MainRunnable);
    }

    @Override protected void onPause()
    {
        CurrentHandler.removeCallbacks(MainRunnable);

        if(Renderer != null)
        {
            Renderer.onPause();
        }
        super.onPause();
    }

    @Override protected void onStop()
    {

        super.onStop();
    }

    @Override protected void onDestroy()
    {
        Renderer.DestroyNativeResources();
        super.onDestroy();
    }


    @Override public void surfaceCreated( SurfaceHolder holder )
    {
        int width = CurrentSurfaceView.getWidth();
        int height = CurrentSurfaceView.getHeight();

        Renderer = new CameraRenderer(this.getBaseContext(), CurrentSurfaceView.getHolder().getSurface(), width, height, this);
        Renderer.onSurfaceCreated();

        //if this call is coming from the CreateSurface in resume  we also need to resume the renderer
        if(FromResume)
        {
            ResumeRenderer();
        }

        FromResume = false;
        SurfaceBeingCreated = false;

    }

    @Override public void surfaceChanged( SurfaceHolder holder, int format, int width, int height )
    {
        Renderer.onSurfaceChanged(width, height);
    }

    @Override public void surfaceDestroyed( SurfaceHolder holder )
    {
        CurrentSurfaceView = null;
        Renderer.onPause();
        Renderer.DestroyNativeResources();
    }

    private static final int PERMISSION_REQUEST_CODE_CAMERA = 1;

    public void RequestCamera() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) !=
                PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(
                    this,
                    new String[]{Manifest.permission.CAMERA},
                    PERMISSION_REQUEST_CODE_CAMERA);
            return;
        }
        CreateSurface();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        /*
         * if any permission failed, the sample could not play
         */
        if (PERMISSION_REQUEST_CODE_CAMERA != requestCode) {
            super.onRequestPermissionsResult(requestCode,
                    permissions,
                    grantResults);
            return;
        }

        Assert.assertEquals(grantResults.length, 1);
        if (grantResults[0] == PackageManager.PERMISSION_GRANTED)
        {
            HasPermission = true;
            Thread initCamera = new Thread(new Runnable() {
                public void run() {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            CreateSurface();
                        }
                    });
                }
            });
            initCamera.start();
        }
    }

    void CreateSurface()
    {
        SurfaceBeingCreated = true;
        CurrentSurfaceView = new SurfaceView(this);
        setContentView(CurrentSurfaceView);
        CurrentSurfaceView.getHolder().addCallback(this);
    }

    void RenderFrame()
    {
        Renderer.onDrawFrame();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus)
    {
        super.onWindowFocusChanged(hasFocus);

        /*
        if (hasFocus) {
            getWindow().getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        }
        */
    }



    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    //public native String stringFromJNI();
}
