/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */

#include "PBuffer.h"
#include "GLEngine.h"
#include <sstream>
/// NV: GLX_NV_float_buffer
#ifndef GLX_FLOAT_COMPONENTS_NV
#define GLX_FLOAT_COMPONENTS_NV         0x20B0
#endif

/// ATI: GLX_ATI_pixel_format_float
#ifndef GLX_RGBA_FLOAT_ATI_BIT
#define GLX_RGBA_FLOAT_ATI_BIT 0x00000100
#endif

/// ARB: GLX_ARB_fbconfig_float
#ifndef GLX_RGBA_FLOAT_BIT
#define GLX_RGBA_FLOAT_BIT 0x00000004
#endif
#ifndef GLX_RGBA_FLOAT_TYPE
#define GLX_RGBA_FLOAT_TYPE 0x20B9
#endif

namespace GPGPU
{
/** GL pbuffer object
*/


PBuffer::PBuffer():
    mContext(0),
    mPBuffer(0),
    mWidth(256),
    mHeight(256),
    mBitDepth(8),
    mIsFloat(false)
{
}

PBuffer::~PBuffer()
{
    /// Release resources
    mBoundTextures.clear();
    mBoundVertexStream.release();
    /// Finally, destroy the buffer
    Display *dpy = GLEngine::getInstance().getDisplay();
    if(mContext)
        glXDestroyContext(dpy, mContext);
    if(mPBuffer)
        glXDestroyPbuffer(dpy, mPBuffer);
}

void PBuffer::initialise()
{
    /// Smaller dimensions are not sane
    if(mWidth < 64)
        mWidth = 64;
    if(mHeight < 64)
        mHeight = 64;

    Display *dpy = GLEngine::getInstance().getDisplay();
	GLXContext context = GLEngine::getInstance().getSharedContext();
	int screen = DefaultScreen(dpy);
    
    /// Find out which extension to use for floating point
/// Possible floating point extensions, in order of preference
#define RTF_NONE 0
#define RTF_NV 1    /** GLX_NV_float_buffer */
#define RTF_ATI 2   /** GLX_ATI_pixel_format_float */
#define RTF_ARB 3   /** GLX_ARB_fbconfig_float */
    
    int floatBuffer = RTF_NONE;
    if(mIsFloat)
    {
        /// Query supported float buffer extensions
        /// Choose the best one
        std::stringstream ext;
        std::string instr;
		ext << glXQueryExtensionsString(dpy, screen) << " " << glXGetClientString(dpy, GLX_EXTENSIONS);
        while(ext >> instr)
        {
            if(instr == "GLX_NV_float_buffer" && floatBuffer<RTF_NV)
                floatBuffer = RTF_NV;
            if(instr == "GLX_ATI_pixel_format_float" && floatBuffer<RTF_ATI)
                floatBuffer = RTF_ATI;
            if(instr == "GLX_ARB_fbconfig_float" && floatBuffer<RTF_ARB)
                floatBuffer = RTF_ARB;
        }
        if(floatBuffer == RTF_NONE)
        {
            GPGPU_EXCEPT(ERR_RENDERSYSTEM, "Floating point rendertargets not supported on this hardware");
        }
    }
    
    /// Make the buffer
	int attribs[50];
	int attrib = 0;
    if (floatBuffer == RTF_ATI) {
	    attribs[attrib++] = GLX_RENDER_TYPE;
	    attribs[attrib++] = GLX_RGBA_FLOAT_ATI_BIT;
    } 
    else if (floatBuffer == RTF_ARB) 
    {
        attribs[attrib++] = GLX_RENDER_TYPE;
	    attribs[attrib++] = GLX_RGBA_FLOAT_BIT;
    }
    else
    {
    	attribs[attrib++] = GLX_RENDER_TYPE;
	    attribs[attrib++] = GLX_RGBA_BIT;
    }
	attribs[attrib++] = GLX_DRAWABLE_TYPE;
	attribs[attrib++] = GLX_PBUFFER_BIT;
	attribs[attrib++] = GLX_STENCIL_SIZE;
	attribs[attrib++] = 8;
	attribs[attrib++] = GLX_DEPTH_SIZE;
	attribs[attrib++] = 24;
	
    attribs[attrib++] = GLX_RED_SIZE;
    attribs[attrib++] = mBitDepth;
    attribs[attrib++] = GLX_GREEN_SIZE;
    attribs[attrib++] = mBitDepth;
    attribs[attrib++] = GLX_BLUE_SIZE;
    attribs[attrib++] = mBitDepth;
    attribs[attrib++] = GLX_ALPHA_SIZE;
    attribs[attrib++] = mBitDepth;
    if (floatBuffer == RTF_NV) {
		attribs[attrib++] = GLX_FLOAT_COMPONENTS_NV;
		attribs[attrib++] = 1;
	}
	attribs[attrib++] = None;

	GLXFBConfig * fbConfigs;
	int nConfigs;
	fbConfigs = glXChooseFBConfig(dpy, screen, attribs, &nConfigs);
	if (nConfigs == 0 || !fbConfigs)
		GPGPU_EXCEPT(ERR_RENDERSYSTEM, "RenderTexture::Initialize() creation error: Couldn't find a suitable pixel format");
    
    /// Determine the type of context to create
    int renderType = GLX_RGBA_TYPE;
    if(floatBuffer == RTF_ARB)
    {
        renderType = GLX_RGBA_FLOAT_TYPE;
    }
    
	/// Pick the first returned format that will return a pbuffer
	for (int i = 0; i < nConfigs; i++) {
        // Check colour format
        int redSize, greenSize, blueSize, alphaSize;
        glXGetFBConfigAttrib(dpy, fbConfigs[i], GLX_RED_SIZE, &redSize);
        glXGetFBConfigAttrib(dpy, fbConfigs[i], GLX_GREEN_SIZE, &greenSize);
        glXGetFBConfigAttrib(dpy, fbConfigs[i], GLX_BLUE_SIZE, &blueSize);
        glXGetFBConfigAttrib(dpy, fbConfigs[i], GLX_ALPHA_SIZE, &alphaSize);
        
        if(redSize == mBitDepth && greenSize == mBitDepth && 
                blueSize == mBitDepth && alphaSize == mBitDepth)
            {
                attrib = 0;
                attribs[attrib++] = GLX_PBUFFER_WIDTH;
                attribs[attrib++] = mWidth; // Get from texture?
                attribs[attrib++] = GLX_PBUFFER_HEIGHT;
                attribs[attrib++] = mHeight; // Get from texture?
                attribs[attrib++] = GLX_PRESERVED_CONTENTS;
                attribs[attrib++] = 1;
                attribs[attrib++] = None;
                mPBuffer =
                    glXCreatePbuffer(dpy, fbConfigs[i], attribs);
                if (mPBuffer) {
                    mContext =
                        glXCreateNewContext(dpy,
                                                    fbConfigs[i],
                                                    renderType,
                                                    context, True);
                    break;
                }
        }
	}
	if (!mPBuffer)
		GPGPU_EXCEPT(ERR_RENDERSYSTEM, "RenderTexture::Initialize() pbuffer creation error: No exact match found");
	if (!mContext) 
        GPGPU_EXCEPT(ERR_RENDERSYSTEM, "RenderTexture::Initialize() creation error:  glXCreateContext() failed");

    /// Read final size
    int _iWidth, _iHeight;
	glXQueryDrawable(dpy, mPBuffer, GLX_WIDTH,
						   (GLuint *) & _iWidth);
	glXQueryDrawable(dpy, mPBuffer, GLX_HEIGHT,
						   (GLuint *) & _iHeight);
	mWidth = _iWidth;
    mHeight = _iHeight;
}
void PBuffer::makeCurrent()
{
    if(glXGetCurrentContext() == mContext)
        // Already active
        return;
    Display *dpy = GLEngine::getInstance().getDisplay();
    glXMakeCurrent(dpy, mPBuffer, mContext);
    GLEngine::getInstance().setCurrentContext(mContext);
}

};
