/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#ifndef H_GPGPU_GLTEXTURE
#define H_GPGPU_GLTEXTURE
#include "InternalPrerequisites.h"
namespace GPGPU
{
    
/** Interface to a Texture for GL.
 */
class GLTexture: public Texture
{
public:
    GLTexture();
    virtual ~GLTexture();
    
    virtual void setSize(size_t width, size_t height=1, size_t depth=1);
    virtual void setType(GLenum type);
    virtual void setMipmaps(size_t nmipmaps);
    virtual void setInternalFormat(PixelComponents format, PixelType type);
    virtual void setFiltering(GLenum min, GLenum mag);
    virtual void setWrapping(GLenum s, GLenum t, GLenum u);
    virtual void setRenderTarget(bool rtt);
    
    virtual void initialise();
    
    virtual void loadData(PixelComponents format, PixelType type, void *data, size_t length=0);
    virtual void storeData(PixelComponents format, PixelType type, void *data, size_t *length=0);

    virtual void getInfo(Info &information);
    
    virtual TexturePtr getDepthTexture();
    virtual TexturePtr clone();
    
    // Copy upper left frame buffer portion to texture
    void copyToTexture();
    
    size_t getWidth() { return mWidth; }
    size_t getHeight() { return mHeight; }
    size_t getDepth() { return mDepth; }
    bool isRectangle() { return mTarget == GL_TEXTURE_RECTANGLE_ARB; }
    PixelType getPixelType() { return mType; }
    GLenum getTarget() { return mTarget; }
    GLuint getID() { return mID; }
    
    /// Keep administration of being bound as target
    /// This is to prevent access while bound
    bool getBoundAsTarget() { return mIsBoundAsTarget; }
    void setBoundAsTarget(bool x) { mIsBoundAsTarget = x; }
private:
    size_t mWidth, mHeight, mDepth;
    GLenum mTarget;
    size_t mNumMipmaps;
    PixelComponents mFormat;
    PixelType mType;
    GLenum mMinFilter, mMagFilter;
    GLenum mWrapS, mWrapT, mWrapR;
    bool mIsRTT;
    bool mIsBoundAsTarget;
    
    GLuint mID;    // GL texture object
};
typedef SharedPtr<GLTexture> GLTexturePtr;
};
#endif
