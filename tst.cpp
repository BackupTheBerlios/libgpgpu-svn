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
        GPGPU::TexturePtr tex = engine->createTexture();
        tex->setSize(16, 16);
        //tex->setType(GL_TEXTURE_2D);
        tex->setInternalFormat(GPGPU::PC_R, GPGPU::PT_FLOAT16);
        tex->setRenderTarget(true);
        tex->initialise();
        std::cerr << "Texture created succesfully" << std::endl;
        
        float data[256];
        for(size_t x=0; x<256; x++)
            data[x] = x;
        tex->loadData(GPGPU::PC_R, GPGPU::PT_FLOAT32, data);
        std::cerr << "Data uploaded succesfully" << std::endl;
        
        float data2[256];
        for(size_t x=0; x<256; x++)
            data2[x] = 0.0f;
        tex->storeData(GPGPU::PC_R, GPGPU::PT_FLOAT32, data2);
        std::cerr << "Data downloaded succesfully" << std::endl;
        
        for(size_t x=0; x<256; x++)
            std::cerr << data2[x] << " ";
        std::cerr << std::endl;
        
        
        std::string programText = loadText("shaders/Test.cg");
        if(programText.empty())
        {
            std::cerr << "Test.cg could not be loaded" << std::endl;
            return 1;
        }
        GPGPU::ProgramPtr coord = engine->createProgram("Cg");
        coord->setSource(programText);
        coord->setEntryPoint("coord");
        coord->initialise();
        
        GPGPU::ProgramPtr fill = engine->createProgram("Cg");
        fill->setSource(programText);
        fill->setEntryPoint("fill");
        fill->initialise();
        fill->setParameter("tintColour", 0.0f, 0.0f, 0.0f, 0.0f);
        
        GPGPU::ProgramPtr add = engine->createProgram("Cg");
        add->setSource(programText);
        add->setEntryPoint("add");
        add->initialise();

        
        std::cerr << "Setting target and program" << std::endl;
        engine->setTarget(tex);
        
        engine->setProgram(fill);
        engine->quadBlit();
        
        engine->setProgram(coord);
        std::cerr << "Setting parameters" << std::endl;
        
        std::cerr << "Blitting" << std::endl;
        GPGPU::Rect tgt(4,8,12,12);
        engine->quadBlit(&tgt);
        
        engine->setTarget(); // Untarget
        
        printTexture(tex);
        /// Create two more textures
        std::cerr << "Create two more textures" << std::endl;
        GPGPU::TexturePtr tex2 = engine->createTexture();
        tex2->setSize(16, 16);
        tex2->setInternalFormat(GPGPU::PC_R, GPGPU::PT_FLOAT16);
        tex2->setRenderTarget(true);
        tex2->initialise();
        GPGPU::TexturePtr tex3 = engine->createTexture();
        tex3->setSize(16, 16);
        tex3->setInternalFormat(GPGPU::PC_R, GPGPU::PT_FLOAT16);
        tex3->setRenderTarget(true);
        tex3->initialise();
        
        /// Fill texture2 with fun
        std::cerr << "Fill texture2 with fun" << std::endl;
        tex2->loadData(GPGPU::PC_R, GPGPU::PT_FLOAT32, data2);
        
        /// Try add operation
        std::cerr << "Add operation" << std::endl;
        engine->setTarget(tex3);
        engine->setProgram(add);
        add->setParameter("a", tex);
        add->setParameter("b", tex2);
        
        engine->quadBlit();
        
        engine->setTarget(); // Untarget
        
         printTexture(tex3);
    } catch(GPGPU::Exception &e) {
        std::cerr << e.getDescription() << std::endl;
    }
    
	return 0;
}
