/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#include "GLTexture.h"
#include "GLEngine.h"

namespace GPGPU
{
    GLTexture::GLTexture():
        mWidth(256), mHeight(256), mDepth(1),
        mTarget(GL_TEXTURE_RECTANGLE_ARB),
        mNumMipmaps(0),
        mFormat(PC_RGBA), mType(PT_BYTE),
        mMinFilter(GL_NEAREST), mMagFilter(GL_NEAREST),
        mWrapS(GL_CLAMP_TO_EDGE), mWrapT(GL_CLAMP_TO_EDGE), mWrapR(GL_CLAMP_TO_EDGE),
        mIsRTT(false), mIsBoundAsTarget(false),
        mID(0)
    {
    }
    GLTexture::~GLTexture()
    {
        if(mID)
        {
            glDeleteTextures(1, &mID);
        }
    }
    
    void GLTexture::setSize(size_t width, size_t height, size_t depth)
    {
        mWidth = width;
        mHeight = height;
        mDepth = depth;
    }
    void GLTexture::setType(GLenum type)
    {
        mTarget = type;
    }
    void GLTexture::setMipmaps(size_t nmipmaps)
    {
        mNumMipmaps = nmipmaps;
    }
    void GLTexture::setInternalFormat(PixelComponents format, PixelType type)
    {
        mFormat = format;
        mType = type;
    }
    void GLTexture::setFiltering(GLenum min, GLenum mag)
    {
        mMinFilter = min;
        mMagFilter = mag;
    }
    void GLTexture::setWrapping(GLenum s, GLenum t, GLenum r)
    {
        mWrapS = s;
        mWrapT = t;
        mWrapR = r;
    }
    void GLTexture::setRenderTarget(bool rtt)
    {
        mIsRTT = rtt;
    }
    
