#ifndef H_WL_DOWD
#define H_WL_DOWD
// GPGPU implementation of "The balanced sorting network" by Dowd et al

#include <GPGPU/Engine.h>
#include <GPGPU/Exception.h>
#include <GPGPU/Texture.h>
#include <GPGPU/TexturePool.h>
#include <GPGPU/Util.h>
class DowdSortingNetwork
{
public:
    DowdSortingNetwork();
    ~DowdSortingNetwork();
    
    /** Set the precision in which to do the sort (Half or Float). Defaults to Half.
        @param id    String containg the word Half or Float
    */
    void setPrecision(const std::string &id)
    {
        precision = id;
    }
    
    /** Set the size of the texture that this sorting network will sort.
    */
    void setSize(size_t w, size_t h);
    
    /** Initialize the sorting network.
    */
    void initialize();
    
    /** Apply sorting network, this sorts the texture by its R component ascending.
        Rows are sorted as if this was one big 1D texture with all rows merged consecutively.
        @param pool    Texture pool to use for processing (must be the same format as the input)
        @param cur     Texture lease pointing to the input texture. This will point to the 
                       sorted output texture after return.
    */
    void sort(GPGPU::TexturePool *pool, GPGPU::TextureLease &cur);
private:
    void buildSortingNetwork();
    void pivot(size_t depth);

    GPGPU::EnginePtr engine;
    GPGPU::ProgramPtr sortProgram;

    GPGPU::VertexStreamPtr stream;
    std::vector<size_t> beginVertices;
    std::vector<size_t> endVertices;
    size_t width, height;
    size_t logwidth, logheight;
    std::string precision;
    
    // Transatory state
    GPGPU::TexturePool *pool;
    GPGPU::TextureLease cur;
};

#endif
