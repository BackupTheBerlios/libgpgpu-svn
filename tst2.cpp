// Test vertex streams
#include <GPGPU/Engine.h>
#include <GPGPU/Exception.h>
#include <GPGPU/Texture.h>
#include <GPGPU/TexturePool.h>
#include <GPGPU/VertexStream.h>
#include <GPGPU/Util.h>
#include <iostream>
#include <fstream>
#include "GenUtil.h"

void printTexture(GPGPU::TexturePtr tex)
{
    GPGPU::Texture::Info ti;
    tex->getInfo(ti);
    float data2[ti.width*ti.height];
    tex->storeData(GPGPU::PC_R, GPGPU::PT_FLOAT32, data2);
    for(size_t y=0; y<ti.height; y++)
    {
        for(size_t x=0; x<ti.width; x++)
            std::cerr << data2[y*ti.width+x] << " ";
        std::cerr << std::endl;
    }
    
}

void dummyLoadTexture(GPGPU::TexturePtr tex)
{
    GPGPU::Texture::Info ti;
    tex->getInfo(ti);
    float data2[ti.width*ti.height*4];
    tex->storeData(GPGPU::PC_RGBA, GPGPU::PT_FLOAT32, data2);
}

int main()
{
    GPGPU::EnginePtr engine = GPGPU::create();
    try
    {
        engine->initialise();
    } catch(GPGPU::Exception &e) {
        std::cerr << e.getDescription() << std::endl;
        return 1;
    }
    try
    {
        std::cerr << "Initialising the pool" << std::endl;
        GPGPU::TexturePool pool;
        pool.setCapacity(3);
        /// Create template
        GPGPU::TexturePtr temp = engine->createTexture();
        temp->setSize(16,16);
		//temp->setType(GL_TEXTURE_2D);
		//temp->setWrapping(GL_REPEAT, GL_REPEAT, GL_REPEAT);

		// PT_FLOAT16 broken on ati
        temp->setInternalFormat(GPGPU::PC_RGBA, GPGPU::PT_FLOAT32);
        temp->setRenderTarget(true);
        pool.setTemplate(temp);
        
        pool.initialise();
        
        /// Load a program
        std::string programText = loadText("shaders/Test.cg");
        if(programText.empty())
        {
            std::cerr << "Test.cg could not be loaded" << std::endl;
            return 1;
        }
        GPGPU::ProgramPtr diffuse = engine->createProgram("Cg");
        diffuse->setSource(programText);
        diffuse->setEntryPoint("coord");
        diffuse->initialise();
        
        /// Get a texture
        GPGPU::TextureLease x;
        x = pool.getLease();
        
        /// Load data into texture
        /*float data[256];
        for(size_t x=0; x<256; x++)
            data[x] = 0.0f;
        data[8*16+8] = 1.0f;
        x->loadData(GPGPU::PC_R, GPGPU::PT_FLOAT32, data);
        */

        GPGPU::VertexStreamPtr stream = engine->createVertexStream();
        stream->setVertexCount(8*8*4);
        stream->declareVertexAttribute(2, GPGPU::AS_POSITION);
        stream->declareVertexAttribute(2, GPGPU::AS_TEXCOORD0);
        stream->initialise();
        
        float *str = stream->lock();
/*
        for(size_t y=0; y<2; y++)
            for(size_t x=0; x<2; x++)
            {
                GPGPU::emitFloats(str, x*8+8, y*8+0, 8, 0); // Right top
                GPGPU::emitFloats(str, x*8+0, y*8+0, 0, 0); // Left top
                GPGPU::emitFloats(str, x*8+0, y*8+8, 0, 8); // Left bottom
                GPGPU::emitFloats(str, x*8+8, y*8+8, 8, 8); // Right bottom
            }
*/
        for(size_t y=0; y<8; y++)
            for(size_t x=0; x<8; x++)
            {
                GPGPU::emitFloats(str, x*2+2, y*2+0, x*2+2, y*2+0); // Right top
                GPGPU::emitFloats(str, x*2+0, y*2+0, x*2+0, y*2+0); // Left top
                GPGPU::emitFloats(str, x*2+0, y*2+2, x*2+0, y*2+2); // Left bottom
                GPGPU::emitFloats(str, x*2+2, y*2+2, x*2+2, y*2+2); // Right bottom
            }

        stream->unlock();
        /// Run the loop
        Timer t;
        t.start();
        for(size_t i=0; i<1; i++)
        {
            GPGPU::TextureLease y = pool.getLease();
            engine->setTarget(y.get());
            //diffuse->setParameter("input", x.get());
            engine->setProgram(diffuse);
            engine->executeVertexStream(stream);
            //engine->quadBlit();
            x = y;
        }
        engine->setTarget();
        dummyLoadTexture(x.get());
        t.stop();
        std::cerr << "Time elapsed: " << t.getSeconds() << std::endl;
        
        
        printTexture(x.get());
    } catch(GPGPU::Exception &e) {
        std::cerr << e.getDescription() << std::endl;
    }
    
	return 0;
}
