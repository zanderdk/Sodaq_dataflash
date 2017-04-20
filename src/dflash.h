#include "Sodaq_dataflash.h"
#include <initializer_list>
#include <stdint.h>
#include <vector>

class dflash {
public:
  dflash(std::initializer_list<uint8_t> pins);
  void init();
  void writeSequential(uint8_t data);
  void readSequential(uint8_t *out);
  void erasePages(uint32_t pages);
  void deactivate();
  void waitTillReady();
  std::vector<Sodaq_Dataflash> flashes;

private:
  uint32_t writeOffset = 0;
  uint8_t currentWriteFlash = 0;
  uint8_t currentReadFlash = 0;
};
