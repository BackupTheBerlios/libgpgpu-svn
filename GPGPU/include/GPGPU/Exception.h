/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#ifndef GPGPU_EXCEPTION
#define GPGPU_EXCEPTION
#include "Prerequisites.h"
#include <string>

#define GPGPU_EXCEPT(code, desc) throw( GPGPU::Exception( code, desc, __FUNCTION__, __FILE__, __LINE__ ) )

/// Assert always
#define GPGPU_ASSERT(a, b) if( !(a) ) GPGPU_EXCEPT( GPGPU::ERR_ASSERTION, (b) )

/// Assert only in non-debug mode
#ifdef NDEBUG
#define GPGPU_ASSERT_DEBUG(a, b)
#else
#define GPGPU_ASSERT_DEBUG(a, b) if( !(a) ) GPGPU_EXCEPT( GPGPU::ERR_ASSERTION, (b) )
#endif

namespace GPGPU
{

enum {
	ERR_UNKNOWN = 0,
	ERR_INVALIDPARAMS = 1,		// Invalid parameter value
    ERR_RENDERSYSTEM = 2,       // Rendersystem error
	ERR_ASSERTION = 3,			// Assertion failed
    ERR_RESOURCEDEPLETED = 4,   // Resource depleted
    ERR_INTERNALERROR = 5    // Resource not found
};

class Exception
{
private:
	uint code;
	std::string description, source, file;
	uint line;	
public:
	Exception(uint code, const std::string &desc, const std::string &src,
		const std::string &file, uint line);
	~Exception();
	
	uint getCode() const { return code; }
	uint getLine() const { return line; }
	std::string getDescription() const { return description; }
	std::string getSource() const { return source; }
	std::string getFile() const { return file; }
	
	std::string getFullDescription();
};

};
#endif
