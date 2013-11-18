#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <cv.h>
#include <highgui.h>

#define MAXLINE 100
#define MAXUSER 50
#define VIDEO_HEIGHT    40
#define VIDEO_WIDTH     100
#define MAXVIDEO    VIDEO_HEIGHT*VIDEO_WIDTH

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "port.h"

#define BUFSIZE 2048
#define BUFLEN 2048

/* AUDIO */
#include <portaudio.h>

/* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (512)
#define NUM_SECONDS     (5)
#define NUM_CHANNELS    (2)
/* #define DITHER_FLAG     (paDitherOff) */
#define DITHER_FLAG     (0) /**/
/** Set to 1 if you want to capture the recording to a file. */
#define WRITE_TO_FILE   (0)

/* Select sample format. */
#if 0
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif 0
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 1
#define PA_SAMPLE_TYPE  paInt8
typedef char SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif

typedef struct
{
  int fd;  /* Index into sample array. */
  int slen;
  struct sockaddr *remaddr;
} paTestData;

static int sendCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData )
{
  paTestData *data = (paTestData*)userData;
  const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
  long framesToCalc = framesPerBuffer;
  long i;

  (void) outputBuffer; /* Prevent unused variable warnings. */
  (void) timeInfo;
  (void) statusFlags;
  (void) userData;

  if( inputBuffer == NULL )
  {
    exit(10);
  }

  if (sendto(data->fd, inputBuffer, (sizeof(SAMPLE) * framesPerBuffer * NUM_CHANNELS), 0, (struct sockaddr *) (data->remaddr), data->slen)==-1) {
    perror("sendto");
    exit(1);
  }

  return paContinue;
}

static int recvCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData )
{
  paTestData *data = (paTestData*)userData;
  SAMPLE *wptr = (SAMPLE*)outputBuffer;
  SAMPLE rptr[framesPerBuffer * NUM_CHANNELS];
  long framesToCalc = framesPerBuffer;
  long i;

  (void) inputBuffer; /* Prevent unused variable warnings. */
  (void) timeInfo;
  (void) statusFlags;
  (void) userData;

  if( outputBuffer == NULL )
  {
    exit(10);
  }

  int recvlen = recvfrom(data->fd, rptr, (sizeof(SAMPLE) * framesPerBuffer * NUM_CHANNELS), 0, (struct sockaddr *) data->remaddr, &(data->slen));
  for (i = 0; i < (framesPerBuffer * NUM_CHANNELS); i ++) {
      if ((recvlen / sizeof(SAMPLE)) > i) {
        *wptr++ = rptr[i];
      } else {
        *wptr++ = SAMPLE_SILENCE;
      }
  }

  return paContinue;
}

