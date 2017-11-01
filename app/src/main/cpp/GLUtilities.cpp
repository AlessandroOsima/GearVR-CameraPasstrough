//
// Created by HF on 04/10/17.
//

#include "GLUtilities.h"

std::string gl_error_string(GLenum const err)
{
    switch (err)
    {
        // opengl 2 errors (8)
        case GL_NO_ERROR:
            return "GL_NO_ERROR";

        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";

        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";

        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";


        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";

            // opengl 3 errors (1)
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";

            // gles 2, 3 and gl 4 error are handled by the switch above
        default:
            return "UKNOWN_ERROR";
    }
}

std::string gl_sync_error_string(GLenum const err) {
    switch (err)
    {
        // opengl 2 errors (8)
        case GL_ALREADY_SIGNALED :
            return "GL_ALREADY_SIGNALED";

        case GL_TIMEOUT_EXPIRED :
            return "GL_TIMEOUT_EXPIRED ";

        case GL_CONDITION_SATISFIED :
            return "GL_CONDITION_SATISFIED ";

        case GL_WAIT_FAILED :
            return "GL_WAIT_FAILED ";

            // gles 2, 3 and gl 4 error are handled by the switch above
        default:
            return "UKNOWN_ERROR";
    }
}
