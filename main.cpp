#include <cstdio>
#define __STDC_CONSTANT_MACROS
extern "C" {
//#include "libavutil/timestamp.h"
//#include "libavformat/avformat.h"
#include "VideoTransCode.h"
//#include "FFmpegRemuxing.h" //ffmepg ���°汾��֤����
#include "FFmpegRemuxing3.3.h" //ffmepg 3.3.6�汾��֤����
}



int main()
{
    printf("hello from FFmpegDemo!\n");

	//IOFiles file;
	//file.inputName = "/home/kelinlang/workspace/testFiles/E1.mp4";
	//file.outputName = "/home/kelinlang/workspace/testFiles/E1.ts";
	//////file.outputName = "udp://192.168.2.34:1234?pkt_size=1316";

	//doTransCode(&file);



	//���Զ����Ƶ�������ts��
	//int inputsNum = 2;
	//char* inputs[] = { "/home/kelinlang/workspace/testFiles/E1.mp4" ,"/home/kelinlang/workspace/testFiles/Record-null.ts" };
	//char* output = "/home/kelinlang/workspace/testFiles/remux.ts";
	//output = "udp://192.168.2.34:1234?pkt_size=1316";
	//char* outFormat = "mpegts";


	int inputsNum = 1;
	//����rtmp��h265��Ƶ��
	char* inputs[] = { "/home/kelinlang/workspace/testFiles/E1h265.mp4"};
	char* output = "rtmp://192.168.0.18:1935/live/test001";
	char* outFormat = "flv";

	//����rtsp��h265��Ƶ��
	/*output = "rtsp://192.168.2.162:5555/live/test";
	outFormat = "rtsp";*/

	//��������ts�ļ���udp��ts��
	//output = "udp://192.168.2.175:1234?pkt_size=1316";
	//output = "/home/kelinlang/workspace/testFiles/E1h265_file_1.ts";
	//outFormat = "mpegts";

	//����ts��Ƭ
	/*output = "/home/kelinlang/workspace/testFiles/E1h265_hls_code_test.m3u8";
	outFormat = "hls";*/

	//��������h265��ʽflv�ļ�
	//char* output = "/home/kelinlang/workspace/testFiles/E1h265.flv";
	//char* outFormat = "flv";

	//testRemuxing(inputsNum,inputs,output, outFormat);//�������°汾
	testRemuxing3(inputsNum,inputs,output, outFormat);//����3.3.6�汾
	printf("hello from FFmpegDemo  finish!\n");

    return 0;
}