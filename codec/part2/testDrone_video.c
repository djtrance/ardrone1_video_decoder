// socket 
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// openCV
#include <cv.h>
#include <highgui.h>

// Ar.Drone
#include "com.h"
#include "video.h"

#define DRONE_VIDEO_PORT   5555
#define DRONE_COMMAND_PORT 5556

#define MAX_COMMAND                1024 // [bytes]
#define MAX_bufferedVideoStream    8192 // [*4 bytes]

#define WIDTH     320
#define HEIGHT    240
#define NCHANNEL    3


int main(int argc, char *argv[]) {
  
  // check command line args //
  if(argc<2) {
    printf("usage : %s <AR.Drone's IP>\n", argv[0]);
    exit(1);
  }
  
  // declare variables
  // etc
  int   i,j,k,l;
  // socket
  int                 sd1, sd2; 
  int                 flag = 0;
  struct hostent      *h;
  struct sockaddr_in  clientAddr_video;
  struct sockaddr_in  clientAddr_command;
  struct sockaddr_in  droneAddr_video;
  struct sockaddr_in  droneAddr_command;  
  socklen_t           socketsize = sizeof(droneAddr_video);
  // command
  char                command[MAX_COMMAND];
  unsigned int        seq = 1;
  // video
  int                 index;
  int                 video_size;
  uint8_t             remainingbits;
  uint32_t            bufferedVideoStream[MAX_bufferedVideoStream];
  // OpenCV
  int                 width, height, nChannels, widthStep;
  char                inputkey;
  CvSize              size;
  IplImage*           img_clr;
  IplImage*           img_gry;
  CvMat               img_mat_clr, img_mat_gry;
  uint8_t             img_data_clr[WIDTH*3*HEIGHT] = {0}; // actual image data for 3 channel RGB
  uint8_t             img_data_gry[WIDTH*1*HEIGHT] = {0}; // actual image data for grayscale image
  
  //socket initialization
  // get server IP address (no check if input is IP address or DNS name //
  h = gethostbyname(argv[1]);
  if(h==NULL) {
    printf("%s: unknown host '%s' \n", argv[0], argv[1]);
    exit(1);
  }
  
  // create socket for each communication port
  createSocket(&sd1, &droneAddr_video,   &clientAddr_video,   h, DRONE_VIDEO_PORT  );
  createSocket(&sd2, &droneAddr_command, &clientAddr_command, h, DRONE_COMMAND_PORT);
  
  // create image container for color image and grayscale image
  size.width  = WIDTH;
  size.height = HEIGHT;
  img_mat_gry = cvMat(HEIGHT, WIDTH, CV_8UC1, img_data_gry);
  img_mat_clr = cvMat(HEIGHT, WIDTH, CV_8UC3, img_data_clr);
  img_gry     = cvCreateImageHeader(size, IPL_DEPTH_8U,1);
  img_clr     = cvCreateImageHeader(size, IPL_DEPTH_8U,3);  
  img_gry     = cvGetImage(&img_mat_gry, img_gry);
  img_clr     = cvGetImage(&img_mat_clr, img_clr);
  
  // create windows
  cvNamedWindow("grayscale", CV_WINDOW_AUTOSIZE);
  cvNamedWindow("color",     CV_WINDOW_AUTOSIZE);
  cvMoveWindow("grayscale", 100, 100);
  cvMoveWindow("color",     200+WIDTH, 100);
    
  // start communication
  // send command to start video stream
  printf("%s: send AT*CONFIG\n",argv[0]); 
  sprintf(command, "AT*CONFIG=%d,\"general:video_enable\",\"TRUE\"\r",seq++);
  sendCommand(sd2, command, flag, droneAddr_command);
  
  // send command to disable autobitrate
  printf("%s: send AT*CONFIG\n",argv[0]);
  sprintf(command, "AT*CONFIG=%d,\"video:bitrate_ctrl_mode\",\"0\"\r",seq++);
  sendCommand(sd2, command, flag, droneAddr_command);
  
  // receive video
  while (1) {
    
    // tickle drone's port: drone starts to send video stream
    printf("%s: tickle video port\n",argv[0]);
    sprintf(command, "\x01\x00");
    sendCommand(sd1, command, flag, droneAddr_video);
    
    // receive data
    memset( bufferedVideoStream, '\0', sizeof(bufferedVideoStream)); 
    video_size = recvfrom(sd1, bufferedVideoStream, sizeof(bufferedVideoStream), 0, (struct sockaddr *)&droneAddr_video, &socketsize);
    printf("%s: received bufferedVideoStream %d bytes\n",argv[0],video_size);
    
    // decode video
    decodeVideo (bufferedVideoStream, &index, &remainingbits, img_clr, img_gry);
    printf("%s: video decoded\n",argv[0]);
    
    /*
    //************************** implement openCV algorithm here. *******************************************************
    height    = img_clr->height;
    width     = img_clr->width;
    nChannels = img_clr->nChannels;
    widthStep = width*nChannels;
    for(i=0;i<height;i++) for(j=0;j<width;j++) for(k=0;k<nChannels;k++)
    img_data_clr[i*widthStep+j*nChannels+k]=255-img_data_clr[i*widthStep+j*nChannels+k];
    //************************** implement openCV algorithm here. *******************************************************
    */
    
    // show the image
    cvShowImage("grayscale", img_gry);
    cvShowImage("color",     img_clr);
    
    // wait for a key
    inputkey = cvWaitKey(10);
    if ( inputkey == 27 ) 
    // ESC to quit
      break;
    else if ( inputkey == 10 ) { 
    // Enter : save images
      cvSaveImage("testimage_gry.jpg",img_gry,0);
      cvSaveImage("testimage_clr.jpg",img_clr,0);
    }
  }

  // send command to stop video stream
  printf("%s: send AT*CONFIG:close video\n",argv[0]); 
  sprintf(command, "AT*CONFIG=%d,\"general:video_enable\",\"FALSE\"\r",seq++);
  sendCommand(sd2, command, flag, droneAddr_command);
  
  
  // close socket
  if (close(sd1)<0) {
    printf("%s: cannnot close socket for video port",argv[0]);
    exit(1);
  }
  if (close(sd2)<0) {
    printf("%s: cannnot close socket for command port",argv[0]);
    exit(1);
  }
  
  return 1;

}

