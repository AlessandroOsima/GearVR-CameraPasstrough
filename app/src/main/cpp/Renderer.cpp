//
// Created by HF on 02/10/17.
//

#include <android/native_window_jni.h>
#include "Renderer.h"
#include "Asserts.h"
#include <cstdlib>
#include <unistd.h>
#include "Filters.h"
#include <stdint.h>

static PFNEGLCREATESYNCKHRPROC			eglCreateSyncKHR;
static PFNEGLDESTROYSYNCKHRPROC		eglDestroySyncKHR;
static PFNEGLCLIENTWAITSYNCKHRPROC		eglClientWaitSyncKHR;


static const int CPU_LEVEL			= 2;
static const int GPU_LEVEL			= 3;
static const int NUM_MULTI_SAMPLES	= 4;

void Renderer::OnCreated(JNIEnv *env, JavaVM *VM, jobject surface, AAssetManager *manager, jobject ActivityObject,
                         int VRRenderGeometryWidth, int VRRenderGeometryHeight)
{
    CurrentEv = env;

    OvrInstanceData.Vm = VM;
    OvrInstanceData.Env = env;
    OvrInstanceData.ActivityObject = env->NewGlobalRef( ActivityObject );

    //VM->AttachCurrentThread(&env, nullptr);


    Logger::GetLogger().LogString("starting VrAPI", LogType::LOG);

    ovrInitParms initParms = vrapi_DefaultInitParms( &OvrInstanceData );
    initParms.GraphicsAPI = VRAPI_GRAPHICS_API_OPENGL_ES_3;
    if ( vrapi_Initialize( &initParms ) != VRAPI_INITIALIZE_SUCCESS )
    {
        Logger::GetLogger().LogString("cannot init vrapi");
        abort();
    }

    int SamplesCount;
    glCheckFunction(glGetIntegerv(GL_MAX_SAMPLES_EXT, &SamplesCount));

    if(SamplesCount < EyeFramebuffersMultisampleCount)
    {
        Logger::GetLogger().LogString("Number of eye framebuffers multisamples is higher than max supported samples, clamping it", LogType::WARNING);
        EyeFramebuffersMultisampleCount = SamplesCount;
    }

    eglCreateSyncKHR		= (PFNEGLCREATESYNCKHRPROC)			eglGetProcAddress( "eglCreateSyncKHR" );
    eglDestroySyncKHR		= (PFNEGLDESTROYSYNCKHRPROC)		eglGetProcAddress( "eglDestroySyncKHR" );
    eglClientWaitSyncKHR	= (PFNEGLCLIENTWAITSYNCKHRPROC)		eglGetProcAddress( "eglClientWaitSyncKHR" );

    Display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    if(Display == EGL_NO_DISPLAY)
    {
        Logger::GetLogger().LogString("Unable to create EGL Display", LogType::ERROR);
        return;
    }

    int GLMaj = 0;
    int GLMin = 0;

    if(!eglInitialize(Display, &GLMaj, &GLMin))
    {
        Logger::GetLogger().LogString("Unable to init EGL", LogType::ERROR);
        return;
    }


    if(!eglBindAPI(EGL_OPENGL_ES_API))
    {
        Logger::GetLogger().LogString("Unable to bind EGL API", LogType::ERROR);
        return;
    }


    /*
    EGLint attribList[] = {
            // 32 bit color
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            // at least 24 bit depth
            EGL_DEPTH_SIZE, 24,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            // want opengl-es 3.x conformant CONTEXT
            EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_NONE
    };
     */

    const int MAX_CONFIGS = 1024;
    EGLConfig configs[MAX_CONFIGS];
    EGLint configsCount = 0;

    const EGLint ConfigAttributes[] =
            {
                    EGL_RED_SIZE,		8,
                    EGL_GREEN_SIZE,		8,
                    EGL_BLUE_SIZE,		8,
                    EGL_ALPHA_SIZE,		8, // need alpha for the multi-pass timewarp compositor
                    EGL_DEPTH_SIZE,		0,
                    EGL_STENCIL_SIZE,	0,
                    EGL_SAMPLES,		0,
                    EGL_NONE
            };

    if(!eglGetConfigs(Display, configs, MAX_CONFIGS, &configsCount))
    {
        Logger::GetLogger().LogString("Unable to get EGL configs set", LogType::ERROR);
        return;
    }

    for(int i = 0; i < configsCount; i++)
    {
        EGLConfig  current = configs[i];

        EGLint  value = 0;

        eglGetConfigAttrib( Display, current, EGL_RENDERABLE_TYPE, &value );
        if ( ( value & EGL_OPENGL_ES3_BIT_KHR ) != EGL_OPENGL_ES3_BIT_KHR )
        {
            continue;
        }

        if(!eglGetConfigAttrib(Display, current, EGL_SURFACE_TYPE, &value))
        {
            Logger::GetLogger().LogString("Unable to get EGL config attrib EGL_SURFACE_TYPE", LogType::ERROR);
            return;
        }

        if(( value & ( EGL_WINDOW_BIT | EGL_PBUFFER_BIT ) ) != ( EGL_WINDOW_BIT | EGL_PBUFFER_BIT ) )
        {
            continue;
        }

        int	j = 0;
        for ( ; ConfigAttributes[j] != EGL_NONE; j += 2 )
        {
            if(!eglGetConfigAttrib( Display, current, ConfigAttributes[j], &value ))
            {
                Logger::GetLogger().LogString("Unable to get EGL config attribs for ConfigAttributes compare", LogType::ERROR);
                return;
            }

            if ( value != ConfigAttributes[j + 1] )
            {
                break;
            }
        }
        if ( ConfigAttributes[j] == EGL_NONE )
        {
            ChosenConfig = current;
            break;
        }
    }

    if(!ChosenConfig)
    {
        Logger::GetLogger().LogString("Unable to find valid EGL config", LogType::ERROR);
        return;
    }

    CurrentSurface = env->NewGlobalRef(surface);
    CurrentWindow = ANativeWindow_fromSurface(CurrentEv, CurrentSurface);

    AssertWithMessage(CurrentWindow, "Unable to get window from surface")

    EGLint contextAttributes[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };

    CurrentContext = eglCreateContext(Display, ChosenConfig, EGL_NO_CONTEXT, contextAttributes);
    auto ErrorCode = eglGetError();
    if(ErrorCode != EGL_SUCCESS)
    {
        std::stringstream stream;
        stream << "Unable to create EGL context with error " << ErrorCode << std::ends;
        Logger::GetLogger().LogString(stream.str() , LogType::ERROR);
        return;
    }

    uint32_t width = (uint32_t)ANativeWindow_getWidth(CurrentWindow);
    uint32_t height = (uint32_t)ANativeWindow_getHeight(CurrentWindow);

    const EGLint surfaceAttribs[] =
            {
                    EGL_WIDTH, 16,
                    EGL_HEIGHT, 16,
                    EGL_NONE
            };

    DummySurface = eglCreatePbufferSurface( Display, ChosenConfig, surfaceAttribs );
    if ( DummySurface == EGL_NO_SURFACE )
    {
        Logger::GetLogger().LogString("Error in DummySurface allocation", LogType::ERROR);
        return;
    }

    eglMakeCurrent(Display, DummySurface, DummySurface, CurrentContext);

    CreateVRSwapChain(width, height);

    CameraOffscreenRendering = std::move(std::unique_ptr<ShaderProgram>(new ShaderProgram()));

    if(!CameraOffscreenRendering->CompileShaderFile("base.vs", ShaderType::Vertex, manager) || !CameraOffscreenRendering->CompileShaderFile("OESSample.fs", ShaderType::Fragment, manager))
    {
        Logger::GetLogger().LogString("program compile failed" , LogType::ERROR);
        return;
    }

    CameraOffscreenRendering->LinkProgram();

    CameraScreenRendering = std::move(std::unique_ptr<ShaderProgram>(new ShaderProgram()));

    if(!CameraScreenRendering->CompileShaderFile("baseTransformed.vs", ShaderType::Vertex, manager) || !CameraScreenRendering->CompileShaderFile("Sample.fs", ShaderType::Fragment, manager))
    {
        Logger::GetLogger().LogString("program compile failed" , LogType::ERROR);
        return;
    }

    CameraScreenRendering->LinkProgram();

    VData = MeshUtilities::CreateFullScreenQuad({0, 0, 0, 1}, true);

    ovrMatrix4f translate = ovrMatrix4f_CreateTranslation(-6.f,-2.7f,-6);
    ovrMatrix4f modelTransposed = ovrMatrix4f_Transpose(&translate);

    float aspect = (float)VRRenderGeometryWidth / (float)VRRenderGeometryHeight;
    CameraOutputMesh.Vertices = MeshUtilities::CreateAspectCorrectEyeQuad(aspect, 2, -3);
    CameraOutputMesh.ModelMatrix = modelTransposed;

   std::vector<uint32_t> DummyTexture(width * height * sizeof(uint32_t));

   for(int i = 0; i < DummyTexture.size(); i++)
   {
       uint32_t r = 255;
       uint32_t g = 0;
       uint32_t b = 0;
       uint32_t a = 255;
       DummyTexture[i] = r << 0 | g << 8 | b << 16 | a << 24;
   }

    CameraTexture = std::move(std::unique_ptr<Texture>(new Texture(GL_TEXTURE_EXTERNAL_OES)));
    OffscreenOESRendertarget = std::move(std::unique_ptr<Framebuffer>(new Framebuffer()));

    ColorTexture = std::move(std::unique_ptr<Texture>(new Texture(GL_TEXTURE_2D)));
    ColorSampler = std::move(std::unique_ptr<Sampler>(new Sampler()));

    ColorTexture->GenerateTextureWithSize(GL_TEXTURE_2D, width, height, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE, DummyTexture.data());



    auto ids = Texture::AllocateIDSet(IDCount);

    for (int i = 0; i < IDCount ; ++i)
    {
        Frames.push_back({nullptr, 0});

        Frames.back().AcquiredTexture = std::move(std::unique_ptr<Texture>(new Texture(GL_TEXTURE_2D, ids[i])));

        Frames.back().AcquiredTexture->GenerateTextureWithSize(GL_TEXTURE_2D, width, height, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE, nullptr);
    }


    glClearColor(0.0, 0.0, 0.0, 1.0);
}

