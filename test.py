import ffmpy

#ff = ffmpy.FFmpeg(inputs={'/dev/video0':None},outputs={'rtp://127.0.0.1:50040':'-c:v h264 -f rtp'})

ff = ffmpy.FFmpeg(inputs={'/dev/video0':None},outputs={'udp://127.0.0.1:50040':'-c:v h264 -preset ultrafast  -f mpegts'})
print ff.cmd
#ff.run()

#ffmpeg -f v4l2 -input_format mjpeg -i /dev/video0 -s 1280x720 -framerate 30 -c:v libx264 -preset veryfast -tune zerolatency -x264opts crf=20:vbv-maxrate=3000:vbv-bufsize=100:intra-refresh=1:slice-max-size=1500:keyint=30:ref=1 -f mpegts udp://127.0.0.1:50040

#ffmpeg -f v4l2 -input_format mjpeg -i /dev/video0 -s 1280x720 -framerate 30 -c:v mpeg2video -q:v 20 -pix_fmt yuv420p -g 1 -threads 4 -f mpegts udp://127.0.0.1:50040

# ffmpeg -f v4l2 -input_format mjpeg -i /dev/video0 -s 1280x720 -framerate 30 -c:v mjpeg  -f mjpeg udp://127.0.0.1:50040

#ffmpeg -f v4l2 -input_format mjpeg -i /dev/video0 -s 1280x720 -framerate 30 -c:v libx264 -preset veryfast -tune zerolatency -sdp_file stream.sdp -x264opts crf=20:vbv-maxrate=3000:vbv-bufsize=100:intra-refresh=1:slice-max-size=1500:keyint=30:ref=1 -f rtp rtp://127.0.0.1:50040


#ffmpeg -f v4l2 -input_format mjpeg -i /dev/video0 -s 1280x720 -framerate 30 -c:v mpeg2video -sdp_file stream.sdp -q:v 20 -pix_fmt yuv420p -g 1 -threads 4 -f rtp rtp://127.0.0.1:50040

#ffmpeg -f v4l2 -input_format mjpeg -i /dev/video0 -r 30 -vcodec libx264 -sdp_file stream.sdp -tune zerolatency -preset ultrafast -pix_fmt yuv420p -x264opts crf=20:vbv-maxrate=3000:vbv-bufsize=100:intra-refresh=1:slice-max-size=1500:keyint=30:ref=1  -f rtp rtp://127.0.0.1:50040

#ffmpeg -f v4l2 -input_format mjpeg -i /dev/video0 -r 30 -vcodec libvpx -sdp_file stream.sdp -quality realtime -force_key_frames  -f rtp rtp://127.0.0.1:50040

#ffmpeg -f v4l2 -input_format mjpeg -i /dev/video0 -r 30 -vcodec h264_qsv -sdp_file stream.sdp -f rtp rtp://127.0.0.1:50040




#### BEST QUALITY
#ffmpeg -f v4l2 -input_format mjpeg -i /dev/video0 -framerate 30 -c:v libx264 -preset ultrafast -tune zerolatency -x264opts crf=20:vbv-maxrate=3000:vbv-bufsize=100:intra-refresh=1:slice-max-size=1500:keyint=30:ref=1 -sdp_file stream.sdp -f rtp rtp://127.0.0.1:50040


#ffmpeg -f mpegts  -i rtp://127.0.0.1:50040 -vcodec mjpeg  -f mpegts test.mp4

#ffmpeg -f v4l2 -input_format mjpeg -i /dev/video0 -framerate 30 -f mpegts rtp://127.0.0.1:50040