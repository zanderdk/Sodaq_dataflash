#include "dflash.h"
#include "Arduino.h"

dflash::dflash(std::initializer_list<uint8_t> pins) {
  for (auto pin : pins) {
    flashes.push_back(Sodaq_Dataflash(pin));
  }
}

void dflash::deactivate() {
  for (auto flash : flashes) {
    flash.deactivate();
  }
}

void dflash::waitTillReady() {
  for (auto flash : flashes) {
    flash.waitTillReady();
  }
}

void dflash::erasePagesBySector(uint32_t pages) {
  uint32_t pageAddr = 0;
  while (pages > 0) {
    for (auto flash : flashes) {
      flash.pageErase(pageAddr);
      pages += 256;
    }
    pageAddr += 256;
  }
}

void dflash::erasePages(uint32_t pages) {
  uint32_t pageAddr = 0;
  while (pages > 0) {
    for (auto flash : flashes) {
      flash.pageErase(pageAddr);
      pages--;
    }
    pageAddr++;
  }
}

void dflash::writeSequential(uint8_t data) {
  flashes[currentWriteFlash].writeSequential(data);
  writeOffset++;
  if (writeOffset == DF_PAGE_SIZE) {
    writeOffset = 0;
    currentWriteFlash++;
    if (currentWriteFlash == flashes.size()) {
      currentWriteFlash = 0;
    }
  }
}

void dflash::readSequential(uint8_t *out) {
  flashes[currentReadFlash].readSequential(out);
  currentReadFlash++;
  if (currentReadFlash == flashes.size()) {
    currentReadFlash = 0;
  }
}

void dflash::init() {
  for (auto x : flashes) {
    x.init();
  }
}
