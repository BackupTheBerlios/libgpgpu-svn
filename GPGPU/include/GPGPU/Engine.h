/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#ifndef H_GPGPU_ENGINE
#define  H_GPGPU_ENGINE
#include <GPGPU/Prerequisites.h>
#include <GPGPU/BaseTypes.h>
#include <GPGPU/Enums.h>
#include <GPGPU/Texture.h>
#include <GPGPU/Program.h>
#include <GPGPU/VertexStream.h>

namespace GPGPU
{

class Engine
{
public:
    Engine();
    virtual ~Engine();
    
    /** Initialise GPGPU engine.
    */
    virtual void initialise() = 0;
    
    /** Create a new texture
    */
    virtual TexturePtr createTexture() = 0;
    
    /** Create a new program
    */
    virtual ProgramPtr createProgram(const std::string &language) = 0;
    
    /** Create a new vertex stream
    */
    virtual VertexStreamPtr createVertexStream() = 0;
    
    /** Set the current render target texture. It must be renderable.
    */
    virtual void setTarget(TexturePtr rtt = TexturePtr(0)) = 0;

    /** Bind a program
        Program have bindings globalVar->parameter
    */
    virtual void setProgram(ProgramPtr program) = 0;
    
    /** Send render state (current target and program and its parameters) to GPU
    */
    virtual void update() = 0;
    
    /** The central operation. Blit a quad from the source texture units to the 
        destination texture, using the bound program. 
        Only one set of texture coordinates is set; set 0. It is automatically bound to the
        position in the destination texture.
        @param dest    Optionally describes the destination rectangle in texture coordinates. 
            If none given, cover entire surface.
    */
    virtual void quadBlit(const Rect *dest = 0) = 0;
    
    /** "Run" a vertex stream directly. The current program and destination is used. If no first and
        last vertex is given, default to entire buffer.
        @param stream    Stream to render
        @param first     First vertex in stream to render
        @param last      Last vertex in stream (exclusive)
    */
    virtual void executeVertexStream(VertexStreamPtr stream, size_t first=0, size_t last=0xFFFFFFFF) = 0;
    
    /** Get information about the hardware
    */
    virtual void getHWInfo(HWInfo &info) = 0;
};

typedef SharedPtr<Engine> EnginePtr;

/**
 * Create the GPGPU engine.
 */
EnginePtr create();

/**
 * Get the GPGPU engine. (it must have been created first with create())
 */
EnginePtr get();

/**
 * Destroy the GPGPU engine. (it must have been created first with create())
 */
void destroy();

};

#endif
