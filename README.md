# transcoder

### General Information

This daemon uses libvlc for transcoding a rtp stream for a source to a default destination.
Is mean to be used in an SDN/NFV enabled network.
Provides the VNF of transcoding and restreaming a video using rtp protocol

### How it works 

The daemon listen on port TCP 50030 for commands.
It will restream and transcode an rtp video from rtp://@:50040
Message format is defined in [es_message_t] (https://github.com/gabrik/transcoder/blob/master/src/include/include.h#L12)

example message
``` 
ADD 0
```

Daemon will launch a thread for transcoding using libvlc.
Different quality of transcoding are avaiable

see this for refecence

```
  ######################### COMANDI ###################
  #                     char[3]int                    #
  # char[3] ADD OR REM                                #
  # int 0 or 1 or 2                                   #
  # 0 high 1 medium 2 low                             #
  # UDP Ports:                                        #
  # |------> 50040 : high                             #
  # |------> 50041 : medium                           #
  # +------> 50042 : low                              #
  #                                                   #
  # Daemon listening on  TCP 50030                    #
  #####################################################
```

### Compiling and running
To compile you can use the 'build.sh' file, we are going to provide a make file