float Renderer::map(float value, float lowBase, float highBase, float lowNew, float highNew)
{
    return lowNew + (value - lowBase) * (highNew - lowNew) / (highBase - lowBase);
}

void Renderer::OnRender(JNIEnv * Env, jobject Surface)
{
    eglMakeCurrent(Display, DummySurface, DummySurface, CurrentContext);

    //auto context = eglGetCurrentContext();

    if(CurrentContext == EGL_NO_CONTEXT)
    {
        return; //no context no rendering, this happens when SurfaceView calls the render method before the onSurfaceCreated method
    }

    Texture * decodedToUse = ColorTexture.get();

    if(Produced)
    {
        FrameInfo & CurrentAcquiredFrame = Frames[Consumer];
        decodedToUse = CurrentAcquiredFrame.AcquiredTexture.get();
    }

    const double predictedDisplayTime = vrapi_GetPredictedDisplayTime(OVRMode, FrameIDx);
    const ovrTracking2 tracking = vrapi_GetPredictedTracking2(OVRMode, predictedDisplayTime);

    float LinearAccelSum = tracking.HeadPose.LinearAcceleration.x + tracking.HeadPose.LinearAcceleration.y + tracking.HeadPose.LinearAcceleration.z;
    float AngularAccelSum = tracking.HeadPose.AngularAcceleration.x + tracking.HeadPose.AngularAcceleration.y + tracking.HeadPose.AngularAcceleration.z;

    float Accelerating = (LinearAccelSum > 0 || AngularAccelSum > 0) ? 1 : 0;

    ovrFrameParms frameParms = vrapi_DefaultFrameParms(&OvrInstanceData, VRAPI_FRAME_INIT_DEFAULT, predictedDisplayTime, nullptr);
    frameParms.FrameIndex = FrameIDx;
    frameParms.PerformanceParms.CpuLevel = CPU_LEVEL;
    frameParms.PerformanceParms.GpuLevel = GPU_LEVEL;

    auto id = gettid();

    frameParms.PerformanceParms.MainThreadTid = id;
    frameParms.PerformanceParms.RenderThreadTid = id;

    for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
    {
        const long long colorTextureSwapChainIndex = FrameIDx % vrapi_GetTextureSwapChainLength( ColorTextureSwapChain[eye] );


        //RENDERING
        VREyeRendertargets[eye][colorTextureSwapChainIndex]->BindFramebuffer(FramebufferBindType::FRAMEBUFFER);


        glClear(GL_COLOR_BUFFER_BIT);

        auto eyeTransform = tracking.Eye[eye];

        //ovrMatrix4f viewTransposed = ovrMatrix4f_Transpose(&eyeTransform.ViewMatrix);
        ovrMatrix4f projectTransposed = ovrMatrix4f_Transpose(&eyeTransform.ProjectionMatrix);


        CameraScreenRendering->UseProgram();
        CameraScreenRendering->SetUniformInt(CameraScreenRendering->GetUniformIndex("cameraFrameSampler"), 0);
        CameraScreenRendering->SetUniformMatrix4(CameraScreenRendering->GetUniformIndex("UVTransform"), UVTransform);
        CameraScreenRendering->SetUniformMatrix4(CameraScreenRendering->GetUniformIndex("ModelTransform"), (float**)CameraOutputMesh.ModelMatrix.M);
        CameraScreenRendering->SetUniformMatrix4(CameraScreenRendering->GetUniformIndex("ViewTransform"), glm::mat4(1));
        CameraScreenRendering->SetUniformMatrix4(CameraScreenRendering->GetUniformIndex("ProjectionTransform"),(float**)projectTransposed.M);

        decodedToUse->Bind(GL_TEXTURE_2D, GL_TEXTURE0);
        ColorSampler->Bind(0);
        CameraOutputMesh.Vertices->Bind();

        glDrawElements(GL_TRIANGLES, (GLsizei) CameraOutputMesh.Vertices->GetIndices().size(), GL_UNSIGNED_INT, 0);

        CameraOutputMesh.Vertices->Unbind();

        VREyeRendertargets[eye][colorTextureSwapChainIndex]->UnBindFramebuffer();

        decodedToUse->UnBind(GL_TEXTURE_2D);

        if(VREyeRendertargetsFences[eye][colorTextureSwapChainIndex] != 0)
        {
            if ( eglDestroySyncKHR( Display, (EGLSyncKHR)VREyeRendertargetsFences[eye][colorTextureSwapChainIndex] ) ==  EGL_FALSE )
            {
                Logger::GetLogger().LogString("eglDestroySyncKHR() : EGL_FALSE", LogType::ERROR);
                return;
            }
        }

        VREyeRendertargetsFences[eye][colorTextureSwapChainIndex] = (size_t)eglCreateSyncKHR( Display, EGL_SYNC_FENCE_KHR, NULL );
        if (VREyeRendertargetsFences[eye][colorTextureSwapChainIndex] == 0 )
        {
            Logger::GetLogger().LogString("eglCreateSyncKHR() : EGL_NO_SYNC_KHR", LogType::ERROR);
            return;
        }

        if ( eglClientWaitSyncKHR( Display, (EGLSyncKHR)VREyeRendertargetsFences[eye][colorTextureSwapChainIndex], EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, 0 ) == EGL_FALSE )
        {
            Logger::GetLogger().LogString("eglClientWaitSyncKHR() : EGL_FALSE", LogType::ERROR);
            return;

        }

        frameParms.Layers[0].Textures[eye].ColorTextureSwapChain = ColorTextureSwapChain[eye];
        frameParms.Layers[0].Textures[eye].TextureSwapChainIndex = colorTextureSwapChainIndex;
        frameParms.Layers[0].Textures[eye].TexCoordsFromTanAngles = ovrMatrix4f_TanAngleMatrixFromProjection( &tracking.Eye[eye].ProjectionMatrix );
        frameParms.Layers[0].Textures[eye].HeadPose = tracking.HeadPose;
        frameParms.Layers[0].Textures[eye].CompletionFence = (size_t)VREyeRendertargetsFences[eye][colorTextureSwapChainIndex];
    }

    vrapi_SubmitFrame(OVRMode, &frameParms);


    FrameIDx++;

   //eglSwapBuffers(Display, CurrentEGLSurface);


}

