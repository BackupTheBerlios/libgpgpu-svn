/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#ifndef H_GPGPU_TEXTURE
#define H_GPGPU_TEXTURE

#include <GPGPU/Prerequisites.h>
#include <GPGPU/BaseTypes.h>
#include <GPGPU/Enums.h>

namespace GPGPU
{
typedef SharedPtr<Texture> TexturePtr;
/** Interface to a Texture, a generic 1D 2D or 3D matrix of vectors of 1,2,3 or 4 values.
 */
class Texture
{
public:
    Texture();
    virtual ~Texture();

    /** Set texture size.
        @note Must be called before initialise to be effective
    */
    virtual void setSize(size_t width, size_t height=1, size_t depth=1) = 0;

    /** Set texture type. Defaults to GL_TEXTURE_RECTANGLE_ARB.
        @param type     One of GL_TEXTURE_1D,GL_TEXTURE_2D,GL_TEXTURE_3D,
            GL_TEXTURE_RECTANGLE_ARB.
        @note Must be called before initialise to be effective
    */
    virtual void setType(GLenum type)  = 0;

    /** Set the number of mipmaps for this texture. This defaults to 0 (no mipmaps).
        In addition to positive integers, one can provide MIP_UNLIMITED for mipmaps up to
        1x1.
        @note Must be called before initialise to be effective
    */
    virtual void setMipmaps(size_t nmipmaps) = 0;
   
    /** Set the internal format and data type of this texture. Defaults to PC_RGBA and
        PT_BYTE.
        @param format    Components in this texture; this is one of PC_RGBA, PC_RG, 
                         PC_R or PC_NONE
        @param type      Data type of this texture; this is one of PT_BYTE, PT_FLOAT16 
                         or PT_FLOAT32
        @note Must be called before initialise to be effective
    */
    virtual void setInternalFormat(PixelComponents format, PixelType type) = 0;
    
    /** Set the filtering method for minimization and magnification. Both default to
        GL_NEAREST.
        @note Must be called before initialise to be effective
    */
    virtual void setFiltering(GLenum min, GLenum mag) = 0;
    
    /** Set wrapping method for the first, second and third coordinate of the texture.
        @param s,t,y    Can be one of GL_CLAMP_TO_EDGE or GL_REPEAT
        @note Must be called before initialise to be effective
    */
    virtual void setWrapping(GLenum s, GLenum t, GLenum r) = 0;
    
    /** Set the texture to be eligible to render to. Defaults to false.
        @param rtt      True if this texture is to be rendered to.
        @note Must be called before initialise to be effective
    */
    virtual void setRenderTarget(bool rtt) = 0;
    
    /** Initialise the texture. This creates the actual texture.
    */
    virtual void initialise() = 0;
    
    /** Load contents from memory into texture.
        @param format    Components in this texture; this is one of PC_RGBA, PC_RG, 
                         PC_R or PC_NONE
        @param type      Data type of this texture; this is one of PT_BYTE, PT_FLOAT16 
                         or PT_FLOAT32
        @param data      Pointer to the data to load. The data must be laid out in
                         slice, row, column fashion.
        @param length    Length of the data. This is an optional parameter which can be
                         specified when loading compressed textures.
    */
    virtual void loadData(PixelComponents format, PixelType type, void *data, size_t length=0) = 0;

    /** Load contents from texture into memory.
        @param format    Components in this texture; this is one of PC_RGBA, PC_RG, 
                         PC_R or PC_NONE
        @param type      Data type of this texture; this is one of PT_BYTE, PT_FLOAT16 
                         or PT_FLOAT32
        @param data      Pointer to where the data will be stored. The data will be laid out in
                         slice, row, column fashion.
        @param length    Length of the data. This is an optional parameter which can be
                         specified when loading compressed textures.
    */
    virtual void storeData(PixelComponents format, PixelType type, void *data, size_t *length=0) = 0;
    
    struct Info
    {
        size_t width, height, depth;
        PixelComponents format;
        PixelType type;
    };

    /** Get the information about this texture. This returns the width, height, depth, format and
        type.
    */
    virtual void getInfo(Info &information) = 0;
    
    /** Get the depth surface as a texture.
    */
    virtual TexturePtr getDepthTexture() = 0;
    
    /** Clone this texture. This creates a new, unitialized texture with the same parameters
        as this one.
    */
    virtual TexturePtr clone() = 0;
};


typedef std::vector<TexturePtr> TexturePtrList;

};

#endif
