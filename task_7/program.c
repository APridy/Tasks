#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define FILE_PATH "./resources/mando_test.mkv"
#define OUTPUT_PATH "./output.mkv"
unsigned int g_interval[2] = {0 , 0};

int cut_video(const char* input_file, int interval_start, int interval_end, const char* output_file) {
	AVOutputFormat *ofmt = NULL;
	AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
	AVPacket pkt;
	int i;

	if (avformat_open_input(&ifmt_ctx, input_file, 0, 0)) {
		printf("Error: failed to open file!: %s", input_file);
		goto end;
	}

	if (avformat_find_stream_info(ifmt_ctx, 0)) {
		printf("Error: failed to get input stream info");
		goto end;
	}


	if (avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, output_file) < 0) { 
		printf("Error: failed to create output context\n");
		goto end;
	}

	ofmt = ofmt_ctx->oformat;

	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		AVStream *in_stream = ifmt_ctx->streams[i];
		AVStream *out_stream = avformat_new_stream(ofmt_ctx, avcodec_find_decoder(in_stream->codecpar->codec_id));
		if (!out_stream) {
			printf("Error: failed to allocate output stream\n");
			goto end;
		}

		if (avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar) < 0) {
			printf("Error: failed to copy context from input to output stream codec context\n");
			goto end;
		}
	}

	if (!(ofmt->flags & AVFMT_NOFILE)) {
		if (avio_open(&ofmt_ctx->pb, output_file, AVIO_FLAG_WRITE)) {
			printf("Error: failed to open output file '%s'", output_file);
			goto end;
		}
	}
	
	if (avformat_write_header(ofmt_ctx, NULL) < 0) {
		printf("Error: failed to write header to output file\n");
		goto end;
	}

	if (av_seek_frame(ifmt_ctx, -1, interval_start*AV_TIME_BASE, AVSEEK_FLAG_ANY) < 0) {
		printf("Error: failed to seek frame\n");
		goto end;
	}

	int64_t *dts_start_from = malloc(sizeof(int64_t) * ifmt_ctx->nb_streams);
	memset(dts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);
	int64_t *pts_start_from = malloc(sizeof(int64_t) * ifmt_ctx->nb_streams);
	memset(pts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);

	while (1) {
		AVStream *in_stream, *out_stream;

		if (av_read_frame(ifmt_ctx, &pkt) < 0) break;

		in_stream  = ifmt_ctx->streams[pkt.stream_index];
		out_stream = ofmt_ctx->streams[pkt.stream_index];

		if (interval_end != 0) {
			if (av_q2d(in_stream->time_base) * pkt.pts > interval_end) {
				av_packet_unref(&pkt);
				break;
			}
		} else {
			if (pkt.pts > ifmt_ctx->duration * av_q2d(in_stream->time_base)) {
				av_packet_unref(&pkt);
				break;
			}
		}

		if (dts_start_from[pkt.stream_index] == 0) {
			dts_start_from[pkt.stream_index] = pkt.dts;
		}
		if (pts_start_from[pkt.stream_index] == 0) {
			pts_start_from[pkt.stream_index] = pkt.pts;
		}

		pkt.pts = av_rescale_q_rnd(pkt.pts - pts_start_from[pkt.stream_index], in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
		pkt.dts = av_rescale_q_rnd(pkt.dts - dts_start_from[pkt.stream_index], in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
		if (pkt.pts < 0) {
			pkt.pts = 0;
		}
		if (pkt.dts < 0) {
			pkt.dts = 0;
		}
		pkt.duration = (int)av_rescale_q((int64_t)pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;

		if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
			printf("Error: failed to mux packet\n");
			break;
		}
		av_packet_unref(&pkt);
	}
	free(dts_start_from);
	free(pts_start_from);

	av_write_trailer(ofmt_ctx);
end:

	avformat_close_input(&ifmt_ctx);
	if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE)) avio_closep(&ofmt_ctx->pb);
	avformat_free_context(ofmt_ctx);

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

	cut_video(FILE_PATH, g_interval[0], g_interval[1], OUTPUT_PATH);

	return 0;
}