void Renderer::OnChange()
{

}

void Renderer::OnDestroy(JNIEnv *Env)
{
    for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
    {
        vrapi_DestroyTextureSwapChain( ColorTextureSwapChain[eye] );
    }
    vrapi_Shutdown();
    Env->DeleteGlobalRef(OvrInstanceData.ActivityObject);
    Env->DeleteGlobalRef(CurrentSurface);
    ANativeWindow_release(CurrentWindow);

    eglDestroySurface(Display, DummySurface);
    //eglDestroySurface(Display, CurrentEGLSurface);
    eglDestroyContext(Display, CurrentContext);
    eglTerminate(Display);


}

int Renderer::GetCameraTextureID()
{
   TextureID Id;
   if(CameraTexture)
   {
       Id = CameraTexture->GetID();
   }
   else
   {
       Id = UINT32_MAX;
   }

   return Id;
}

void Renderer::DecodeAndEnqueueCameraTexture(long Timestamp)
{
   auto context = eglGetCurrentContext();

   if(context == EGL_NO_CONTEXT)
   {
       return; //no context no rendering, this happens when SurfaceView calls the render method before the onSurfaceCreated method
   }

   if(Produced)
   {
       Producer++;
       if(Producer >= Frames.size())
       {
           Producer = 0;
           Wrapped = true;
       }
   }

   CameraTexture->Bind(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE0);


   //float r = map(rand() % 255, 0, 255.f, 0, 1.f);
   //float g = map(rand() % 255, 0, 255.f, 0, 1.f);
   //float b = map(rand() % 255, 0, 255.f, 0, 1.f);

   OffscreenOESRendertarget->BindFramebuffer(FramebufferBindType::FRAMEBUFFER);
    OffscreenOESRendertarget->BindTextureToFramebuffer(*Frames[Producer].AcquiredTexture,
                                                       GL_COLOR_ATTACHMENT0, 0, 0);


   glClear(GL_COLOR_BUFFER_BIT);

   Assert(OffscreenOESRendertarget->IsComplete());
   CameraOffscreenRendering->UseProgram();
   ColorSampler->Bind(0);
   CameraOffscreenRendering->SetUniformInt(CameraOffscreenRendering->GetUniformIndex("cameraFrameSampler"), 0);
   CameraOffscreenRendering->SetUniformMatrix4(CameraOffscreenRendering->GetUniformIndex("UVTransform"), UVTransform);
   VData->Bind();


   glDrawElements(GL_TRIANGLES, (GLsizei) VData->GetIndices().size(), GL_UNSIGNED_INT, 0);

   OffscreenOESRendertarget->UnbindFramebufferAttachment(GL_COLOR_ATTACHMENT0);
   OffscreenOESRendertarget->UnBindFramebuffer();

   Produced = true;

   Consumer = Producer - Delay;
   if(!Wrapped && Consumer < 0)
   {
       Consumer = 0;
   }
   else if(Wrapped && Consumer < 0)
   {
       Consumer += Frames.size();

       if(Consumer < 0)
       {
           Consumer = 0;
       }

   }
}

