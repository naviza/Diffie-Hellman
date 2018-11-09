#include <Arduino.h>
#include "read_int.h"

const int gen_pin = 1;

void setup() {
    /*
        sets up the gen_pin as an input and initializes the
        2 serial ports to be used by the code. It flushed
        both ports to ensure that the buffer is cleared before
        the user enters anything.
    */
	init();
	pinMode(gen_pin, INPUT);
	Serial.begin(9600);
	Serial3.begin(9600);
	Serial.flush();
	Serial3.flush();
}

/* creates an array stored in memory with
the binary representation of the input b
NOTE: the least significant bit is on the left*/
void num_to_binary(uint32_t b, int bin_arr[]){
		int remain;
		for(int i = 0; i < 32; i++){
			remain = b % 2;
			b = b / 2;
			*(bin_arr + i) = remain;
		}
	}

	/* when calculating a^b mod m, it is
	necessary to determine the modular congruent numbers
	as described in class so as to prevent bit overflow
	- will make alter c_arr in the stack of the function that called it
	*/
void modular_congruent(uint32_t a, uint32_t c_arr[], uint32_t m) {
	  uint32_t kangaroo = a % m;  // pls dont deduct marks
	  for(int i = 0; i < 32; i++){
	    *(c_arr + i) = kangaroo;  // kangaroo sounds like congruent
	    kangaroo = kangaroo * kangaroo;
	    kangaroo = kangaroo % m;
		}
	}
/*
	Compute and return (a to the power of b) mod m.
 	Assumes 1 <= m < 2^16 (i.e. that m fits in a uint16_t).
	Example: powMod(2, 5, 13) should return 6.
	Performs the operation:
			(a^b) mod m
	Inputs: a, b, m - 32 bit, unsigned integers
	Output : result - the value calculated by the operation
*/

// multiplies num 1 and 2 then mods them with m
// perform (num1 * num2) mod m
uint32_t modding(uint32_t num1, uint32_t num2, uint32_t m){
	uint32_t result = (num1 * num2) % m;
	return result;
}

uint32_t powMod(uint32_t a, uint32_t b, uint32_t m) {

	int bin_arr[32];  // binary representation of b. LSB on the left
	num_to_binary(b, bin_arr);
	uint32_t cong_arr[32];  // an array of modular conguents of a
	uint32_t result = 1;
	modular_congruent(a, cong_arr, m);

	for (int i=0;i<32;i++) {
		if (bin_arr[i] == 1) {
			result = modding(cong_arr[i], result, m);
		}
	}
	return result;
}

/*
For each index in the length, write a bit to the key from the gen_pin
(analog pin connected to nothing for randomness). It does so by reading a
random integer from gen_pin. This integer is converted to binary
and the random bit to the least significant bit of the key

Input: bits - how many bits the key is supposed to be
*/
uint32_t gen_key(uint32_t bits) {
  // Start with an empty key
  uint32_t key = 0;
  for (uint32_t i = 0; i < bits; i++) {
    // Read the least significant bit from the analog pin
    int bit = bitRead(analogRead(gen_pin), 0);
    // Shift the bits to the left by 1 bit to make room for a new bit
    key = key << 1;
    // Write the random bit to the least significant bit of the key
    bitWrite(key, 0, bit);
    // Delay 50ms to give the analog pin time to fluctuate
    delay(50);
  }
  // Return the key
  return uint32_t(key);
}


/*
		- Prompt the user to enter the other arduino's public key
		- Wait until the other other arduino's public key is entered
		and return key is pressed before continuing   ///MAYBE EDIT HERE LATER

		Outputs: sharedSecretKey - the 16 bit key for encrpting and decrypting
																(only 16 bits for Part 1)
*/
uint32_t diffieHellman() {
	const uint32_t p = 19211;
	const uint32_t g = 6;

	// step 1 of setup procedure
	// Generate a 16 bit random number
	uint32_t myPrivateKey = gen_key(16);

	// step 2 of setup procedure
	// Compute public key
    // print this message so the users know that it isn't frozen
	Serial.println("Generating private key, please wait...");
	uint32_t myPublicKey = powMod(g, myPrivateKey, p);

	// step 3 of setup procedure
	// print the public key to the screen
	Serial.print("Your public key is: ");
	Serial.println(myPublicKey);


	Serial.print("Enter the other arduino's public key: ");
	// step 4 of setup procedure
	uint32_t otherPublicKey = readUnsigned32();  // should read from serial mon

	// step 5 of setup procedure
	uint16_t sharedSecretKey = ((uint16_t)powMod(otherPublicKey, myPrivateKey, p));

	return uint16_t(sharedSecretKey);  // shared key is 16 bits for part 1
}


/*
		recieves an unencrypted byte from the user and send this to the
		other arduino using Serial3.write
		Inputs: byte - the unencrypted byte to be sent
						sharedKey - the key used to encrypt using bitwise XOR
*/
void send_byte(uint16_t byte, uint16_t sharedKey) {
	uint16_t encrypted_byte;
	encrypted_byte = (byte ^ sharedKey);  // perform bitwise XOR
	Serial3.write(encrypted_byte);
}


/*
		reads an input from the serial-monitor and prints it to the serial-monitor.
		encrypts the bytes by calling the user-defined function for every byte

		Inputs: sharedKey - the key used to encrypt using bitwise XOR
*/
void sending_bytes(uint16_t sharedKey) {  // shared key is 16 bits for part 1
    char byteSend = Serial.read();
		Serial.print(byteSend);
		if (static_cast<int>(byteSend) == 13) {    // if 'enter' is pressed
            /* print a newline character to user's serial mon
            as well as send this character to the other arduino*/
			send_byte(byteSend, sharedKey);
			Serial.print("\n");
			uint16_t newline = 10;
			send_byte(newline, sharedKey);
		} else {
			send_byte(byteSend, sharedKey);
	}
}

/*
		reads a byte from the other arduino (Serial3) and performs bitwise XOR
		using sharedKey to decrypt a message. This message is then printed
		on to the user's own serial-monitor.

		Inputs: sharedKey - the key used to encrypt using bitwise XOR
*/
void recieving_bytes(uint16_t sharedKey) {  // shared key is 16 bits for part 1
    char byteGet = Serial3.read();
    byteGet = (byteGet ^ sharedKey);  // perform bitwise XOR
		// print the unencrypted byte to arduino's own serial monitor
        Serial.print(byteGet);
}


/*
		Obtains a the key to encrypt and decrypt by calling diffieHellman().
		Prints to the user's own serial-monitor and goes through the infinite loop
		of reading the other arduino's message by calling recieving_bytes()
		and writing to the other arduino by calling sending_bytes.

		Output - 0; for consistency
*/
int main() {
	setup();

	uint16_t sharedKey = diffieHellman();  // shared key is 16 bits for part 1
	Serial.print("Your Shared key is: ");
	Serial.println(sharedKey);

	while (true) {
        // if there are bytes in the user's arduino buffer
		if (Serial.available() > 0) {
			sending_bytes(sharedKey);
		}

        // if there are bytes recieved from other arduino
		if (Serial3.available() > 0) {
			recieving_bytes(sharedKey);
		}
	}
	Serial.end();
	return 0;
}
