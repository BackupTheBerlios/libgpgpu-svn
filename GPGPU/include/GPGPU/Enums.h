/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#ifndef GPGPU_ENUMS
#define GPGPU_ENUMS
#include <GPGPU/Prerequisites.h>

namespace GPGPU {

enum Mipmaps
{
    MIP_NONE = 0,
    MIP_UNLIMITED = 0xFFFFFFFF
};
enum PixelComponents 
{
    PC_RGBA = 4,    /// Red green blue and alpha
    PC_RG = 2,      /// Red and green
    PC_R = 1,       /// Red only
    PC_NONE = 0     /// No components (for depth only)
};
enum PixelType 
{
    PT_BYTE = 0,    /// Byte per component (8 bit fixed 0.0..1.0)
    PT_SHORT = 1,   /// Short per component (16 bit fixed 0.0..1.0))
    PT_FLOAT16 = 2, /// 16 bit float per component
    PT_FLOAT32 = 3, /// 32 bit float per component
    PT_COUNT = 4    /// Number of pixel types
};
enum AttributeSemantic
{
    AS_POSITION = 0,
    AS_BLENDWEIGHTS = 1,
    AS_NORMAL = 2,
    AS_COLOR0 = 3,
    AS_COLOR1 = 4,
    AS_FOGCOORD = 5,
    AS_UNKNOWN = 6,
    AS_BLENDINDICES = 7,
    AS_TEXCOORD0 = 8,
    AS_TEXCOORD1 = 9,
    AS_TEXCOORD2 = 10,
    AS_TEXCOORD3 = 11,
    AS_TEXCOORD4 = 12,
    AS_TEXCOORD5 = 13,
    AS_TEXCOORD6 = 14,
    AS_TEXCOORD7 = 15,
    AS_TEXCOORD8 = 16,
    AS_TEXCOORD9 = 17,
    AS_TEXCOORD10 = 18,
    AS_TEXCOORD11 = 19,
    AS_TEXCOORD12 = 20,
    AS_TEXCOORD13 = 21,
    AS_TEXCOORD14 = 22,
    AS_TEXCOORD15 = 23,
    AS_TEXCOORD16 = 24,
    AS_TEXCOORD17 = 25,
    AS_TEXCOORD18 = 26,
    AS_TEXCOORD19 = 27,
    AS_TEXCOORD20 = 28,
    AS_TEXCOORD21 = 29,
    AS_TEXCOORD22 = 30,
    AS_TEXCOORD23 = 31,
    AS_TEXCOORD24 = 32,
    AS_TEXCOORD25 = 33,
    AS_TEXCOORD26 = 34,
    AS_TEXCOORD27 = 35,
    AS_TEXCOORD28 = 36,
    AS_TEXCOORD29 = 37,
    AS_TEXCOORD30 = 38,
    AS_TEXCOORD31 = 39
};
enum PrimitiveType
{
    PT_QUADS,
    PT_TRIANGLES,
    PT_LINES,
    PT_POINTS
};
enum BufferUsage
{
    BU_STATIC,
    BU_DYNAMIC,
    BU_STREAM
};

};

#endif
