#include "OS.h"
//0x000 to 0x1FF is used for sprites - 0x200 to 0xFFF is program data
unsigned char RAM[0x1000];
//Registers V0 to VF - VF is used as a flag and should not be used
unsigned char V[0x10];
//Register I is used as a register that stores addresses - uses 12 bits only
unsigned short I;
//Delay Timer - if not 0 subtract 1
unsigned char DT;
//Sound timer - if not 0 subtract 1
unsigned char ST;
//Program Counter - stores currently executing address
unsigned short PC = 0x200;
//Stack Pointer - points to the latest address on the stack
unsigned char SP = 0;
//The stack remembers where to jump back to after a return - 16 times
unsigned short stack[0x10];
//Stores the pixels on the display
unsigned char display[8][32];

void process(unsigned short in) {
	//Stores the possible values into variables
	unsigned short nnn = in & 0x0FFF;
	unsigned char x, y, kk, bytes;
	x = (in & 0x0F00) >> 8;
	y = (in & 0x00F0) >> 4;
	kk = in & 0x00FF;
	bytes = in & 0x000F;
	//Clear display
	if(in == 0x00E0) {
		clearScreen();
		for(int x = 0; x < 8; x++) {
			for(int y = 0; y < 32; y++) {
				display[x][y] = 0;
			}
		}
		return;
	}
	//Returns back to another address
	if(in == 0x00EE) {
		PC = stack[SP];
		//Prevents the Stack Pointer from going negative
		if(SP != 0) {
			SP--;
		}
		return;
	}
	//Jumps to address
	if((in & 0xF000) == 0x1000) {
		PC = nnn;
	}
	//Calls to address and remembers where it left off
	if((in & 0xF000) == 0x2000) {
		stack[++SP] = PC;
		PC = nnn;
		return;
	}
	//Checks if x and kk are equal, if they are skip an instruction
	if((in & 0xF000) == 0x3000) {
		if(V[x] == kk) {
			PC += 2;
		}
		return;
	}
	//Checks if x and kk are NOT equal, if they are skip an instruction
	if((in & 0xF000) == 0x4000) {
		if(V[x] != kk) {
			PC += 2;
		}
		return;
	}
	//Check if x and y are equal, if so skip the next instruction
	if((in & 0xF00F) == 0x5000) {
		if(V[x] == V[y]) {
			PC += 2;
		}
		return;
	}
	//Put kk into register x
	if((in & 0xF000) == 0x6000) {
		V[x] = kk;
		return;
	}
	//Add kk ontop of x
	if((in & 0xF000) == 0x7000) {
		V[x] += kk;
		return;
	}
	//Store y in x
	if((in & 0xF00F) == 0x8000) {
		V[x] = V[y];
		return;
	}
	//x or bitwise y
	if((in & 0xF00F) == 0x8001) {
		V[x] |= V[y];
		return;
	}
	//x and bitwise y
	if((in & 0xF00F) == 0x8002) {
		V[x] &= V[y];
		return;
	}
	//x xor bitwise y
	if((in & 0xF00F) == 0x8003) {
		V[x] ^= V[y];
		return;
	}
	//x + y  with carry
	if((in & 0xF00F) == 0x8004) {
		V[x] += V[y];
		if(V[x] < V[y]) {
			V[0xF] = 1;
		} else {
			V[0xF] = 0;
		}
		return;
	}
	//x -= y with flag
	if((in & 0xF00F) == 0x8005) {
		if(V[x] > V[y]) {
			V[0xF] = 1;
		} else {
			V[0xF] = 0;
		}
		V[x] -= V[y];
		return;
	}
	//shift x one to the right and least significant bit to the flag register
	if((in & 0xF00F) == 0x8006) {
		V[0xF] = V[x] & 0x0001;
		V[x] = V[x] >> 1;
		return;
	}
	//x = y - x with flag
	if((in & 0xF00F) == 0x8007) {
		if(V[x] < V[y]) {
			V[0xF] = 1;
		} else {
			V[0xF] = 0;
		}
		V[x] = V[y] - V[x];
		return;
	}
	//shift x one to the left and most significant bit in the flag
	if((in & 0xF00F) == 0x800E) {
		V[0xF] = V[x] >> 15;
		V[x] = V[y] << 1;
		return;
	}
	//Skip next instruction if x != y
	if((in & 0xF00F) == 0x9000) {
		if(V[x] != V[y]) {
			PC +=2;
		}
		return;
	}
	//Set I to nnn
	if((in & 0xF000) == 0xA000) {
		I = nnn;
		return;
	}
	//Jump to nnn + V0
	if((in & 0xF000) == 0xB000) {
		PC = nnn + V[0x0];
		return;
	}
	//x = RND & kk
	if((in & 0xF000) == 0xC000) {
		V[x] = kk & randChar();
		return;
	}
	//Draw on screen ------------------------------------------------------------------------------- COLLISION not working correctly
	if((in & 0xF000) == 0xD000) {
		unsigned char segmentrep = V[x] / 8;
		unsigned char positionrep = V[x] % 8;
		unsigned char part1rep;
		unsigned char part2rep;
		for(int ypos = 0; ypos < bytes; ypos++) {
			part1rep = RAM[I+ypos] >> positionrep;
			part2rep = RAM[I+ypos] << 8-positionrep;
			//Checks if only one or two segments are being used
			if(!(V[x] % 8)) {
				//Stores the sprites row in the VRAM
				display[segmentrep][V[y]+ypos] ^= RAM[I+ypos];
				//Checks whether or not a pixel has been turned off and sets the flag accordingly
				if(~display[segmentrep][V[y]+ypos] & RAM[I+ypos]) {
					V[0xF] = 1;
				} else {
					V[0xF] = 0;
				}
				//Draws the sprites row on the screen
				for(int i = 0; i < 8; i++) {
					drawPixel(V[x]+i, V[y]+ypos, display[segmentrep][V[y]+ypos] & (0x80 >> i));
				}
			} else {
				//Stores the first part of the sprites row in the VRAM
				display[segmentrep][V[y]+ypos] ^= part1rep;
				//Checks whether the first part of the sprites row turned off a pixel and sets the flag accordingly
				if(~display[segmentrep][V[y]+ypos] & part1rep) {
					V[0xF] = 1;
				} else {
					V[0xF] = 0;
				}
				//Draws the first part of the sprites row on the screen
				for(int i = positionrep; i < 8; i++) {
					drawPixel(V[x]+i-positionrep, V[y]+ypos, display[segmentrep][V[y]+ypos] & (0x80 >> i));
				}
				//Stores the second part of the sprites row in the VRAM
				display[segmentrep+1][V[y]+ypos] ^= part2rep;
				//Checks whether the second part of the sprites row turned off a pixel and sets the flag accordingly
				if(~display[segmentrep+1][V[y]+ypos] & part2rep) {
					V[0xF] = 1;
				} else {
					V[0xF] = 0;
				}
				//Draws the second part of the sprites row on the screen
				for(int i = 0; i < positionrep; i++) {
					drawPixel(V[x]+i+8-positionrep, V[y]+ypos, display[segmentrep+1][V[y]+ypos] & (0x80 >> i));
				}
			}
		}
		return;
	}
	//Checks if key x is pressed, if it is skip next instruction
	if((in & 0xF0FF) == 0xE09E) {
		if(checkKeyPress(V[x])) {
			PC += 2;
		}
		return;
	}
	//Checks if key x is not pressed, if it isn't skip the next instruction
	if((in & 0xF0FF) == 0xE0A1) {
		if(!checkKeyPress(V[x])) {
			PC += 2;
		}
		return;
	}
	//Sets the delay timer to the value of x
	if((in & 0xF0FF) == 0xF007) {
		V[x] = DT;
		return;
	}
	//Wait for a key to be pressed before it continues and stores it in x
	if((in & 0xF0FF) == 0xF00A) {
		V[x] = waitKeyPress();
	}
	//Set the delay timer
	if((in & 0xF0FF) == 0xF015) {
		DT = V[x];
		return;
	}
	//Sound timer is set to x
	if((in & 0xF0FF) == 0xF018) {
		ST = V[x];
		buzzer(ST);
		return;
	}
	//Get I and add x
	if((in & 0xF0FF) == 0xF01E) {
		I += V[x];
		return;
	}
	//Set I to the location of the sprite of x
	if((in & 0xF0FF) == 0xF029) {
		I = 5 * V[x];
		return;
	}
	//Puts the hundreds digit in I, tens in I+1 and ones in I+2
	if((in & 0xF0FF) == 0xF033) {
		RAM[I] = V[x]/100;
		RAM[I+1] = (V[x]-V[x]/100*100)/10;
		RAM[I+2] = V[x]-V[x]/100*100-(V[x]-V[x]/100*100)/10*10;
		return;
	}
	//Store the values of register 0 to x into address I to I + x
	if((in & 0xF0FF) == 0xF055) {
		for(int n = 0; n <= x; n++) {
			RAM[I+n] = V[n];
		}
		return;
	}
	//Store the values of addresses from I to I + x into register 0 to x
	if((in & 0xF0FF) == 0xF065) {
		for(int n = 0; n <= x; n++) {
			V[n] = RAM[I+n];
		}
		return;
	}
}

