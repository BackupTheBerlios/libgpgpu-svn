/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#include "CgProgram.h"

namespace GPGPU
{
    CgProgramFactory::CgProgramFactory()
    {
    }
    CgProgramFactory::~CgProgramFactory()
    {
    }
    
    void CgProgramFactory::initialise(HWInfo &info)
    {
        info.cgBestFP = cgGetProfileString(cgGLGetLatestProfile(CG_GL_FRAGMENT));
        info.cgBestVP = cgGetProfileString(cgGLGetLatestProfile(CG_GL_VERTEX));
    }

    ProgramPtr CgProgramFactory::createProgram(const std::string &language)
    {
        return ProgramPtr(new CgProgram());
    }
    
    void CgProgram::checkCgError()
    {
        CGerror err=cgGetError();
        if(err != CG_NO_ERROR)
        {
            std::string msg;
            msg = cgGetErrorString(err);
            if (err == CG_COMPILER_ERROR)
            {
                // Get listing with full compile errors
                msg = msg + ":" + cgGetLastListing(mContext);
            }
            GPGPU_EXCEPT(ERR_RENDERSYSTEM, msg);
        }
    }
    CgProgram::CgProgram():
        mProfileFP(CG_PROFILE_UNKNOWN),mProfileVP(CG_PROFILE_UNKNOWN),
        mContext(0), mProgram(0), mEntryPoint("main")
    {
        
    }
    CgProgram::~CgProgram()
    {
        if(mProgram)
            cgDestroyProgram(mProgram);
        if(mContext)
            cgDestroyContext(mContext);
    }
    
    void CgProgram::setSource(const std::string &src)
    {
        mSourceCode = src;
    }
    
    void CgProgram::setEntryPoint(const std::string &src)
    {
        mEntryPoint = src;
    }
   
    void CgProgram::initialise()
    {
        mContext = cgCreateContext();
        /// Find best profile
        mProfileFP = cgGLGetLatestProfile(CG_GL_FRAGMENT);
        mProfileVP = cgGLGetLatestProfile(CG_GL_VERTEX);
        /// Set optimal options for profile
        cgGLSetOptimalOptions(mProfileFP);
        cgGLSetOptimalOptions(mProfileVP);
        /// Create fragment program
        mProgram = cgCreateProgram(mContext,  // The context where we want it added
            CG_SOURCE,              // The following parameter is an array 
                                    // of bytes containing Cg source
            mSourceCode.c_str(),  // The sourcefile
            mProfileFP,           // The profile 
            mEntryPoint.c_str(),  // The entry function 
            NULL                  // Compiler options
            );
        checkCgError();
        //std::cerr << cgGetProgramString(mProgram, CG_COMPILED_PROGRAM) << std::endl;
        cgGLLoadProgram(mProgram);
        checkCgError();
    }
    
    void CgProgram::setParameter(const std::string &name, float x, float y, float z, float w)
    {
        CGparameter param = cgGetNamedParameter(mProgram, name.c_str());
        if(!param)
            GPGPU_EXCEPT(ERR_RENDERSYSTEM, "Cg: Unknown parameter "+name);
        cgSetParameter4f(param, x, y, z, w);
        checkCgError();
    }

    void CgProgram::setParameter(const std::string &name, const float matrix[16])
    {
        CGparameter param = cgGetNamedParameter(mProgram, name.c_str());
        if(!param)
            GPGPU_EXCEPT(ERR_RENDERSYSTEM, "Cg: Unknown parameter "+name);
        cgSetMatrixParameterfc(param, matrix);
        checkCgError();
    }

    void CgProgram::setParameter(const std::string &name, TexturePtr tex)
    {
        CGparameter param = cgGetNamedParameter(mProgram, name.c_str());
        if(!param)
            GPGPU_EXCEPT(ERR_RENDERSYSTEM, "Cg: Unknown parameter "+name);
        /// Remember texture so it can be set later on
        size_t texunit = cgGLGetTextureEnum(param) - GL_TEXTURE0_ARB;
        if(texunit>=GPGPU_TEXUNITS)
            GPGPU_EXCEPT(ERR_RENDERSYSTEM, "Cg: Unknown texture unit");
        if(texunit>=mTextureUnits.size())
            mTextureUnits.resize(texunit+1);
        mTextureUnits[texunit] = tex;
    }
    
    std::vector<TexturePtr> &CgProgram::getTextureUnits()
    {
        return mTextureUnits;
    }
    
    /// Bind program and its parameters
    void CgProgram::bind()
    {
        cgGLEnableProfile(mProfileFP);
        checkCgError();
        cgGLBindProgram(mProgram);
        checkCgError();
    }
    /// Unbind program and its parameters
    void CgProgram::unbind()
    {
        cgGLUnbindProgram(mProfileFP);
    }
};
