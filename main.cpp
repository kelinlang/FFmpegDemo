#include <cstdio>
#define __STDC_CONSTANT_MACROS
extern "C" {
//#include "libavutil/timestamp.h"
//#include "libavformat/avformat.h"
#include "VideoTransCode.h"
//#include "FFmpegRemuxing.h" //ffmepg 最新版本验证代码
#include "FFmpegRemuxing3.3.h" //ffmepg 3.3.6版本验证代码
}



int main()
{
    printf("hello from FFmpegDemo!\n");

	//IOFiles file;
	//file.inputName = "/home/kelinlang/workspace/testFiles/E1.mp4";
	//file.outputName = "/home/kelinlang/workspace/testFiles/E1.ts";
	//////file.outputName = "udp://192.168.2.34:1234?pkt_size=1316";

	//doTransCode(&file);



	//测试多个视频流输出到ts流
	//int inputsNum = 2;
	//char* inputs[] = { "/home/kelinlang/workspace/testFiles/E1.mp4" ,"/home/kelinlang/workspace/testFiles/Record-null.ts" };
	//char* output = "/home/kelinlang/workspace/testFiles/remux.ts";
	//output = "udp://192.168.2.34:1234?pkt_size=1316";
	//char* outFormat = "mpegts";


	int inputsNum = 1;
	//测试rtmp推h265视频流
	char* inputs[] = { "/home/kelinlang/workspace/testFiles/E1h265.mp4"};
	char* output = "rtmp://192.168.0.18:1935/live/test001";
	char* outFormat = "flv";

	//测试rtsp推h265视频流
	/*output = "rtsp://192.168.2.162:5555/live/test";
	outFormat = "rtsp";*/

	//测试生成ts文件和udp推ts流
	//output = "udp://192.168.2.175:1234?pkt_size=1316";
	//output = "/home/kelinlang/workspace/testFiles/E1h265_file_1.ts";
	//outFormat = "mpegts";

	//测试ts切片
	/*output = "/home/kelinlang/workspace/testFiles/E1h265_hls_code_test.m3u8";
	outFormat = "hls";*/

	//测试生成h265格式flv文件
	//char* output = "/home/kelinlang/workspace/testFiles/E1h265.flv";
	//char* outFormat = "flv";

	//testRemuxing(inputsNum,inputs,output, outFormat);//测试最新版本
	testRemuxing3(inputsNum,inputs,output, outFormat);//测试3.3.6版本
	printf("hello from FFmpegDemo  finish!\n");

    return 0;
}