#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"
#include "buzzer.h"
#include "led.h"

// WARNING: LCD DISPLAY USES P1.0.  Do not touch!!! 

#define LED BIT6		/* note that bit zero req'd for display */

#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8

#define SWITCHES 15 //The same as (SWITCH1|SWITCH2|SWITCH3|SWITCH4) 

#define BG_COLOR COLOR_BLUE

#define Em 329.63
#define F  349.228
#define C  261.63
#define Am 440
#define G  392
#define Dm 293.66

#define Ab 415.30
#define Gm 392.00
#define Eb 311.13

char blue = 31, green = 0, red = 31;
unsigned char step = 0;

static char 
switch_update_interrupt_sense()
{
  char p2val = P2IN;
  /* update switch interrupt to detect changes from current buttons */
  P2IES |= (p2val & SWITCHES);	/* if switch up, sense down */
  P2IES &= (p2val | ~SWITCHES);	/* if switch down, sense up */
  return p2val;
}

void 
switch_init()			/* setup switch */
{  
  P2REN |= SWITCHES;		/* enables resistors for switches */
  P2IE |= SWITCHES;		/* enable interrupts from switches */
  P2OUT |= SWITCHES;		/* pull-ups for switches */
  P2DIR &= ~SWITCHES;		/* set switches' bits for input */
  switch_update_interrupt_sense();
}

int switches = 0;

void
switch_interrupt_handler()
{
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;
}


// axis zero for col, axis 1 for row

short drawPos[2] = {1,10}, controlPos[2] = {2, 10};
short colVelocity = 1, colLimits[2] = {1, screenWidth/2};

void singsong(double *note, int *temp, int size){
  
  for (int i = 0; i < size; i++){
    buzzer_set_period(1000000 / note[i]);
    int time = temp[i];

    while (time--) {
      __delay_cycles(10000);
    }

    buzzer_set_period(0);
  }
  
}

int cCol = screenHeight >> 1;
int cRow = screenWidth >> 1;

void pic1(){
  clearScreen(COLOR_PURPLE);
  for (int i = 0; i <= cRow; i++){
    //fillRectangle(cCol - i, cRow - i,30,30,COLOR_LIGHT_BLUE);
    for (int j = i-1; j <= i; j++){
      fillRectangle(cCol + i, cRow - j,30,30,COLOR_LIGHT_BLUE);
    }
  }
  
}

void pic2(){//cow
  clearScreen(COLOR_BROWN);
  fillRectangle(0,30,30,20,COLOR_TAN);
  fillRectangle(100,30,30,20,COLOR_TAN);
  fillRectangle(0,50,30,20,COLOR_WHITE);//left eye
  fillRectangle(0,50,15,20,COLOR_BLACK);
 
  fillRectangle(100,50,30,20,COLOR_WHITE);//right eye
  fillRectangle(115,50,15,20,COLOR_BLACK);

  fillRectangle(30,85,70,50,COLOR_TAN);//mouth
  fillRectangle(20,95,90,40,COLOR_TAN);
  fillRectangle(30,95,70,40,COLOR_GRAY);
  fillRectangle(30,95,20,20,COLOR_BLACK);
  fillRectangle(80,95,20,20,COLOR_BLACK); 

  drawString5x7(60,0, "COW" ,COLOR_WHITE,COLOR_BROWN); 
  
  
}

void pic3(){//creeper
  clearScreen(COLOR_LIME_GREEN);
  fillRectangle(10,30,40,40,COLOR_BLACK);

  fillRectangle(80,30,40,40,COLOR_BLACK);//end eyes

  fillRectangle(50,70,30,50,COLOR_BLACK);

  fillRectangle(27,90,23,50,COLOR_BLACK);

  fillRectangle(80,90,23,50,COLOR_BLACK);
  
}

void pic4(){
  clearScreen(COLOR_PURPLE);
  fillRectangle(90,90,50,50,COLOR_ORANGE);
}

void song1(){//i wonder

  pic1();
  double song[] = {Em, F, C, Am, Em, F, C, Am, G, Dm, Am, Em};
  int time[]  = {600,600,1200, 1200, 600, 600, 1200, 1200, 800, 800, 1300, 1300};

  int amount = sizeof(song) / sizeof(song[0]);
  int times = 2;
  while (times--){ 
    singsong(song, time, amount);
  }
  //pic1();
}

void song2(){//everything i am

  pic2();
  double song[] = {Ab, Gm, Eb, Gm};
  int time[] = {670, 670, 670, 1000};

  int amount = sizeof(song) / sizeof(song[0]);
  int times = 3;
  while (times--){
    singsong(song, time, amount);
  }
  //pic2();
}

void song3(){//cat

  pic3();
  double song[] = {659, 784, 880, 784, 659, 587, 523};
  int time[] = {200, 200, 200, 200, 200, 200, 400};

  int amount = sizeof(song) / sizeof(song[0]);
  int times = 3;
  while (times--) {
    singsong(song, time, amount);
  }
}

void song4(){

  pic4();
  double song[] = {200,400,100,600};
  int time[] = {500,200,700,100};

  int amount = sizeof(song) / sizeof(song[0]);
  singsong(song, time, amount);
  //pic4();
}


short redrawScreen = 1;
u_int controlFontColor = COLOR_GREEN;

void wdt_c_handler()
{
  static int secCount = 0;

  //int song[] = {300, 1000, 200, 100};
  secCount ++;
  if (secCount >= 10) {		/* 10/sec */

    {				/* update hourglass */
      if (switches & SW3) song3();
      if (switches & SW2) song2();
      if (switches & SW1) song1();
      if (step <= 50)
	step ++;
      else
	step = 0;
      secCount = 0;
    }
    if (switches & SW4) return;
    redrawScreen = 1;
  }
}
  
void update_shape();

void main()
{
  
  P1DIR |= LED;		/**< Green led on when CPU on */
  P1OUT |= LED;
  configureClocks();
  lcd_init();
  switch_init();
  buzzer_init();
  
  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */
  
  clearScreen(COLOR_BLUE);
  while (1) {			/* forever */
    if (redrawScreen) {
      redrawScreen = 0;
      //clearScreen(COLOR_RED);
      //update_shape();
    }

    
    P1OUT &= ~LED;	/* led off */
    or_sr(0x10);	/**< CPU OFF */
    P1OUT |= LED;	/* led on */
  }
}    
void
update_shape()
{
  //screen_update_ball();
  //screen_update_hourglass();
}
   


void
__interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & SWITCHES) {	      /* did a button cause this interrupt? */
    P2IFG &= ~SWITCHES;		      /* clear pending sw interrupts */
    switch_interrupt_handler();	/* single handler for all switches */
  }
}
