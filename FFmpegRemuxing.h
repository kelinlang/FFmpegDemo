#ifndef FFMPEG_REMUXING
#define FFMPEG_REMUXING

#include "libavutil/timestamp.h"
#include "libavformat/avformat.h"
#include "libavutil/mathematics.h"


typedef struct InputFile {
	char* fileName;
	AVFormatContext* formatContext;
}InputFile;


typedef struct OutputFile {
	char* fileName;
	AVFormatContext* formatContext;
	AVDictionary* opts;
}OutputFile;

typedef struct RemuxingContext {
	InputFile** inputFiles;
	int        nbInputFiles;

	OutputFile* outputFile;
}RemuxingContext;



void testRemuxing(int numInputFile, char** inputFileName, char* outputFileName);



#endif