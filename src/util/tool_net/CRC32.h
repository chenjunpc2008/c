// =============================================================================
//
// A copy from https://github.com/bakercp/CRC32
// =============================================================================

#ifndef CRC32_H
#define CRC32_H

#include <stdlib.h>
#include <stdint.h>

#if defined(SPARK) || defined(PARTICLE)
    #include "application.h"
#elif defined(ARDUINO)
    #if ARDUINO >= 100
        #include "Arduino.h"
    #else
        #include "WProgram.h"
    #endif
#endif

class CRC32
{
public:
  static uint32_t calculate(const void* data, size_t size);

public:
  CRC32() { reset(); }

  void reset();
  void update(uint8_t data);
  void update(const void* data, size_t size);
  uint32_t finalize(const void* data, size_t size);
  uint32_t finalize() const;

private:
  uint32_t state;
};

#endif
