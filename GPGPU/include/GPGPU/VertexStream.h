/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#ifndef H_GPGPU_VERTEXSTREAM
#define H_GPGPU_VERTEXSTREAM
#include <GPGPU/Prerequisites.h>
#include <GPGPU/BaseTypes.h>
#include <GPGPU/Enums.h>

namespace GPGPU
{

/** Class that encapsulates an arbitrary number of vertex records with arbitrary per-vertex data.
 */
class VertexStream
{
public:
    VertexStream();
    virtual ~VertexStream();
    
    /** Set the number of vertices in this stream.
        @note Only useful when called before initialise.
    */
    virtual void setVertexCount(size_t count) = 0;

    /** Set the usage of this stream. This is either BU_STATIC (written once and then left untouched),
        BU_DYNAMIC (changes a lot) or BU_STREAM (changes a lot, and every time this stream is locked the
        entire contents are discarded). Defaults to BU_STATIC.
        
        @note Only useful when called before initialise.
    */
    virtual void setUsage(BufferUsage usage) = 0;
    
    /** Set the type of primitive this vertex stream will render. Default to quad.
        @note Only useful when called before initialise.
    */
    virtual void setPrimitiveType(PrimitiveType t) = 0;
    
    /** Define a vertex attribute. This attribute will start at the end of the currently defined structure.
        @param components    Number of float components of this attribute
        @param semantic      What this attribute represents (POSITION, NORMAL, TEXCOORD0, ...)
        @note Only useful when called before initialise;
    */
    virtual void declareVertexAttribute(size_t components, AttributeSemantic semantic) = 0;
    
    /** Initialise the object.
    */
    virtual void initialise() = 0;
    
    /** Get the total size (in floats) of the buffer of this stream
    */
    virtual size_t getSize() = 0;
    
    /** Get a pointer to the vertex stream so that the program can write to it.
    */
    virtual float* lock() = 0;
    
    /** Return buffer to graphics subsystem.
    */
    virtual void unlock() = 0;
};
typedef SharedPtr<VertexStream> VertexStreamPtr;
}

#endif
