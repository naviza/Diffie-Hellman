#include <Arduino.h>
#include "read_int.h"

/* Adapted from the read_int file presented in class by Dr.Kube */

/*
	Given an array of characters str[] of length len.
	Reads character from the serial monitor until len-1 are
	read or '\r' is encountered and stores them sequentially in str[].
	Adds the null terminator to str[] at the end of this sequence.
*/
void readString(char str[], int len) {
	// we didn't use a 'for' loop because we need the value of 'index' when
	// 'while' exits, so that we know where to add the null terminator '\0'

	int index = 0;
	while (index < len - 1) {
		// if something is waiting to be read on Serial0
		if (Serial.available() > 0) {
			char byteRead = Serial.read();
			Serial.print(byteRead);
			// did the user press enter? break and print a newline
			if (byteRead == '\r') {
				Serial.println();
				break;
			}
			else {
				str[index] = byteRead;
				index += 1;
			}
		}
	}
	str[index] = '\0';
}

// read a string (of digits 0-9) from the serial monitor by reading
// characters until enter is pressed, and return an unsigned 32-bit 'int'
uint32_t readUnsigned32() {
	char str[32];
	readString(str, 32);
	return atol(str);
	// Tricky question: why does this work even when entering 4,000,000,000
	// which fits in an unsigned long but not a signed long (as atol returns)?
}
