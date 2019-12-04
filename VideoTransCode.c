#include "VideoTransCode.h"

static char* outFormat = "mpegts";

void doTransCode(IOFiles* files)
{

	av_log_set_level(AV_LOG_DEBUG);
//#if CONFIG_AVDEVICE
	avdevice_register_all();
//#endif
	avformat_network_init();
	//av_register_all();

	AVOutputFormat* ofmt = NULL;
	AVFormatContext* ifmt_ctx = NULL, * ofmt_ctx = NULL;
	AVPacket pkt;

	int ret = 0;
	if ((ret = avformat_open_input(&ifmt_ctx, files->inputName, NULL, NULL)) < 0)
	{
		printf("Error: Open input file failed.\n");
		goto end;
	}

	//获取输入视频文件中的流信息
	if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0)
	{
		printf("Error: Failed to retrieve input stream information.\n");
		goto end;
	}
	av_dump_format(ifmt_ctx, 0, files->inputName, 0);

	//按照文件名获取输出文件的句柄
	avformat_alloc_output_context2(&ofmt_ctx, NULL, outFormat, files->outputName);
	if (!ofmt_ctx)
	{
		printf("Error: Could not create output context.\n");
		goto end;
	}
	ofmt = ofmt_ctx->oformat;

	for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++)
	{
		AVStream* inStream = ifmt_ctx->streams[i];
		AVCodecParameters* avCodecParameters = inStream->codecpar;
		AVCodecContext* inAVCodecContext = avcodec_alloc_context3(NULL);
		if (!inAVCodecContext) {
			printf("alloc avcodec fail.");
			goto end;
		}
		ret = avcodec_parameters_to_context(inAVCodecContext, avCodecParameters);
		if (ret < 0) {
			printf("avcodec_parameters_to_context fail.");
			goto end;
		}

		AVStream* outStream = avformat_new_stream(ofmt_ctx, inAVCodecContext->codec);
		if (!outStream)
		{
			printf("Error: Could not allocate output stream.\n");
			goto end;
		}

		AVCodecContext* outAVCodecContext = avcodec_alloc_context3(NULL);
		if (!outAVCodecContext) {
			printf("alloc out avcodec fail.");
			goto end;
		}
		ret = avcodec_parameters_to_context(outAVCodecContext, avCodecParameters);
		if (ret < 0) {
			printf("out avcodec_parameters_to_context fail.");
			goto end;
		}

		ret = avcodec_copy_context(outAVCodecContext, inAVCodecContext);
		outAVCodecContext->codec_tag = 0;
		if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		{
			outAVCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}

		//AVCodecParameters* outAVCodecParameters = avcodec_parameters_alloc();
		ret = avcodec_parameters_from_context(outStream->codecpar, outAVCodecContext);

		if (ret < 0) {
			printf("avcodec_parameters_from_context fail.");
			goto end;
		}
		//outStream->codecpar = outAVCodecParameters;

		


		avcodec_free_context(&inAVCodecContext);
		avcodec_free_context(&outAVCodecContext);
	}

	AVProgram* program = av_new_program(ofmt_ctx, 1);
	av_dict_set(&program->metadata, "title", "CCTV1", 0);
	av_program_add_stream_index(ofmt_ctx, 1, 0);
	av_program_add_stream_index(ofmt_ctx, 1, 1);

	av_dump_format(ofmt_ctx, 0, files->outputName, 1);

	if (!(ofmt->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&ofmt_ctx->pb, files->outputName, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			printf("Error: Could not open output file.\n");
			goto end;
		}
	}

	ret = avformat_write_header(ofmt_ctx, NULL);
	if (ret < 0)
	{
		printf("Error: Could not write output file header.\n");
		goto end;
	}

	int readTimes = 0;
	while (1)
	{
		AVStream* in_stream, * out_stream;

		ret = av_read_frame(ifmt_ctx, &pkt);
		//av_log(NULL, AV_LOG_ERROR,"av_read_frame ret : %d\n",readTimes++);
		if (ret == AVERROR(EAGAIN)) {
			av_usleep(10000);
			continue;
		}
		if (ret < 0) {
			printf("av_read_frame ret : %d\n",ret);
			av_log(NULL, AV_LOG_ERROR,
				"av_read_frame ret : %d\n",
				ret);
			break;
		}
	

		in_stream = ifmt_ctx->streams[pkt.stream_index];
		out_stream = ofmt_ctx->streams[pkt.stream_index];

		/* copy packet */
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, /*(AVRounding)*/(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, /*(AVRounding)*/(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;

		ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
		if (ret < 0)
		{
			fprintf(stderr, "Error muxing packet\n");
			break;
		}
		av_free_packet(&pkt);
	}

	av_write_trailer(ofmt_ctx);

end:
	avformat_close_input(&ifmt_ctx);

	/* close output */
	if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
		avio_closep(&ofmt_ctx->pb);

	avformat_free_context(ofmt_ctx);

	if (ret < 0 && ret != AVERROR_EOF)
	{
		fprintf(stderr, "Error failed to write packet to output file.\n");
	}
}
