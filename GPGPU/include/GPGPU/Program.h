/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#ifndef H_GPGPU_PROGRAM

#include <GPGPU/Prerequisites.h>
#include <GPGPU/BaseTypes.h>
#include <GPGPU/Enums.h>

namespace GPGPU
{

class Program
{
public:
    Program();
    virtual ~Program();
    
    /** Set program source code
        @param src    Source code
    */
    virtual void setSource(const std::string &src) = 0;
    
    /** Set program entry function; defaults to "main".
    */
    virtual void setEntryPoint(const std::string &src) = 0;
    
    /** Compile and initialize the program
    */
    virtual void initialise() = 0;
    
    /// Functions to set parameters
    /// 1,2,3 or 4 floats
    virtual void setParameter(const std::string &name, float x, float y=0.0f, float z=0.0f, float w=0.0f) = 0;
    /// 4x4 matrix
    virtual void setParameter(const std::string &name, const float matrix[16]) = 0;
    /// Texture (sampler)
    virtual void setParameter(const std::string &name, TexturePtr tex) = 0;
};
typedef SharedPtr<Program> ProgramPtr;
};

#endif