int main() {
	//Sprites for the characters 0x0 - 0xF
	//0				//1				//2				//3
	RAM[0] = 0xF0;	RAM[5] = 0x20;	RAM[10] = 0xF0;	RAM[15] = 0xF0;
	RAM[1] = 0x90;	RAM[6] = 0x60;	RAM[11] = 0x10;	RAM[16] = 0x10;
	RAM[2] = 0x90;	RAM[7] = 0x20;	RAM[12] = 0xF0;	RAM[17] = 0xF0;
	RAM[3] = 0x90;	RAM[8] = 0x20;	RAM[13] = 0x80;	RAM[18] = 0x10;
	RAM[4] = 0xF0;	RAM[9] = 0x70;	RAM[14] = 0xF0;	RAM[19] = 0xF0;
	//4				//5				//6				//7
	RAM[20] = 0x90;	RAM[25] = 0xF0;	RAM[30] = 0xF0;	RAM[35] = 0xF0;
	RAM[21] = 0x90;	RAM[26] = 0x80;	RAM[31] = 0x80;	RAM[36] = 0x10;
	RAM[22] = 0xF0;	RAM[27] = 0xF0;	RAM[32] = 0xF0;	RAM[37] = 0x20;
	RAM[23] = 0x10;	RAM[28] = 0x10;	RAM[33] = 0x90;	RAM[38] = 0x40;
	RAM[24] = 0x10;	RAM[29] = 0xF0;	RAM[34] = 0xF0;	RAM[39] = 0x40;
	//8				//9				//A				//B
	RAM[40] = 0xF0;	RAM[45] = 0xF0;	RAM[50] = 0xF0;	RAM[55] = 0xE0;
	RAM[41] = 0x90;	RAM[46] = 0x90;	RAM[51] = 0x90;	RAM[56] = 0x90;
	RAM[42] = 0xF0;	RAM[47] = 0xF0;	RAM[52] = 0xF0;	RAM[57] = 0xE0;
	RAM[43] = 0x90;	RAM[48] = 0x10;	RAM[53] = 0x90;	RAM[58] = 0x90;
	RAM[44] = 0xF0;	RAM[49] = 0xF0;	RAM[54] = 0x90;	RAM[59] = 0xE0;
	//C				//D				//E				//F
	RAM[60] = 0xF0;	RAM[65] = 0xE0;	RAM[70] = 0xF0;	RAM[75] = 0xF0;
	RAM[61] = 0x80;	RAM[66] = 0x90;	RAM[71] = 0x80;	RAM[76] = 0x80;
	RAM[62] = 0x80;	RAM[67] = 0x90;	RAM[72] = 0xF0;	RAM[77] = 0xF0;
	RAM[63] = 0x80;	RAM[68] = 0x90;	RAM[73] = 0x80;	RAM[78] = 0x80;
	RAM[64] = 0xF0;	RAM[69] = 0xE0;	RAM[74] = 0xF0;	RAM[79] = 0x80;
	//Opportunity for OS to initialise certain things
	init();
	//Read File an put it in RAM
	readToRam(RAM);
	//Clears the screen
	clearScreen();
	//Set first place in stack to the start
	stack[0] = 0x200;
	//Current Instruction
	unsigned short CI;
	while(1) {
		//Gets the instruction and stores it in CI
		CI = RAM[PC++] << 8;
		CI += RAM[PC++];
		//Processes the current instruction
		process(CI);
		//Decrement the timers by 1 if higher then 0
		if(DT > 0) {
			DT--;
		}
		if(ST > 0) {
			ST--;
		}
	}
}