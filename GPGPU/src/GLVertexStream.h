/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#ifndef H_GPGPU_GLVERTEXSTREAM
#define H_GPGPU_GLVERTEXSTREAM


#include "InternalPrerequisites.h"

namespace GPGPU
{

class GLVertexStream: public VertexStream
{
public:
    GLVertexStream();
    virtual ~GLVertexStream();

    virtual void setVertexCount(size_t count);
    virtual void setUsage(BufferUsage usage);
    virtual void setPrimitiveType(PrimitiveType t);
    virtual void declareVertexAttribute(size_t components, AttributeSemantic semantic);

    virtual void initialise();
    virtual size_t getSize();

    virtual float* lock();
    virtual void unlock();
    
    void bind();
    void unbind();
    size_t getVertexCount() { return mVertexCount; }
    GLenum getPrimitiveType() { return mPrimitiveType; }
private:
    struct VertexAttributeDeclaration
    {
        size_t offset;
        size_t components;
        AttributeSemantic semantic;
    };
    typedef std::vector<VertexAttributeDeclaration> VertexAttributeDeclarationList;
    
    VertexAttributeDeclarationList mAttribs;
    size_t mVertexCount;
    size_t mVertexSize;
    GLenum mPrimitiveType;
    GLenum mUsage;
    GLuint mBufferId;
    bool mIsLocked;
};
typedef SharedPtr<GLVertexStream> GLVertexStreamPtr;
}

#endif
