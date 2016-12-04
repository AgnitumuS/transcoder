/*
 * Copyright (c) 2010 Nicolas George
 * Copyright (c) 2011 Stefano Sabatini
 * Copyright (c) 2014 Andrey Utkin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file
 * API example for demuxing, decoding, filtering, encoding and muxing
 * @example transcoding.c
 */

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>

#include <libavformat/avio.h>
#include <libavutil/file.h>

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>

#include <signal.h>
#include <unistd.h>

#define AV_CODEC_CAP_DELAY (1 << 5)
#define AV_CODEC_FLAG_GLOBAL_HEADER (1 << 22)


AVFormatContext *input_ctx;
AVFormatContext *output_ctx;



AVFormatContext *open_rtp_stream(char *url,int *index)
{
    AVFormatContext *context = avformat_alloc_context();
    int video_stream_index;
	AVInputFormat *ifmt=NULL;
	AVDictionary *options = NULL;

	ifmt = av_find_input_format("mpegts");
	if (!ifmt) {
        av_log(0, AV_LOG_ERROR, "Cannot find input format\n");
        return -1;
	}

	/*ifmt_ctx->flags |= AVFMT_FLAG_NONBLOCK;
	av_dict_set(&options, "input_format", "h264", 0);
	/*av_dict_set(&options, "framerate", "25", 0);
	av_dict_set(&options, "input_format", "mjpeg", 0);
	av_dict_set(&options, "video_size", "640x480", 0);
	if (avformat_open_input(&ifmt_ctx, src_filename, ifmt, &options)*/

    if (avformat_open_input(&context, url, ifmt, NULL) != 0){
		return EXIT_FAILURE;
    }

    if (avformat_find_stream_info(context, NULL) < 0){
		return EXIT_FAILURE;
	}

	 for(int i =0;i<context->nb_streams;i++){
        if(context->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
            video_stream_index = i;
    }
    //av_dump_format(context, 0, url, 0);
	(*index)=video_stream_index;
    return context;
}



AVFormatContext *open_output_file(const char *filename,AVFormatContext *ifmt_ctx,unsigned int stream_index){
	AVStream *out_stream;
	AVStream *in_stream;
	AVCodecContext *dec_ctx, *enc_ctx;
	AVFormatContext *ofmt_ctx;
	AVCodec *encoder;
	int ret;
	unsigned int i;
	i=stream_index;

	ofmt_ctx = NULL;
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);
	if (!ofmt_ctx) {
		av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
		return NULL;
	}


	//for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		out_stream = avformat_new_stream(ofmt_ctx, NULL);
		if (!out_stream) {
			av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
			return NULL;
		}

		in_stream = ifmt_ctx->streams[i];
		dec_ctx = in_stream->codec;
		enc_ctx = out_stream->codec;

		/* in this example, we choose transcoding to same codec */
		//TO DO cambiare encoder in h264
		//encoder=avcodec_find_encoder(AV_CODEC_ID_MJPEG);
		encoder=avcodec_find_encoder(AV_CODEC_ID_H264);
		if (!encoder) {
			av_log(NULL, AV_LOG_FATAL, "Necessary encoder not found\n");
			return NULL;
		}
		enc_ctx=avcodec_alloc_context3(encoder);
	
		out_stream->codec=enc_ctx;
		enc_ctx->height = dec_ctx->height;
		enc_ctx->width = dec_ctx->width;
		enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;

		out_stream->time_base=in_stream->time_base;//(AVRational){1,30};
		//enc_ctx->time_base = (AVRational){1,30};//dec_ctx->time_base;//(AVRational){1,30}; //(1,fps del video )
		//enc_ctx->framerate = (AVRational) {30,1};
		enc_ctx->pix_fmt = encoder->pix_fmts[0];

		
   
		/* Third parameter can be used to pass settings to encoder */
		ret = avcodec_open2(enc_ctx, encoder, NULL);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot open video encoder for stream #%u\n", i);
			perror("Error");
			return NULL;
		}

		if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
			enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	//}

	//av_dump_format(ofmt_ctx, 0, filename, 1);

	if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
		ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Could not open output file '%s'", filename);
			return NULL;
		}
	}

	/* init muxer, write output file header */
	ret = avformat_write_header(ofmt_ctx, NULL);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Error occurred when opening output file\n");
		return NULL;
	}

	return ofmt_ctx;
}