void Renderer::SetDisplayDelay(unsigned NewDelay)
{
    Delay = IDCount;
    if(NewDelay < IDCount)
    {
        Delay = NewDelay;
    }
}

void Renderer::CreateVRSwapChain(int Width, int Height)
{
    const int suggestedEyeTextureWidth = vrapi_GetSystemPropertyInt( &OvrInstanceData, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_WIDTH );
    const int suggestedEyeTextureHeight = vrapi_GetSystemPropertyInt( &OvrInstanceData, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_HEIGHT );

    for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
    {
        ColorTextureSwapChain[eye] = vrapi_CreateTextureSwapChain( VRAPI_TEXTURE_TYPE_2D, VRAPI_TEXTURE_FORMAT_8888,
                                                                   suggestedEyeTextureWidth,
                                                                   suggestedEyeTextureHeight,
                                                                   1, true );

        int lenght = vrapi_GetTextureSwapChainLength(ColorTextureSwapChain[eye]);

        for(int tx = 0; tx < lenght; tx++)
        {
            std::unique_ptr<Framebuffer> buffer = std::unique_ptr<Framebuffer>(new Framebuffer());
            const unsigned int textureId = vrapi_GetTextureSwapChainHandle( ColorTextureSwapChain[eye], tx );

            glCheckFunction(glBindTexture(GL_TEXTURE_2D, textureId));
            glCheckFunction(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            glCheckFunction(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            glCheckFunction(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            glCheckFunction(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            glCheckFunction(glBindTexture(GL_TEXTURE_2D, 0));

            buffer->BindFramebuffer(FramebufferBindType::FRAMEBUFFER);
            buffer->BindTextureToFramebufferWithID(textureId, GL_COLOR_ATTACHMENT0, 0, EyeFramebuffersMultisampleCount);

            Assert(buffer->IsComplete());

            buffer->UnBindFramebuffer();

            VREyeRendertargetsFences[eye].push_back(0);

            VREyeRendertargets[eye].push_back(std::move(buffer));

        }
    }

}

void Renderer::OnResume()
{
    eglMakeCurrent(Display, DummySurface, DummySurface, CurrentContext);

    if(CurrentContext == EGL_NO_CONTEXT)
    {
        return; //no context no rendering, this happens when SurfaceView calls the render method before the onSurfaceCreated method
    }

    ovrModeParms modeParms = vrapi_DefaultModeParms( &OvrInstanceData );
    modeParms.Flags |= VRAPI_MODE_FLAG_NATIVE_WINDOW;

    //ANativeWindow*  window = ANativeWindow_fromSurface(Env, Surface);

    modeParms.Display = (size_t)(Display);
    modeParms.WindowSurface = (size_t)(CurrentWindow);
    modeParms.ShareContext = (size_t)(CurrentContext);

    OVRMode = vrapi_EnterVrMode( &modeParms );

    AssertWithMessage(OVRMode != 0, "Not able to enter vr mode")

    FrameIDx = 1;
}

void Renderer::OnPause()
{
   if(OVRMode)
   {
       vrapi_LeaveVrMode(OVRMode);
   }
}


