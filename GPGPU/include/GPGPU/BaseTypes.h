/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#ifndef H_GPGPU_BASETYPES
#define H_GPGPU_BASETYPES

#include <GPGPU/Prerequisites.h>

namespace GPGPU {


/** 3D box description
*/
struct Box
{
    /// POD constructor
    Box() { }
    /// 2D constructor
    /// Note that left, top and front are inclusive but right, bottom and back are
    /// not. This means the box includes left<=x<right && top<=y<bottom && front<=z<back
    Box(float left, float top, float right, float bottom):
        left(left), right(right),
        top(top), bottom(bottom),
        front(0), back(1) {}
    /// 3D constructor
    Box(float left, float top, float front,
        float right, float bottom, float back):
        left(left), right(right),
        top(top), bottom(bottom),
        front(front), back(back) {}
        
    /// Tribial accessors
    float getWidth() { return right-left; }
    float getHeight() { return bottom-top; }
    float getDepth() { return back-front; }
    /// Members
    float left, right;
    float top, bottom;
    float front, back;
};

typedef Box Rect;

/** Information about GPU hardware.
 */
struct HWInfo
{
    std::vector<std::string> languages;
    std::string model;
    size_t textureUnits;
    /// Floating point texture support
    bool mFloatTextures;
    /// Full floating point texture support (if false, only for rectangle textures)
    bool mFullFloatTextures;
    /// Cg: best profile for FP and VP supported by card
    std::string cgBestFP, cgBestVP;
};

};

#endif
