#ifndef ffmpeg
#define ffmpeg
#include "config.h"

#include "libavformat/avformat.h"
#include "libavformat/avio.h"

#include "libavcodec/avcodec.h"

#include "libavfilter/avfilter.h"

#include "libavutil/avutil.h"
#include "libavutil/dict.h"
#include "libavutil/eval.h"
#include "libavutil/fifo.h"
#include "libavutil/hwcontext.h"
#include "libavutil/pixfmt.h"
#include "libavutil/rational.h"
#include "libavutil/thread.h"
#include "libavutil/threadmessage.h"

#include "libswresample/swresample.h"

typedef struct InputStream {
	int file_index;
	AVStream* st;
	int discard;             /* true if stream data should be discarded */
	int user_set_discard;
	int decoding_needed;     /* non zero if the packets must be decoded in 'raw_fifo', see DECODING_FOR_* */
#define DECODING_FOR_OST    1
#define DECODING_FOR_FILTER 2

	AVCodecContext* dec_ctx;
	AVCodec* dec;
	AVFrame* decoded_frame;
	AVFrame* filter_frame; /* a ref of decoded_frame, to be sent to filters */

	int64_t       start;     /* time when read started */
	/* predicted dts of the next packet read for this stream or (when there are
	 * several frames in a packet) of the next frame in current packet (in AV_TIME_BASE units) */
	int64_t       next_dts;
	int64_t       dts;       ///< dts of the last packet read for this stream (in AV_TIME_BASE units)

	int64_t       next_pts;  ///< synthetic pts for the next decode frame (in AV_TIME_BASE units)
	int64_t       pts;       ///< current pts of the decoded frame  (in AV_TIME_BASE units)
	int           wrap_correction_done;

	int64_t filter_in_rescale_delta_last;

	int64_t min_pts; /* pts with the smallest value in a current stream */
	int64_t max_pts; /* pts with the higher value in a current stream */

	// when forcing constant input framerate through -r,
	// this contains the pts that will be given to the next decoded frame
	int64_t cfr_next_pts;

	int64_t nb_samples; /* number of samples in the last decoded audio frame before looping */

	double ts_scale;
	int saw_first_ts;
	AVDictionary* decoder_opts;
	AVRational framerate;               /* framerate forced with -r */
	int top_field_first;
	int guess_layout_max;

	int autorotate;

	int fix_sub_duration;
	struct { /* previous decoded subtitle and related variables */
		int got_output;
		int ret;
		AVSubtitle subtitle;
	} prev_sub;

	struct sub2video {
		int64_t last_pts;
		int64_t end_pts;
		AVFifoBuffer* sub_queue;    ///< queue of AVSubtitle* before filter init
		AVFrame* frame;
		int w, h;
	} sub2video;

	int dr1;


	/* stats */
	// combined size of all the packets read
	uint64_t data_size;
	/* number of packets successfully read for this stream */
	uint64_t nb_packets;
	// number of frames/samples retrieved from the decoder
	uint64_t frames_decoded;
	uint64_t samples_decoded;

	int64_t* dts_buffer;
	int nb_dts_buffer;

	int got_output;
} InputStream;

typedef struct InputFile {
	AVFormatContext* ctx;
	int eof_reached;      /* true if eof reached */
	int eagain;           /* true if last read attempt returned EAGAIN */
	int ist_index;        /* index of first stream in input_streams */
	int loop;             /* set number of times input stream should be looped */
	int64_t duration;     /* actual duration of the longest stream in a file
							 at the moment when looping happens */
	AVRational time_base; /* time base of the duration */
	int64_t input_ts_offset;

	int64_t ts_offset;
	int64_t last_ts;
	int64_t start_time;   /* user-specified start time in AV_TIME_BASE or AV_NOPTS_VALUE */
	int seek_timestamp;
	int64_t recording_time;
	int nb_streams;       /* number of stream that ffmpeg is aware of; may be different
							 from ctx.nb_streams if new streams appear during av_read_frame() */
	int nb_streams_warn;  /* number of streams that the user was warned of */
	int rate_emu;
	int accurate_seek;

#if HAVE_THREADS
	AVThreadMessageQueue* in_thread_queue;
	pthread_t thread;           /* thread reading from this file */
	int non_blocking;           /* reading packets from the thread should not block */
	int joined;                 /* the thread has been joined */
	int thread_queue_size;      /* maximum number of queued packets */
#endif
} InputFile;

