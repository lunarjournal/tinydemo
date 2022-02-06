#include "sys/sys.h"
#include "subs/mem/mem.h"
#include "subs/audio/hxcmod.h"
#include "subs/file/tinf.h"
#include "subs/r3d/r3d.h"

#include "subs/audio/mouth/reciter.h"
#include "subs/audio/mouth/sam.h"

extern char mod_start[];
extern int mod_size;

#define CLIENT_WIDTH 600
#define CLIENT_HEIGHT 300
#define PLAYBACK_SAMPLES 500

#define MOUTH_RATE 22050
#define MOUTH_BUF_SIZE 256

modcontext ctx;
int pos = 0;
char mouth_buf[MOUTH_BUF_SIZE];


static unsigned int read_le32(const unsigned char *p)
{
    
	return ((unsigned int) p[0])
	     | ((unsigned int) p[1] << 8)
	     | ((unsigned int) p[2] << 16)
	     | ((unsigned int) p[3] << 24);
}

void mouth_mix(void *unk, Uint8 *stream, int len){
    
    int bufferpos = GetBufferLength();
    char *buffer = GetBuffer();
    int i;
    if (pos >= bufferpos) return;
    if ((bufferpos-pos) < len) len = (bufferpos-pos);
    for(i=0; i<len; i++)
    {
        stream[i] = buffer[pos];
        pos++;
    }
}

void snd_mix(void *unk, Uint8 *stream, int len)
{
	hxcmod_fillbuffer(&ctx, (unsigned short*)stream, len /2 , 0);
}

int main(int argc, char* argv[])
{
    int i = 0x0;
    int res = 0x0;
    unsigned int size = 0x0;
    unsigned char *dest = 0x0;
    msample *buffer = 0x0;
    tracker_buffer_state state;
    RD_Context context;
    RD_Client client;
    RD_Cell bClr;
    SDL_AudioSpec snd_fmt;
    SDL_AudioSpec mouth_fmt;

    memclear(&ctx, 0, sizeof(modcontext));
    memclear(&state, 0, sizeof(tracker_buffer_state));
    memclear(&snd_fmt, 0, sizeof(SDL_AudioSpec));
    memclear(&mouth_fmt, 0, sizeof(SDL_AudioSpec));
    memclear(mouth_buf, 0, sizeof(char) * MOUTH_BUF_SIZE);

    hxcmod_init( &ctx );
    hxcmod_setcfg(&ctx, SAMPLE_RATE , 0, 0);

    size = read_le32(&mod_start[mod_size - 4]);
    dest = mallocate(sizeof(char) * size);
    res = tinf_gzip_uncompress(dest, &size, mod_start, mod_size);

    buffer = mallocate(sizeof(msample) * SAMPLE_RATE );

    snd_fmt.freq = SAMPLE_RATE;
	snd_fmt.format = AUDIO_S16;
	snd_fmt.channels = 1;
	snd_fmt.samples = PLAYBACK_SAMPLES/2;
	snd_fmt.callback = snd_mix;
	snd_fmt.userdata = NULL;

    mouth_fmt.freq = MOUTH_RATE;
    mouth_fmt.format = AUDIO_U8;
    mouth_fmt.channels = 1;
    mouth_fmt.samples = 2048;
    mouth_fmt.callback = mouth_mix;
    mouth_fmt.userdata = NULL;
    
  
    for(i=0; i<256; i++) mouth_buf[i] = 0;
    strcpy(mouth_buf, "Welcome to the loader");
  
    // config sam
    SetPitch(255);

    TextToPhonemes((unsigned char *)mouth_buf);
    SetInput(mouth_buf);
    SAMMain();
    int bufferpos = GetBufferLength();
     bufferpos /= 50;
     SDL_OpenAudio(&mouth_fmt, NULL);
     SDL_PauseAudio(0);
       while (pos < bufferpos)
    {
        SDL_Delay(100);
    }
    SDL_CloseAudio();

    if(hxcmod_load(&ctx, dest, size) 
       && !SDL_OpenAudio(&snd_fmt, NULL)){
        SDL_PauseAudio(0);
          bClr = RD_CreateColor(0x0, 0x0, 0x0);
        RD_CreateClient(CLIENT_WIDTH, CLIENT_HEIGHT, 1, 1,
                    "Empty Project", &client);
        RD_CreateContext(client, RD_VIDEO | RD_TIMER | RD_AUDIO, &context);
        RD_Video_Subsystem *video = (RD_Video_Subsystem *)
                                    context.sys->video;

        while(RD_PollEvents(&context)){
        RD_Begin(&context);
        RD_FillRect(0, 0, client.w, client.h, &bClr, &context);
        RD_Present(&context);

        }
    }

    RD_FreeContext(&context);
    hxcmod_unload(&ctx);

    return 0;
}