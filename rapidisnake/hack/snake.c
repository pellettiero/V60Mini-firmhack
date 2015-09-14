#include "os.h"
#include "snake.h"

//Quick and dirty Snake implementation

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */


//We use the keys that line up somewhat linear to make a 10x5 playfield. These are
//offsets in the PWM table for the specified positions.
const char playfield[]={
	 0, 8,16,24,32,-1,40,48,56,64,
	 9,17,25,33,41,49,57,65,73,81,
	10,18,26,34,42,50,58,66,74,82,
	11,19,27,35,43,51,59,67,75,83,
	20,28,36,44,52,60,68,76,84,92,
};

//Throughout the code, we use both nibbles of a char to indicate a position.
//These routines decode that.
#define XP(p) (p&0xf)
#define YP(p) (p>>4)


//Set the PWM value of the led at pos to the specified PWM value.
static void setLed(char p, int val) {
	int pos;
	pos=XP(p)+YP(p)*10;
	if (pos<0 || pos>50) return;
	if (playfield[pos]<0) return;
	ledIntensity[playfield[pos]]=val;
}


#define ST_NOTINMODE 0
#define ST_PLAYING 1
#define ST_DEAD 2

//State of the snake game
struct snakeStStruct {
	int len;
	char snakeElPos[64];
	char foodPos;
	int delayCtr;
	int blinkCtr;
	int dirx;
	int diry;
	char state;
} snakeSt;


static void gameInit() {
	int x;
	for (x=0; x<0xA5; x++) setLed(x, 0xfff); //Quick and dirty clear screen
	snakeSt.len=2;
	//Reset snake element pos
	for (x=0; x<snakeSt.len; x++) {
		snakeSt.snakeElPos[x]=0x20+(x+5);
	}
	snakeSt.foodPos=0x24;
	snakeSt.delayCtr=0;
	snakeSt.dirx=-1;
	snakeSt.diry=0;
}


void snakeSetup() {
	snakeSt.state=ST_NOTINMODE;
}


//Quick and dirty rand() implementation
static int randnext;
int rand(void) {
	randnext = randnext * 1103515245 + 12345;
	return (unsigned int)(randnext/65536) % 32768;
}


//Uses USB scan codes - http://www.mindrunway.ru/IgorPlHex/USBKeyScan.pdf
void snakeKeydown(int key) {
	if (key==0x52 && snakeSt.diry==0) {//up
		snakeSt.dirx=0; snakeSt.diry=-1;
	} else if (key==0x51 && snakeSt.diry==0) {//down
		snakeSt.dirx=0; snakeSt.diry=1;
	} else if (key==0x50 && snakeSt.dirx==0) {//left
		snakeSt.dirx=-1; snakeSt.diry=0;
	} else if (key==0x4F && snakeSt.dirx==0) {//right
		snakeSt.dirx=1; snakeSt.diry=0;
	} else if (key==0x2C && snakeSt.state==ST_DEAD) {
		gameInit();
		snakeSt.state=ST_PLAYING;
	}
	//Also whack the random number generator
	randnext^=(snakeSt.delayCtr+(snakeSt.delayCtr<<16));
}


//Called in the kbds idle loop.
void snakeTick() {
	int i, px, py;

	if (snakeSt.state==ST_NOTINMODE && lightMode==LM_WASD) {
		//Yay! Start a game of snake.
		gameInit();
		snakeSt.state=ST_DEAD;
	}
	//Check if user changed mode away from wasd
	if (lightMode!=LM_WASD) snakeSt.state=ST_NOTINMODE;

	//Not active. Return.
	if (snakeSt.state==ST_NOTINMODE) return;

	//make food key blink
	snakeSt.blinkCtr++;
	setLed(snakeSt.foodPos, 0xfff-(snakeSt.blinkCtr&0x1ff));

	//should the snake move?
	snakeSt.delayCtr++;
	if (snakeSt.delayCtr<600) return; //Nope -> we're done
	snakeSt.delayCtr=0;

	//Also don't move if snake is dead.
	if (snakeSt.state==ST_DEAD) return;

	//Kill end of snake LED
	setLed(snakeSt.snakeElPos[snakeSt.len-1], 0xfff);
	//Move snake forward by one.
	for (i=snakeSt.len; i>0; --i) snakeSt.snakeElPos[i]=snakeSt.snakeElPos[i-1];
	//Calculate new pos for head
	px=XP(snakeSt.snakeElPos[0])+snakeSt.dirx;
	py=YP(snakeSt.snakeElPos[0])+snakeSt.diry;
	//Wraparound if needed
	if (px<0) px=9;
	if (px>9) px=0;
	if (py<0) py=4;
	if (py>4) py=0;
	//Dim old head
	setLed(snakeSt.snakeElPos[0], 0xaff);
	//Set new head
	snakeSt.snakeElPos[0]=(py<<4)+px;
	//and light it up
	setLed(snakeSt.snakeElPos[0], 0);

	//Check for the snake eating itself
	for (i=1; i<snakeSt.len; i++) {
		if (snakeSt.snakeElPos[i]==snakeSt.snakeElPos[0]) snakeSt.state=ST_DEAD;
	}

	//Check for noms
	if (snakeSt.snakeElPos[0]==snakeSt.foodPos) {
		int inSnake=1;
		snakeSt.len++;
		//Place new noms, but not inside snake
		while(inSnake) {
			i=rand();
			px=i%10;
			py=(i>>4)%5;
			snakeSt.foodPos=(py<<4)+px;
			inSnake=0;
			for (i=0; i<snakeSt.len; i++) {
				if (snakeSt.snakeElPos[i]==snakeSt.foodPos) inSnake=1;
			}
		}
	}
}
