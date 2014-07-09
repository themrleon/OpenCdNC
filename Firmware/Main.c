/*The MIT License (MIT)

Copyright (c) 2014 Leonardo Ciocari

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

//COMMAND EXAMPLES (With CNC at reset position "x00000y00000z1"):
//x00147y00147z1 - Invalid, can't move X and Y axis with Z down
//x00147y00147z2 - Valid, can move X and Y axis with Z up (will move X first, then Y)
//x00147y00000z1 - Valid, can move X to 00147 from actual 00000 position
//x00000y00294z1 - Valid, can move Y to 00294 from actual 00000 position

//CNC INFO
//Step motor driver at full step mode - each step = 0,147 mm = 147 mic

#include <htc.h>
#include <string.h>
#include <stdlib.h>

__CONFIG(INTIO & WDTDIS & MCLRDIS & BORDIS & LVPDIS);

#define _XTAL_FREQ 4000000

//Z Axis
#define Z_DIR	RB4	//1 - down, 0 - up
#define Z_STEP 	RB5
unsigned char Z_POS='1'; //'1' = down, '2' = up

//X Axis
#define X_DIR	RB0 //1 - left, 0 - right
#define X_STEP 	RA2	
#define X_CM	68	//Steps for 1 cm
unsigned int X_POS=0;	//Start at zero, upper left position

//Y Axis
#define Y_DIR	RA0	//1 - down, 0 - up
#define Y_STEP 	RA1
#define Y_CM	44	//Steps for 1 cm
unsigned int Y_POS=0;	//Start at zero, upper left position

//--------------------------------
void Xmove(unsigned int pos)
{
 unsigned int n,c;

 if(X_POS < pos)
  X_DIR=1;
 else
  X_DIR=0;

 //Get absolute value
 c=abs(pos-X_POS);

 for(n=0; n<c; n++)
 {
  X_STEP=1;
  __delay_ms(2);
  X_STEP=0;
  __delay_ms(2);

  if(X_DIR==1)
   X_POS++;
  else
   X_POS--;
 }
}

//--------------------------------
void Ymove(unsigned int pos)
{
 unsigned int n,c;

 if(Y_POS < pos)
  Y_DIR=1;
 else
  Y_DIR=0;

 //Get absolute value
 c=abs(pos-Y_POS);

 for(n=0; n<c; n++)
 {
  Y_STEP=1;
  __delay_ms(2);
  Y_STEP=0;
  __delay_ms(2);

  if(Y_DIR==1)
   Y_POS++;
  else
   Y_POS--;
 }
}

//--------------------------------
void Zmove(unsigned char pos)
{
 unsigned char n;

 if(Z_POS != pos)
 {
  if(pos == '1')
  {
   Z_DIR=1;
   Z_POS = '1';
  
   for(n=0; n<10; n++)
   {
    Z_STEP=1;
    __delay_ms(5);
    Z_STEP=0;
    __delay_ms(5);
   }
  }

  if(pos == '2')
  {
   Z_DIR=0;
   Z_POS = '2';
   
   for(n=0; n<10; n++)
   {
    Z_STEP=1;
    __delay_ms(2);
    Z_STEP=0;
    __delay_ms(2);
   }
  }
 }
}

//--------------------------------
void ScanUart()
{
 unsigned char x[6],y[6],z,tmp;
 unsigned int x_pos,y_pos;

 tmp=RCREG;
 
 if(tmp=='x')
 {
  //Get X
  while(RCIF!=1); x[0]=RCREG;
  while(RCIF!=1); x[1]=RCREG;
  while(RCIF!=1); x[2]=RCREG;
  while(RCIF!=1); x[3]=RCREG;
  while(RCIF!=1); x[4]=RCREG;

  //Get Y
  while(RCIF!=1); tmp=RCREG;
  while(RCIF!=1); y[0]=RCREG;
  while(RCIF!=1); y[1]=RCREG;
  while(RCIF!=1); y[2]=RCREG;
  while(RCIF!=1); y[3]=RCREG;
  while(RCIF!=1); y[4]=RCREG;

  //Get Z
  while(RCIF!=1); tmp=RCREG;
  while(RCIF!=1); z=RCREG;
 
  Zmove(z);	//First of X and Y, move Z axis

  //Convert from mic to steps
  x_pos=(atol(x)/(10000/X_CM));
  y_pos=(atol(y)/(10000/Y_CM));

  //If is a single axis move command (X or Y) move
  if( (X_POS == x_pos) || (Y_POS == y_pos) )
  {
   if( X_POS != x_pos )
    Xmove(x_pos);

   if( Y_POS != y_pos )
    Ymove(y_pos);
  }
  else if(z == '2') //else is a two axis move command, but only move if Z axis is down
  {
   Xmove(x_pos);	//First X then Y
   Ymove(y_pos);
  }

  while(TXIF!=1); TXREG='*';	//After execute command succesful, send back to software a flag '*'
 }
}

//---------------------------------------------------
int main()
{
 TRISA=0b00000000;

 //PORTB
 RBPU=0;	//Pull-up on
 TRISB=0b00000010;
 PORTB=0b00000000;

 //Turn off analog comparator
 CM2=1;
 CM1=1;
 CM0=1;

 //USART
 BRGH=1;	//High Speed
 SPBRG=25;	//9600 at 4Mhz
 SYNC=0;
 SPEN=1;	//USART enabled
 TXEN=1;
 CREN=1;
 
 //Wait 1 second (circuit stabilization)
 __delay_ms(250);
 __delay_ms(250);
 __delay_ms(250);
 __delay_ms(250);

 X_DIR=0;
 Y_DIR=0;
 Z_DIR=1;
 
 X_STEP=0;
 Y_STEP=0;
 Z_STEP=0;

 Y_POS=0;
 X_POS=0;
 Z_POS='1';

 while(10)
 {
  if(OERR==1)	//Avoid serial overflow hangs
  {
   CREN=0;
   CREN=1;
  }

  if(RCIF==1)	//Wait serial receive something
   ScanUart();
 }

}
