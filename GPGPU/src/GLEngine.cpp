/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#include "InternalPrerequisites.h"
#include "GLEngine.h"
#include "GLVertexStream.h"

#include "CgProgram.h"
#include "GLSLProgram.h"
namespace GPGPU
{
    GLEngine *GLEngine::instance = 0;
    GLEngine::GLEngine():
        mCurrentContext(0),
        mMaxTextureUnits(0),
        mFloatTexType(FTT_NONE)
    {
        instance = this;
        /// Sane defaults for hardware info
        mHWInfo.textureUnits = 0;
        mHWInfo.mFloatTextures = false;
        mHWInfo.mFullFloatTextures = false;
        
        
    }
    GLEngine::~GLEngine()
    {
        mBoundProgram.release();
        mProgramFactories.clear();
        mPBuffers.clear(); /// This is needed so that the pbuffers are destroyed before this
        mCurrentPBuffer.release();
        instance = 0;
    }
    
    /** Initialise GPGPU engine.
    */
    void GLEngine::initialise()
    {
        /// Initialize GL extension wrangler
        mDisplay = glXGetCurrentDisplay();
        if(mDisplay == 0)
        {
            /// If there is none, create one
            mDisplay = XOpenDisplay(NULL);
            if(!mDisplay)
                GPGPU_EXCEPT(ERR_RENDERSYSTEM, "Could not open X display");
        }
        mPBuffers.resize(PT_COUNT);
        
        /// Initialize PBuffer for BYTE format
        getPBuffer(PT_BYTE, 256, 256)->makeCurrent();
        
        /// Initialize GL here (load extensions etc)
        glewInit();
        
        /// Get card model
        std::string vendor(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
        std::string renderer(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
        mHWInfo.model = vendor + " " + renderer;
        
        /// Get number of texture units
        GLint maxTextureUnits;
        glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &maxTextureUnits);
        mMaxTextureUnits = (size_t)maxTextureUnits;
        if(mMaxTextureUnits > GPGPU_TEXUNITS)
            mMaxTextureUnits = GPGPU_TEXUNITS;
        mHWInfo.textureUnits = mMaxTextureUnits;
        // Check for NV or ATI float textures
        if(GLEW_ATI_texture_float)
        {
            mFloatTexType = FTT_ATI;
            mHWInfo.mFloatTextures = true;
            mHWInfo.mFullFloatTextures = true;
        }
        else if(GLEW_NV_float_buffer)
        {
            mFloatTexType = FTT_NV;
            mHWInfo.mFloatTextures = true;
            mHWInfo.mFullFloatTextures = false;
        }
        
        /// Register program factories
        mProgramFactories["Cg"] = ProgramFactoryPtr(new CgProgramFactory());
        if(GLEW_ARB_shader_objects)
            mProgramFactories["GLSL"] = ProgramFactoryPtr(new GLSLProgramFactory());
        
        // Initialize program managers
        for(ProgramFactoryMap::iterator i=mProgramFactories.begin(); i!=mProgramFactories.end(); ++i)
        {
            mHWInfo.languages.push_back(i->first);
            i->second->initialise(mHWInfo);
        }
    }
    
    /** Create a new texture
    */
    TexturePtr GLEngine::createTexture()
    {
        return TexturePtr(new GLTexture());
    }
    
    /** Create a new program
    */
    ProgramPtr GLEngine::createProgram(const std::string &language)
    {
        ProgramFactoryMap::iterator i = mProgramFactories.find(language);
        if(i == mProgramFactories.end())
        {
            GPGPU_EXCEPT(ERR_INVALIDPARAMS, "Language "+language+" not supported");
        }
        return i->second->createProgram(language);
    }
    
    VertexStreamPtr GLEngine::createVertexStream()
    {
        return VertexStreamPtr(new GLVertexStream());
    }
    
    /** Set the current render target texture. It must be renderable.
    */
    void GLEngine::setTarget(TexturePtr rtt)
    {
        checkContextExternallyChanged();
        if(!mTarget.isNull())
        {
            /// Copy contents of current context to old texture
            mTarget->copyToTexture();
            mTarget->setBoundAsTarget(false);
        }
        
        mTarget = rtt;
        if(mTarget.isNull())
            return;
        mTarget->setBoundAsTarget(true);
        
        /// (optionally) set context
        setPBuffer(mTarget->getPixelType(), 
            mTarget->getWidth(), mTarget->getHeight());
        
        /// Set up viewport so that x and y match texture coordinates
        /// z points into depth
        glViewport(0,0,mTarget->getWidth(), mTarget->getHeight());
        GPGPU_CHECKGLERR("glViewport");
        
        /// Set viewport to span texture area
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        if(mTarget->isRectangle())
            glOrtho(0.0f, mTarget->getWidth(), 0.0f, mTarget->getHeight(), 0.0f, 1.0f);
        else
            glOrtho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
        GPGPU_CHECKGLERR("glOrtho");
    }
    


    /** Bind a program
        Program have bindings globalVar->parameter
    */
    void GLEngine::setProgram(ProgramPtr program)
    {
        if(mCurrentProgram.get() == program.get())
            return;
        mCurrentProgram = program;
    }
    
    /** Send render state (current target and program and its parameters) to GPU
    */
    void GLEngine::update()
    {
        checkContextExternallyChanged();
        /// Bind parameters and program
        if(mBoundProgram.get() != mCurrentProgram.get())
        {
            if(!mBoundProgram.isNull())
                /// Unbind old program
                mBoundProgram->unbind();
            mCurrentProgram->bind();
            mBoundProgram = mCurrentProgram;
        }
        /// Bind texture units
        std::vector<TexturePtr> &units = mCurrentProgram->getTextureUnits();
        for(size_t x=0; x<mMaxTextureUnits; x++)
        {
            if(x<units.size() && mCurrentPBuffer->mBoundTextures[x].get() == units[x].get())
                // Texture already bound to this texture unit
                continue;
            glActiveTexture(GL_TEXTURE0 + x);
            GPGPU_CHECKGLERR("glActiveTexture");
            if(!mCurrentPBuffer->mBoundTextures[x].isNull())
            {
                // There is a texture bound to this unit
                // Disable and release
                glDisable(mCurrentPBuffer->mBoundTextures[x]->getTarget());
                GPGPU_CHECKGLERR("glDisable (texture unit)");
                mCurrentPBuffer->mBoundTextures[x].release();
            }
            if(x<units.size() && !units[x].isNull())
            {
                GLTexturePtr newtex = units[x];
                // Enable and bind
                glEnable(newtex->getTarget());
                GPGPU_CHECKGLERR("glEnable (texture unit)");
                glBindTexture(newtex->getTarget(), newtex->getID());
                GPGPU_CHECKGLERR("glBindTexture (texture unit)");
                // Reference
                mCurrentPBuffer->mBoundTextures[x] = units[x];
            }
        }
        glActiveTexture(GL_TEXTURE0);
    }
    
    /** Blit a quad from the source texture units to the destination texture, 
        using the bound program.
        Only one set of texture coordinates is set; set 0. It is automatically bound to the
        position in the destination texture.
        @param dest    Optionally describes the destination rectangle in texture coordinates. 
            If none given, cover entire surface.
    */
    void GLEngine::quadBlit(const Rect *dest)
    {
        update();
        Rect temp;
        if(dest == 0)
        {
            // Default value -- full texture
            temp.left = temp.top = 0.0f;
            if(mTarget->isRectangle())
            {
                temp.right = mTarget->getWidth();
                temp.bottom = mTarget->getHeight();
            }
            else
            {
                temp.right = 1.0f;
                temp.bottom = 1.0f;
            }
            dest = &temp;
        }

        /// Draw the quad
        glBegin(GL_QUADS);
        glTexCoord2f(dest->right, dest->top);
        glVertex2f(dest->right, dest->top);
        
        glTexCoord2f(dest->left, dest->top);
        glVertex2f(dest->left, dest->top);
        
        glTexCoord2f(dest->left, dest->bottom);
        glVertex2f(dest->left, dest->bottom);
        
        glTexCoord2f(dest->right, dest->bottom);
        glVertex2f(dest->right, dest->bottom);
        glEnd();
        
        GPGPU_CHECKGLERR("Blit quad");
    }
    
    void GLEngine::executeVertexStream(VertexStreamPtr str, size_t first, size_t last)
    {
        GLVertexStream *stream = static_cast<GLVertexStream *>(str.get());
        update();
        
        if(last > stream->getVertexCount())
            last = stream->getVertexCount();
        if(first == last)
            // Empty domain
            return;
        /// Bind stream only when not already bound, this might save some cycles
        if(mCurrentPBuffer->mBoundVertexStream.get() != stream)
        {
            if(!mCurrentPBuffer->mBoundVertexStream.isNull())
                mCurrentPBuffer->mBoundVertexStream->unbind();
            stream->bind();
            mCurrentPBuffer->mBoundVertexStream = str;
        }
        /// Do the render!
        glDrawArrays(stream->getPrimitiveType(), first, last-first);
        GPGPU_CHECKGLERR("glDrawArrays");
    }
    
    void GLEngine::getHWInfo(HWInfo &info)
    {
        info = mHWInfo;
    }
    
    GLXContext GLEngine::getSharedContext()
    {
        /// Iterate over pbuffer pointers, return the first with a context
        for(size_t x=0; x<mPBuffers.size(); x++)
        {
            if(!mPBuffers[x].isNull())
                return mPBuffers[x]->getContext();
        }
        /// Otherwise, try to share current context (returns 0 if there is none)
        return glXGetCurrentContext();
    }
    void GLEngine::checkContextExternallyChanged() 
    { 
       if(glXGetCurrentContext() != mCurrentContext)
       {
           GPGPU_EXCEPT(ERR_RENDERSYSTEM, "GL context changed unexpectedly");
       }
    }
    PBufferPtr GLEngine::getPBuffer(PixelType format, size_t width, size_t height)
    {
        if(!mPBuffers[format].isNull() &&
            mPBuffers[format]->getWidth() >= width &&
            mPBuffers[format]->getHeight() >= height)
            return mPBuffers[format];
        /// Create a new pbuffer bigger or equal to the requested dimensions
        PBufferPtr pb = PBufferPtr(new PBuffer());
        pb->setSize(width, height);
        switch(format)
        {
            case PT_BYTE:    /// Byte per component (8 bit fixed 0.0..1.0)
                pb->setFormat(8, false);
                break;
            case PT_SHORT:   /// Short per component (16 bit fixed 0.0..1.0))
                pb->setFormat(16, false);
                break;
            case PT_FLOAT16:/// 16 bit float per component
                pb->setFormat(16, true);
                break;
            case PT_FLOAT32: /// 32 bit float per component
                pb->setFormat(32, true);
                break;
            default:
                GPGPU_EXCEPT(ERR_INVALIDPARAMS, "Invalid PBuffer format");
        }
        pb->initialise();
        mPBuffers[format] = pb;
        
        /// Set state parameters
        pb->mBoundTextures.resize(GPGPU_TEXUNITS);
        return pb;
    }
    void GLEngine::setPBuffer(PixelType format, size_t width, size_t height)
    {
        /// Get a (cached) pbuffer of the requested dimensions
        PBufferPtr newPBuffer = getPBuffer(format, width, height);
        /// Only do an expensive context switch when needed
        if(mCurrentPBuffer.get() != newPBuffer.get())
        {
            /// We must unbind the current program when switching to another context
            /// when there is 1) a current context 2) there is a bound program
            if(!mCurrentPBuffer.isNull() && 
                !mBoundProgram.isNull())
            {
                /// Unbind the program when switching targets
                mBoundProgram->unbind();
                mBoundProgram.release();
            }
            mCurrentPBuffer = newPBuffer;
            mCurrentPBuffer->makeCurrent();
        }
    }
    void GLEngine::reportGLError(const std::string &func)
    {
        GLenum err = glGetError();
        if(err != GL_NO_ERROR)
            GPGPU_EXCEPT(ERR_RENDERSYSTEM, "GL call failed: "+func);
    }
    

};
