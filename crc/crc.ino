/*
   CRC16 function creation
   SparkFun Electronics
   Pete Lewis
   Aug 2019
   
   Goal: create a function that can calculate a 16 bit CRC on a chunk of data.
   Start with the example in the video by Ben Eater

   Ben Eater: Hardware Build: CRC Calculation
   https://www.youtube.com/watch?v=sNkERQlK8j8

   message "Hi!"
   
   poly:            x^16 +          x^12 +                      x^5 +               1
   poly bit num:    16  15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0
   poly (as BIN):   1   0   0   0   1   0   0   0   0   0   0   1   0   0   0   0   1
   poly (HEX):      0x1021
                    note, the 16th bit (when viewed as HEX) is ommited from this poly,
                    because all good poly's have the left-most bit as a "1")
                    
   correct CRC:     0   0   0   1   1   0   0   0   1   1   1   1   1   1   1   0   1


   Note, the final goal of this code example (my first attempt at understanding CRCs),
   is to eventually understand the CRC creation necessary for the ATECC508a.
   The application note for this chip from Microchip, has a function provided to
   create CRCs:

   http://ww1.microchip.com/downloads/en/AppNotes/Atmel-8936-CryptoAuth-Data-Zone-CRC-Calculation-ApplicationNote.pdf

   There are a few strange things going on in there, that (as of now)
   don't seem to align with the more "standard" CRC functions I have seen in my initial
   research.

*/

const char *message = "Hi!";
//uint8_t message[4] = {0x48, 0x69, 0x21}; // optional array - instead of pointer style, note, not sure this works.
//uint8_t message[] = {0x07, 0x30, 0x00, 0x00, 0x00}; // example command for ATECC508a "get info"
// includes count, command, param1, param2a, param2b

const uint16_t poly = 0x1021;
//const uint16_t poly = 0x8005; // poly for ATECC508a
uint16_t crc = 0;
uint16_t buffer = 0;
int time_delay = 100; // used to slow things down during development and actually see what's happening

const int lengthOfMessage = (sizeof(message) + 1); // sizeof() returns the number of bytes, position "0" not included, so 2 actually equals 3 bytes.
const int messageBitSize = ((lengthOfMessage * 8) + 16); // * 8 gets from bytes to bits, +16 to make room for the CRC.
// Note, the additional 16 spots in this array (aka bits) are only used to know when to stop, the actual crc (as it is generated) is stored in buffer.
boolean messageBitArray[messageBitSize] = {}; // array of bits we will use for shifting into buffer
int shiftCount = 0; // used to keep track of how many bits we have shifted from messageBitArray into buffer

void setup()
{
  Serial.begin(9600);
  Serial.println("CRC16 example");

  printMessage();


  loadMessageBitArray();
  printMessageBitArray();

  printPoly();
  delay(1000);

  while (shiftCount < messageBitSize) // keep shifting and Xor'ing until we've completed the entire message
  {
    shiftToValidXor();
    xorBufferPoly();
  }

  crc = buffer;
  printcrc();
}

void loop()
{
  // do nothing
}

void shiftToValidXor()
{
  // shift in data until it's MSB lines up with the MSB of the poly (which is always 16, until the end)
  // print data that got shifted in
  // keep shifting in until the MSB of the data coming in (buffer) is actually bit 16
  // print poly below

  // So I've got this long array of bits...
  // I need to shift in one bit at a time into buffer.
  // I need to keep shifting until the MSB of the buffer is the 16th bit,
  // then it will be time to Xor with the poly!!

  printBuffer();
  delay(1);

  while ((buffer & 0x8000) == false) // keep shifting until the 16th bit is set, for reference 0x8000 = 0b 1000 0000 0000 0000
  {
    buffer <<= 1; // shift over to the left one
    // pull in bit from messageBitArray (that is, set the LSB-of-buffer to the new bit from messageBitArray).
    buffer |= messageBitArray[shiftCount]; // using the |= commmand we only effect the LSB (aka bit-masking)
    shiftCount++;
    printBuffer();
    delay(time_delay);
  }
  buffer <<= 1; // shift over to the left one more time, because the 16th bit of the poly is omitted and so this corrects alignment befor the XOR operation.
  buffer |= messageBitArray[shiftCount];
  shiftCount++;
  printBuffer();

}

void loadMessageBitArray()
{
  for (int i = 0 ; i <= sizeof(message) ; i++) // loop through each byte of message
  {
    // load byte 0,1,2 into individual slots in array
    byte localByte = *(message + i);

    // Start with the most signibicant bit B1000 0000 (aka 0x80)
    // then shift to the right through all of them
    // until bit_check is no longer true and end for loop.
    byte bitCounter = 0 ;
    for (byte bit_check = 0x80 ; bit_check ; bit_check >>= 1)
    {
      if (localByte & bit_check) // if there's a bit, laod it into the array
      {
        messageBitArray[((i * 8) + bitCounter)] = 1; // position in array is which byte + which bit position we are at.
      }
      //else do nothing, that is... leave it a 0 bit in the messageBitArray.
      bitCounter++; // keep track of which bit within the byte we are on.
    }
  }
}

void xorBufferPoly()
{
  Serial.println("Time to XOR");
  delay(time_delay);
  printBuffer();
  printPoly();
  buffer ^= poly;
  printBuffer();
  Serial.println("XOR COMPLETE");
}

///////////////////// Printing function below /////////////////////////////

void printcrc()
{
  Serial.print("crc:");
  uint8_t highByteCrc = (crc >> 8);
  uint8_t lowByteCrc = (crc & 0xFF);
  printBinary(highByteCrc);
  printBinary(lowByteCrc);
  Serial.println();
}

void printMessage()
{
  Serial.print("mesg(ASCII):\t\t");
  for (int i = 0 ; i <= sizeof(message) ; i++)
  {
    Serial.print(*(message + i));
  }
  Serial.println();

  Serial.print("lengthOfMessage:");
  Serial.println(lengthOfMessage);
  Serial.print("messageBitSize:");
  Serial.println(messageBitSize);

  Serial.print("mesg(BIN):\t\t");
  for (int i = 0 ; i <= sizeof(message) ; i++)
  {
    //Serial.print(" "); // optional space to "split up" each byte - for readability
    printBinary(*(message + i));
  }
  Serial.println();
}

void printBinary(byte inByte)
{
  // Start with the most significant bit B1000 0000 (aka 0x80)
  // then shift to the right through all of them
  // until bit_check is no longer true, and this will end the for loop.
  for (byte bit_check = 0x80 ; bit_check ; bit_check >>= 1)
  {
    if (inByte & bit_check) Serial.write('1');
    else Serial.write('0');
  }
}

void printPoly()
{
  //  Serial.print("Poly: 0x");
  //  Serial.println(poly, HEX);
  Serial.print("p:");
  uint8_t highBytePoly = (poly >> 8);
  uint8_t lowBytePoly = (poly & 0xFF);
  printBinary(highBytePoly);
  printBinary(lowBytePoly);
  Serial.println();
}

void printMessageBitArray()
{
  Serial.print("messageBitArray:\t");
  for (int i = 0 ; i < sizeof(messageBitArray) ; i++)
  {
    //if (i % 8 == false) Serial.print(" ");
    Serial.print(messageBitArray[i]);
  }
  Serial.println();
}

void printBuffer()
{
  Serial.print("b:");
  printBinary(highByte(buffer));
  printBinary(lowByte(buffer));
  Serial.print("\tshiftCount:");
  Serial.print(shiftCount);
  Serial.println();
}
