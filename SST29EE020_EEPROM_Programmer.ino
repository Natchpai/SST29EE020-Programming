#define DATA 12
#define CLK 10
#define LATCH 11
#define name value

#define DQ0 2
#define DQ7 9
#define Write_Enable 13

//7segment pin connect: DQ0: G DQ1: f, ..., DQ6: a, DG7: dp


void setAddress(int address, bool OE = false /* false, OE is HIGH*/) {

  shiftOut(DATA, CLK, MSBFIRST, (address >> 8) | (OE ? 0x00 : 0x80)); // 0x__
  shiftOut(DATA, CLK, MSBFIRST, address);  // 0x __

  digitalWrite(LATCH, 0);
  digitalWrite(LATCH, 1);
  digitalWrite(LATCH, 0);
}

byte readEEPROM(int address) {
  
  for(int pin = DQ0; pin <= DQ7; pin++) {
    pinMode(pin, INPUT);
  }

  setAddress(address, true);
  byte data = 0;
  for(uint8_t pin = DQ7; pin >= DQ0; pin--) {
    data = (data << 1) + digitalRead(pin);
  }

  return data;

}

void readContents(int lastAddress = 255) {

  Serial.println("BASE    x0 x1 x2 x3 x4 x5 x6 x7   x8 x9 xa xb xc xd xe xf");

  delay(100); // delay for setup 

  for(int base = 0; base <= lastAddress; base += 16) { // 0-255 , 2048 is Max address 111 1111 1111 >> base{XXX XXXX} offset{XXXX} >> Hex : 7ff
    byte data[16];
    for(int offset = 0; offset <= 15; offset++) {
      data[offset] = readEEPROM(base + offset);
      delayMicroseconds(1);
    }
    delayMicroseconds(1);
    char buf[80];
    sprintf(buf, "%03x :   %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x", base,
    data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], 
    data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);

    Serial.println(buf);

  }

  Serial.println("**             ** DONE **              **");
}

void writeEEPROM(int address, byte data) {
  for(int pin = DQ0; pin <= DQ7; pin++) {
    pinMode(pin, OUTPUT);
  }

  setAddress(address, false);

  for(int pin = DQ0; pin <= DQ7; pin++) {
    digitalWrite(pin, data & 1); // 1010 1010
    data = data >> 1;                   //  101 0101 bit Right shift
  }
  
  digitalWrite(Write_Enable, LOW);
  delayMicroseconds(10);
  digitalWrite(Write_Enable, HIGH);
  delay(5); // wait for setup to write the next page (SST29EE020)


}

byte digits[] = { 0x7e, 0x30, 0x6d, 0x79, 0x33, 0x5b, 0x5f, 0x70, 0x7f, 0x7b, 0x77, 0x1f, 0x4e, 0x3d, 0x4f, 0x47};
//                   0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f

// write eeprom to set one segment display 0 to f.
void writeOneSegment() { 
  for(int address = 0; address <= 16; address++) {
    writeEEPROM(address, digits[address]);
  }
}


