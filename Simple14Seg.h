#ifndef Simple14Seg_h
#define Simple14Seg_h

#include <Arduino.h>
#include <Wire.h>

// Minimalist Font: 0-9, +, -, A-Z (Uppercase only)
static const uint16_t PROGMEM _liteFont[] = {
  0x0C3F, 0x0006, 0x00DB, 0x008F, 0x00E6, 0x00ED, 0x00FD, 0x0007, 0x00FF, 0x00EF, // 0-9
  0x12C0, 0x0040, // +, -
  0x00F7, 0x128F, 0x0039, 0x120F, 0x0079, 0x0071, 0x00BD, 0x00F6, 0x1209, 0x001E, // A-J
  0x2470, 0x0038, 0x0536, 0x2136, 0x003F, 0x00F3, 0x083F, 0x08F3, 0x00ED, 0x1201, // K-T
  0x003E, 0x0C30, 0x2836, 0x2D00, 0x1500, 0x0C09  // U-Z
};

class Simple14Seg : public Print {
private:
    uint8_t _addr;
    uint16_t _buffer[4]; 
    bool _firstChar = true; // auto-clear tracker

    void writeCmd(uint8_t cmd) {
        Wire.beginTransmission(_addr);
        Wire.write(cmd);
        Wire.endTransmission();
    }

    void _flush() {
        Wire.beginTransmission(_addr);
        Wire.write(0x00); 
        for (uint8_t i = 0; i < 4; i++) {
            Wire.write(_buffer[i] & 0xFF);
            Wire.write(_buffer[i] >> 8);
        }
        Wire.endTransmission();
    }

    uint16_t charToMask(char c) {
        if (c >= '0' && c <= '9') return pgm_read_word(&_liteFont[c - '0']);
        else if (c >= 'A' && c <= 'Z') return pgm_read_word(&_liteFont[c - 'A' + 12]);
        else if (c >= 'a' && c <= 'z') return pgm_read_word(&_liteFont[c - 'a' + 12]);
        else if (c == '+') return pgm_read_word(&_liteFont[10]);
        else if (c == '-') return pgm_read_word(&_liteFont[11]);
        else return 0x0000; // space or unsupported
    }

    void shiftLeft(uint16_t mask, bool dp) {
        _buffer[0] = _buffer[1];
        _buffer[1] = _buffer[2];
        _buffer[2] = _buffer[3];
        _buffer[3] = mask | (dp ? 0x4000 : 0);
    }

public:
    Simple14Seg(uint8_t addr = 0x70) : _addr(addr) {}

    void begin() {
        Wire.begin();
        writeCmd(0x21);      // System oscillator ON
        setBrightness(10);   // Default brightness
        setBlink(0);         // No blink
        clear();
        _flush();
    }

    void clear() {
        for(uint8_t i = 0; i < 4; i++) _buffer[i] = 0;
        _firstChar = true;
    }

    void setBrightness(uint8_t level) {
        if (level > 15) level = 15;
        writeCmd(0xE0 | level);
    }

    void setBlink(uint8_t mode) {
        writeCmd(0x80 | 0x01 | (mode << 1));
    }

    void scrollLeft() {
        for(uint8_t i = 0; i < 3; i++) _buffer[i] = _buffer[i + 1];
        _buffer[3] = 0;
        _flush();
    }

    // --- core write ---
    virtual size_t write(uint8_t c) override {
        static bool dpNext = false;

        if (_firstChar) {
            clear();
            _firstChar = false;
        }

        // decimal point
        if (c == '.') {
            dpNext = true;
            return 1;
        }

        uint16_t mask = charToMask(c);
        shiftLeft(mask, dpNext);
        dpNext = false;

        _flush();
        return 1;
    }

    // --- print overloads ---
    size_t print(char c) { return write(c); }
    size_t print(const char* s) { 
        _firstChar = true; 
        size_t count = 0; 
        while(*s) count += write(*s++); 
        return count; 
    }
    size_t print(String s) { return print(s.c_str()); }
    size_t print(int n, int base=10) { 
        char buf[12]; itoa(n, buf, base); return print(buf); 
    }
    size_t print(unsigned int n, int base=10) { 
        char buf[12]; utoa(n, buf, base); return print(buf); 
    }
    size_t print(long n, int base=10) { 
        char buf[12]; ltoa(n, buf, base); return print(buf); 
    }
    size_t print(unsigned long n, int base=10) { 
        char buf[12]; ultoa(n, buf, base); return print(buf); 
    }
    size_t print(float f, int decimals=1) {
        char buf[12];
        dtostrf(f, 1 + decimals, decimals, buf);
        return print(buf);
    }
    size_t print(double f, int decimals=1) { return print((float)f, decimals); }

    // --- println variants ---
    size_t println() { return print("\0"); }
    size_t println(char c) { return print(c); }
    size_t println(const char* s) { return print(s); }
    size_t println(String s) { return print(s); }
    size_t println(int n, int base=10) { return print(n, base); }
    size_t println(unsigned int n, int base=10) { return print(n, base); }
    size_t println(long n, int base=10) { return print(n, base); }
    size_t println(unsigned long n, int base=10) { return print(n, base); }
    size_t println(float f, int decimals=1) { return print(f, decimals); }
    size_t println(double f, int decimals=1) { return print(f, decimals); }

    void display() { _flush(); }
};

#endif

