// Test texture pool
#include <GPGPU/Engine.h>
#include <GPGPU/Exception.h>
#include <GPGPU/Texture.h>
#include <GPGPU/TexturePool.h>
#include <iostream>
#include <fstream>
#include "GenUtil.h"
#include "DowdSortingNetwork.h"
void printTexture(GPGPU::TexturePtr tex, size_t channel)
{
    GPGPU::Texture::Info ti;
    tex->getInfo(ti);
    float data2[ti.width*ti.height*4];
    tex->storeData(GPGPU::PC_RGBA, GPGPU::PT_FLOAT32, data2);
    for(size_t y=0; y<ti.height; y++)
    {
        for(size_t x=0; x<ti.width; x++)
            std::cerr << data2[(y*ti.width+x)*4+channel] << " ";
        std::cerr << std::endl;
    }
    
}
void printTextureWX(GPGPU::TexturePtr tex)
{
    GPGPU::Texture::Info ti;
    tex->getInfo(ti);
    float data2[ti.width*ti.height*4];
    tex->storeData(GPGPU::PC_RGBA, GPGPU::PT_FLOAT32, data2);
    for(size_t y=0; y<ti.height; y++)
    {
        for(size_t x=0; x<ti.width; x++)
            std::cerr << data2[(y*ti.width+x)*4+0] << ","
                      << data2[(y*ti.width+x)*4+3] << " ";
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
            /// Distribute particles randomly
			data2[(y*ti.width+x)*4 + 0] = 16*(float)rand()/RAND_MAX;
            data2[(y*ti.width+x)*4 + 1] = 16*(float)rand()/RAND_MAX;
            data2[(y*ti.width+x)*4 + 2] = 16*(float)rand()/RAND_MAX;
        }
    }
    
    tex->loadData(GPGPU::PC_RGBA, GPGPU::PT_FLOAT32, data2);
    delete [] data2;
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
        pool.setCapacity(5);
        /// Create template
        GPGPU::TexturePtr temp = engine->createTexture();

        const size_t cellsx = 32;
        const size_t cellsy = 32;
        const size_t cellsz = 32;
        const size_t logwidth = 4;
        const size_t logheight = 4;
        const size_t height = 1<<logwidth;
        const size_t width = 1<<logheight;
        const float cellsizex = 4;
        const float cellsizey = 4;
        const float cellsizez = 4;
        temp->setSize(height, width);
        temp->setInternalFormat(GPGPU::PC_RGBA, GPGPU::PT_FLOAT32);
        temp->setRenderTarget(true);
        pool.setTemplate(temp);
        
        pool.initialise();
        
        /// Load a program
        std::string programText = loadText("shaders/Particle.cg");
        if(programText.empty())
        {
            std::cerr << "Particle.cg could not be loaded" << std::endl;
            return 1;
        }
        GPGPU::ProgramPtr posToCell = engine->createProgram("Cg");
        posToCell->setSource(programText);
        posToCell->setEntryPoint("posToCell");
        posToCell->initialise();
        
        GPGPU::ProgramPtr initial = engine->createProgram("Cg");
        initial->setSource(programText);
        initial->setEntryPoint("initial");
        initial->initialise();
             
        GPGPU::ProgramPtr searchV = engine->createProgram("Cg");
        searchV->setSource(programText);
        searchV->setEntryPoint("searchVertical");
        searchV->initialise();
        
        GPGPU::ProgramPtr initH = engine->createProgram("Cg");
        initH->setSource(programText);
        initH->setEntryPoint("initialHorizontal");
        initH->initialise();
           
        GPGPU::ProgramPtr searchH = engine->createProgram("Cg");
        searchH->setSource(programText);
        searchH->setEntryPoint("searchHorizontal");
        searchH->initialise();
        
        GPGPU::ProgramPtr testLookup = engine->createProgram("Cg");
        testLookup->setSource(programText);
        testLookup->setEntryPoint("testLookup");
        testLookup->initialise();
        
        /// Create sorting network
        DowdSortingNetwork *sorter = new DowdSortingNetwork();
        sorter->setSize(width, height);
        sorter->setPrecision("Float");
        sorter->initialize();
        
        /// Get a texture
        GPGPU::TextureLease positions; /// Here are the values to look up
        positions = pool.getLease();
        fillTestData(positions.get());
        std::cerr << "Positions are: " << std::endl;
        printTexture(positions.get(), 0);
        std::cerr << std::endl;
        printTexture(positions.get(), 1);
        std::cerr << std::endl;
        printTexture(positions.get(), 2);
        
        std::cerr << "Cell IDS (search space) are: " << std::endl;
        
        /// Positions to cellids
        GPGPU::TextureLease cellids = pool.getLease();
        engine->setProgram(posToCell);
        posToCell->setParameter("positions", positions.get());
        posToCell->setParameter("stagger", 0,0,0);
        posToCell->setParameter("cellmult", 1,cellsx,cellsx*cellsy);
        posToCell->setParameter("cellsize", cellsizex,cellsizey,cellsizez);
        engine->setTarget(cellids.get());
        engine->quadBlit();
        
        /// Define search space by sorting cellids
        GPGPU::TextureLease searchspace = cellids;
        sorter->sort(&pool, searchspace);
        engine->setTarget();
        printTexture(searchspace.get(), 0);
        
        /// Create another texture (needle)
        GPGPU::TextureLease x;
        x = pool.getLease();      /// Working texture
        
        initial->setParameter("height", height);
        initial->setParameter("cellids", cellids.get());
        engine->setTarget(x.get());
        engine->setProgram(initial);
        engine->quadBlit();
        
        engine->setTarget();
        std::cerr << "Needle space is: " << std::endl;
        printTexture(x.get(), 2);
        
        /// Run the loop
        Timer t;
        t.start();
        
        // Sort vertical
        engine->setProgram(searchV);
        searchV->setParameter("haystack", searchspace.get());
        for(size_t i=0; i<logwidth; i++)
        {
            GPGPU::TextureLease y = pool.getLease();
            searchV->setParameter("needle", x.get());
            engine->setTarget(y.get());
            engine->quadBlit();
            x = y;
        }
        // Initialize horizontal sorting
        {
            GPGPU::TextureLease y = pool.getLease();
            engine->setProgram(initH);
            initH->setParameter("width", width);
            initH->setParameter("needle", x.get());
            engine->setTarget(y.get());
            engine->quadBlit();
            x = y;
        }
        // Sort horizontal
        engine->setProgram(searchH);
        searchH->setParameter("haystack", searchspace.get());
        for(size_t i=0; i<logheight; i++)
        {
            GPGPU::TextureLease y = pool.getLease();
            searchH->setParameter("needle", x.get());
            engine->setTarget(y.get());
            engine->quadBlit();
            x = y;
        }        
        
        engine->setTarget();
        dummyLoadTexture(x.get());
        t.stop();
        std::cerr << "Time elapsed: " << t.getSeconds() << std::endl;
        
        // x.get now has x components in [0] and y components in [3] of lastmost particle in this cell
        // all other particles in this cell well appear before this position
        printTextureWX(x.get());
        
        // Test lookup
        {
            GPGPU::TextureLease y = pool.getLease();
            engine->setProgram(testLookup);
            testLookup->setParameter("width", width);
            testLookup->setParameter("haystack", searchspace.get());
            testLookup->setParameter("needle", x.get());
            testLookup->setParameter("positions", positions.get());
            engine->setTarget(y.get());
            engine->quadBlit();
            
            engine->setTarget();
            printTexture(y.get(), 0);
        }
    } catch(GPGPU::Exception &e) {
        std::cerr << e.getDescription() << std::endl;
    }
    
	return 0;
}
