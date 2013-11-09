/*
	send udp messages
	This sends a sequence of messages (the # of messages is defined in MSGS)
	The messages are sent to a port defined in SERVICE_PORT 

        usage:  udp-send

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "port.h"


#define BUFLEN 2048
#define MSGS 5	/* number of messages to send */

/* AUDIO */
#include "/Users/bwasti/Desktop/portaudio/include/portaudio.h"

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
//      for( i=0; i<framesToCalc; i++ )
//      {
//          *rptr++ = SAMPLE_SILENCE;  /* left */
//          if( NUM_CHANNELS == 2 ) *rptr++ = SAMPLE_SILENCE;  /* right */
//      }
  }

  if (sendto(data->fd, inputBuffer, (sizeof(SAMPLE) * framesPerBuffer * NUM_CHANNELS), 0, (struct sockaddr *) (data->remaddr), data->slen)==-1) {
    perror("sendto");
    exit(1);
  }

  return paContinue;
}

int main(void)
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
	char *server = "127.0.0.1";	/* change this to use a different server */

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
  printf("\n=== Now listenning!! Please speak into the microphone. ===\n"); fflush(stdout);

  while( ( err = Pa_IsStreamActive( stream ) ) == 1 )
  {
    Pa_Sleep(1000);
    printf("index = \n"); fflush(stdout);
  }
  if( err < 0 ) exit(1);

  err = Pa_CloseStream( stream );
  if( err != paNoError ) exit(1);
	/* now let's send the messages */

	//close(fd);
	return 0;
}
