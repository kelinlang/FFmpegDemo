#include "FFmpegRemuxing.h"
RemuxingContext remuxingContext;

static void freeInputFile(InputFile* inputfile) {
	if (inputfile != NULL) {
		avformat_close_input(inputfile->formatContext);
		if (inputfile->fileName != NULL) {
			av_free(inputfile->fileName);
		}
		av_free(inputfile);
	}
}

static InputFile* allocInputFile() {
	InputFile*  inputfile = av_mallocz(sizeof(InputFile));
	return inputfile;
}


static int openInputFile(InputFile* inputfile,int index,char* fileName) {
	int ret = -1;
	AVFormatContext* fc = NULL;
	
	if ((ret = avformat_open_input(&fc, fileName, NULL, NULL)) < 0)
	{
		printf("Error: Open input file failed.\n");
		return ret;
	}

	//获取输入视频文件中的流信息
	if ((ret = avformat_find_stream_info(fc, NULL)) >= 0)
	{
		printf("Error: Failed to retrieve input stream information.\n");
		return ret;
	}
	av_dump_format(fc, 0, fileName, 0);

	inputfile->formatContext = fc;
	inputfile->index = index;
	inputfile->numStream = fc->nb_streams;
	strcpy(inputfile->fileName, fileName);

	printf("%s stream is %d.\n",fileName, inputfile->numStream);
	return ret;
}

static void freeOutputFile(OutputFile* outputfile) {
	if (outputfile != NULL) {
		avformat_close_input(outputfile->formatContext);
		if (outputfile->fileName != NULL) {
			av_free(outputfile->fileName);
		}
		av_free(outputfile);
	}
}

static OutputFile* allocOutputFile() {
	OutputFile* outputfile = av_mallocz(sizeof(OutputFile));
	return outputfile;
}

static int openOutputFile(RemuxingContext* remuxingContext, OutputFile* outputfile, int index, char* fileName, char* format) {
	AVFormatContext* fc = NULL;
	int ret = -1;
	//按照文件名获取输出文件的句柄
	avformat_alloc_output_context2(&fc, NULL, format, fileName);
	if (!fc)
	{
		printf("Error: Could not create output context.\n");
		return ret;
	}
	AVOutputFormat* ofmt = NULL;
	outputfile->formatContext = fc;
	outputfile->ofmt = fc->oformat;
	strcpy(outputfile->fileName, fileName);

	int curStNum = 0;
	for (int i = 0; i < remuxingContext->nbInputFiles;i++) {
		InputFile* inputFile = remuxingContext->inputFiles[i];

		AVProgram* program = av_new_program(fc, i+1);
		av_dict_set(&program->metadata, "title", inputFile->fileName, 0);

		for (int j = 0; j < inputFile->numStream; j++) {
			av_program_add_stream_index(fc, i + 1, curStNum++);//设置输出流id

			AVStream* inStream = inputFile->formatContext->streams[i];

			AVCodecParameters* avCodecParameters = inStream->codecpar;
			AVCodecContext* inAVCodecContext = avcodec_alloc_context3(NULL);
			if (!inAVCodecContext) {
				printf("alloc avcodec fail.");
				return ret;
			}
			ret = avcodec_parameters_to_context(inAVCodecContext, avCodecParameters);
			if (ret < 0) {
				printf("avcodec_parameters_to_context fail.");
				return ret;
			}

			AVStream* outStream = avformat_new_stream(fc, inAVCodecContext->codec);
			if (!outStream)
			{
				printf("Error: Could not allocate output stream.\n");
				return ret;
			}

			AVCodecContext* outAVCodecContext = avcodec_alloc_context3(NULL);
			if (!outAVCodecContext) {
				printf("alloc out avcodec fail.");
				return ret;
			}
			ret = avcodec_parameters_to_context(outAVCodecContext, avCodecParameters);
			if (ret < 0) {
				printf("out avcodec_parameters_to_context fail.");
				return ret;
			}

			ret = avcodec_copy_context(outAVCodecContext, inAVCodecContext);
			outAVCodecContext->codec_tag = 0;
			if (fc->oformat->flags & AVFMT_GLOBALHEADER)
			{
				outAVCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
			}

			ret = avcodec_parameters_from_context(outStream->codecpar, outAVCodecContext);
			if (ret < 0) {
				printf("avcodec_parameters_from_context fail.");
				return ret;
			}

			avcodec_free_context(&inAVCodecContext);
			avcodec_free_context(&outAVCodecContext);
		}

	}

	av_dump_format(fc, 0, fileName, 1);

	if (!(ofmt->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&fc->pb, fileName, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			printf("Error: Could not open output file.\n");
			return ret;
		}
	}

	return ret;
}


void testRemuxing(int numInputFile,char** inputFileName, char* outputFileName)
{
	RemuxingContext remuxingContext;
	RemuxingContext* pRemuxingContext = &remuxingContext;
	memset(&remuxingContext, 0, sizeof(struct RemuxingContext));
	remuxingContext.nbInputFiles = numInputFile;

	for (int i = 0; i < numInputFile;i++) {
		GROW_ARRAY(remuxingContext.inputFiles, remuxingContext.nbInputFiles);
		InputFile* inputFile = allocInputFile();
		if (openInputFile(inputFile,i, inputFileName[i]) >= 0) {
			remuxingContext.inputFiles[remuxingContext.nbInputFiles - 1] = inputFile;
			pRemuxingContext->totalStreams += inputFile->numStream;
		}else {
			freeInputFile(inputFile);
		}
	}

}
