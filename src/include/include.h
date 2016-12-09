#include <inttypes.h>
#include <stdio.h>

#define IPSIZE 16
#define PORT 50030
#define THREADPOOLSIZE 3
#define BUFFSIZE 1024

typedef struct es_message {
    char cmd_type[3];
    int8_t quality;
} es_message_t;

typedef struct {
	pthread_t pth;
	char *ip;
	int quality;
} thread_pool;




void error(char *msg)
{
  perror(msg);
  exit(EXIT_FAILURE);
}


int parse_command(char *input_cmd,es_message_t *cmd){
  int ret=0;
  int q;
  calloc(1,sizeof(es_message_t));
  ret = sscanf (input_cmd, "%s %hhi", cmd->cmd_type, &(cmd->quality));
  if (ret != 2){
    printf ("Command '%s' didn't scan properly\n", input_cmd);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;


}
