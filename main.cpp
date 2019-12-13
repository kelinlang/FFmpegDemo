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


	//int inputsNum = 2;
	//char* inputs[] = { "/home/kelinlang/workspace/testFiles/E1.mp4" ,"/home/kelinlang/workspace/testFiles/Record-null.ts"};
	//char* output = "/home/kelinlang/workspace/testFiles/remux.ts";
	//output = "udp://192.168.2.34:1234?pkt_size=1316";

	//int inputsNum = 2;
	//char* inputs[] = { "/home/kelinlang/workspace/testFiles/E1.mp4" ,"/home/kelinlang/workspace/testFiles/Record-null.ts" };
	//char* output = "/home/kelinlang/workspace/testFiles/remux.ts";
	//output = "udp://192.168.2.34:1234?pkt_size=1316";
	//char* outFormat = "mpegts";

	/*int inputsNum = 1;
	char* inputs[] = { "/home/kelinlang/workspace/testFiles/E1.mp4"};
	char* output = "/home/kelinlang/workspace/testFiles/remux.ts";
	output = "udp://192.168.2.34:1234?pkt_size=1316";*/

	int inputsNum = 1;
	//char* inputs[] = { "/home/kelinlang/workspace/testFiles/E1.mp4"};
	char* inputs[] = { "/home/kelinlang/workspace/testFiles/E1h265.mp4"};
	char* output = "rtmp://192.168.2.162:1935/live/test";
	char* outFormat = "flv";

	testRemuxing(inputsNum,inputs,output, outFormat);
	printf("hello from FFmpegDemo  finish!\n");

    return 0;
}