int encode_write_frame(AVFrame *frame, unsigned int stream_index, int *got_frame,AVFormatContext *ofmt_ctx) {
	int ret;
	int got_frame_local;
	AVPacket enc_pkt;
 

	if (!got_frame)
		got_frame = &got_frame_local;

	//av_log(NULL, AV_LOG_INFO, "Encoding frame\n");
	/* encode filtered frame */
	enc_pkt.data = NULL;
	enc_pkt.size = 0;
	av_init_packet(&enc_pkt);
	ret = avcodec_encode_video2(ofmt_ctx->streams[stream_index]->codec, &enc_pkt,
			frame, got_frame);
	//av_frame_free(&filt_frame);
	if (ret < 0){
		printf("Error encode!!!\n");
		exit(EXIT_FAILURE);
		}
	if (!(*got_frame))
		return 0;

	/* prepare packet for muxing */
	enc_pkt.stream_index = stream_index;
	

	if (enc_pkt.pts != AV_NOPTS_VALUE)
     enc_pkt.pts =  av_rescale_q(enc_pkt.pts, ofmt_ctx->streams[stream_index]->codec->time_base,ofmt_ctx->streams[stream_index]->time_base);
 	if (enc_pkt.dts != AV_NOPTS_VALUE)
     enc_pkt.dts = av_rescale_q(enc_pkt.dts, ofmt_ctx->streams[stream_index]->codec->time_base, ofmt_ctx->streams[stream_index]->time_base);
 

	//av_packet_rescale_ts(&enc_pkt, ofmt_ctx->streams[stream_index]->codec->time_base, ofmt_ctx->streams[stream_index]->time_base);


	//av_log(NULL, AV_LOG_DEBUG, "Muxing frame\n");
	/* mux encoded frame */
	ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);
	return ret;
}

int flush_encoder(unsigned int stream_index,AVFormatContext *ofmt_ctx)
{
	int ret;
	int got_frame;

	if (!(ofmt_ctx->streams[stream_index]->codec->codec->capabilities &
				AV_CODEC_CAP_DELAY))
		return 0;

	while (1) {
		printf("Flushing encoder %d \n",stream_index);
		ret = encode_write_frame(NULL, stream_index, &got_frame,ofmt_ctx);
		if (ret < 0)
			break;
		if (!got_frame)
			return 0;
	}
	return ret;
}

void sig_handler(int signo)
{

    int ret;
	int i;
    if (signo == SIGINT) {
		for (i = 0; i < input_ctx->nb_streams; i++) {
			/* flush encoder */ 
			ret = flush_encoder(i,output_ctx);
			if (ret < 0) {
				printf("Error on flush encoder!\n");
				exit(EXIT_FAILURE);
			}
		}

		av_write_trailer(output_ctx);
		exit(EXIT_SUCCESS);
    }
}

