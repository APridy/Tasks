#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/frame.h"
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

static int select_sample_rate(const AVCodec *codec)
{
	const int *p;
	int best_samplerate = 0;
	if (!codec->supported_samplerates)
		return 44100;
	p = codec->supported_samplerates;
	while (*p) {
		best_samplerate = FFMAX(*p, best_samplerate);
		p++;
	}
	return best_samplerate;
}

int create_streams(StreamingContext *input_media, StreamingContext *output_media) {
	int ret;
	for (int i = 0; i < input_media->avfc->nb_streams; i++) {
		if (input_media->avfc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			input_media->video_avs = input_media->avfc->streams[i];
			output_media->video_avs = avformat_new_stream(output_media->avfc, avcodec_find_decoder(input_media->avfc->streams[i]->codecpar->codec_id));;
			if (!output_media->video_avs) {
				printf("Failed allocating output stream\n");
				return AVERROR_UNKNOWN;
			}
			ret = avcodec_parameters_copy(output_media->video_avs->codecpar, input_media->video_avs->codecpar);
			if (ret < 0) {
				printf("Error: failed to copy codec parameters from input to output stream codec parameters\n");
				return ret; 
			}
		}
		if (input_media->avfc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			input_media->audio_avs = input_media->avfc->streams[i];
			output_media->audio_avs = avformat_new_stream(output_media->avfc, avcodec_find_decoder(input_media->avfc->streams[i]->codecpar->codec_id));;

			if (!output_media->audio_avs) {
				printf("Failed allocating output stream\n");
				return AVERROR_UNKNOWN;
			}

			ret = avcodec_parameters_copy(output_media->audio_avs->codecpar, input_media->audio_avs->codecpar);
			if (ret < 0) {
				printf("Error: failed to copy codec parameters from input to output stream codec parameters\n");
				return ret;
			}
			output_media->audio_avs->time_base = input_media->audio_avs->time_base;
			output_media->audio_avs->codecpar->codec_id = AV_CODEC_ID_AC3;
		}
	}
}

int prepare_video_decoder(StreamingContext *input_media) {
	input_media->video_avc = avcodec_find_decoder(input_media->video_avs->codecpar->codec_id);
	input_media->video_avcc = avcodec_alloc_context3(input_media->video_avc);
	avcodec_parameters_to_context(input_media->video_avcc, input_media->video_avs->codecpar);
	avcodec_open2(input_media->video_avcc, input_media->video_avc, NULL);
	return 0;
}

int prepare_video_encoder(StreamingContext *output_media) {
	output_media->video_avc = avcodec_find_encoder(output_media->video_avs->codecpar->codec_id);
	output_media->video_avcc = avcodec_alloc_context3(output_media->video_avc);

	avcodec_parameters_to_context(output_media->video_avcc, output_media->video_avs->codecpar);
	output_media->video_avcc->time_base = output_media->audio_avs->time_base;
	output_media->video_avcc->height = output_media->video_avcc->height / SCALER;
	output_media->video_avcc->width = output_media->video_avcc->width / SCALER;

	avcodec_open2(output_media->video_avcc, output_media->video_avc, NULL);
	return 0;
}

int prepare_audio_decoder(StreamingContext *input_media) {
	input_media->audio_avc = avcodec_find_decoder(input_media->audio_avs->codecpar->codec_id);
	input_media->audio_avcc = avcodec_alloc_context3(input_media->audio_avc);
	avcodec_parameters_to_context(input_media->audio_avcc, input_media->audio_avs->codecpar);
	avcodec_open2(input_media->audio_avcc, input_media->audio_avc, NULL);
	return 0;
}

int prepare_audio_encoder(StreamingContext *output_media) {
	output_media->audio_avc = avcodec_find_encoder(AV_CODEC_ID_AC3);
	output_media->audio_avcc = avcodec_alloc_context3(output_media->audio_avc);

	output_media->audio_avcc->time_base = output_media->audio_avs->time_base;
	output_media->audio_avcc->sample_fmt = AV_SAMPLE_FMT_FLTP;
	output_media->audio_avcc->sample_rate = select_sample_rate(output_media->audio_avc);
	output_media->audio_avcc->channel_layout = AV_CH_LAYOUT_STEREO;
	output_media->audio_avcc->channels = av_get_channel_layout_nb_channels(output_media->audio_avcc->channel_layout);

	avcodec_open2(output_media->audio_avcc, output_media->audio_avc, NULL);
	return 0;
}

