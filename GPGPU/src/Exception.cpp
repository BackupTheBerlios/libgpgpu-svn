/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#include <GPGPU/Exception.h>
#include <sstream>
namespace GPGPU
{

Exception::Exception(uint code, const std::string &desc, const std::string &src,
		const std::string &file, uint line):
	code(code), description(desc), source(src), file(file), line(line)
{
}
Exception::~Exception()
{
}

std::string errString(uint code)
{
	switch(code)
	{
		case ERR_UNKNOWN: return "UNKNOWN";
		case ERR_INVALIDPARAMS: return "INVALIDPARAMS";
		case ERR_RENDERSYSTEM: return "RENDERSYSTEM";
		case ERR_ASSERTION: return "ASSERTION";
        case ERR_RESOURCEDEPLETED: return "RESOURCEDEPLETED";
        case ERR_INTERNALERROR: return "INTERNALERROR";
	}
    return "UNKNOWN";
}

std::string Exception::getFullDescription()
{
	std::stringstream ss;
	ss << "GPGPU Exception " << errString(code) << " at " << file << ":" << line << "(" << source << "): " <<
		description;
	return ss.str();
}

};
