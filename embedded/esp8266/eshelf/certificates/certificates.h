#ifndef certificates_h
#define certificates_h

static const uint8_t rsakey[] PROGMEM = {
    #include "key.h"
};

static const uint8_t x509[] PROGMEM = {
    #include "x509.h"
};

#endif