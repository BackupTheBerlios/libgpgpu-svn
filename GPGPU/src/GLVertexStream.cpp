/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#include "GLVertexStream.h"
#include "GLEngine.h"
#define VBO_BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace GPGPU
{

    GLVertexStream::GLVertexStream():
        mVertexCount(0),
        mVertexSize(0),
        mPrimitiveType(GL_QUADS),
        mUsage(GL_STATIC_DRAW_ARB),
        mIsLocked(false)
    {
    }
    GLVertexStream::~GLVertexStream()
    {
        if(mBufferId)
            glDeleteBuffersARB(1, &mBufferId);
    }    

    void GLVertexStream::setVertexCount(size_t count)
    {
        mVertexCount = count;
    }
    
    void GLVertexStream::setUsage(BufferUsage usage)
    {
        switch(usage)
        {
        case BU_STATIC:
            mUsage = GL_STATIC_DRAW_ARB;
            break;
        case BU_STREAM:
            mUsage = GL_STREAM_DRAW_ARB;
            break;
        case BU_DYNAMIC:
        default:
            mUsage = GL_DYNAMIC_DRAW_ARB;
            break;
        }
    }
    
    void GLVertexStream::setPrimitiveType(PrimitiveType t)
    {
        switch(t)
        {
        case PT_QUADS:     mPrimitiveType = GL_QUADS; return;
        case PT_TRIANGLES: mPrimitiveType = GL_TRIANGLES; return;
        case PT_LINES:     mPrimitiveType = GL_LINES; return;
        case PT_POINTS:    mPrimitiveType = GL_POINTS; return;
        }
        GPGPU_EXCEPT(ERR_INVALIDPARAMS, "Invalid primitive type specified");
    }
    
    void GLVertexStream::declareVertexAttribute(size_t components, AttributeSemantic semantic)
    {
        /// Add the attribute
        VertexAttributeDeclaration decl;
        decl.offset = mVertexSize;
        decl.components = components;
        decl.semantic = semantic;
        mAttribs.push_back(decl);
        mVertexSize += components;
    }
    

    void GLVertexStream::initialise()
    {
        GLEngine::getInstance().checkContextExternallyChanged();
        if(mVertexSize==0 || mVertexCount == 0)
            GPGPU_EXCEPT(ERR_INVALIDPARAMS, "Cannot initialise vertex stream with zero elements or no declared attributes");
        /// Generate buffer
        glGenBuffersARB(1, &mBufferId);
        if (!mBufferId)
            GPGPU_EXCEPT(ERR_RENDERSYSTEM, 
                "Cannot create GL vertex buffer");

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, mBufferId);
        GPGPU_CHECKGLERR("glBindBufferARB");

        // Initialise mapped buffer and set usage
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, getSize()*sizeof(float), 0, mUsage);
        GPGPU_CHECKGLERR("glBufferDataARB");
    }
    
    size_t GLVertexStream::getSize()
    {
        return mVertexSize * mVertexCount;
    }

    float* GLVertexStream::lock()
    {
        GLEngine::getInstance().checkContextExternallyChanged();
        if(mIsLocked)
             GPGPU_EXCEPT(ERR_RENDERSYSTEM, "Vertex stream was already locked");
        /// Bind buffer as current
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, mBufferId);
        GPGPU_CHECKGLERR("glBindBufferARB");
        
        /// Map to CPU memory
        void* pBuffer = glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY);

        if(pBuffer == 0)
            GPGPU_EXCEPT(ERR_RENDERSYSTEM, "glMapBufferARB returned null pointer; probably out of memory");
        
        mIsLocked = true;
        return reinterpret_cast<float*>(pBuffer);
    }
    
    void GLVertexStream::unlock()
    {
        GLEngine::getInstance().checkContextExternallyChanged();
        if(!mIsLocked)
             GPGPU_EXCEPT(ERR_RENDERSYSTEM, "Vertex stream was not locked");
        
        /// Bind buffer as current
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, mBufferId);
        GPGPU_CHECKGLERR("glBindBufferARB");

        glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
        GPGPU_CHECKGLERR("glUnmapBufferARB");
        
        mIsLocked = false;
    }
    
    void GLVertexStream::bind()
    {
        if(mIsLocked)
             GPGPU_EXCEPT(ERR_RENDERSYSTEM, "Cannot render: Vertex stream is still locked");
        /// Bind buffer as current
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, mBufferId);
        GPGPU_CHECKGLERR("glBindBufferARB");
        
        /// Set attribute pointers
        size_t vertexSize = mVertexSize * sizeof(float);
        for(VertexAttributeDeclarationList::iterator i=mAttribs.begin(); i!=mAttribs.end(); ++i)
        {
            void *offset = VBO_BUFFER_OFFSET(i->offset*sizeof(float));
            /// Some semantics must be passed the old-fashioned way, otherwise it will not work without
            /// vertex program
            if(i->semantic == AS_POSITION)
            {
                glVertexPointer(i->components, GL_FLOAT, vertexSize, offset);
                glEnableClientState(GL_VERTEX_ARRAY);
            } else if(i->semantic == AS_NORMAL)
            {
                glNormalPointer(GL_FLOAT, vertexSize, offset);
                glEnableClientState( GL_NORMAL_ARRAY);
            } else if(i->semantic == AS_COLOR0)
            {
                glColorPointer(i->components, GL_FLOAT, vertexSize, offset);
                glEnableClientState( GL_COLOR_ARRAY );
            } else if(i->semantic == AS_COLOR1)
            { 
                glSecondaryColorPointerEXT(i->components, GL_FLOAT, vertexSize, offset);
                glEnableClientState( GL_SECONDARY_COLOR_ARRAY );
            } else
            if(i->semantic >= AS_TEXCOORD0 && i->semantic <= AS_TEXCOORD31)
            {
                glClientActiveTextureARB(GL_TEXTURE0 + (i->semantic-AS_TEXCOORD0));
                glTexCoordPointer(i->components, GL_FLOAT, vertexSize, offset);
                glEnableClientState( GL_TEXTURE_COORD_ARRAY );
            }
            else 
            {
                glVertexAttribPointerARB(i->semantic, i->components, GL_FLOAT, GL_FALSE, vertexSize, offset);
                GPGPU_CHECKGLERR("glVertexAttribPointerARB");
                glEnableVertexAttribArrayARB(i->semantic);
            }
        }
    }
    void GLVertexStream::unbind()
    {
        if(mIsLocked)
             GPGPU_EXCEPT(ERR_RENDERSYSTEM, "Cannot render: Vertex stream is still locked");
 
        /// Disable state
        for(VertexAttributeDeclarationList::iterator i=mAttribs.begin(); i!=mAttribs.end(); ++i)
        {
            if(i->semantic == AS_POSITION)
            {
                glDisableClientState(GL_VERTEX_ARRAY);
            } else if(i->semantic == AS_NORMAL)
            {
                glDisableClientState( GL_NORMAL_ARRAY);
            } else if(i->semantic == AS_COLOR0)
            {
                glDisableClientState( GL_COLOR_ARRAY );
            } else if(i->semantic == AS_COLOR1)
            { 
                glDisableClientState( GL_SECONDARY_COLOR_ARRAY );
            } else
            if(i->semantic >= AS_TEXCOORD0 && i->semantic <= AS_TEXCOORD31)
            {
                glClientActiveTextureARB(GL_TEXTURE0 + (i->semantic-AS_TEXCOORD0));
                glDisableClientState( GL_TEXTURE_COORD_ARRAY );
            }
            else 
            {
                glDisableVertexAttribArrayARB(i->semantic);
            }
        }
    }
    
};

