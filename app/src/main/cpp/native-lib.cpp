#include <jni.h>
#include <string>
#include "Renderer.h"
#include "Asserts.h"
#include <android/asset_manager_jni.h>

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_anothereality_testcamerarender_CameraRenderer_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_anothereality_testcamerarender_CameraRenderer_setCameraTransform(
        JNIEnv *env,
        jobject instance,
        jlong ndkRendererObj,
        jfloatArray matrix)
{
    jsize matrixSize = env->GetArrayLength(matrix);

    Assert(matrixSize == 16) //it must be a 4x4 matrix

    jfloat * elements = env->GetFloatArrayElements(matrix, 0);

    glm::mat4 nativeMatrix;

    for(int x = 0; x < 4; x++)
    {
        for(int y = 0; y < 4; y++)
        {
            nativeMatrix[x][y] = elements[4 * y + x];
        }
    }

    Renderer * pRenderer = reinterpret_cast<Renderer*>(ndkRendererObj);
    pRenderer->SetUVTransform(nativeMatrix);

    env->ReleaseFloatArrayElements(matrix, elements, 0);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_example_anothereality_testcamerarender_CameraRenderer_createRenderer(JNIEnv *env,
                                                        jobject instance)
{
    Renderer * pRenderer = new Renderer();
    return reinterpret_cast<jlong>(pRenderer);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_anothereality_testcamerarender_CameraRenderer_onSurfaceCreated(JNIEnv *env,
                                                          jobject instance,
                                                          jlong ndkRendererObj,
                                                          jobject surface,
                                                          jobject assetManagerObj,
                                                          jobject activityObject,
                                                          jint VRRenderSurfaceWidth,
                                                          jint VRRenderSurfaceHeight)
{
    AAssetManager * assetManager = AAssetManager_fromJava(env, assetManagerObj);
    Assert(assetManager);
    Renderer * pRenderer = reinterpret_cast<Renderer*>(ndkRendererObj);
    JavaVM * vm;
    env->GetJavaVM(&vm);
    pRenderer->OnCreated(env, vm, surface, assetManager, activityObject, VRRenderSurfaceWidth, VRRenderSurfaceHeight);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_anothereality_testcamerarender_CameraRenderer_decodeAndEnqueueCameraTexture(JNIEnv *env,
                                                                                jobject instance,
                                                                                jlong ndkRendererObj, jlong timestamp)
{

    Renderer * pRenderer = reinterpret_cast<Renderer*>(ndkRendererObj);
    pRenderer->DecodeAndEnqueueCameraTexture(static_cast<long>(timestamp));
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_anothereality_testcamerarender_CameraRenderer_onSurfaceChanged(JNIEnv *env,
                                                          jobject instance,
                                                          jlong ndkRendererObj)
{
    Renderer * pRenderer = reinterpret_cast<Renderer*>(ndkRendererObj);
    pRenderer->OnChange();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_anothereality_testcamerarender_CameraRenderer_onSurfaceRender(JNIEnv *env,
                                                         jobject instance,
                                                         jlong ndkRendererObj,
                                                         jobject surface)
{
    Renderer * pRenderer = reinterpret_cast<Renderer*>(ndkRendererObj);
    pRenderer->OnRender(env, surface);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_anothereality_testcamerarender_CameraRenderer_getGLCameraTextureID(JNIEnv *env,
                                                              jobject instance,
                                                              jlong ndkRendererObj)
{
    Renderer * pRenderer = reinterpret_cast<Renderer*>(ndkRendererObj);
    return pRenderer->GetCameraTextureID();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_anothereality_testcamerarender_CameraRenderer_onResume(JNIEnv *env,
                                                                                    jobject instance,
                                                                                    jlong ndkRendererObj)
{
    Renderer * pRenderer = reinterpret_cast<Renderer*>(ndkRendererObj);
    return pRenderer->OnResume();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_anothereality_testcamerarender_CameraRenderer_onPause(JNIEnv *env,
                                                                        jobject instance,
                                                                        jlong ndkRendererObj)
{
    Renderer * pRenderer = reinterpret_cast<Renderer*>(ndkRendererObj);
    return pRenderer->OnPause();
}



        extern "C" JNIEXPORT void JNICALL
Java_com_example_anothereality_testcamerarender_CameraRenderer_deleteRenderer(JNIEnv *env,
                                                        jobject instance,
                                                        jlong ndkRendererObj)
{
    Renderer * pRenderer = reinterpret_cast<Renderer*>(ndkRendererObj);
    pRenderer->OnDestroy(env);
    delete pRenderer;
}