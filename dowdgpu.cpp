// GPGPU implementation of "The balanced sorting network" by Dowd et al

#include <GPGPU/Engine.h>
#include <GPGPU/Exception.h>
#include <GPGPU/Texture.h>
#include <GPGPU/TexturePool.h>
#include <GPGPU/Util.h>
#include <iostream>
#include <fstream>
#include "GenUtil.h"
#include "DowdSortingNetwork.h"

void printTexture(GPGPU::TexturePtr tex)
{
    GPGPU::Texture::Info ti;
    tex->getInfo(ti);
    float *data2 = new float[ti.width*ti.height*4];
    tex->storeData(GPGPU::PC_R, GPGPU::PT_FLOAT32, data2);
    for(size_t y=0; y<ti.height; y++)
    {
        for(size_t x=0; x<ti.width; x++)
            std::cerr << data2[y*ti.width+x] << " ";
        std::cerr << std::endl;
    }
    delete [] data2;
}

void printTextureRow(GPGPU::TexturePtr tex)
{
    GPGPU::Texture::Info ti;
    tex->getInfo(ti);
    float *data2 = new float[ti.width*ti.height*4];
    
    tex->storeData(GPGPU::PC_RGBA, GPGPU::PT_FLOAT32, data2);
    for(size_t y=0; y<ti.height; y++)
    {
        for(size_t x=0; x<ti.width; x++)
            std::cerr << data2[(y*ti.width+x)*4+0] << " ";
        std::cerr << std::endl;
    }
    
    delete [] data2;
    
}
void dummyLoadTexture(GPGPU::TexturePtr tex)
{
    GPGPU::Texture::Info ti;
    tex->getInfo(ti);
    float *data2 = new float[ti.width*ti.height*4];
    tex->storeData(GPGPU::PC_RGBA, GPGPU::PT_FLOAT32, data2);
    delete [] data2;
}

void fillTestData(GPGPU::TexturePtr tex)
{
    GPGPU::Texture::Info ti;
    tex->getInfo(ti);
    float *data2 = new float[ti.width*ti.height*4];
    
    for(size_t y=0; y<ti.height; y++)
    {
        for(size_t x=0; x<ti.width; x++)
        {
            /// Sort key
            //data2[(y*ti.width+x)*4 + 0] = (float)rand()/RAND_MAX;
			data2[(y*ti.width+x)*4 + 0] = (float)rand()/RAND_MAX;
            /// Some id data
            data2[(y*ti.width+x)*4 + 1] = x;
            data2[(y*ti.width+x)*4 + 2] = y;
        }
    }
    
    tex->loadData(GPGPU::PC_RGBA, GPGPU::PT_FLOAT32, data2);
    delete [] data2;
}

void validateSorted(GPGPU::TexturePtr tex)
{
    GPGPU::Texture::Info ti;
    tex->getInfo(ti);
    size_t dsize = ti.width*ti.height;
    float *data2 = new float[ti.width*ti.height*4];
    
    tex->storeData(GPGPU::PC_R, GPGPU::PT_FLOAT32, data2);
    int errors = 0;
	float total_error = 0;
    for(size_t y=1; y<dsize; y++)
    {
        if(data2[y]<data2[y-1])
        {
            //std::cerr << "Error in sort! " << data2[y] << " comes after " << data2[y+1] << std::endl;
			total_error += data2[y-1]-data2[y];
            errors ++;
        }
    }
    if(!errors)
    {
        std::cerr << "Sort completed perfectly" << std::endl;
    }
    else
    {
        std::cerr << "There were " << errors << " sequence errors, totalling in " << total_error << std::endl;
    }
    delete data2;
}


int main(int argc, char **argv)
{
    if(argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <width> <height> [Half|Float]" << std::endl;
        return 1;
    }
    size_t width = atoi(argv[1]);
    size_t height = atoi(argv[2]);
    if(width < 1 || height < 1) 
    {
        std::cerr << "Invalid texture size specifiec" << std::endl;
        return 1;
    }
        
    std::string precision = "Half";
    if(argc > 3)
        precision = argv[3];
    GPGPU::EnginePtr engine;
    try
    {
        engine =  GPGPU::create();
        engine->initialise();
    } catch(GPGPU::Exception &e) {
        std::cerr << e.getDescription() << std::endl;
        return 1;
    }
    try
    {
        GPGPU::HWInfo inf;
        engine->getHWInfo(inf);

        std::cerr << GPGPU::getDescription(inf) << std::endl;
        std::cerr << "Initialising the pool" << std::endl;
        GPGPU::TexturePool pool;
        pool.setCapacity(3);
        /// Create template
        GPGPU::TexturePtr temp = engine->createTexture();
        temp->setSize(width, height);
		if(precision=="Float")
				temp->setInternalFormat(GPGPU::PC_RGBA, GPGPU::PT_FLOAT32);
        else	temp->setInternalFormat(GPGPU::PC_RGBA, GPGPU::PT_FLOAT16);
        temp->setRenderTarget(true);
        pool.setTemplate(temp);
        
        pool.initialise();

        /// Load test data
        std::cerr << "Loading test data" << std::endl;
        GPGPU::TextureLease cur = pool.getLease();
        fillTestData(cur.get());
        
        /// Create sorting network
        DowdSortingNetwork *sorter = new DowdSortingNetwork();
        sorter->setSize(width, height);
        sorter->setPrecision(precision);
        sorter->initialize();
        
        /// Iterate
        std::cerr << "Iteration" << std::endl;
        Timer t;
        t.start();
        
        sorter->sort(&pool, cur);   
        
        engine->setTarget(); // Unbind target
        dummyLoadTexture(cur.get());
        t.stop();
        delete sorter;
        std::cerr << "Time elapsed: " << t.getSeconds() << std::endl;
        validateSorted(cur.get());
        //printTextureRow(cur.get());
    } catch(GPGPU::Exception &e) {
        std::cerr << "Exception: " << e.getDescription() << std::endl;
    }
    
	return 0;
}