void write4Segmentes() {  

  // Unsign bit +0 -> +255
  // x000 , Counter
  for(int address = 0; address <= 255; address++) {
    writeEEPROM(address, digits[(address % 10)]);
  }

  // x001 , Counter
  for(int address = 0; address <= 255; address++) {
    writeEEPROM(address + 256, digits[(address / 10) % 10]);
  }

  // x010 , Counter
  for(int address = 0; address <= 255; address++) {
    writeEEPROM(address + 512, digits[(address / 100) % 10]);
  }

  // x011 , Counter
  for(int address = 0; address <= 255; address++) {
    writeEEPROM(address + 768, 0);
  }

  Serial.println("@ Programming Unsigned bit Complete!");

  // Sign bit -128 -> +127
  // x100 , Counter
  for(int address = -128; address <= 127; address+=1) {
    writeEEPROM((byte)address + 1024, digits[abs(address) % 10]);
  }

  // x101 , Counter
  for(int address = -128; address <= 127; address+=1) {
    writeEEPROM((byte)address + 1280, digits[abs(address / 10) % 10]);
  }

  // x110 , Counter
  for(int address = -128; address <= 127; address+=1) {
    writeEEPROM((byte)address + 1536, digits[abs(address / 100) % 10]);
  }

  // x111 , Counter
  for(int address = -128; address <= 127; address+=1) {
    if(address < 0) {writeEEPROM((byte)address + 1792, 0x01); }
    else{writeEEPROM((byte)address + 1792, 0); }
  }

  Serial.println("@ Programming Signed bit Complete!");

  delay(1000);

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(DATA, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(LATCH, OUTPUT);
  digitalWrite(Write_Enable, HIGH); //set Pullup Resistor
  pinMode(Write_Enable, OUTPUT); //enable Pin for Prevent data writing during setup
  // setup


  delay(500);

  Serial.println("@ Contents Before");

  readContents(2048);

  Serial.println("@ Writing the contents");

  // writeOneSegment();

  write4Segmentes();

  Serial.println("@ Wait for read contents");

  delay(5000);

  readContents(2048);

}

void loop() {
  // put your main code here, to run repeatedly:

}

/* Results :

BASE    x0 x1 x2 x3 x4 x5 x6 x7   x8 x9 xa xb xc xd xe xf
000 :   7e 30 6d 79 33 5b 5f 70   7f 7b 7e 30 6d 79 33 5b
010 :   5f 70 7f 7b 7e 30 6d 79   33 5b 5f 70 7f 7b 7e 30
020 :   6d 79 33 5b 5f 70 7f 7b   7e 30 6d 79 33 5b 5f 70
030 :   7f 7b 7e 30 6d 79 33 5b   5f 70 7f 7b 7e 30 6d 79
040 :   33 5b 5f 70 7f 7b 7e 30   6d 79 33 5b 5f 70 7f 7b
050 :   7e 30 6d 79 33 5b 5f 70   7f 7b 7e 30 6d 79 33 5b
060 :   5f 70 7f 7b 7e 30 6d 79   33 5b 5f 70 7f 7b 7e 30
070 :   6d 79 33 5b 5f 70 7f 7b   7e 30 6d 79 33 5b 5f 70
080 :   7f 7b 7e 30 6d 79 33 5b   5f 70 7f 7b 7e 30 6d 79
090 :   33 5b 5f 70 7f 7b 7e 30   6d 79 33 5b 5f 70 7f 7b
0a0 :   7e 30 6d 79 33 5b 5f 70   7f 7b 7e 30 6d 79 33 5b
0b0 :   5f 70 7f 7b 7e 30 6d 79   33 5b 5f 70 7f 7b 7e 30
0c0 :   6d 79 33 5b 5f 70 7f 7b   7e 30 6d 79 33 5b 5f 70
0d0 :   7f 7b 7e 30 6d 79 33 5b   5f 70 7f 7b 7e 30 6d 79
0e0 :   33 5b 5f 70 7f 7b 7e 30   6d 79 33 5b 5f 70 7f 7b
0f0 :   7e 30 6d 79 33 5b 5f 70   7f 7b 7e 30 6d 79 33 5b
100 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 30 30 30 30 30 30
110 :   30 30 30 30 6d 6d 6d 6d   6d 6d 6d 6d 6d 6d 79 79
120 :   79 79 79 79 79 79 79 79   33 33 33 33 33 33 33 33
130 :   33 33 5b 5b 5b 5b 5b 5b   5b 5b 5b 5b 5f 5f 5f 5f
140 :   5f 5f 5f 5f 5f 5f 70 70   70 70 70 70 70 70 70 70
150 :   7f 7f 7f 7f 7f 7f 7f 7f   7f 7f 7b 7b 7b 7b 7b 7b
160 :   7b 7b 7b 7b 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 30 30
170 :   30 30 30 30 30 30 30 30   6d 6d 6d 6d 6d 6d 6d 6d
180 :   6d 6d 79 79 79 79 79 79   79 79 79 79 33 33 33 33
190 :   33 33 33 33 33 33 5b 5b   5b 5b 5b 5b 5b 5b 5b 5b
1a0 :   5f 5f 5f 5f 5f 5f 5f 5f   5f 5f 70 70 70 70 70 70
1b0 :   70 70 70 70 7f 7f 7f 7f   7f 7f 7f 7f 7f 7f 7b 7b
1c0 :   7b 7b 7b 7b 7b 7b 7b 7b   7e 7e 7e 7e 7e 7e 7e 7e
1d0 :   7e 7e 30 30 30 30 30 30   30 30 30 30 6d 6d 6d 6d
1e0 :   6d 6d 6d 6d 6d 6d 79 79   79 79 79 79 79 79 79 79
1f0 :   33 33 33 33 33 33 33 33   33 33 5b 5b 5b 5b 5b 5b
200 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 7e 7e
210 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 7e 7e
220 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 7e 7e
230 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 7e 7e
240 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 7e 7e
250 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 7e 7e
260 :   7e 7e 7e 7e 30 30 30 30   30 30 30 30 30 30 30 30
270 :   30 30 30 30 30 30 30 30   30 30 30 30 30 30 30 30
280 :   30 30 30 30 30 30 30 30   30 30 30 30 30 30 30 30
290 :   30 30 30 30 30 30 30 30   30 30 30 30 30 30 30 30
2a0 :   30 30 30 30 30 30 30 30   30 30 30 30 30 30 30 30
2b0 :   30 30 30 30 30 30 30 30   30 30 30 30 30 30 30 30
2c0 :   30 30 30 30 30 30 30 30   6d 6d 6d 6d 6d 6d 6d 6d
2d0 :   6d 6d 6d 6d 6d 6d 6d 6d   6d 6d 6d 6d 6d 6d 6d 6d
2e0 :   6d 6d 6d 6d 6d 6d 6d 6d   6d 6d 6d 6d 6d 6d 6d 6d
2f0 :   6d 6d 6d 6d 6d 6d 6d 6d   6d 6d 6d 6d 6d 6d 6d 6d
300 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
310 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
320 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
330 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
340 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
350 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
360 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
370 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
380 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
390 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
3a0 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
3b0 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
3c0 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
3d0 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
3e0 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
3f0 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
400 :   7e 30 6d 79 33 5b 5f 70   7f 7b 7e 30 6d 79 33 5b
410 :   5f 70 7f 7b 7e 30 6d 79   33 5b 5f 70 7f 7b 7e 30
420 :   6d 79 33 5b 5f 70 7f 7b   7e 30 6d 79 33 5b 5f 70
430 :   7f 7b 7e 30 6d 79 33 5b   5f 70 7f 7b 7e 30 6d 79
440 :   33 5b 5f 70 7f 7b 7e 30   6d 79 33 5b 5f 70 7f 7b
450 :   7e 30 6d 79 33 5b 5f 70   7f 7b 7e 30 6d 79 33 5b
460 :   5f 70 7f 7b 7e 30 6d 79   33 5b 5f 70 7f 7b 7e 30
470 :   6d 79 33 5b 5f 70 7f 7b   7e 30 6d 79 33 5b 5f 70
480 :   7f 70 5f 5b 33 79 6d 30   7e 7b 7f 70 5f 5b 33 79
490 :   6d 30 7e 7b 7f 70 5f 5b   33 79 6d 30 7e 7b 7f 70
4a0 :   5f 5b 33 79 6d 30 7e 7b   7f 70 5f 5b 33 79 6d 30
4b0 :   7e 7b 7f 70 5f 5b 33 79   6d 30 7e 7b 7f 70 5f 5b
4c0 :   33 79 6d 30 7e 7b 7f 70   5f 5b 33 79 6d 30 7e 7b
4d0 :   7f 70 5f 5b 33 79 6d 30   7e 7b 7f 70 5f 5b 33 79
4e0 :   6d 30 7e 7b 7f 70 5f 5b   33 79 6d 30 7e 7b 7f 70
4f0 :   5f 5b 33 79 6d 30 7e 7b   7f 70 5f 5b 33 79 6d 30
500 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 30 30 30 30 30 30
510 :   30 30 30 30 6d 6d 6d 6d   6d 6d 6d 6d 6d 6d 79 79
520 :   79 79 79 79 79 79 79 79   33 33 33 33 33 33 33 33
530 :   33 33 5b 5b 5b 5b 5b 5b   5b 5b 5b 5b 5f 5f 5f 5f
540 :   5f 5f 5f 5f 5f 5f 70 70   70 70 70 70 70 70 70 70
550 :   7f 7f 7f 7f 7f 7f 7f 7f   7f 7f 7b 7b 7b 7b 7b 7b
560 :   7b 7b 7b 7b 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 30 30
570 :   30 30 30 30 30 30 30 30   6d 6d 6d 6d 6d 6d 6d 6d
580 :   6d 6d 6d 6d 6d 6d 6d 6d   6d 30 30 30 30 30 30 30
590 :   30 30 30 7e 7e 7e 7e 7e   7e 7e 7e 7e 7e 7b 7b 7b
5a0 :   7b 7b 7b 7b 7b 7b 7b 7f   7f 7f 7f 7f 7f 7f 7f 7f
5b0 :   7f 70 70 70 70 70 70 70   70 70 70 5f 5f 5f 5f 5f
5c0 :   5f 5f 5f 5f 5f 5b 5b 5b   5b 5b 5b 5b 5b 5b 5b 33
5d0 :   33 33 33 33 33 33 33 33   33 79 79 79 79 79 79 79
5e0 :   79 79 79 6d 6d 6d 6d 6d   6d 6d 6d 6d 6d 30 30 30
5f0 :   30 30 30 30 30 30 30 7e   7e 7e 7e 7e 7e 7e 7e 7e
600 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 7e 7e
610 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 7e 7e
620 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 7e 7e
630 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 7e 7e
640 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 7e 7e
650 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 7e 7e
660 :   7e 7e 7e 7e 30 30 30 30   30 30 30 30 30 30 30 30
670 :   30 30 30 30 30 30 30 30   30 30 30 30 30 30 30 30
680 :   30 30 30 30 30 30 30 30   30 30 30 30 30 30 30 30
690 :   30 30 30 30 30 30 30 30   30 30 30 30 30 7e 7e 7e
6a0 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 7e 7e
6b0 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 7e 7e
6c0 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 7e 7e
6d0 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 7e 7e
6e0 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 7e 7e
6f0 :   7e 7e 7e 7e 7e 7e 7e 7e   7e 7e 7e 7e 7e 7e 7e 7e
700 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
710 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
720 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
730 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
740 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
750 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
760 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
770 :   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00
780 :   01 01 01 01 01 01 01 01   01 01 01 01 01 01 01 01
790 :   01 01 01 01 01 01 01 01   01 01 01 01 01 01 01 01
7a0 :   01 01 01 01 01 01 01 01   01 01 01 01 01 01 01 01
7b0 :   01 01 01 01 01 01 01 01   01 01 01 01 01 01 01 01
7c0 :   01 01 01 01 01 01 01 01   01 01 01 01 01 01 01 01
7d0 :   01 01 01 01 01 01 01 01   01 01 01 01 01 01 01 01
7e0 :   01 01 01 01 01 01 01 01   01 01 01 01 01 01 01 01
7f0 :   01 01 01 01 01 01 01 01   01 01 01 01 01 01 01 01
800 :   7e 30 6d 79 33 5b 5f 70   7f 7b 7e 30 6d 79 33 5b

*/
