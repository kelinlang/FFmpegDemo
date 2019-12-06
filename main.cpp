#include <cstdio>
#define __STDC_CONSTANT_MACROS
extern "C" {
//#include "libavutil/timestamp.h"
//#include "libavformat/avformat.h"
#include "VideoTransCode.h"
#include "FFmpegRemuxing.h"
}



int main()
{
    printf("hello from FFmpegDemo!\n");
	//av_register_all();
	//avformat_network_init();

	//IOFiles file;
	//file.inputName = "/home/kelinlang/workspace/testFiles/E1.mp4";
	//file.outputName = "/home/kelinlang/workspace/testFiles/E1.ts";
	//////file.outputName = "udp://192.168.2.34:1234?pkt_size=1316";

	//doTransCode(&file);


	int inputsNum = 2;
	char* inputs[] = { "/home/kelinlang/workspace/testFiles/E1.mp4" ,"/home/kelinlang/workspace/testFiles/E1.mp4"};
	char* output = "/home/kelinlang/workspace/testFiles/remux.ts";
	testRemuxing(inputsNum,inputs,output);
	printf("hello from FFmpegDemo  finish!\n");

    return 0;
}