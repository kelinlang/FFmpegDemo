#include "FFmpegRemuxing3.3.h"

#define GROW_ARRAY(array, nb_elems)\
    array = grow_array(array, sizeof(*array), &nb_elems, nb_elems + 1)

void* grow_array(void* array, int elem_size, int* size, int new_size)
{
	if (new_size >= INT_MAX / elem_size) {
		av_log(NULL, AV_LOG_ERROR, "Array too big.\n");
		//exit_program(1);
	}
	if (*size < new_size) {
		uint8_t* tmp = av_realloc_array(array, new_size, elem_size);
		if (!tmp) {
			av_log(NULL, AV_LOG_ERROR, "Could not alloc buffer.\n");
			//exit_program(1);
		}
		memset(tmp + *size * elem_size, 0, (new_size - *size) * elem_size);
		*size = new_size;
		return tmp;
	}
	return array;
}

RemuxingContext remuxingContext;

static void freeInputFile(InputFile* inputfile) {
	if (inputfile != NULL) {
		avformat_close_input(&inputfile->formatContext);
		if (inputfile->fileName != NULL) {
			av_free(inputfile->fileName);
		}
		av_free(inputfile);
	}
}

static InputFile* allocInputFile() {
	InputFile* inputfile = av_mallocz(sizeof(InputFile));
	return inputfile;
}


static int openInputFile(RemuxingContext* remuxingContext, InputFile* inputfile, int index, char* fileName) {
	int ret = -1;
	AVFormatContext* fc = NULL;

	if ((ret = avformat_open_input(&fc, fileName, NULL, NULL)) < 0)
	{
		printf("Error: Open input file failed.\n");
		return ret;
	}

	//��ȡ������Ƶ�ļ��е�����Ϣ
	if ((ret = avformat_find_stream_info(fc, NULL)) < 0)
	{
		printf("Error: Failed to retrieve input stream information.\n");
		return ret;
	}
	av_dump_format(fc, index, fileName, 0);

	inputfile->formatContext = fc;
	inputfile->index = index;
	inputfile->numStream = fc->nb_streams;
	inputfile->fileName = av_strdup(fileName);

	//strcpy(inputfile->fileName, fileName);

	printf("%s stream is %d.\n", inputfile->fileName, inputfile->numStream);

	for (int i = 0; i < fc->nb_streams;i++) {
		GROW_ARRAY(remuxingContext->inStreams, remuxingContext->numInStream);
		AVStream* as = fc->streams[i];
		InputStream* inputStream = av_mallocz(sizeof(InputStream));
		inputStream->file_index = index;
		inputStream->st = as;
		remuxingContext->inStreams[i] = inputStream;
	}

	return ret;
}

