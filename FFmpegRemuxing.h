#ifndef FFMPEG_REMUXING
#define FFMPEG_REMUXING

#include "libavutil/timestamp.h"
#include "libavformat/avformat.h"
#include "libavutil/mathematics.h"



typedef struct InputFile {
	char* fileName;
	AVFormatContext* formatContext;
	int index;
	int numStream;
}InputFile;


typedef struct OutputFile {
	char* fileName;
	AVFormatContext* formatContext;
	AVOutputFormat* ofmt;
	AVDictionary* opts;
}OutputFile;

typedef struct RemuxingContext {
	InputFile** inputFiles;
	int        nbInputFiles;

	OutputFile* outputFile;

	int totalStreams;
} RemuxingContext;



void testRemuxing(int numInputFile, char** inputFileName, char* outputFileName);



#endif