int encode_audio(StreamingContext *input_media, StreamingContext *output_media, AVFrame *frame, int stream_index, uint64_t *pts_start_from, uint64_t *dts_start_from) {
	AVPacket *out_pkt = av_packet_alloc();
	if (!out_pkt) {
		return -1;
	}

	frame->nb_samples = output_media->audio_avcc->frame_size;
	frame->format = output_media->audio_avcc->sample_fmt;
	frame->channel_layout = output_media->audio_avcc->channel_layout;
	int response = avcodec_send_frame(output_media->audio_avcc, frame);
	//printf("Send Frame: %d, %s\n",response , av_err2str(response));
	while (response >= 0) {

		response = avcodec_receive_packet(output_media->audio_avcc, out_pkt);
		//printf("Recieve Packet: %d, %s\n",response , av_err2str(response));
		if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
			break;
		} else if (response < 0) {
			return -1;
		}

		out_pkt->pts = av_rescale_q_rnd(out_pkt->pts - pts_start_from[out_pkt->stream_index], input_media->audio_avs->time_base, output_media->audio_avs->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
		out_pkt->dts = av_rescale_q_rnd(out_pkt->dts - dts_start_from[out_pkt->stream_index], input_media->audio_avs->time_base, output_media->audio_avs->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
		if (out_pkt->pts < 0) {
			out_pkt->pts = 0;
			}
		if (out_pkt->dts < 0) {
			out_pkt->dts = 0;
		}

		out_pkt->duration = (int)av_rescale_q((int64_t)out_pkt->duration, input_media->audio_avs->time_base, output_media->audio_avs->time_base);
		out_pkt->pos = -1;

		out_pkt->stream_index = stream_index;	
		av_packet_rescale_ts(out_pkt, input_media->audio_avs->time_base, output_media->audio_avs->time_base);

		response = av_interleaved_write_frame(output_media->avfc, out_pkt);
		if (response != 0) { 
			return -1;
		}
	}

	av_packet_unref(out_pkt);
	av_packet_free(&out_pkt);
	return 0;
}

int transcode_audio(StreamingContext *input_media, StreamingContext *output_media, AVPacket *pkt, AVFrame *frame, uint64_t *pts_start_from, uint64_t *dts_start_from) {
	int response = avcodec_send_packet(input_media->audio_avcc, pkt);
	//printf("Send Packet: %d, %s\n",response , av_err2str(response));
	while (response >= 0) {
		response = avcodec_receive_frame(input_media->audio_avcc, frame);
		//printf("Recieve Frame: %d, %s\n",response , av_err2str(response));
		if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
			break;
		} else if (response < 0) {
			return response;
		}
		if (response >= 0) {
			if (encode_audio(input_media, output_media, frame, pkt->stream_index, pts_start_from, dts_start_from)) return -1;
		}
		av_frame_unref(frame);
	}
	return 0;
}