int initialize_audio_send(void)
{
  PaStreamParameters  inputParameters,
                      outputParameters;
  PaStream*           stream;
  PaError             err = paNoError;
  paTestData          data;
  int                 totalFrames;
  int                 numSamples;
  int                 numBytes;
  SAMPLE              max, val;
  double              average;
	struct sockaddr_in myaddr, remaddr;
	int fd, i, slen=sizeof(remaddr);
	char buf[BUFLEN];	/* message buffer */
	int recvlen;		/* # bytes in acknowledgement message */
	char *server = "172.26.11.140";	/* change this to use a different server */

	/* create a socket */

	if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
		printf("socket created\n");

	/* bind it to all local addresses and pick any port number */

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}       

	/* now define remaddr, the address to whom we want to send messages */
	/* For convenience, the host address is expressed as a numeric IP address */
	/* that we will convert to a binary format via inet_aton */

	memset((char *) &remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(SERVICE_PORT);
	if (inet_aton(server, &remaddr.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

  data.fd = fd;
  data.remaddr = (struct sockaddr *)  &remaddr;
  data.slen = slen;

  err = Pa_Initialize();
  if( err != paNoError ) exit(1);

  inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
  if (inputParameters.device == paNoDevice) {
      fprintf(stderr,"Error: No default input device.\n");
      exit(1);
  }
  inputParameters.channelCount = 2;                    /* stereo input */
  inputParameters.sampleFormat = PA_SAMPLE_TYPE;
  inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
  inputParameters.hostApiSpecificStreamInfo = NULL;

  /* Record some audio. -------------------------------------------- */
  err = Pa_OpenStream(
            &stream,
            &inputParameters,
            NULL,                  /* &outputParameters, */
            SAMPLE_RATE,
            FRAMES_PER_BUFFER,
            paClipOff,      /* we won't output out of range samples so don't bother clipping them */
            sendCallback,
            &data );
  if( err != paNoError ) exit(1);

  err = Pa_StartStream( stream );
  if( err != paNoError ) exit(1);
  //printf("\n=== Now listenning!! Please speak into the microphone. ===\n"); fflush(stdout);

  while( ( err = Pa_IsStreamActive( stream ) ) == 1 )
  {
    Pa_Sleep(1000);
  }
  if( err < 0 ) exit(1);

  err = Pa_CloseStream( stream );
  if( err != paNoError ) exit(1);
	/* now let's send the messages */

	close(fd);
	return 0;
}


int initialize_audio_recv()
{
  PaStreamParameters  inputParameters,
                      outputParameters;
  PaStream*           stream;
  PaError             err = paNoError;
  paTestData          data;
  int                 totalFrames;
  int                 numSamples;
  int                 numBytes;
  SAMPLE              max, val;
  double              average;
	struct sockaddr_in myaddr;	/* our address */
	struct sockaddr_in remaddr;	/* remote address */
	socklen_t addrlen = sizeof(remaddr);		/* length of addresses */
	int recvlen;			/* # bytes received */
	int fd;				/* our socket */
	int msgcnt = 0;			/* count # of messages we received */
	unsigned char buf[BUFSIZE];	/* receive buffer */


	/* create a UDP socket */

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return 0;
	}

	/* bind the socket to any valid IP address and a specific port */

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(SERVICE_PORT);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}

  err = Pa_Initialize();
  if( err != paNoError ) printf("\a");

  data.remaddr = (struct sockaddr *) &remaddr;
  data.fd = fd;
  data.slen = addrlen;

  outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
  if (outputParameters.device == paNoDevice) {
      fprintf(stderr,"Error: No default output device.\n");
      printf("\a");
  }
  outputParameters.channelCount = 2;                     /* stereo output */
  outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
  outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = NULL;

  //printf("\n=== Now playing back. ===\n"); fflush(stdout);
  err = Pa_OpenStream(
            &stream,
            NULL, /* no input */
            &outputParameters,
            SAMPLE_RATE,
            FRAMES_PER_BUFFER,
            paClipOff,      /* we won't output out of range samples so don't bother clipping them */
            recvCallback,
            &data );
  if( err != paNoError ) printf("\a");

  if( stream )
  {
      err = Pa_StartStream( stream );
      if( err != paNoError ) printf("\a");
      
      //printf("Waiting for playback to finish.\n"); fflush(stdout);

      while( ( err = Pa_IsStreamActive( stream ) ) == 1 ) Pa_Sleep(100);
      if( err < 0 ) printf("\a");
      
      err = Pa_CloseStream( stream );
      if( err != paNoError ) printf("\a");
      
      //printf("Done.\n"); fflush(stdout);
  }
	/* never exits */
}



/* globals to make life easier */
WINDOW* main_window;
WINDOW* type_window;
WINDOW* video_window;
int video_on;

CvCapture* cv_cap;

/* initializing socket shit */
int sockfd = 0,n = 0;
char recvBuff[MAXLINE];
char videoBuffOut[MAXVIDEO];
char videoBuffIn[MAXVIDEO];

