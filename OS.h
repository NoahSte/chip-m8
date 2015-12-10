//OS specific includes//
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <conio.h>
////////////////////////
void init();
void readToRam(unsigned char RAM[]);
void sleep(int ms);
unsigned char randChar();
unsigned char checkKeyPress(unsigned char key);
unsigned char waitKeyPress();
void drawPixel(unsigned char x, unsigned char y, unsigned char state);
void clearScreen();
void buzzer(unsigned char ms);