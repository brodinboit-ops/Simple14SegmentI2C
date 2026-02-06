#include "Simple14Seg.h"

Simple14Seg seg(0x70);

void setup() {
  seg.begin();
  seg.print("A1-B"); // Works with Strings
  seg.display();
  
  delay(2000);
  
  seg.clear();
  seg.print(123);    // Works with Ints
  seg.display();
}

void loop() {}