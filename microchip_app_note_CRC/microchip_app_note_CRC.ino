/*
 * CRC16 function testing of provided CRC function from Microchip app note
 * SparkFun Electronics
 * Pete Lewis
 * Aug 2019
 * 
 * Using the function provided by Microchip in the app note for the ATECC508a found here:
 * http://ww1.microchip.com/downloads/en/AppNotes/Atmel-8936-CryptoAuth-Data-Zone-CRC-Calculation-ApplicationNote.pdf
 * 
*/

uint8_t crc[2] = {0, 0};
uint8_t message[] = {0x07, 0x30, 0x00, 0x00, 0x00}; // example command for ATECC508a "get info"
// includes count, command, param1, param2a, param2b
// correct CRC: 0x035D

void setup()
{

  Serial.begin(9600);

  atca_calculate_crc(5, message, crc);

  Serial.println(crc[0], HEX);
  Serial.println(crc[1], HEX);

}

void loop()
{
  // do nothing
}

/** \brief This function calculates CRC.

    http://ww1.microchip.com/downloads/en/AppNotes/Atmel-8936-CryptoAuth-Data-Zone-CRC-Calculation-ApplicationNote.pdf

  \param[in] length number of bytes in buffer
  \param[in] data pointer to data for which CRC should be calculated
  \param[out] crc pointer to 16-bit CRC
*/

void atca_calculate_crc(uint8_t length, uint8_t *data, uint8_t *crc)
{
  uint8_t counter;
  uint16_t crc_register = 0;
  uint16_t polynom = 0x8005;
  uint8_t shift_register;
  uint8_t data_bit, crc_bit;
  for (counter = 0; counter < length; counter++) {
    for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1) {
      data_bit = (data[counter] & shift_register) ? 1 : 0;
      crc_bit = crc_register >> 15;
      crc_register <<= 1;
      if (data_bit != crc_bit)
        crc_register ^= polynom;
    }
  }
  crc[0] = (uint8_t) (crc_register & 0x00FF);
  crc[1] = (uint8_t) (crc_register >> 8);
}
