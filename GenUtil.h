#ifndef H_GENUTIL
#define H_GENUTIL

std::string loadText(const std::string &filename);
#include <sys/time.h>
struct Timer
{
    struct timeval starttime;
    struct timeval stoptime;
	void start()
	{
		gettimeofday(&starttime, NULL);
	}
    void stop()
    {
        gettimeofday(&stoptime, NULL);
    }

	unsigned long getMilliseconds()
	{
	    return (stoptime.tv_sec-starttime.tv_sec)*1000+(stoptime.tv_usec-starttime.tv_usec)/1000;
	}

	unsigned long getMicroseconds()
	{
	    return (stoptime.tv_sec-starttime.tv_sec)*1000000+(stoptime.tv_usec-starttime.tv_usec);
	}
    
    float getSeconds()
    {
	    return (float)(stoptime.tv_sec-starttime.tv_sec)+(stoptime.tv_usec-starttime.tv_usec)/1000000.0f;
    }
};

#endif
