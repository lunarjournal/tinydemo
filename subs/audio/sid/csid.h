#ifndef CSID_H
#define CSID_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <SDL2/SDL.h>

typedef unsigned char byte;

//global constants and variables
#define C64_PAL_CPUCLK 985248
#define SID_CHANNEL_AMOUNT 3
#define MAX_DATA_LEN 65536
#define PAL_FRAMERATE 49.4	//important to match, otherwise some ADSR-sensitive tunes suffer.
#define DEFAULT_SAMPLERATE 44100

extern int OUTPUT_SCALEDOWN;
//raw output divided by this after multiplied by main volume, this also compensates for filter-resonance emphasis to avoid distotion

enum
{ GATE_BITMASK = 0x01, SYNC_BITMASK = 0x02, RING_BITMASK = 0x04,
  TEST_BITMASK = 0x08, TRI_BITMASK = 0x10, SAW_BITMASK = 0x20,
  PULSE_BITMASK = 0x40, NOISE_BITMASK = 0x80, HOLDZERO_BITMASK = 0x10,
  DECAYSUSTAIN_BITMASK = 0x40, ATTACK_BITMASK = 0x80, LOWPASS_BITMASK = 0x10,
  BANDPASS_BITMASK = 0x20, HIGHPASS_BITMASK = 0x40, OFF3_BITMASK = 0x80
};

extern const byte FILTSW[9];

extern byte ADSRstate[9], expcnt[9], envcnt[9], sourceMSBrise[9];
extern unsigned int clock_ratio, ratecnt[9], prevwfout[9];
extern unsigned long int phaseaccu[9], prevaccu[9], sourceMSB[3], noise_LFSR[9];
extern long int prevlowpass[3], prevbandpass[3];
extern float cutoff_ratio_8580, cutoff_ratio_6581, cutoff_bias_6581;
extern int SIDamount, SID_model[3], requested_SID_model, sampleratio;
extern byte *filedata;
extern byte *memory;
extern byte timermode[0x20],SIDtitle[0x20], SIDauthor[0x20], SIDinfo[0x20];
extern int subtune, tunelength;
extern unsigned int initaddr, playaddr, playaddf, SID_address[3];

extern long int samplerate;
extern int framecnt, frame_sampleperiod;
//CPU (and CIA/VIC-IRQ) emulation constants and variables - avoiding internal/automatic variables to retain speed
extern const byte flagsw[], branchflag[];
extern unsigned int PC, pPC , addr , storadd;
extern short int A, T, SP;
extern byte X, Y, IR, ST ;	//STATUS-flags: N V - B D I Z C
extern char CPUtime,cycles , finished, dynCIA;

//function prototypes
void cSID_init (int samplerate);
int SID (char num, unsigned int baseaddr);
void initSID ();
void loadSID(char data[], int len);
void initCPU (unsigned int mempos);
byte CPU ();
void init (byte subtune);
void sid_play (void *userdata, Uint8 * stream, int len);
unsigned int combinedWF (char num, char channel, unsigned int *wfarray,
			 int index, char differ6581);
void createCombinedWF (unsigned int *wfarray, float bitmul, float bitstrength,
		       float treshold);

#endif