

#ifndef _VIDEO_REMUXING_HEADER_H_
#define _VIDEO_REMUXING_HEADER_H_

//#define snprintf(buf,len, format,...) _snprintf_s(buf, len,len, format, __VA_ARGS__)


#include "libavutil/timestamp.h"
#include "libavformat/avformat.h"
#include "libavutil/mathematics.h"



typedef struct IOFiles
{
	const char* inputName;
	const char* outputName;
}IOFiles;
void doTransCode(IOFiles* IOFiles);


#endif


