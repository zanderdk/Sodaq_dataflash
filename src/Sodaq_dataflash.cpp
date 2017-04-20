#include <Arduino.h>
#include <SPI.h>
#include <stdint.h>

#include "Sodaq_dataflash.h"

// Dataflash commands
#define FlashPageRead 0xD2 // Main memory page read
#define StatusReg 0xD7     // Status register
#define ReadMfgID 0x9F     // Read Manufacturer and Device ID
#define PageErase 0x81     // Page erase
#define ReadSecReg 0x77    // Read Security Register

#define FlashToBuf1Transfer 0x53 // Main memory page to buffer 1 transfer
#define Buf1Read 0xD4            // Buffer 1 read
#define Buf1ToFlashWE                                                          \
  0x88 // Buffer 1 to main memory page program with built-in erase
#define Buf1Write 0x84 // Buffer 1 write

#define FlashToBuf2Transfer 0x55 // Main memory page to buffer 2 transfer
#define Buf2Read 0xD6            // Buffer 2 read
#define Buf2ToFlashWE                                                          \
  0x89 // Buffer 2 to main memory page program with built-in erase
#define Buf2Write 0x87   // Buffer 2 write
#define SectorErase 0x7C // Sector Erase

Sodaq_Dataflash::Sodaq_Dataflash(uint8_t csPin) {
  // Setup the slave select pin
  _csPin = csPin;

  // This is used when CS != SS
  pinMode(_csPin, OUTPUT);
  deactivate();
}

void Sodaq_Dataflash::init() {

  // Call the standard SPI initialisation
  SPI.setBitOrder(SPI_MSBFIRST);
  SPI.setFrequency(20000000);
  SPI.setDataMode(SPI_MODE0);
  SPI.begin();
}

void Sodaq_Dataflash::sectorErase(uint16_t pageAddr) {
  activate();
  write(SectorErase);
  setPageAddr(pageAddr);
  deactivate();
  waitTillReady();
}

uint8_t Sodaq_Dataflash::transmit(uint8_t data) {
  // Call the standard SPI transfer method
  return SPI.transfer(data);
}

void Sodaq_Dataflash::transmitStr(uint8_t *data, uint8_t *out, uint32_t size) {
  SPI.transferBytes(data, out, size);
}

void Sodaq_Dataflash::write(uint8_t data) { SPI.write(data); }

void Sodaq_Dataflash::writeStr(uint8_t *data, size_t size) {
  SPI.writeBytes(data, size);
}

uint8_t Sodaq_Dataflash::readStatus() {
  unsigned char result;

  activate();
  result = transmit(StatusReg);
  result = transmit(0x00);
  deactivate();

  return result;
}

// Monitor the status register, wait until busy-flag is high
void Sodaq_Dataflash::waitTillReady() {
  while (!(readStatus() & 0x80)) {
    // WDT reset maybe??
  }
}

void Sodaq_Dataflash::readID(uint8_t *data) {
  activate();
  write(ReadMfgID);
  data[0] = transmit(0x00);
  data[1] = transmit(0x00);
  data[2] = transmit(0x00);
  data[3] = transmit(0x00);
  deactivate();
}

void Sodaq_Dataflash::readSequential(uint8_t *buf) {
  readPageToBuf1(readAddr++);
  waitTillReady();
  readStrBuf1(0, buf, DF_PAGE_SIZE);
}

void Sodaq_Dataflash::InitSequential() {
  activate();
  beginWriteBuf1(0);
}

void Sodaq_Dataflash::writeSequential(uint8_t data) {
  if (offset == 0) {
    activate();
    if (!nextBuf) {
      beginWriteBuf1(0);
    } else {
      beginWriteBuf2(0);
    }
  }

  write(data);
  ++offset;

  if (offset == DF_PAGE_SIZE) {
    offset = 0;
    deactivate();
    if (addr % 2 == 0) {
      waitTillReady();
      writeBuf1ToPage(addr++);
    } else {
      waitTillReady();
      writeBuf2ToPage(addr++);
    }
    nextBuf = !nextBuf;
  }
}

