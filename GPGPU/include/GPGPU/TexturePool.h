/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#ifndef H_GPGPU_TEXTUREPOOL
#define H_GPGPU_TEXTUREPOOL
#include <GPGPU/Prerequisites.h>
#include <GPGPU/Texture.h>
namespace GPGPU
{
    
    /** Texture pool, a quick way to get temporary textures
    */
    class TexturePool
    {
    public:
        TexturePool();
        ~TexturePool();
        
        /** Set the capacity (fixed size) of this pool.
            @note can only be set before initialise
         */
        void setCapacity(size_t capacity) { mCapacity = capacity; }
        void setTemplate(TexturePtr templ) { mTemplate = templ; }
        
        /** Initialise the pool object. This will create and initialize capacity
            textures identical to the template texture.
        */
        void initialise();
        
        struct PoolObject
        {
            TexturePtr ptr;
            unsigned    count;
        };
        /// Counted pointer
        class TextureLease
        {
        public:
            TextureLease(PoolObject * p=0)
                : itsCounter(p) {}
            ~TextureLease()
                {release();}
            TextureLease(const TextureLease& r) throw()
                {acquire(r.itsCounter);}
            TextureLease& operator=(const TextureLease& r)
            {
                if (this != &r) {
                    release();
                    acquire(r.itsCounter);
                }
                return *this;
            }
    
            Texture& operator*()  const throw()   {return *itsCounter->ptr.get();}
            Texture* operator->() const throw()   {return itsCounter->ptr.get(); }
            TexturePtr get()      const throw()   {return itsCounter->ptr; }
            void release()
            { // decrement the count, delete if it is 0
                if (itsCounter) {
                    if (--itsCounter->count == 0) {
                        // Do nothing -- 0 means deleted
                    }
                    itsCounter = 0;
                }
            }
        private:
            
            void acquire(PoolObject* c) throw()
            { // increment the count
                itsCounter = c;
                if (c) ++c->count;
            }
            PoolObject *itsCounter;
        };
        
        /** Get a texture lease from the pool
        */
        TextureLease getLease();
    private:
        size_t mCapacity;
        TexturePtr mTemplate;
        
        std::vector<PoolObject> mPool;
    };
    typedef TexturePool::TextureLease TextureLease;
    
};
#endif