static void freeOutputFile(OutputFile* outputfile) {
	if (outputfile != NULL) {
		/*avformat_close_input(outputfile->formatContext);*/
		/* close output */
		if (outputfile->formatContext && !(outputfile->ofmt->flags & AVFMT_NOFILE)) {
			avio_closep(&outputfile->formatContext->pb);
		}


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

static int openOutputFile(RemuxingContext* remuxingContext, OutputFile* outputfile, char* fileName, char* format) {
	AVFormatContext* fc = NULL;
	int ret = -1;
	//�����ļ�����ȡ����ļ��ľ��
	avformat_alloc_output_context2(&fc, NULL, format, fileName);
	if (!fc)
	{
		printf("Error: Could not create output context.\n");
		return ret;
	}
	AVOutputFormat* ofmt = fc->oformat;
	outputfile->formatContext = fc;
	outputfile->ofmt = fc->oformat;
	remuxingContext->outputFile = outputfile;
	outputfile->fileName = av_strdup(fileName);
	//strcpy(outputfile->fileName, fileName);

	int curStId = 0;
	for (int i = 0; i < remuxingContext->nbInputFiles;i++) {
		InputFile* inputFile = remuxingContext->inputFiles[i];



		for (int j = 0; j < inputFile->numStream; j++) {
			//av_program_add_stream_index(fc, program->id, curStId);//���������id

			AVStream* inStream = inputFile->formatContext->streams[j];

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
			outStream->id = curStId;

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
			curStId++;
		}


	}

	curStId = 0;
	if (!strcmp(format, "mpegts") || !strcmp(format, "hls")) {
		for (int i = 0; i < remuxingContext->nbInputFiles;i++) {//��Ŀ��Ϣ���ڴ�����֮�󣬻�ûŪ���Ե��
			AVProgram* program = av_new_program(fc, i + 1);
			//av_dict_set(&program->metadata, "title", inputFile->fileName, 0);

			av_dict_set(&program->metadata, "title", "tv", 0);
			InputFile* inputFile = remuxingContext->inputFiles[i];

			for (int j = 0; j < inputFile->numStream; j++) {
				av_program_add_stream_index(fc, program->id, curStId++);//���������id
			}
		}
	}


	av_dump_format(fc, 0, fileName, 1);

	if (!(ofmt->flags & AVFMT_NOFILE))
	{
		AVDictionary* options = NULL;
		av_dict_set(&options, "protocol_whitelist", "file,udp,rtp,rtmp,rtsp,tcp", 0);
		ret = avio_open2(&fc->pb, fileName, AVIO_FLAG_WRITE, NULL, &options);
		//ret = avio_open(&fc->pb, fileName, AVIO_FLAG_WRITE);
		//ret = avio_open(&fc->pb, fileName, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			printf("Error: Could not open output file ��%s\n", fileName);
			return ret;
		}
	}

	return ret;
}


void freeRemuxingContext(RemuxingContext* rc) {
	if (rc) {
		for (int i = 0; i < rc->nbInputFiles;i++) {
			freeInputFile(rc->inputFiles[i]);
		}
		freeOutputFile(rc->outputFile);
	}
}


void testRemuxing3(int numInputFile, char** inputFileName, char* outputFileName, char* outputFormat)
{
	av_log_set_level(AV_LOG_DEBUG);
	//#if CONFIG_AVDEVICE
	//avdevice_register_all();
	//#endif
	av_register_all();
	avformat_network_init();


	RemuxingContext remuxingContext;
	RemuxingContext* pRemuxingContext = &remuxingContext;
	memset(&remuxingContext, 0, sizeof(struct RemuxingContext));
	remuxingContext.nbInputFiles = 0;//
	printf("open in put file\n");

	for (int i = 0; i < numInputFile;i++) {
		GROW_ARRAY(remuxingContext.inputFiles, remuxingContext.nbInputFiles);
		InputFile* inputFile = allocInputFile();
		if (openInputFile(pRemuxingContext, inputFile, i, inputFileName[i]) >= 0) {
			remuxingContext.inputFiles[remuxingContext.nbInputFiles - 1] = inputFile;
			pRemuxingContext->totalStreams += inputFile->numStream;
		}
		else {
			printf("open input error");
			freeInputFile(inputFile);
		}
	}
	printf("open out put file\n");
	OutputFile* outputFile = allocOutputFile();
	openOutputFile(pRemuxingContext, outputFile, outputFileName, outputFormat);


	int needRead[numInputFile];
	for (int i = 0; i < numInputFile;i++) {
		needRead[i] = 1;
	}

	int ret = 0;
	ret = avformat_write_header(outputFile->formatContext, NULL);
	if (ret < 0)
	{
		printf("Error: Could not write output file header.\n");
		goto end;
	}


	AVPacket pkt;
	int loop = numInputFile;
	int  readNum = 0;
	while (loop)
	{
		AVStream* in_stream, * out_stream;
		int curStId = 0;

		OutputFile* outputFile = pRemuxingContext->outputFile;
		for (int i = 0; i < numInputFile;i++) {
			if (needRead[i] == 0) {
				continue;
			}
			InputFile* inputFile = pRemuxingContext->inputFiles[i];

			ret = av_read_frame(inputFile->formatContext, &pkt);
			if (ret == AVERROR(EAGAIN)) {
				av_usleep(10000);
				continue;
			}
			readNum++;
			if (ret < 0) {
				printf("    av_read_frame ret : %d\n", -ret);
				//av_log(NULL, AV_LOG_ERROR,"av_read_frame ret : %d\n",-ret);
				loop--;
				needRead[i] = 0;
				break;
			}
			in_stream = inputFile->formatContext->streams[pkt.stream_index];
			if (i == 1) {
				out_stream = outputFile->formatContext->streams[pkt.stream_index + 2];//ȡ��Ӧ�������id
			}
			else {
				out_stream = outputFile->formatContext->streams[pkt.stream_index];//ȡ��Ӧ�������id
			}


			/* copy packet */
			pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, /*(AVRounding)*/(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, /*(AVRounding)*/(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
			pkt.pos = -1;

			if (i == 1) {
				pkt.stream_index = pkt.stream_index + 2;//ȡ��Ӧ�������id
			}


			ret = av_interleaved_write_frame(outputFile->formatContext, &pkt);
			if (ret < 0)
			{
				fprintf(stderr, "Error muxing packet\n");
				break;
			}
			av_free_packet(&pkt);

		}

		//av_usleep(10000);//�ӳ�10ms��Ҫ��Ȼһ�������������Ŷ˻��治����ô�࣬��������

	}
	printf("    readNum : %d\n", readNum);
	av_write_trailer(pRemuxingContext->outputFile->formatContext);
	printf("finish remux------------------------------\n");
end:
	freeRemuxingContext(pRemuxingContext);
}
