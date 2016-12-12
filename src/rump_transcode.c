
#include <unistd.h>
#include <stdbool.h>
#include <stddef.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#include "include/include.h"
#include "include/vlc/libvlc.h"

#include <pthread.h> //thread 
#include <sys/types.h> //socket 
#include <sys/socket.h> //socket
#include <netinet/in.h> //socket
#include <semaphore.h> //semafori
#include <stdio.h>
#include <string.h>

thread_pool tpool[THREADPOOLSIZE];
sem_t t_sem;
pthread_t thread_to_kill=(pthread_t)0;

void sig_handler(int signo){

    int ret;
	int i;
    if (signo == SIGINT) {

        //chiusura gentile dei thread
        /*for(int i=0;i<THREADPOOLSIZE;i++){
            if (tpool[i]!=NULL){
                thread_to_kill=tpool[i].pth;
                sem_post(&t_sem);
            }
        }*/

		exit(EXIT_SUCCESS);
    }
}




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

void *start_stream(void *arg){
    thread_pool *tp = (thread_pool *) arg;

    libvlc_instance_t *vlc;


    const char *sout_high = "#rtp{dst=%s,port=50040,mux=ts}";
    const char *sout_medium = "#transcode{vcodec=h264,scale=0.5,venc=x264{preset=ultrafast,tune=zerolatency,intra-refresh,lookahead=10,keyint=15}}:rtp{dst=%s,port=50041,mux=ts,caching=50000}";
    const char *sout_low = "#transcode{vcodec=h264,scale=0.25,venc=x264{preset=ultrafast,tune=zerolatency,intra-refresh,lookahead=10,keyint=15}}:rtp{dst=%s,port=50042,mux=ts,caching=50000}";

    const char *sin="%s";//" --rtp-caching=100";

    char sout_cmd[255];
    char sin_cmd[255];

    int quality;

    const char *url;
    const char *media_name="Foo";
    const char *destination;

    url="rtp://@:50040";//argv[1];
    destination = tp->ip;
    quality = tp->quality;


    printf("Media name %s\n",media_name);
    printf("Url %s\n",url);
    printf("Destination is %s\n",destination);
    printf("Quality is %d\n",quality);

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
                 return;


    }
    
    printf("sout command is %s\n",sout_cmd);
    printf("sin command is %s\n",sin_cmd);

    libvlc_vlm_add_broadcast(vlc, media_name, sin_cmd, sout_cmd, 0, NULL, true, false);
    libvlc_vlm_play_media(vlc, media_name);

    for (;;) {
        sem_wait(&t_sem);
        if(pthread_self()==thread_to_kill){
            printf("Thread %ud is stopping...\n",thread_to_kill);
            thread_to_kill=(pthread_t)0;
            break;
        }
        sem_post(&t_sem);
    }



    libvlc_vlm_stop_media(vlc, media_name);
    libvlc_vlm_release(vlc);
    

}

int search_stream(int8_t quality,int size){
    for(int i=0;i<size;i++){
        if(tpool[i].quality==quality)
            return i;
    }

    return -1;
}



int main(int argc, char **argv) {




    int sock,new_sock;
    struct sockaddr_in serv_addr,cli_addr;
    socklen_t clilen;
    char buffer[BUFFSIZE];
    int t_pool_index=0;
    int n;

    printf("Starting....\n");
    printf("Initializing semaphore...\n");
    
    sem_init(&t_sem,0,0);
    
    printf("[DONE]\n");
    printf("Opening socket...\n");
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		error("[ERROR] opening socket");

    printf("[DONE]\n");
    printf("Binding to %d...\n",PORT);
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	if(bind(sock, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		error("[ERROR] on binding");
	listen(sock,5);
	clilen = sizeof(cli_addr);
    
    printf("[DONE]\n");
    printf("Waiting for connections...\n\n");

    while(1){
        new_sock=accept(sock,(struct sockaddr *)&cli_addr,&clilen);
        if (new_sock < 0)
		 	error("ERROR on accept");
        
        printf("New connection incoming....\n");
        
        bzero(buffer,BUFFSIZE);
        n=read(new_sock,buffer,BUFFSIZE);
        if(n<0)
            error("[ERROR] reading from socket\n");

        
        thread_pool t;

        es_message_t msg;

        strtok(buffer, "\n");
        if(parse_command(&buffer,&msg)!=0){
            printf("Error on command...\n");
        }else{
            if(strncmp(msg.cmd_type,"ADD",3)==0){
                int pos = search_stream(msg.quality,t_pool_index);
                if(pos<0){
                     t.quality=msg.quality;
                    t.ip="192.168.34.41";
                    pthread_create(&t.pth,NULL,start_stream,(void *)&t);
                    tpool[t_pool_index]=t;
                    t_pool_index++;
                    printf("New thread launched with quality %d\n",msg.quality);
                    
                }else{
                   printf("Stream already present...\n");
                }
                
            }

            if(strncmp(msg.cmd_type,"DEL",3)==0){
                int pos = search_stream(msg.quality,t_pool_index);
                if(pos>=0){
                    thread_to_kill=tpool[pos].pth;
                    sem_post(&t_sem);
                }
            
            }
        }
        


        close(new_sock);
    }
    
}


    /*


    /////////////////// DA QUI IN GIU' E' LA PARTE DEI THREAD ////////////////

    libvlc_instance_t *vlc;
    const char *url;
    //crf=20:vbv-maxrate=3000:vbv-bufsize=100:intra-refresh=1:slice-max-size=1500:keyint=30:ref=1
    //transcode{vcodec=vp8}:
    const char *sout_high = "#rtp{dst=%s,port=50040,mux=ts}";
    const char *sout_medium = "#transcode{vcodec=h264,scale=0.5,venc=x264{preset=ultrafast,tune=zerolatency,intra-refresh,lookahead=10,keyint=15}}:rtp{dst=%s,port=50042,mux=ts,caching=50000}";
    const char *sout_low = "#transcode{vcodec=h264,scale=0.25,venc=x264{preset=ultrafast,tune=zerolatency,intra-refresh,lookahead=10,keyint=15}}:rtp{dst=%s,port=50042,mux=ts,caching=50000}";

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



    for (;;) pause();


    libvlc_vlm_stop_media(vlc, media_name);
    libvlc_vlm_release(vlc);
    return 0;*/
