/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#include "InternalPrerequisites.h"
#include "GLEngine.h"

namespace GPGPU
{
    EnginePtr theEngine;
    /** Dummy constructors and destructors */
    Engine::Engine()
    {
    }
    Engine::~Engine()
    {
    }
    Program::Program()
    {
    }
    Program::~Program()
    {
    }
    Texture::Texture()
    {
    }
    Texture::~Texture()
    {
    }
    VertexStream::VertexStream()
    {
    }
    VertexStream::~VertexStream()
    {
    }
    ProgramFactory::ProgramFactory()
    {
    }
    ProgramFactory::~ProgramFactory()
    {
    }
    GLProgram::GLProgram()
    {
    }
    GLProgram::~GLProgram()
    {
    }
    
    /** Singleton management */
    EnginePtr create()
    {
        GPGPU_ASSERT(theEngine.isNull(), "GPGPU engine was already created");
        theEngine = EnginePtr(new GLEngine());
        return theEngine;
    }
    EnginePtr get()
    {
        return theEngine;
    }

    void destroy()
    {
        theEngine.release();
    }
};
