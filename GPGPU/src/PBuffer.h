/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#ifndef H_GPGPU_PBUFFER
#define H_GPGPU_PBUFFER
#include "InternalPrerequisites.h"
#include "GLTexture.h"
#include "GLVertexStream.h"
namespace GPGPU
{
/** GL pbuffer object
*/
class PBuffer
{
public:
    PBuffer();
    ~PBuffer();
    
    void setSize(size_t width, size_t height)
    {
        mWidth = width;
        mHeight = height;
    }
    void setFormat(size_t bits, bool isFloat)
    {
        mBitDepth = bits;
        mIsFloat = isFloat;
    }
    
    void initialise();
    /// Make this pbuffer the current rendering context
    void makeCurrent();
    
    GLXContext getContext() { return mContext; }
    size_t getWidth() { return mWidth; }
    size_t getHeight() { return mHeight; }
    
    // Per-context state data
    
    /// Currently bound vertex stream
    GLVertexStreamPtr mBoundVertexStream;
    /// Texture currently bound to each texture unit
    std::vector<GLTexturePtr>  mBoundTextures;
    
private:
    GLXContext mContext;
    GLXPbuffer mPBuffer;
    
    size_t mWidth, mHeight;
    size_t mBitDepth;
    bool mIsFloat;
};
typedef SharedPtr<PBuffer> PBufferPtr;

};
#endif
