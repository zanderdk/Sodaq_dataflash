#pragma once
#include <SPI.h>
#include <stddef.h>
#include <stdint.h>

#define DF_PAGE_ADDR_BITS 12
#define DF_PAGE_SIZE 512
#define DF_PAGE_BITS 10

#define DF_NR_PAGES (1 << DF_PAGE_ADDR_BITS)

class Sodaq_Dataflash {
public:
  Sodaq_Dataflash(uint8_t csPin = SS);
  void init();
  void readID(uint8_t *data);
  void readSecurityReg(uint8_t *data, size_t size);
  void activate();
  void deactivate();
  void beginWriteBuf1(uint16_t addr);
  void beginWriteBuf2(uint16_t addr);
  void writeSequential(uint8_t data);

  void beginReadBuf1(uint16_t addr);
  void beginReadBuf2(uint16_t addr);
  void write(uint8_t data);
  void writeStr(uint8_t *data, size_t size);
  void transmitStr(uint8_t *data, uint8_t *out, uint32_t size);
  void readSequential(uint8_t *buf);
  void sectorErase(uint16_t pageAddr);

  uint8_t readByteBuf1(uint16_t pageAddr);
  void readStrBuf1(uint16_t addr, uint8_t *data, size_t size);
  void writeByteBuf1(uint16_t addr, uint8_t data);
  void writeStrBuf1(uint16_t addr, uint8_t *data, size_t size);
  void writeBuf1ToPage(uint16_t pageAddr);
  void readPageToBuf1(uint16_t PageAdr);

  uint8_t readByteBuf2(uint16_t pageAddr);
  void readStrBuf2(uint16_t addr, uint8_t *data, size_t size);
  void writeByteBuf2(uint16_t addr, uint8_t data);
  void writeStrBuf2(uint16_t addr, uint8_t *data, size_t size);
  void writeBuf2ToPage(uint16_t pageAddr);
  void readPageToBuf2(uint16_t PageAdr);
  void InitSequential();

  void pageErase(uint16_t pageAddr);
  void chipErase();

  void waitTillReady();
  void settings(SPISettings settings);

private:
  uint8_t readStatus();
  uint8_t transmit(uint8_t data);
  void setPageAddr(unsigned int PageAdr);
  uint8_t getPageAddrByte0(uint16_t pageAddr);
  uint8_t getPageAddrByte1(uint16_t pageAddr);
  uint8_t getPageAddrByte2(uint16_t pageAddr);
  uint16_t addr = 0;
  uint16_t offset = 0;
  uint16_t readAddr = 0;
  uint8_t nextBuf = 0;
  uint8_t _csPin;
  size_t _pageAddrShift;
};