int main(int argc, char **argv)
{
    int ret;
	int i_video_index;
	int stream_index;
	int got_frame=0;

	int64_t position=0;
	int64_t prev_pts=0;


	int64_t pts=0;

    AVPacket packet;

	AVFormatContext *ifmt_ctx;
	AVFormatContext *ofmt_ctx;

	AVCodecContext *ic_ctx;
	AVCodecContext *oc_ctx;


	AVCodec *i_codec = NULL;
	AVCodec *o_codec = NULL;

	AVStream *i_stream=NULL;
	AVStream *o_stream=NULL;

	AVFrame *frame = NULL;


    if (argc != 3){
		av_log(NULL, AV_LOG_ERROR, "Usage: %s <input file> <output file>\n", argv[0]);
		return 1;
    }

    if (signal(SIGINT, sig_handler) == SIG_ERR){
		printf("\ncan't catch SIGINT\n");
		exit(-1);
    }

    av_register_all();
    avformat_network_init();
    avfilter_register_all();
    avdevice_register_all();

    ifmt_ctx = open_rtp_stream(argv[1],&i_video_index);
    if (!ifmt_ctx){
		fprintf(stderr, "Error on opening %s", argv[1]);
		exit(EXIT_FAILURE);
    }
	printf("video stream index is %d\n",i_video_index);
	ofmt_ctx = open_output_file(argv[2],ifmt_ctx,i_video_index);
	if (!ofmt_ctx){
		fprintf(stderr,"Error on opening %s",argv[2]);
		exit(EXIT_FAILURE);
	}

	

	//copio nelle variabili globali per la gestione di SIGINT
	input_ctx=ifmt_ctx;
	output_ctx=ofmt_ctx;

	av_init_packet(&packet);

	printf("Transcoding...\n");

	while(1){
		
			//leggo la frame 
		ret=av_read_frame(ifmt_ctx,&packet);
		if(ret>=0) {

			//decodifico la frame per poterla riscrivere
			stream_index=packet.stream_index;
	
			frame=av_frame_alloc();

			//ifmt_ctx->streams[stream_index]->codec->time_base = (AVRational){1,30};

			//av_packet_rescale_ts(&packet,ifmt_ctx->streams[stream_index]->time_base,ifmt_ctx->streams[stream_index]->codec->time_base);
			
			if (packet.pts != AV_NOPTS_VALUE)
     			packet.pts =  av_rescale_q(packet.pts, ifmt_ctx->streams[stream_index]->codec->time_base,ifmt_ctx->streams[stream_index]->time_base);
 			if (packet.dts != AV_NOPTS_VALUE)
     			packet.dts = av_rescale_q(packet.dts, ifmt_ctx->streams[stream_index]->codec->time_base, ifmt_ctx->streams[stream_index]->time_base);


			oc_ctx=ifmt_ctx->streams[stream_index]->codec;
			o_codec=avcodec_find_decoder(oc_ctx->codec_id);
			ret = avcodec_open2(oc_ctx,o_codec,NULL);
			if(ret<0){
				printf("Error on codec opening...\n");
				exit(EXIT_FAILURE);
			}

			//decodifico
			ret=avcodec_decode_video2(oc_ctx,frame,&got_frame,&packet);
			if(ret<0){
				printf("Error on decoding...\n");
				exit(EXIT_FAILURE);
			}
			if(got_frame){

				//printf("packet flags: %d picture type: %c\n", packet.flags,av_get_picture_type_char(frame->pict_type));
				//do{
					position = av_frame_get_best_effort_timestamp(frame);
				//}while (!((position != AV_NOPTS_VALUE) && (position >=0)));
				
				//prev_pts=position;

				if (!((position != AV_NOPTS_VALUE) && (position >=0))){
					printf("PTS %d Error\n",position);
					break;//exit(EXIT_FAILURE);
				}

				//if (frame) {
			    //   frame->pts = pts;
        		//	pts += frame->nb_samples;
    			//}

				frame->pts=position;
				
				//frame->dts=frame->pts;
				frame->pict_type=av_get_picture_type_char(frame->pict_type);//AV_PICTURE_TYPE_NONE;
				ret = encode_write_frame(frame,stream_index,&got_frame,ofmt_ctx);
				if(ret<0){
					printf("Error on write frame...\n");
					exit(EXIT_FAILURE);
				}
				av_frame_free(&frame);
			} else {
				av_frame_free(&frame);
			}
		}
		av_packet_unref(&packet);

		

	}




	printf("close\n");
	exit(EXIT_SUCCESS);




}