struct sockaddr_in serv_addr;
int vidsockfd = 0,m = 0;
struct sockaddr_in video_serv_addr;

char *ip_address = "127.0.0.1";

pthread_mutex_t data_lock;

typedef struct _chat_buffer {
    int length;
    char user[MAXUSER];
    char text[MAXLINE]; /* 50 char limit on text */
} chat_buffer;

typedef struct _window_buffer{
    int num_messages;
    chat_buffer **chats;
} window_buffer;

chat_buffer *text_buf;
window_buffer *window_buf;

static void finish(int sig);
void draw_screen();
void read_loop();
void video_loop();
void submit_text();
void video_feed();

void initialize_video(){
    video_on = 1;
    video_window = subwin(main_window, 10, 10, 0, 100);
    
    /* initialize video socket */
    memset(recvBuff, '0' ,sizeof(recvBuff));

    if((vidsockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
       printf("\n Error : Could not create socket \n");
       exit(1);
    }

    video_serv_addr.sin_family = AF_INET;
    video_serv_addr.sin_port = htons(5555);
    video_serv_addr.sin_addr.s_addr = inet_addr(ip_address);

    if(connect(vidsockfd, (struct sockaddr *)&video_serv_addr,
    sizeof(video_serv_addr))<0)
    {
       printf("\n Error : Connect Failed \n");
       exit(1);
    }

    /* spawn a read loop */
    pthread_t video_thread;
    pthread_t video_feed_thread;
    int val,val2;
    val = pthread_create(&video_thread, NULL, (void*)video_loop, NULL);
    val2 = pthread_create(&video_feed_thread, NULL, (void*)video_feed, NULL);
    
}

void print_image(IplImage *img){
    int i,j;
    int height = img->height;
    int width = img->width;
	//Map grayscale to these ascii density values
	char *asciiArr = " .:-=+*#%%@#";
    for(i=0;i<(img->height)*(img->width);i++)
    { 
        videoBuffOut[i] = 
                asciiArr[((unsigned char)img->imageData[i])/25];
    }
    write(vidsockfd, videoBuffOut, sizeof(videoBuffOut));
    
}

void video_feed(){
    int c;
    IplImage* color_img; //= cvCreateImage(cvSize(100,100),8,3);

	color_img = cvQueryFrame(cv_cap);

	IplImage* resize_img = cvCreateImage(cvSize(VIDEO_WIDTH,VIDEO_HEIGHT),8,3);
	IplImage* gray_img = cvCreateImage(cvSize(VIDEO_WIDTH,VIDEO_HEIGHT),8,1);
	
    //cvNamedWindow("Video",0); // create window

    for(;;) {
        color_img = cvQueryFrame(cv_cap); // get frame
				// gray_img = cvCvtColor(cv_cap, gray, CV_BGR2GRAY);
        if(color_img != 0){
						cvResize(color_img, resize_img, CV_INTER_AREA);
						cvCvtColor(resize_img, gray_img, CV_BGR2GRAY);
            // printf("%d\n",(color_img->imageData)[0]);
						// printf("%d\n", color_img->nChannels);
            print_image(gray_img);
            //cvShowImage("Video", gray_img); // show frame
        }
        /* reduce the load on the CPU by a billion */
        
        struct timespec tim, tim2;
        tim.tv_sec  = 0;
        tim.tv_nsec = 100000000L; // 1/20th of a second
        
        nanosleep(&tim,&tim2);
    }
    cvReleaseImage(&color_img);
    cvReleaseImage(&gray_img);
    cvReleaseCapture(&cv_cap);
    //cvDestroyWindow("Video");
}

void video_loop(){
    while(1){
        //printf("\a");
        while((m = read(vidsockfd, videoBuffIn, sizeof(videoBuffIn)-1)) > 1)
          {
            //videoBuffIn[] = 0;
            
            //memset(videoBuffOut, '0' ,sizeof(videoBuffOut));
            
            
            //if(fputs(recvBuff, stdout) == EOF)
            
          }
          draw_screen();

        if( n < 0)
          {
            /* error */
          }          
        /* reduce the load on the CPU by a billion */
        
        struct timespec tim, tim2;
        tim.tv_sec  = 0;
        tim.tv_nsec = 40000000L; // 1/20th of a second
        
        nanosleep(&tim,&tim2);
    }
}

void submit_text(){
    
    write(sockfd, text_buf->text, text_buf->length);
    window_buf->chats[window_buf->num_messages]=calloc(1,sizeof(chat_buffer));
    memcpy(window_buf->chats[window_buf->num_messages]->text,
        text_buf->text,MAXLINE);
    window_buf->chats[window_buf->num_messages]->length = text_buf->length;
    window_buf->num_messages++;
    draw_screen();
}

void draw_screen(){
    clear();

    /* no video? :( */
    int i,j;
    for(i=0;i<window_buf->num_messages;i++){
        mvwprintw(main_window, i, 0, window_buf->chats[i]->text);
    }
    touchwin(main_window);
    
    /* if video draw that! */
    //char widthBuffer[VIDEO_WIDTH]; 
    if(video_on){
        //mvwprintw(video_window, 0, 0,"VIDEO HERE");
        for(i=0;i<VIDEO_HEIGHT;i++){
            //memcpy(widthBuffer,videoBuffOut+i*VIDEO_WIDTH,VIDEO_WIDTH);
            //mvwprintw(video_window, i, 0, videoBuffOut);
            for(j=0;j<VIDEO_WIDTH;j++){
                if(j*i > MAXVIDEO)
                    break;
                move(i,j+COLS-VIDEO_WIDTH);
                char c = videoBuffIn[i*VIDEO_WIDTH+j];
                //" .:-=+*#%%@#"
                addch(c); // FIX ME
            }
        }
    }
    
    mvwprintw(type_window, 1, 0, text_buf->text);
    wnoutrefresh(main_window);
    wnoutrefresh(video_window);
    wnoutrefresh(type_window);

    doupdate();
}

void read_loop(){
    while(1){
        //printf("\a");
        while((n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
          {
            recvBuff[n] = 0;
            pthread_mutex_lock(&data_lock);
            
            chat_buffer *rec_buf=calloc(1,sizeof(chat_buffer));
            int j;
            for(j=0;j<n;j++){
                rec_buf->text[j]=recvBuff[j];
            }
            rec_buf->length = n;
            window_buf->chats[window_buf->num_messages] = rec_buf;
            window_buf->num_messages++;
            
            memset(recvBuff, '0' ,sizeof(recvBuff));
           
            pthread_mutex_unlock(&data_lock);
            
            draw_screen();
            
            //if(fputs(recvBuff, stdout) == EOF)
            
          }

        if( n < 0)
          {
            /* error */
          }          
        sleep(1);
    }
}

int main(int argc, char *argv[])
{
    /* initialize the chat client with arguments */
    int port;
    char *user_name;
    char *chat_room;
    chat_room = "none";
    int opt;
    video_on = 0;
    //videoBuffOut = malloc(MAXVIDEO*sizeof(char));
    cv_cap = cvCaptureFromCAM(CV_CAP_ANY);
    
    pthread_mutex_init (&data_lock, NULL);

    /* initialize chat socket */
    memset(recvBuff, '0' ,sizeof(recvBuff));
    memset(videoBuffOut, '0' ,sizeof(videoBuffOut));
    memset(videoBuffIn, '0' ,sizeof(videoBuffOut));
    
    pthread_t recv_audio_thread;
    int val123;
    val123 = pthread_create(&recv_audio_thread, NULL,
        (void*)initialize_audio_recv, NULL);
    
    pthread_t send_audio_thread;
    int val124;
    val124 = pthread_create(&send_audio_thread, NULL,
        (void*)initialize_audio_send, NULL);
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(1337);
    serv_addr.sin_addr.s_addr = inet_addr(ip_address);
    
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }
    
    /* spawn a read loop */
    pthread_t read_thread;
    int val;
    val = pthread_create(&read_thread, NULL, (void*)read_loop, NULL);
    
    /* initializing ncurses buffers */
    if((window_buf = calloc(1,sizeof(window_buffer)))<0)
        exit(1);
    window_buf->num_messages = 0;
    if((text_buf = malloc(sizeof(chat_buffer)))<0)
        exit(1);
    if((window_buf->chats = calloc(1000,sizeof(chat_buffer*)))<0)
        exit(1);
    text_buf->length=0;
    memset(text_buf->text, 0, sizeof(text_buf->text));
    
    /* initialize your non-curses data structures here */

    (void) signal(SIGINT, finish);      /* arrange interrupts to terminate */

    if ((main_window=initscr()) == NULL){      /* init the curses library */
    	fprintf(stderr, "Error initialising ncurses.\n");
    	exit(EXIT_FAILURE);
    }
    keypad(main_window, TRUE);  /* enable keyboard mapping */
    nodelay(main_window, TRUE);
    (void) nonl();         /* tell curses not to do NL->CR/NL on output */
    (void) cbreak();       /* take input chars one at a time, no wait for \n */
    (void) noecho();         

    /* set up structure */
    
    type_window = subwin(main_window, 2, COLS, LINES-2 ,0);

    if (has_colors())
    {
        start_color();

        /*
         * Simple color assignment, often all we need.  Color pair 0 cannot
	 * be redefined.  This example uses the same value for the color
	 * pair as for the foreground color, though of course that is not
	 * necessary:
         */
        init_pair(1, COLOR_RED,     COLOR_BLACK);
        init_pair(2, COLOR_GREEN,   COLOR_BLACK);
        init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
        init_pair(4, COLOR_BLUE,    COLOR_BLACK);
        init_pair(5, COLOR_CYAN,    COLOR_BLACK);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(7, COLOR_WHITE,   COLOR_BLACK);
    }
    //set color
    
    while(-1 != (opt = getopt(argc, argv, "hvp:u:s:t:")))
    {
        switch(opt){
            case 'h':
                printf("sorry no instructions yet :(\n");
                return 0;
            case 'p':
                port = atoi(optarg);
                break;
            case 'u':
                memcpy(text_buf->user,optarg,MAXUSER); //buffer overflow!
                break;
            case 'r':
                chat_room = optarg; //buffer overflow!
                break;
            case 's':
                ip_address = optarg; //buffer overflow!
                break;
            case 'v':
                initialize_video();
                break;
            default:
                break;
        }
    }

    
    for (;;)
    {
        int c = getch();     /* refresh, accept single keystroke of input */
        if (c==13)
        {
            submit_text();
            text_buf->length=0;
            memset(text_buf->text, 0, sizeof(text_buf->text));
            draw_screen();
            
        } else if (c==127)
        {
            if(text_buf->length>=0){
                text_buf->text[text_buf->length] = '\0';
                text_buf->length--;
            }
            draw_screen();
        }
        else if (c>0)
        {
            if(text_buf->length>=MAXLINE)
            {
                submit_text();
                text_buf->length=0;
                memset(text_buf->text, 0, sizeof(text_buf->text));
            }
            
            text_buf->text[text_buf->length] = (char)c;
            text_buf->length++;
            draw_screen();
            
        }
        
        /* reduce the load on the CPU by a billion */
        
        struct timespec tim, tim2;
        tim.tv_sec  = 0;
        tim.tv_nsec = 10000000L; // 1/100th of a second
        
        nanosleep(&tim,&tim2);
    }

    finish(0);               /* we're done */
}

static void finish(int sig)
{
    endwin();

    /* do your non-curses wrapup here */

    exit(0);
}
