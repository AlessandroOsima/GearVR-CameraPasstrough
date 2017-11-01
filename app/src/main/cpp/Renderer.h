//
// Created by HF on 02/10/17.
//

#ifndef CAMERA_RENDERER_H
#define CAMERA_RENDERER_H


#define OVR_BUILD_DEBUG 1
#include <VrApi.h>
#include <VrApi_Types.h>
#include <VrApi_Helpers.h>
#include <Math/MathTypes.h>


#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <jni.h>
#include "Shader/ShaderProgram.h"


#include <memory>
#include <VertexData.h>
#include <Texture/Texture.h>
#include <Texture/Sampler.h>
#include <android/asset_manager.h>
#include <Framebuffer/Framebuffer.h>
#include <Renderable.h>
#include <deque>



struct FrameInfo
{
    std::unique_ptr<Texture>  AcquiredTexture;
    long Timestamp;
};

class Renderer
{
public:
    void OnCreated(JNIEnv *env, JavaVM *VM, jobject surface, AAssetManager *manager, jobject ActivityObject,
                       int VRRenderGeometryWidth, int VRRenderGeometryHeight);

    void OnRender(JNIEnv * Env, jobject Surface);

    void OnResume();

    void OnPause();

    void OnChange();

    void OnDestroy(JNIEnv *Env);

    int GetCameraTextureID();

    void DecodeAndEnqueueCameraTexture(long Timestamp);

    inline void SetUVTransform(glm::mat4 Transform)
    {
        UVTransform = Transform;
    }

    void SetDisplayDelay(unsigned NewDelay);

private:
    EGLDisplay Display;
    jobject CurrentSurface = nullptr;
    EGLSurface  CurrentEGLSurface;
    EGLContext CurrentContext;
    ANativeWindow * CurrentWindow = nullptr;
    JNIEnv * CurrentEv = nullptr;

    ovrJava OvrInstanceData;
    ovrTextureSwapChain * ColorTextureSwapChain[VRAPI_FRAME_LAYER_EYE_MAX];

    EGLSurface  DummySurface;
    ovrMobile * OVRMode = nullptr;
    size_t Sync = 0;
    unsigned  long long timeCounter = 0;

    std::unique_ptr<ShaderProgram> CameraOffscreenRendering;
    std::unique_ptr<ShaderProgram> CameraScreenRendering;
    std::unique_ptr<Texture> ColorTexture;
    std::unique_ptr<Texture> CameraTexture;
    std::unique_ptr<Sampler> ColorSampler;
    std::unique_ptr<Framebuffer> OffscreenOESRendertarget;
    std::vector<std::unique_ptr<Framebuffer>> VREyeRendertargets[VRAPI_FRAME_LAYER_EYE_MAX];
    std::vector<size_t > VREyeRendertargetsFences[VRAPI_FRAME_LAYER_EYE_MAX];
    Renderable CameraOutputMesh;


    std::shared_ptr<VertexData> VData;
    glm::mat4 UVTransform;
    std::vector<FrameInfo> Frames;


    float map(float value, float lowBase, float highBase, float lowNew, float highNew);

    bool Produced = false;

    long Consumer = 0;
    long Producer = 0;

    bool Wrapped = false;
    const size_t  IDCount = 20;
    unsigned long Delay = 10;
    EGLConfig  ChosenConfig = nullptr;
    long long FrameIDx = 1;
    uint32_t EyeFramebuffersMultisampleCount = 4;

    void CreateVRSwapChain(int Width, int Height);
};


#endif //CAMERA_RENDERER_H