    void GLTexture::initialise()
    {
        GLEngine::getInstance().checkContextExternallyChanged();
        if(mFormat == PC_NONE)
            // Depth only? Nothing to do
            return;
        // Create texture
        glGenTextures(1, &mID);
        GPGPU_CHECKGLERR("glGenTextures");
        // Bind texture to unit
        glBindTexture(mTarget, mID);
        GPGPU_CHECKGLERR("glBindTexture");

        /// Set texture parameters
		glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, mWrapS);
        if(mTarget != GL_TEXTURE_1D)
		    glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, mWrapT);
        if(mTarget == GL_TEXTURE_3D)
            glTexParameteri(mTarget, GL_TEXTURE_WRAP_R, mWrapR);
		glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, mMinFilter);
		glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, mMagFilter);
        if(mTarget != GL_TEXTURE_RECTANGLE_ARB)
            glTexParameteri(mTarget, GL_TEXTURE_MAX_LEVEL, mNumMipmaps);
        
        
        GLenum internalFormat;
        /// Devise GL internal format
        switch(mType)
        {
        case PT_BYTE:    /// Byte per component (8 bit fixed 0.0..1.0)
            switch(mFormat)
            {
            case PC_RGBA: internalFormat = GL_RGBA8; break;
            case PC_RG: internalFormat = GL_RGBA8; break;
            case PC_R: internalFormat = GL_LUMINANCE8; break;
            default:
                GPGPU_EXCEPT(ERR_INVALIDPARAMS, "Invalid number of texture components");
            }
            break;
        case PT_SHORT:   /// Short per component (16 bit fixed 0.0..1.0))
            switch(mFormat)
            {
            case PC_RGBA: internalFormat = GL_RGBA16; break;
            case PC_RG: internalFormat = GL_RGBA16; break;
            case PC_R: internalFormat = GL_LUMINANCE16; break;
            default:
                GPGPU_EXCEPT(ERR_INVALIDPARAMS, "Invalid number of texture components");
            }
            break;
        case PT_FLOAT16:/// 16 bit float per component
            switch(GLEngine::getInstance().getFloatTexType())
            {
            case FTT_NV:
                switch(mFormat)
                {
                case PC_RGBA: internalFormat = GL_FLOAT_RGBA16_NV; break;
                case PC_RG: internalFormat = GL_FLOAT_RG16_NV; break;
                case PC_R: internalFormat = GL_FLOAT_R16_NV; break;
                default:
                    GPGPU_EXCEPT(ERR_INVALIDPARAMS, "Invalid number of texture components");
                }
                break;
            case FTT_ATI:
                switch(mFormat)
                {
                case PC_RGBA: internalFormat = GL_RGBA_FLOAT16_ATI; break;
                case PC_RG: internalFormat = GL_RGBA_FLOAT16_ATI; break; /// No direct match
                case PC_R: internalFormat = GL_INTENSITY_FLOAT16_ATI; break;
                default:
                    GPGPU_EXCEPT(ERR_INVALIDPARAMS, "Invalid number of texture components");
                }
                break;               
            default:
                GPGPU_EXCEPT(ERR_RENDERSYSTEM, "Float16 textures unsupported for this card");
            }
            break;
        case PT_FLOAT32: /// 32 bit float per component
             switch(GLEngine::getInstance().getFloatTexType())
            {
            case FTT_NV:
                switch(mFormat)
                {
                case PC_RGBA: internalFormat = GL_FLOAT_RGBA32_NV; break;
                case PC_RG: internalFormat = GL_FLOAT_RG32_NV; break;
                case PC_R: internalFormat = GL_FLOAT_R32_NV; break;
                default:
                    GPGPU_EXCEPT(ERR_INVALIDPARAMS, "Invalid number of texture components");
                }
                break;
            case FTT_ATI:
                switch(mFormat)
                {
                case PC_RGBA: internalFormat = GL_RGBA_FLOAT32_ATI; break;
                case PC_RG: internalFormat = GL_RGBA_FLOAT32_ATI; break; /// No direct match
                case PC_R: internalFormat = GL_INTENSITY_FLOAT32_ATI; break;
                default:
                    GPGPU_EXCEPT(ERR_INVALIDPARAMS, "Invalid number of texture components");
                }
                break;
            default:
                GPGPU_EXCEPT(ERR_RENDERSYSTEM, "Float32 textures unsupported for this card");
            }
            break;
        default:
            GPGPU_EXCEPT(ERR_INVALIDPARAMS, "Invalid texture data type");
        }
        /// Allocate memory
        /// TODO 1D+3D
        glTexImage2D(mTarget,
                    0,
                    internalFormat,
                    mWidth, mHeight,
                    0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        GPGPU_CHECKGLERR("glTexImage2D");
    }
    
    GLenum PixelComponentsToGL(PixelComponents pc)
    {
        switch(pc)
        {
        case PC_RGBA: return GL_RGBA;
        case PC_R: return GL_RED;
        default:
            GPGPU_EXCEPT(ERR_INVALIDPARAMS, "Unsupported number of texture upload components");
        }
    }
    GLenum PixelTypeToGL(PixelType pt)
    {
        switch(pt)
        {
        case PT_BYTE: return GL_UNSIGNED_BYTE;
        case PT_SHORT: return GL_UNSIGNED_SHORT;
        case PT_FLOAT16: return GL_HALF_FLOAT_NV;
        case PT_FLOAT32: return GL_FLOAT;
        default:
            GPGPU_EXCEPT(ERR_INVALIDPARAMS, "Unsupported texture upload data type");
        }
    }
    
    void GLTexture::loadData(PixelComponents format, PixelType type, void *data, size_t length)
    {
        GPGPU_ASSERT(!mIsBoundAsTarget, "Cannot access texture data while bound as target");
        GLEngine::getInstance().checkContextExternallyChanged();
        // Bind texture to unit
        glBindTexture(mTarget, mID);
        // Transfer data
        /// TODO 1D+3D
        glTexSubImage2D(mTarget, 0, 0, 0, mWidth, mHeight, 
            PixelComponentsToGL(format), PixelTypeToGL(type), data);
        GPGPU_CHECKGLERR("glTexSubImage2D");
    }
    void GLTexture::storeData(PixelComponents format, PixelType type, void *data, size_t *length)
    {
        GPGPU_ASSERT(!mIsBoundAsTarget, "Cannot access texture data while bound as target");
        GLEngine::getInstance().checkContextExternallyChanged();
        // TODO check size
        // Bind texture to unit
        glBindTexture(mTarget, mID);
        // Transfer data
        glGetTexImage(mTarget, 0,
            PixelComponentsToGL(format), PixelTypeToGL(type), data);
        GPGPU_CHECKGLERR("glGetTexImage");
    }

    void GLTexture::getInfo(Info &i)
    {
        i.width = mWidth;
        i.height = mHeight;
        i.depth = mDepth;
        i.format = mFormat;
        i.type = mType;
    }
    
    TexturePtr GLTexture::getDepthTexture()
    {
        GPGPU_EXCEPT(ERR_INVALIDPARAMS, "Operation not yet implemented");
    }
    TexturePtr GLTexture::clone()
    {
        GLTexture *clone = new GLTexture();
        
        clone->setSize(mWidth, mHeight, mDepth);
        clone->setType(mTarget);
        clone->setMipmaps(mNumMipmaps);
        clone->setInternalFormat(mFormat, mType);
        clone->setFiltering(mMinFilter, mMagFilter);
        clone->setWrapping(mWrapS, mWrapT, mWrapR);
        clone->setRenderTarget(mIsRTT);
        return TexturePtr(clone);
    }
    
    void GLTexture::copyToTexture()
    {
        glBindTexture(mTarget, mID);
        glCopyTexSubImage2D(mTarget, 0, 0, 0, 0, 0, mWidth, mHeight);
    }
};
