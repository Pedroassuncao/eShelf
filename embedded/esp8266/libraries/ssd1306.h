#ifndef ssd1306_h
#define ssd1306_h

#define DISPLAY_WIDTH 128 // width of the display in pixels
#define DISPLAY_HIGHT 64 // hight of the display in pixels
#define DISPLAY_ADDR 0x3C // default i2c address in most cases. if not working try DISPLAY_ADDR_
#define DISPLAY_ADDR_ 0x3D // default i2c address in some cases. if not working try DISPLAY_ADDR
#define DISPLAY_BITMAP_MAX_SIZE DISPLAY_WIDTH * DISPLAY_HIGHT // the biggest image that can be displayed at once in bytes

#endif
