#include "sys/sys.h"
#include "subs/mem/mem.h"
#include "subs/audio/sid/csid.h"
#include "subs/file/tinf.h"
#include "subs/r3d/r3d.h"


extern char mod_start[];
extern int mod_size;

#define CLIENT_WIDTH 600
#define CLIENT_HEIGHT 300
#define PLAYBACK_SAMPLES 1000


static unsigned int read_le32(const unsigned char *p)
{
	return ((unsigned int) p[0])
	     | ((unsigned int) p[1] << 8)
	     | ((unsigned int) p[2] << 16)
	     | ((unsigned int) p[3] << 24);
}

int main(int argc, char* argv[])
{

    int res = 0x0;
    unsigned int size = 0x0;
    unsigned char *dest = 0x0;

    RD_Context context;
    RD_Client client;
    RD_Cell bClr;
    SDL_AudioSpec snd_fmt;

    RD_CreateClient(CLIENT_WIDTH, CLIENT_HEIGHT, 1, 1,
                        "Blank Demo", &client);
    RD_CreateContext(client, RD_VIDEO | RD_TIMER, &context);

    size = read_le32(&mod_start[mod_size - 4]);
    dest = mallocate(sizeof(char) * size);
    res = tinf_gzip_uncompress(dest, &size, mod_start, mod_size);

    loadSID(dest, size);
  
    snd_fmt.freq = samplerate;
	snd_fmt.format = AUDIO_S16;
	snd_fmt.channels = 1;
	snd_fmt.samples = 1000;
	snd_fmt.callback = sid_play;
	snd_fmt.userdata = NULL;
    
    bClr = RD_CreateColor(0x0, 0x0, 0x0);
  
    if(!SDL_OpenAudio(&snd_fmt, NULL)){

        SDL_PauseAudio(0);
   
        RD_Video_Subsystem *video = (RD_Video_Subsystem *)
                                    context.sys->video;
        while(RD_PollEvents(&context)){
        RD_Begin(&context);
        RD_FillRect(0, 0, client.w, client.h, &bClr, &context);
        RD_Present(&context);

        }
    }
    RD_FreeContext(&context);
  

    return 0;
}