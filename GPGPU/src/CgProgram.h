/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#ifndef H_GPGPU_CGPROGRAM
#define H_GPGPU_CGPROGRAM
#include "InternalPrerequisites.h"
#include "ProgramFactory.h"
#include "GLProgram.h"
#include <Cg/cg.h>
#include <Cg/cgGL.h>
namespace GPGPU
{
class CgProgramFactory: public ProgramFactory
{
public:
    CgProgramFactory();
    ~CgProgramFactory();
    
    virtual void initialise(HWInfo &info);

    virtual ProgramPtr createProgram(const std::string &language);
};

class CgProgram: public GLProgram
{
public:
    CgProgram();
    virtual ~CgProgram();
    
    /** Set program source code
    */
    virtual void setSource(const std::string &src);
    
    virtual void setEntryPoint(const std::string &src);
    
    /** Compile and initialize the program
    */
    virtual void initialise();
    
    /// Functions to set parameters
    /// 1,2,3,4 floats
    virtual void setParameter(const std::string &name, float x, float y, float z, float w);
    /// 4x4 matrix
    virtual void setParameter(const std::string &name, const float matrix[16]);
    /// Texture (sampler)
    virtual void setParameter(const std::string &name, TexturePtr tex);
    
    /// Get texture units configuration for program from parameters
    std::vector<TexturePtr> &getTextureUnits();
    
    /// Bind program and its parameters
    /// Changes to the parameters while the program is bound must reflect in the rendering API
    void bind();
    /// Unbind program and its parameters
    void unbind();
private:
    void checkCgError();
    
    CGprofile mProfileFP, mProfileVP;
    CGcontext mContext;
    CGprogram mProgram;
    std::string mSourceCode, mEntryPoint;
    std::vector<TexturePtr> mTextureUnits;
    
};
typedef SharedPtr<CgProgram> CgProgramPtr;
};
#endif
