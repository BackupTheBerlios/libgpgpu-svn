/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#ifndef GPGPU_UTIL
#define GPGPU_UTIL
#include <GPGPU/Prerequisites.h>
#include <GPGPU/BaseTypes.h>
namespace GPGPU {

    /// Some convience functions for using the GPGPU lib
    std::string getDescription(HWInfo &info);
    /// Utility functions for filling vertex streams
    inline void emitFloats(float *&str, float x)
    {
        str[0] = x;
        str += 1;
    }
    inline void emitFloats(float *&str, float x, float y)
    {
        str[0] = x;
        str[1] = y;
        str += 2;
    }
    inline void emitFloats(float *&str, float x, float y, float z)
    {
        str[0] = x;
        str[1] = y;
        str[2] = z;
        str += 3;
    }
    inline void emitFloats(float *&str, float x, float y, float z, float w)
    {
        str[0] = x;
        str[1] = y;
        str[2] = z;
        str[3] = w;
        str += 4;
    }
};

#endif
