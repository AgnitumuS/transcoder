#include <vlc/libvlc.h>
#include <unistd.h>
#include <stdbool.h>
#include <stddef.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

void sig_handler(int signo){

    int ret;
	int i;
    if (signo == SIGINT) {
		exit(EXIT_SUCCESS);
    }
}


typedef struct es_message {
    char cmd_type[3];
    int8_t quality;
} es_message_t;

/*
  ######################### COMANDI ###################
  #                     char[3]int                    #
  # char[3] ADD OR REM                                #
  # int 0 or 1 or 2                                   #
  # 0 alta 1 media 2 bassa                            #
  # Porte:                                            #
  # |------> 50040 : alta                             #
  # |------> 50041 : media                            #
  # +------> 50042 : bassa                            #
  #                                                   #
  # Demone in ascolto su TCP 50030                    #
  #####################################################
*/



int main(int argc, char **argv) {
    libvlc_instance_t *vlc;
    const char *url;
    //crf=20:vbv-maxrate=3000:vbv-bufsize=100:intra-refresh=1:slice-max-size=1500:keyint=30:ref=1
    //transcode{vcodec=vp8}:
    const char *sout_high = "#rtp{dst=%s,port=50040,mux=ts}";
    const char *sout_medium = "#transcode{vcodec=h264,scale=1,width=1280,height=720,fps=25,venc=x264{preset=ultrafast,tune=zerolatency,vbv-bufsize=1000,intra-refresh,lookahead=10,keyint=25},acodec=none}:rtp{dst=%s,port=50041,mux=ts}";
    const char *sout_low = "#transcode{vcodec=h264,scale=1,width=854,height=480,threads=8,venc=x264{preset=ultrafast,tune=zerolatency,intra-refresh,lookahead=10,keyint=15},acodec=none}:rtp{dst=%s,port=50042,mux=ts}";

    const char *sin="%s";//" --rtp-caching=100";

    char sout_cmd[255];
    char sin_cmd[255];

    int quality = 0;

    const char *media_name;
    const char *destination;

    if (argc != 4){
		printf("Usage: %s <input file> <output file> <quality>\n", argv[0]);
		return 1;
    }
    media_name="test";
    url=argv[1];
    destination = argv[2];
    quality = atoi(argv[3]);

    

    printf("Media name %s\n",media_name);
    printf("Url %s\n",url);
    printf("Destination is %s\n",destination);
    printf("Quality is %d\n",quality);

     if (signal(SIGINT, sig_handler) == SIG_ERR){
		printf("\ncan't catch SIGINT\n");
		exit(-1);
    }

    vlc = libvlc_new(0,NULL);

    snprintf(sin_cmd, sizeof(sin_cmd) , sin, url);
    
    switch(quality){
        case 0: snprintf(sout_cmd, sizeof(sout_cmd) , sout_high, destination);
                break;
        case 1: snprintf(sout_cmd, sizeof(sout_cmd) , sout_medium, destination);
                break;
        case 2: snprintf(sout_cmd, sizeof(sout_cmd) , sout_low, destination);
                break;
        default: printf("Wrong parameter quality %d\n",quality);
                 exit(EXIT_FAILURE);


    }


    
    printf("sout command is %s\n",sout_cmd);
    printf("sin command is %s\n",sin_cmd);

    libvlc_vlm_add_broadcast(vlc, media_name, sin_cmd, sout_cmd, 0, NULL, true, false);
    libvlc_vlm_play_media(vlc, media_name);


/*
    char *vlcOptions[] = {""};
    vlcInstance = libvlc_new(1, vlcOptions);

    libvlc_vlm_add_broadcast(vlcInstance,
            "test",
            "file:///home/student/vlc-1.1.9/VlcIgranje/mission_impossible.ts",          "#transcode{vcodec=h264,vb=0,width=100,acodec=mp4a,ab=128,channels=2,samplerate=44100\}:rtp\{dst=192.168.3.63,port=6969,mux=ts},dst=display}",
            0,
            NULL,
            TRUE,
            0);

   libvlc_vlm_play_media(vlcInstance, "test");*/
    for (;;) pause();
    //sleep(50); /* Let it play for a minute */

    libvlc_vlm_stop_media(vlc, media_name);
    libvlc_vlm_release(vlc);
    return 0;
}