// Reads a number of bytes from one of the Dataflash security register
void Sodaq_Dataflash::readSecurityReg(uint8_t *data, size_t size) {
  activate();
  write(ReadSecReg);
  write(0x00);
  write(0x00);
  write(0x00);
  for (size_t i = 0; i < size; i++) {
    *data++ = transmit(0x00);
  }
  deactivate();
}

// Transfers a page from flash to Dataflash SRAM buffer
void Sodaq_Dataflash::readPageToBuf1(uint16_t pageAddr) {
  activate();
  write(FlashToBuf1Transfer);
  setPageAddr(pageAddr);
  deactivate();
}

// Transfers a page from flash to Dataflash SRAM buffer
void Sodaq_Dataflash::readPageToBuf2(uint16_t pageAddr) {
  activate();
  write(FlashToBuf2Transfer);
  setPageAddr(pageAddr);
  deactivate();
}

// Reads one byte from one of the Dataflash internal SRAM buffer 1
uint8_t Sodaq_Dataflash::readByteBuf1(uint16_t addr) {
  unsigned char data = 0;

  activate();
  write(Buf1Read);
  write(0x00); // don't care
  write((uint8_t)(addr >> 8));
  write((uint8_t)(addr));
  write(0x00);           // don't care
  data = transmit(0x00); // read byte
  deactivate();

  return data;
}

// Reads one byte from one of the Dataflash internal SRAM buffer 1
uint8_t Sodaq_Dataflash::readByteBuf2(uint16_t addr) {
  unsigned char data = 0;

  activate();
  write(Buf2Read);
  write(0x00); // don't care
  write((uint8_t)(addr >> 8));
  write((uint8_t)(addr));
  write(0x00);           // don't care
  data = transmit(0x00); // read byte
  deactivate();

  return data;
}

void Sodaq_Dataflash::beginReadBuf1(uint16_t addr) {
  write(Buf1Read);
  write(0x00); // don't care
  write((uint8_t)(addr >> 8));
  write((uint8_t)(addr));
  write(0x00); // don't care
}

void Sodaq_Dataflash::beginReadBuf2(uint16_t addr) {
  write(Buf2Read);
  write(0x00); // don't care
  write((uint8_t)(addr >> 8));
  write((uint8_t)(addr));
  write(0x00); // don't care
}

// Reads a number of bytes from one of the Dataflash internal SRAM buffer 1
void Sodaq_Dataflash::readStrBuf1(uint16_t addr, uint8_t *data, size_t size) {
  activate();
  beginReadBuf1(addr);
  for (size_t i = 0; i < size; i++) {
    *data++ = transmit(0x00);
  }
  deactivate();
}

// Reads a number of bytes from one of the Dataflash internal SRAM buffer 1
void Sodaq_Dataflash::readStrBuf2(uint16_t addr, uint8_t *data, size_t size) {
  activate();
  beginReadBuf2(addr);
  for (size_t i = 0; i < size; i++) {
    *data++ = transmit(0x00);
  }
  deactivate();
}

// Writes one byte to one to the Dataflash internal SRAM buffer 1
void Sodaq_Dataflash::writeByteBuf1(uint16_t addr, uint8_t data) {
  activate();
  write(Buf1Write);
  write(0x00); // don't care
  write((uint8_t)(addr >> 8));
  write((uint8_t)(addr));
  write(data); // write data byte
  deactivate();
}

// Writes one byte to one to the Dataflash internal SRAM buffer 1
void Sodaq_Dataflash::writeByteBuf2(uint16_t addr, uint8_t data) {
  activate();
  write(Buf2Write);
  write(0x00); // don't care
  write((uint8_t)(addr >> 8));
  write((uint8_t)(addr));
  write(data); // write data byte
  deactivate();
}

void Sodaq_Dataflash::beginWriteBuf1(uint16_t addr) {
  write(Buf1Write);
  write(0x00); // don't care
  write((uint8_t)(addr >> 8));
  write((uint8_t)(addr));
}

