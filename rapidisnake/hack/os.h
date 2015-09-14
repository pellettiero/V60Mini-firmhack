
//Per key intensity. 0xfff = off, 0 = fully on
extern int ledIntensity[];

#define LM_WASD 0
#define LM_ALLON 1
#define LM_BREATHE 2
#define LM_PRESS 3
#define LM_PRESSFADE 4
#define LM_DIMFADE 5
#define LM_ROWCOL 6
extern char lightMode;

extern char keyBitmap[];

int flashorg(int subfn, int startAddr, int endAddr);

