/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#include <GPGPU/TexturePool.h>
#include <GPGPU/Exception.h>
namespace GPGPU
{
    TexturePool::TexturePool():
        mCapacity(1)
    {
    }
    TexturePool::~TexturePool()
    {
    }

    void TexturePool::initialise()
    {
        if(mTemplate.isNull())
            GPGPU_EXCEPT(ERR_INVALIDPARAMS, "No template provided for pool");
        mPool.resize(mCapacity);
        /// Initialise textures in pool
        for(size_t x=0; x<mCapacity; x++)
        {
            mPool[x].count = 0;
            mPool[x].ptr = mTemplate->clone();
            mPool[x].ptr->initialise();
        }
        /// Template is no longer needed now
        mTemplate.release();
    }

    TexturePool::TextureLease TexturePool::getLease()
    {
        /// Find a free pool slot
        for(size_t x=0; x<mCapacity; x++)
        {
            if(mPool[x].count == 0)
            {
                /// Found a free pool slot
                ++mPool[x].count;
                return TextureLease(&mPool[x]);
            }
        }
        GPGPU_EXCEPT(ERR_RESOURCEDEPLETED, "Could not get texture lease, pool is full");
    }
};
