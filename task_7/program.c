#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define FILE_PATH "./resources/mando_test.mkv"
#define OUTPUT_PATH "./output.mkv"
#define SCALER 2

unsigned int g_interval[2] = {0 , 0};

typedef struct StreamingParams {
	char copy_video;
	char copy_audio;
	char *output_extension;
	char *muxer_opt_key;
	char *muxer_opt_value;
	char *video_codec;
	char *audio_codec;
	char *codec_priv_key;
	char *codec_priv_value;
} StreamingParams;

typedef struct StreamingContext {
	AVFormatContext *avfc;
	AVCodec *video_avc;
	AVCodec *audio_avc;
	AVStream *video_avs;
	AVStream *audio_avs;
	AVCodecContext *video_avcc;
	AVCodecContext *audio_avcc;
	AVCodecParameters *video_avcp;
	AVCodecParameters *audio_avcp;
	int video_index;
	int audio_index;
	char *filename;
} StreamingContext;

int cut_video(char* input_file, char* output_file, int interval_start, int interval_end) {
	int ret;
	
	StreamingContext *input_media = (StreamingContext*) calloc(1, sizeof(StreamingContext));
	input_media->filename = input_file;

	input_media->avfc = avformat_alloc_context();

	ret = avformat_open_input(&(input_media->avfc), input_media->filename, 0, 0);	
	if (ret < 0) {
		printf("Error: failed to open file!: %s", input_file);
		goto end;
	}

	ret = avformat_find_stream_info(input_media->avfc, 0);
	if (ret < 0) {
		printf("Error: failed to get input stream info");
		goto end;
	}
	
	StreamingContext *output_media = (StreamingContext*) calloc(1, sizeof(StreamingContext));
	output_media->filename = output_file;

	ret = avformat_alloc_output_context2(&(output_media->avfc), NULL, NULL, output_file);
	if (ret < 0) {
		printf("Error: failed to create output context\n");
		goto end;
	}

	for (int i = 0; i < input_media->avfc->nb_streams; i++) {
		AVStream *out_stream = avformat_new_stream(output_media->avfc, avcodec_find_decoder(input_media->avfc->streams[i]->codecpar->codec_id));

		if (!out_stream) {
			printf("Failed allocating output stream\n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}

		if (avcodec_parameters_copy(out_stream->codecpar, input_media->avfc->streams[i]->codecpar) < 0) {
			printf("Error: failed to copy codec parameters from input to output stream codec parameters\n");
			goto end;
		}

		if (input_media->avfc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			output_media->video_avs = out_stream;
		}
		if (input_media->avfc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			output_media->audio_avs = out_stream;
		}
	}

	printf("------------------------------\n");	
	av_dump_format(input_media->avfc, 0, input_file, 0);
	printf("------------------------------\n");
	av_dump_format(output_media->avfc, 0, output_file, 1);
	printf("------------------------------\n");
	
	if (!(output_media->avfc->oformat->flags & AVFMT_NOFILE)) {
		ret = avio_open(&output_media->avfc->pb, output_file, AVIO_FLAG_WRITE);
		if (ret < 0) {
			printf("Could not open output file '%s'", output_file);
			goto end;
		}
	}

	ret = avformat_write_header(output_media->avfc, NULL);
	if (ret < 0) {
		printf("Error occurred when opening output file\n");
		goto end;
	}

	ret = av_seek_frame(input_media->avfc, -1, interval_start*AV_TIME_BASE, AVSEEK_FLAG_ANY);
	if (ret < 0) {
		printf("Error seek\n");
		goto end;
	}

	int64_t *dts_start_from = malloc(sizeof(int64_t) * input_media->avfc->nb_streams);
	memset(dts_start_from, 0, sizeof(int64_t) * input_media->avfc->nb_streams);
	int64_t *pts_start_from = malloc(sizeof(int64_t) * input_media->avfc->nb_streams);
	memset(pts_start_from, 0, sizeof(int64_t) * input_media->avfc->nb_streams);

	AVPacket *pkt = av_packet_alloc();

	while (1) {
		AVStream *in_stream, *out_stream;
		
		ret = av_read_frame(input_media->avfc, pkt);
		if (ret < 0) break;

		in_stream  = input_media->avfc->streams[pkt->stream_index];
		out_stream = output_media->avfc->streams[pkt->stream_index];

		if (interval_end != 0) { //if interval_end is not specified (interval_end == 0) read packets till video ends
			if (av_q2d(in_stream->time_base) * pkt->pts > interval_end) {
				av_packet_unref(pkt);
				break;
			}
		} else {
			if (pkt->pts > input_media->avfc->duration * av_q2d(in_stream->time_base)) {
				av_packet_unref(pkt);
				break;
			}
		}

		if (dts_start_from[pkt->stream_index] == 0) {
			dts_start_from[pkt->stream_index] = pkt->dts;
			printf("dts_start_from: %s\n", av_ts2str(dts_start_from[pkt->stream_index]));
		}
		if (pts_start_from[pkt->stream_index] == 0) {
			pts_start_from[pkt->stream_index] = pkt->pts;
			printf("pts_start_from: %s\n", av_ts2str(pts_start_from[pkt->stream_index]));
		}

		/* copy packet */
		pkt->pts = av_rescale_q_rnd(pkt->pts - pts_start_from[pkt->stream_index], in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
		pkt->dts = av_rescale_q_rnd(pkt->dts - dts_start_from[pkt->stream_index], in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
		if (pkt->pts < 0) {
			pkt->pts = 0;
		}
		if (pkt->dts < 0) {
			pkt->dts = 0;
		}

		pkt->duration = (int)av_rescale_q((int64_t)pkt->duration, in_stream->time_base, out_stream->time_base);
		pkt->pos = -1;

		ret = av_interleaved_write_frame(output_media->avfc, pkt);
		if (ret < 0) {
			printf("Error muxing packet\n");
			break;
		}
		av_packet_unref(pkt);	

	}

	free(dts_start_from);
	free(pts_start_from);

	av_write_trailer(output_media->avfc);

end:
	
	avformat_close_input(&(input_media->avfc));

	/* close output */
	if (output_media->avfc && !(output_media->avfc->oformat->flags & AVFMT_NOFILE))
		avio_closep(&output_media->avfc->pb);
	avformat_free_context(output_media->avfc);

	if (ret < 0 && ret != AVERROR_EOF) {
		printf("Error occurred: %s\n", av_err2str(ret));
		return 1;
	}

	return 0;
}

int parse_args(int argc, char **argv) {
	const char *errstr;
	if (argc > 3) return -1;
	if (argc > 1) {
		if (!(g_interval[0] = atoi(argv[1])) && (strcmp(argv[1], "0")) ||
		(atoi(argv[1]) < 0)) return -1;
	}
	if (argc > 2) {
		if (!(g_interval[1] = atoi(argv[2]))  && (strcmp(argv[2], "0")) ||
		(atoi(argv[2]) < 0) || 
		(atoi(argv[1]) >= atoi(argv[2]))) return -1;
	}
	return 0;
}

int main(int argc, char **argv) {
	if (parse_args(argc, argv)) {
		printf("Invalid arguments! Try ./program <interval_start> <interval_end>\n");
		return -1;
	}

	cut_video(FILE_PATH, OUTPUT_PATH, g_interval[0], g_interval[1]);

	return 0;
}
