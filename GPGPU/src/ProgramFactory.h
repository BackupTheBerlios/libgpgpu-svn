/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#ifndef H_GPGPU_PROGRAMFACTORY
#define H_GPGPU_PROGRAMFACTORY
#include "InternalPrerequisites.h"

namespace GPGPU
{
class ProgramFactory
{
public:
    ProgramFactory();
    virtual ~ProgramFactory();
    
    /** Initialize the program factory and optionally update hardware info with information about this program type.
    */
    virtual void initialise(HWInfo &info) = 0;
    
   /** Create a new program
    */
    virtual ProgramPtr createProgram(const std::string &language) = 0;
};
typedef SharedPtr<ProgramFactory> ProgramFactoryPtr;
};
#endif
