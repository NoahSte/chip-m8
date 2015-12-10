#include "OS.h"
//OS specific initialisations//
unsigned char keys[] = {'x', '1', '2', '3', 'q', 'w', 'e', 'a', 's', 'd', 'z', 'c', '4', 'r', 'f', 'v'};
struct _COORD pos;
///////////////////////////////

//Gives the OS an opportunity to initialise certain things
void init() {
	srand(time(NULL));
}

//Loads the selected program to RAM
void readToRam(unsigned char RAM[]) {
	unsigned char filename[256];
	printf("Select a file to open:\n");
	scanf("%s", &filename);
	FILE *file;
	file = fopen(filename, "rb");
	unsigned int c;
	for(int i = 0x200; (c = fgetc(file)) != EOF; i++) {
		RAM[i] = c;
	}
}

//Sleep for ms Milliseconds
void sleep(int ms) {
	Sleep(ms);
}

//Return a random generated character
unsigned char randChar() {
	return (rand() % 255);
}

//Checks if the given key is pressed, if it is return 1 else 0
unsigned char checkKeyPress(unsigned char key) {
	if(kbhit()) {
		return (keys[key] == getch());
	} else {
		return 0;	
	}
}

//Wait for a key press and return the value of the pressed key
unsigned char waitKeyPress() {
	while(1) {
		char c = getch();
		for(int i = 0; i < 0x10; i++) {
			if(c == keys[i]) {
				return i;
			}
		}
	}
}

//Turns a certain cell of the console on or off
void drawPixel(unsigned char x, unsigned char y, unsigned char state) {
	if(x < 64 && y < 32) {
		pos.X = x;
		pos.Y = y;
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
		if(state) {
			printf("%c", 219);	
		} else {
			printf(" ");
		}
	}
}

//Clears the screen
void clearScreen() {
	system("cls");
}

//TODO WORK THE BUZZER OUT ---------------------------------------------------
//Set buzzer on if state = 1 else turn buzzer off
void buzzer(unsigned char ms) {
	Beep(500, ms);
}