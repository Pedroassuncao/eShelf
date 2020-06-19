#ifndef ntag213_h
#define ntag213_h

#define NTAG_START_PAGE 5 // WARNING: this value must be greater than 4 so you don't brick the tag
#define NTAG_STOP_PAGE 39 // WARNING: this value must be less than 40 so you don't brick the tag
#define NTAG_WRITABLE_SPACE (NTAG_STOP_PAGE - NTAG_START_PAGE + 1) * 4 // available writable space on the tag in bytes
#define NTAG_STORAGE_SPACE NTAG_WRITABLE_SPACE // available space on the tag in bytes for storing data

#endif
