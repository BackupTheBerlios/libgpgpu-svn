#include "DowdSortingNetwork.h"
#include <iostream>
#include "GenUtil.h"
#include "DowdSortingNetwork.h"

DowdSortingNetwork::DowdSortingNetwork():
    width(1), height(1), logwidth(0), logheight(0)
{ 
    precision = "Half";
}
DowdSortingNetwork::~DowdSortingNetwork()
{
}
void DowdSortingNetwork::setSize(size_t w, size_t h)
{
    if(w<1 || h<1)
        GPGPU_EXCEPT(GPGPU::ERR_INVALIDPARAMS, "Size in each dimension must be larger or equal to 1");
    width = w;
    height = h;
    // Get log2 of width
    logwidth = 0;
    while((w&1)==0) { logwidth++;  w >>= 1; }
    if(w != 1)
        GPGPU_EXCEPT(GPGPU::ERR_INVALIDPARAMS, "Width is not a power of two");
    // Get log2 of height
    logheight = 0;
    while((h&1)==0) { logheight++;  h >>= 1; }
    if(h != 1)
        GPGPU_EXCEPT(GPGPU::ERR_INVALIDPARAMS, "Height is not a power of two");
}
void DowdSortingNetwork::initialize()
{
    if(width<2 && height<2)
        GPGPU_EXCEPT(GPGPU::ERR_INVALIDPARAMS, "Size in at least one dimension must be larger than one, otherwise sorting makes no sense");
    engine = GPGPU::get();
    
    /// Load program text
    std::string programText = loadText("shaders/sortdowdoptimal.cg");
    if(programText.empty())
    {
        GPGPU_EXCEPT(GPGPU::ERR_INTERNALERROR, "Shader program could not be loaded from disk");
    }
    
    /// Loading bitonic sort program
    std::cerr << "Loading Dowd sort program [" << precision << "]" << std::endl;
    sortProgram = engine->createProgram("Cg");
    sortProgram->setSource(programText);
    sortProgram->setEntryPoint("pivot"+precision);
    sortProgram->initialise();
    
    std::cerr << "Building sort network" << std::endl;
    buildSortingNetwork();
}
void DowdSortingNetwork::pivot(size_t depth)
{
    for(size_t i=0; i<depth; i++)
    {
        GPGPU::TextureLease y = pool->getLease();
        /// Set output texture for this step
        engine->setTarget(y.get());
        /// Set input texture for this step
        sortProgram->setParameter("input", cur.get());
        /// All other paraneters are embedded in the vertex stream
        engine->executeVertexStream(stream, beginVertices[i], endVertices[i]);
        
        cur = y;
    }
}

void DowdSortingNetwork::sort(GPGPU::TexturePool *p, GPGPU::TextureLease &c)
{
    pool = p;
    cur = c;
    // Sink c
    c.release();
    
    engine->setProgram(sortProgram);
    
    size_t N = logwidth+logheight;
    for(size_t depth = 2; depth <= N; ++depth)
    {
        pivot(depth);
    }
    pivot(N);
    
    // Transfer result back to c
    c = cur;
    cur.release();
    pool = 0;
}

