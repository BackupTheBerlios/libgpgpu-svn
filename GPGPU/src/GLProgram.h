/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#ifndef H_GPGPU_GLPROGRAM
#define H_GPGPU_GLPROGRAM
#include "InternalPrerequisites.h"

namespace GPGPU
{

/** Unified interface to program languages implemented for GL
 */
class GLProgram: public Program
{
public:
    GLProgram();
    virtual ~GLProgram();
    
    /// Get texture units configuration for program from parameters
    virtual std::vector<TexturePtr> &getTextureUnits() = 0;
    
    /// Bind program and its parameters
    /// Changes to the parameters while the program is bound must reflect in the rendering API
    virtual void bind() = 0;
    /// Unbind program and its parameters
    virtual void unbind() = 0;
    
};
typedef SharedPtr<GLProgram> GLProgramPtr;
};
#endif
