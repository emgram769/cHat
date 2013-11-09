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
#include <stdio.h>

#define MAXLINE 100
#define MAXUSER 50
#define VIDEO_HEIGHT    40
#define VIDEO_WIDTH     100
#define MAXVIDEO    VIDEO_HEIGHT*VIDEO_WIDTH


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
    video_window = subwin(main_window, 10, 0, 0 ,0);
    
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
    
        
    draw_screen();
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
        tim.tv_nsec = 40000000L; // 1/20th of a second
        
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
        while((m = read(vidsockfd, videoBuffIn, sizeof(videoBuffIn)-1)) > 0)
          {
            //videoBuffIn[] = 0;
            
            //memset(videoBuffOut, '0' ,sizeof(videoBuffOut));
            
            draw_screen();
            
            //if(fputs(recvBuff, stdout) == EOF)
            
          }

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
                move(i,j);
                addch(videoBuffIn[i*VIDEO_WIDTH+j]); // FIX ME
            }
        }
    }
    
    mvwprintw(type_window, 1, 0, text_buf->text);
    
    wrefresh(main_window);
    wrefresh(video_window);
    wrefresh(type_window);
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