int encode_video(StreamingContext *input_media, StreamingContext *output_media, AVFrame *frame, int stream_index, uint64_t *pts_start_from, uint64_t *dts_start_from) {
	int num_bytes;
	uint8_t *buffer;
	if (frame) frame->pict_type = AV_PICTURE_TYPE_NONE;
	AVPacket *out_pkt = av_packet_alloc();
	if (!out_pkt) {
		return -1;
	}
	AVFrame *out_frame = av_frame_alloc();
	out_frame->format = frame->format;
	out_frame->height = frame->height / SCALER;
	out_frame->width = frame->width / SCALER;

	if(out_frame==NULL) return -1;

	av_image_alloc(out_frame->data, out_frame->linesize, out_frame->width, out_frame->height, output_media->video_avcc->pix_fmt, 32);

	static struct SwsContext *img_convert_ctx;
	if(img_convert_ctx == NULL) {
		int w = output_media->video_avcc->width;
		int h = output_media->video_avcc->height;

		img_convert_ctx = sws_getContext(frame->width, frame->height, frame->format, out_frame->width, out_frame->height, frame->format, 0, NULL, NULL, NULL);
		if(img_convert_ctx == NULL) {
			fprintf(stderr, "Cannot initialize the conversion context!\n");
			exit(1);
		}
	}

	sws_scale(img_convert_ctx, (const uint8_t * const*)frame->data, frame->linesize, 0, frame->height, out_frame->data, out_frame->linesize);	
	out_frame->pts = frame->pts;
	int response = avcodec_send_frame(output_media->video_avcc, out_frame);

	while (response >= 0) {
		response = avcodec_receive_packet(output_media->video_avcc, out_pkt);
		if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
			break;
		} else if (response < 0) {
			return -1;
		}

		out_pkt->pts = av_rescale_q_rnd(out_pkt->pts - pts_start_from[out_pkt->stream_index], input_media->video_avs->time_base, output_media->video_avs->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
		out_pkt->dts = av_rescale_q_rnd(out_pkt->dts - dts_start_from[out_pkt->stream_index], input_media->video_avs->time_base, output_media->video_avs->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
		if (out_pkt->pts < 0) {
			out_pkt->pts = 0;
			}
		if (out_pkt->dts < 0) {
			out_pkt->dts = 0;
		}

		out_pkt->duration = (int)av_rescale_q((int64_t)out_pkt->duration, input_media->video_avs->time_base, output_media->video_avs->time_base);
		out_pkt->pos = -1;

		out_pkt->stream_index = stream_index;

		av_packet_rescale_ts(out_pkt, input_media->video_avs->time_base, output_media->video_avs->time_base);

		response = av_interleaved_write_frame(output_media->avfc, out_pkt);
		if (response != 0) {
			return -1;
		}
	}

	av_packet_unref(out_pkt);
	av_packet_free(&out_pkt);
	return 0;
}

int transcode_video(StreamingContext *input_media, StreamingContext *output_media, AVPacket *pkt, AVFrame *frame, uint64_t *pts_start_from, uint64_t *dts_start_from) {
	int response = avcodec_send_packet(input_media->video_avcc, pkt);
	while (response >= 0) {
		response = avcodec_receive_frame(input_media->video_avcc, frame);
		if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
			break;
		} else if (response < 0) {
			return response;
		}
		if (response >= 0) {
			if (encode_video(input_media, output_media, frame, pkt->stream_index, pts_start_from, dts_start_from)) return -1;
		}
		av_frame_unref(frame);
	}
	return 0;
}


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

	ret = create_streams(input_media, output_media);
	if (ret < 0) {
		printf("Error: failed to create output media streams\n");
		goto end;
	}
	
	prepare_video_decoder(input_media);
	prepare_video_encoder(output_media);
	prepare_audio_decoder(input_media);
	prepare_audio_encoder(output_media);

	printf("------------------------------\n");	
	av_dump_format(input_media->avfc, 0, input_file, 0);
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
	AVFrame *frame = av_frame_alloc();

	while (1) {
		AVStream *in_stream, *out_stream;

		ret = av_read_frame(input_media->avfc, pkt);
		if (ret < 0) break;

		in_stream  = input_media->avfc->streams[pkt->stream_index];
		out_stream = output_media->avfc->streams[pkt->stream_index];

		if (interval_end > input_media->avfc->duration * av_q2d(in_stream->time_base)) {
			interval_end = 0;
		}

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
			//printf("dts_start_from: %s\n", av_ts2str(dts_start_from[pkt->stream_index]));
		}
		if (pts_start_from[pkt->stream_index] == 0) {
			pts_start_from[pkt->stream_index] = pkt->pts;
			//printf("pts_start_from: %s\n", av_ts2str(pts_start_from[pkt->stream_index]));
		}

		if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			//if (transcode_audio(input_media, output_media, pkt, frame, pts_start_from, dts_start_from)) return -1;
			av_packet_unref(pkt);
		}
		if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			if (transcode_video(input_media, output_media, pkt, frame, pts_start_from, dts_start_from)) return -1;
			av_packet_unref(pkt);
		}

	}

	free(dts_start_from);
	free(pts_start_from);

	av_write_trailer(output_media->avfc);

	printf("------------------------------\n");
	av_dump_format(output_media->avfc, 0, output_file, 1);
	printf("------------------------------\n");

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
