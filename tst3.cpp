// Test texture pool
#include <GPGPU/Engine.h>
#include <GPGPU/Exception.h>
#include <GPGPU/Texture.h>
#include <GPGPU/TexturePool.h>
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
        temp->setSize(256, 256);
        temp->setInternalFormat(GPGPU::PC_RGBA, GPGPU::PT_FLOAT16);
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
        diffuse->setEntryPoint("diffuse");
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
        /// Run the loop
        Timer t;
        t.start();
        for(size_t i=0; i<100; i++)
        {
            GPGPU::TextureLease y = pool.getLease();
            diffuse->setParameter("input", x.get());
            engine->setTarget(y.get());
            engine->setProgram(diffuse);
            engine->quadBlit();
            x = y;
        }
        engine->setTarget();
        dummyLoadTexture(x.get());
        t.stop();
        std::cerr << "Time elapsed: " << t.getSeconds() << std::endl;
        
        
        //printTexture(x.get());
    } catch(GPGPU::Exception &e) {
        std::cerr << e.getDescription() << std::endl;
    }
    
	return 0;
}