enum forced_keyframes_const {
	FKF_N,
	FKF_N_FORCED,
	FKF_PREV_FORCED_N,
	FKF_PREV_FORCED_T,
	FKF_T,
	FKF_NB
};

#define ABORT_ON_FLAG_EMPTY_OUTPUT (1 <<  0)

extern const char* const forced_keyframes_const_names[];

typedef enum {
	ENCODER_FINISHED = 1,
	MUXER_FINISHED = 2,
} OSTFinished;

typedef struct OutputStream {
	int file_index;          /* file index */
	int index;               /* stream index in the output file */
	int source_index;        /* InputStream index */
	AVStream* st;            /* stream in the output file */
	int encoding_needed;     /* true if encoding needed for this stream */
	int frame_number;
	/* input pts and corresponding output pts
	   for A/V sync */
	struct InputStream* sync_ist; /* input stream to sync against */
	int64_t sync_opts;       /* output frame counter, could be changed to some true timestamp */ // FIXME look at frame_number
	/* pts of the first frame encoded for this stream, used for limiting
	 * recording time */
	int64_t first_pts;
	/* dts of the last packet sent to the muxer */
	int64_t last_mux_dts;
	// the timebase of the packets sent to the muxer
	AVRational mux_timebase;
	AVRational enc_timebase;

	int                    nb_bitstream_filters;
	AVBSFContext** bsf_ctx;

	AVCodecContext* enc_ctx;
	AVCodecParameters* ref_par; /* associated input codec parameters with encoders options applied */
	AVCodec* enc;
	int64_t max_frames;
	AVFrame* filtered_frame;
	AVFrame* last_frame;
	int last_dropped;
	int last_nb0_frames[3];

	void* hwaccel_ctx;

	/* video only */
	AVRational frame_rate;
	int is_cfr;
	int force_fps;
	int top_field_first;
	int rotate_overridden;
	double rotate_override_value;

	AVRational frame_aspect_ratio;

	/* forced key frames */
	int64_t forced_kf_ref_pts;
	int64_t* forced_kf_pts;
	int forced_kf_count;
	int forced_kf_index;
	char* forced_keyframes;
	AVExpr* forced_keyframes_pexpr;
	double forced_keyframes_expr_const_values[FKF_NB];

	/* audio only */
	int* audio_channels_map;             /* list of the channels id to pick from the source stream */
	int audio_channels_mapped;           /* number of channels in audio_channels_map */

	char* logfile_prefix;
	FILE* logfile;



	AVDictionary* encoder_opts;
	AVDictionary* sws_dict;
	AVDictionary* swr_opts;
	AVDictionary* resample_opts;
	char* apad;
	OSTFinished finished;        /* no more packets should be written for this stream */
	int unavailable;                     /* true if the steram is unavailable (possibly temporarily) */
	int stream_copy;

	// init_output_stream() has been called for this stream
	// The encoder and the bitstream filters have been initialized and the stream
	// parameters are set in the AVStream.
	int initialized;

	int inputs_done;

	const char* attachment_filename;
	int copy_initial_nonkeyframes;
	int copy_prior_start;
	char* disposition;

	int keep_pix_fmt;

	/* stats */
	// combined size of all the packets written
	uint64_t data_size;
	// number of packets send to the muxer
	uint64_t packets_written;
	// number of frames/samples sent to the encoder
	uint64_t frames_encoded;
	uint64_t samples_encoded;

	/* packet quality factor */
	int quality;

	int max_muxing_queue_size;

	/* the packets are buffered here until the muxer is ready to be initialized */
	AVFifoBuffer* muxing_queue;

	/* packet picture type */
	int pict_type;

	/* frame encode sum of squared error values */
	int64_t error[4];
} OutputStream;

typedef struct OutputFile {
	AVFormatContext* ctx;
	AVDictionary* opts;
	int ost_index;       /* index of the first stream in output_streams */
	int64_t recording_time;  ///< desired length of the resulting file in microseconds == AV_TIME_BASE units
	int64_t start_time;      ///< start time in microseconds == AV_TIME_BASE units
	uint64_t limit_filesize; /* filesize limit expressed in bytes */

	int shortest;

	int header_written;
} OutputFile;

#endif // !ffmpeg

