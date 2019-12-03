#include <cstdio>
#define __STDC_CONSTANT_MACROS
extern "C" {
#include "libavutil/timestamp.h"
#include "libavformat/avformat.h"
//#include "VideoTransCode.h"

}



int main()
{
    printf("hello from FFmpegDemo!\n");
	av_register_all();
	//avformat_network_init();

	/*IOFiles file = {
		"/home/kelinlang/workspace/testFiles/E1.mp4",
		"/home/kelinlang/workspace/testFiles/E1.ts"
	};

	doTransCode(&file);*/


	printf("hello from FFmpegDemo  finish!\n");

    return 0;
}