void Sodaq_Dataflash::beginWriteBuf2(uint16_t addr) {
  write(Buf2Write);
  write(0x00); // don't care
  write((uint8_t)(addr >> 8));
  write((uint8_t)(addr));
}

// Writes a number of bytes to one of the Dataflash internal SRAM buffer 1
void Sodaq_Dataflash::writeStrBuf1(uint16_t addr, uint8_t *data, size_t size) {
  activate();
  beginWriteBuf1(addr);
  writeStr(data, size);
  deactivate();
}

// Writes a number of bytes to one of the Dataflash internal SRAM buffer 1
void Sodaq_Dataflash::writeStrBuf2(uint16_t addr, uint8_t *data, size_t size) {
  activate();
  beginWriteBuf2(addr);
  writeStr(data, size);
  deactivate();
}

// Transfers Dataflash SRAM buffer 1 to flash page
void Sodaq_Dataflash::writeBuf1ToPage(uint16_t pageAddr) {
  activate();
  write(Buf1ToFlashWE);
  setPageAddr(pageAddr);
  deactivate();
  // waitTillReady();
}

// Transfers Dataflash SRAM buffer 1 to flash page
void Sodaq_Dataflash::writeBuf2ToPage(uint16_t pageAddr) {
  activate();
  write(Buf2ToFlashWE);
  setPageAddr(pageAddr);
  deactivate();
  // waitTillReady();
}

void Sodaq_Dataflash::pageErase(uint16_t pageAddr) {
  activate();
  write(PageErase);
  setPageAddr(pageAddr);
  deactivate();
  waitTillReady();
}

void Sodaq_Dataflash::chipErase() {
  activate();
  write(0xC7);
  write(0x94);
  write(0x80);
  write(0x9A);
  deactivate();
  waitTillReady();
}

void Sodaq_Dataflash::settings(SPISettings settings) {
  SPI.setBitOrder(settings._bitOrder);
  SPI.setDataMode(settings._dataMode);
  SPI.setFrequency(settings._clock);
}

void Sodaq_Dataflash::deactivate() { digitalWrite(_csPin, HIGH); }
void Sodaq_Dataflash::activate() { digitalWrite(_csPin, LOW); }

void Sodaq_Dataflash::setPageAddr(unsigned int pageAddr) {
  write(getPageAddrByte0(pageAddr));
  write(getPageAddrByte1(pageAddr));
  write(getPageAddrByte2(pageAddr));
}

/*
 * From the AT45DB081D documentation (other variants are not really identical)
 *   "For the DataFlash standard page size (DF_PAGE_SIZE-bytes), the opcode must
 * be followed by three address bytes consist of three don’t care bits, 12 page
 * address bits (PA11 - PA0) that specify the page in the main memory to be
 * written and nine don’t care bits."
 */
/*
 * From the AT45DB161B documentation
 *   "For the standard DataFlash page size (528 bytes), the opcode must be
 *    followed by three address bytes consist of 2 don’t care bits, 12 page
 *    address bits (PA11 - PA0) that specify the page in the main memory to
 *    be written and 10 don’t care bits."
 */
/*
 * From the AT45DB041D documentation
 *   "For the DataFlash standard page size (DF_PAGE_SIZE-bytes), the opcode must
 * be followed by three address bytes consist of four don’t care bits, 11 page
 *   address bits (PA10 - PA0) that specify the page in the main memory to
 *   be written and nine don’t care bits."
 */
uint8_t Sodaq_Dataflash::getPageAddrByte0(uint16_t pageAddr) {
  // More correct would be to use a 24 bits number
  // shift to the left by number of bits. But the uint16_t can be considered
  // as if it was already shifted by 8.
  return (pageAddr << (DF_PAGE_BITS - 8)) >> 8;
}
uint8_t Sodaq_Dataflash::getPageAddrByte1(uint16_t page) {
  return page << (DF_PAGE_BITS - 8);
}
uint8_t Sodaq_Dataflash::getPageAddrByte2(uint16_t page) { return 0; }
