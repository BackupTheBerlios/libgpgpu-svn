/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#ifndef H_GPGPU_GLENGINE
#define H_GPGPU_GLENGINE

#include "InternalPrerequisites.h"
#include "GLTexture.h"
#include "GLProgram.h"
#include "GLVertexStream.h"
#include "PBuffer.h"
#include "ProgramFactory.h"

namespace GPGPU
{
enum FloatTexType
{
    FTT_NONE,
    FTT_ATI,
    FTT_NV
};
class GLEngine: public Engine
{
public:
    GLEngine();
    virtual ~GLEngine();
    
    /** Initialise GPGPU engine.
    */
    virtual void initialise();
    
    /** Create a new texture
    */
    virtual TexturePtr createTexture();
    
    /** Create a new program
    */
    virtual ProgramPtr createProgram(const std::string &language);

    
    virtual VertexStreamPtr createVertexStream();
    
    /** Set the current render target texture. It must be renderable.
    */
    virtual void setTarget(TexturePtr rtt);

    /** Bind a program
        Program have bindings globalVar->parameter
    */
    virtual void setProgram(ProgramPtr program);
    
    /** Send render state (current target and program and its parameters) to GPU
    */
    virtual void update();
    
    /** Blit a quad from the source texture units to the destination texture, 
        using the bound program.
        Only one set of texture coordinates is set; set 0. It is automatically bound to the
        position in the destination texture.
        @param dest    Optionally describes the destination rectangle in texture coordinates. 
            If none given, cover entire surface.
    */
    virtual void quadBlit(const Rect *dest = 0);
    

    virtual void executeVertexStream(VertexStreamPtr stream, size_t first, size_t last);
    
    /** Get information about the hardware
    */
    virtual void getHWInfo(HWInfo &info);
    
    /// Get the X display connection
    Display *getDisplay() { return mDisplay; }
    /// Get some GL context, or 0 if there is none yet
    GLXContext getSharedContext();
    /// Check if current context changed outside the GPGPU engine
    void checkContextExternallyChanged();
    /// Keep current context for administration
    void setCurrentContext(GLXContext ctx) { mCurrentContext = ctx; }
    /// Get PBuffer for a certain format, create if not available
    PBufferPtr getPBuffer(PixelType format, size_t width, size_t height);
    /// Set current pbuffer
    void setPBuffer(PixelType format, size_t width, size_t height);
    /// Report GL error
    void reportGLError(const std::string &func);
    /// Get number of texture units
    size_t getTexUnitCount() { return mMaxTextureUnits; }
    /// Get float texture type
    FloatTexType getFloatTexType() { return mFloatTexType; }
    
    /// Singleton accessor
    static GLEngine &getInstance() { return *instance; }
private:
    Display      *mDisplay;
    GLXContext   mCurrentContext;
    GLProgramPtr mCurrentProgram;
    PBufferPtr   mCurrentPBuffer;
    /// Current render target
    GLTexturePtr    mTarget; 

    /// Number of texture units on this card
    size_t mMaxTextureUnits;
    FloatTexType mFloatTexType;

    static GLEngine *instance;
    
    /// Contains a pbuffer for each pixel data type
    std::vector<PBufferPtr> mPBuffers;
    
    /// Program currently bound (only one at a time, not on a per-context basis otherwise 
    /// setting parameters would be complicated)
    GLProgramPtr mBoundProgram;
    
    /// Map program types to factories
    typedef std::map<std::string, ProgramFactoryPtr> ProgramFactoryMap;
    ProgramFactoryMap mProgramFactories;
    
    /// Hardware info as returned by getHWInfo
    HWInfo mHWInfo;
};

// Simple macro to check for errors after a GL call and throw if needed
#define GPGPU_CHECKGLERR(func) GPGPU::GLEngine::getInstance().reportGLError(func)
// Clear the GL error flag
#define GPGPU_IGNOREGLERR() glGetError()
};

#endif
