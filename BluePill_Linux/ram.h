#pragma once

#include <SPI.h>
SPIClass PSRAM(2);

#define PSRAM_CS PB12

#define PSRAM_CMD_RES_EN 0x66
#define PSRAM_CMD_RESET 0x99
#define PSRAM_CMD_READ_ID 0x9F
#define PSRAM_CMD_READ 0x03
#define PSRAM_CMD_READ_FAST 0x0B
#define PSRAM_CMD_WRITE 0x02
#define PSRAM_KGD 0x5D

void initPSRAM() {
  PSRAM.begin();
  PSRAM.setBitOrder(MSBFIRST);
  PSRAM.setDataMode(SPI_MODE0);
  PSRAM.setClockDivider(SPI_CLOCK_DIV2);
  
  pinMode(PSRAM_CS, OUTPUT);
  digitalWrite(PSRAM_CS, LOW);
  PSRAM.transfer(PSRAM_CMD_RES_EN);
  digitalWrite(PSRAM_CS, HIGH);
  
  digitalWrite(PSRAM_CS, LOW);
  PSRAM.transfer(PSRAM_CMD_RESET);
  digitalWrite(PSRAM_CS, HIGH);
  delay(10);
}

void store1(int address, uint8_t data) {
  digitalWrite(PSRAM_CS, LOW);
  PSRAM.transfer(0x02);
  PSRAM.transfer16(address);

  SPI.transfer(data);

  digitalWrite(PSRAM_CS, HIGH);
}
void store2(int address, uint16_t data) {
  digitalWrite(PSRAM_CS, LOW);
  PSRAM.transfer(0x02);
  PSRAM.transfer16(address);

  SPI.transfer16(data);

  digitalWrite(PSRAM_CS, HIGH);
}
void store4(int address, uint32_t data) {
  digitalWrite(PSRAM_CS, LOW);
  PSRAM.transfer(0x02);
  PSRAM.transfer16(address);

  SPI.transfer16(data >> 16);
  SPI.transfer16(data & 0xFFFF);

  digitalWrite(PSRAM_CS, HIGH);
}

uint8_t load1(int address) {
  digitalWrite(PSRAM_CS, LOW);
  PSRAM.transfer(0x03);
  PSRAM.transfer16(address);

  uint8_t b = SPI.transfer(0x00);

  digitalWrite(PSRAM_CS, HIGH);

  return b;
}

uint16_t load2(int address) {
  digitalWrite(PSRAM_CS, LOW);
  PSRAM.transfer(0x03);
  PSRAM.transfer16(address);

  uint16_t b = SPI.transfer16(0x00);

  digitalWrite(PSRAM_CS, HIGH);

  return b;
}

uint32_t load4(int address) {
  digitalWrite(PSRAM_CS, LOW);
  PSRAM.transfer(0x03);
  PSRAM.transfer16(address);

  uint32_t b = SPI.transfer16(0x00) << 16;
  b += SPI.transfer16(0x00) << 16;

  digitalWrite(PSRAM_CS, HIGH);

  return b;
}
