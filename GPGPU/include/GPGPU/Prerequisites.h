/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#ifndef H_GPGPU_PREREQUISITES
#define H_GPGPU_PREREQUISITES


/// Initialize GL extension wrangler
/*        
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/wglew.h>
#else
#include <GL/glxew.h>
#endif
*/
#include <GL/glew.h>
//#include <GL/gl.h>
//#include <GL/glext.h>

#include <vector>
#include <map>
#include <string>


namespace GPGPU
{
    /// Basic types
    typedef unsigned char uint8;
    typedef unsigned short uint16;
    typedef unsigned int uint32;

    typedef char int8;
    typedef short int16;
    typedef int int32;

    typedef unsigned char uchar;
    typedef unsigned short ushort;
    typedef unsigned int uint;
    
    /// Counted pointer
    template <class X> class SharedPtr
    {
    public:
        typedef X element_type;
    
        explicit SharedPtr(X* p = 0) // allocate a new counter
            : itsCounter(0) {if (p) itsCounter = new counter(p);}
        ~SharedPtr()
            {release();}
        SharedPtr(const SharedPtr& r) throw()
            {acquire(r.itsCounter);}
        SharedPtr& operator=(const SharedPtr& r)
        {
            if (this != &r) {
                release();
                acquire(r.itsCounter);
            }
            return *this;
        }
        bool isNull() const { return itsCounter == 0; }
        
        //template <class Y> friend class GPGPU::SharedPtr<Y>;
        template <class Y> SharedPtr(const SharedPtr<Y>& r) throw()
            {acquire(reinterpret_cast<counter*>(r.itsCounter));}
        template <class Y> SharedPtr& operator=(const SharedPtr<Y>& r)
        {
            if ((void*)this != (void*)&r) {
                release();
                acquire(reinterpret_cast<counter*>(r.itsCounter));
            }
            return *this;
        }

        X& operator*()  const throw()   {return *itsCounter->ptr;}
        X* operator->() const throw()   {return itsCounter->ptr;}
        X* get()        const throw()   {return itsCounter ? itsCounter->ptr : 0;}
        bool unique()   const throw()
            {return (itsCounter ? itsCounter->count == 1 : true);}
        void release()
        { // decrement the count, delete if it is 0
            if (itsCounter) {
                if (--itsCounter->count == 0) {
                    delete itsCounter->ptr;
                    delete itsCounter;
                }
                itsCounter = 0;
            }
        }
    //
    
        struct counter {
            counter(X* p = 0, unsigned c = 1) : ptr(p), count(c) {}
            X*          ptr;
            unsigned    count;
        }* itsCounter;
    private:
        void acquire(counter* c) throw()
        { // increment the count
            itsCounter = c;
            if (c) ++c->count;
        }
    

    };

    /// Predefine classes
    class Texture;
    class Engine;
    class Program;
    class VertexStream;
};

#endif