/** Build the sorting network as one vertex stream with (height + width - 2) quads. Offsets to the beginning
    and ending vertices of passes are kept in seperate vectors.
*/
void DowdSortingNetwork::buildSortingNetwork()
{
    stream = engine->createVertexStream();
    stream->setUsage(GPGPU::BU_STATIC);
    /// 2^0 + ... + 2^(logheight-1) + 2^0 + ... + 2^(logwidth-1)
    size_t totalQuads = height + width - 2;
    stream->setVertexCount(4 * totalQuads);
    stream->declareVertexAttribute(2, GPGPU::AS_POSITION);
    stream->declareVertexAttribute(2, GPGPU::AS_TEXCOORD0); // Texcoord 0 output coord (+input coord 1)
    stream->declareVertexAttribute(2, GPGPU::AS_TEXCOORD1); // Texcoord 1 input coord 2
    stream->declareVertexAttribute(2, GPGPU::AS_TEXCOORD2); // Texcoord 2 operation (sign 1.0..-1.0, 0 must fall on pivot)
    stream->initialise();
    size_t vertices = 0;
    float *str = stream->lock();
    
    // Vertical sorts
    for(size_t i=0; i<logheight; i++)
    {
        beginVertices.push_back(vertices);
        /// Build contents
        size_t offset = height >> i;
        for(size_t sub=0; sub<height; sub+=offset)
        {
            GPGPU::Rect rect;
            rect.left = 0;
            rect.right = width;
            rect.top = sub;
            rect.bottom = sub+offset;
            GPGPU::emitFloats(str, rect.right, rect.top); // Position
            GPGPU::emitFloats(str, rect.right, rect.top); // Texcoord 0
            GPGPU::emitFloats(str, rect.left, rect.bottom);  // Texcoord 1 (both coord flipped)
            GPGPU::emitFloats(str, 1.0f, 0.0f);          // Texcoord 2 (op) above pivot=1
            
            GPGPU::emitFloats(str, rect.left, rect.top); // Position
            GPGPU::emitFloats(str, rect.left, rect.top); // Texcoord 0
            GPGPU::emitFloats(str, rect.right, rect.bottom); // Texcoord 1 (both coord flipped)
            GPGPU::emitFloats(str, 1.0f, 0.0f);          // Texcoord 2 (op) above pivot=1
            
            GPGPU::emitFloats(str, rect.left, rect.bottom); // Position
            GPGPU::emitFloats(str, rect.left, rect.bottom); // Texcoord 0
            GPGPU::emitFloats(str, rect.right, rect.top); // Texcoord 1 (both coord flipped)
            GPGPU::emitFloats(str, -1.0f, 0.0f);          // Texcoord 2 (op) below pivot=-1
            
            GPGPU::emitFloats(str, rect.right, rect.bottom); // Position
            GPGPU::emitFloats(str, rect.right, rect.bottom); // Texcoord 0
            GPGPU::emitFloats(str, rect.left, rect.top); // Texcoord 1 (both coord flipped)
            GPGPU::emitFloats(str, -1.0f, 0.0f);          // Texcoord 2 (op) below pivot=-1
            vertices += 4;
        }
        endVertices.push_back(vertices);
    }
    
    // Horizontal sorts
    for(size_t i=0; i<logwidth; i++)
    {
        beginVertices.push_back(vertices);
        /// Build contents
        size_t offset = width >> i;
        for(size_t sub=0; sub<width; sub+=offset)
        {
            GPGPU::Rect rect;
            rect.left = sub;
            rect.right = sub+offset;
            rect.top = 0;
            rect.bottom = height;
            GPGPU::emitFloats(str, rect.right, rect.top); // Position
            GPGPU::emitFloats(str, rect.right, rect.top); // Texcoord 0
            GPGPU::emitFloats(str, rect.left, rect.top);  // Texcoord 1 (coord 0 flipped)
            GPGPU::emitFloats(str, -1.0f, 0.0f);          // Texcoord 2 (op) right of pivot=-1
            
            GPGPU::emitFloats(str, rect.left, rect.top); // Position
            GPGPU::emitFloats(str, rect.left, rect.top); // Texcoord 0
            GPGPU::emitFloats(str, rect.right, rect.top); // Texcoord 1 (coord 0 flipped)
            GPGPU::emitFloats(str, 1.0f, 0.0f);          // Texcoord 2 (op) left of pivot=1
            
            GPGPU::emitFloats(str, rect.left, rect.bottom); // Position
            GPGPU::emitFloats(str, rect.left, rect.bottom); // Texcoord 0
            GPGPU::emitFloats(str, rect.right, rect.bottom); // Texcoord 1 (coord 0 flipped)
            GPGPU::emitFloats(str, 1.0f, 0.0f);          // Texcoord 2 (op) left of pivot=1
            
            GPGPU::emitFloats(str, rect.right, rect.bottom); // Position
            GPGPU::emitFloats(str, rect.right, rect.bottom); // Texcoord 0
            GPGPU::emitFloats(str, rect.left, rect.bottom); // Texcoord 1 (coord 0 flipped)
            GPGPU::emitFloats(str, -1.0f, 0.0f);          // Texcoord 2 (op) right of pivot=-1
            vertices += 4;
        }
        endVertices.push_back(vertices);
    }
    stream->unlock();
}

