/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#include <GPGPU/Util.h>
#include <GPGPU/BaseTypes.h>
#include <sstream>
namespace GPGPU {

    
    std::string getDescription(HWInfo &info)
    {
        std::stringstream ss;
        ss << "libGPGPU hardware description" << std::endl;
        ss << "  Model                      = " << info.model << std::endl;
        ss << "  Number of texture units    = " << info.textureUnits << std::endl;
        ss << "  Floating point textures    = " << (info.mFloatTextures?"yes":"no") << std::endl;
        ss << "    Non GL_TEXTURE_RECTANGLE = " << (info.mFullFloatTextures?"yes":"no") << std::endl;
        ss << "  Supported languages        = ";
        for(std::vector<std::string>::iterator i=info.languages.begin(); i!=info.languages.end(); i++)
            ss << *i << " ";
        ss << std::endl;
        ss << "  Best VP profile            = " << info.cgBestVP << std::endl;
        ss << "  Best FP profile            = " << info.cgBestFP << std::endl;
        return ss.str();
    }

};
