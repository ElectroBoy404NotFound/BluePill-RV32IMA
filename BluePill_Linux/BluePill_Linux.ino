#include "ram.h"
#include "rv32ima.h"
#include "defines.h"
#include <SD.h>

static uint64_t GetTimeMicroseconds();
static uint32_t HandleException(uint32_t ir, uint32_t retval);
static uint32_t HandleControlStore(uint32_t addy, uint32_t val);
static uint32_t HandleControlLoad(uint32_t addy );
static void HandleOtherCSRWrite(uint8_t *image, uint16_t csrno, uint32_t value);
static int32_t HandleOtherCSRRead(uint8_t *image, uint16_t csrno);
static void MiniSleep();
static int IsKBHit();
static int ReadKBByte();

static struct MiniRV32IMAState core;

void setup() {
  delay(30000);
  Serial.begin(115200);

  Serial.print("Initializing SD card...");
  while (!SD.begin(PA15))
  {
    delay(10);
  }
  Serial.println("Initializing PSRAM...");

  initPSRAM();

  Serial.println("Initialization done!");


  File linux = SD.open("Linux");
  if(linux) {
    int i = 0;
    while (linux.available()) {
      Serial.print("Percent done: ");
      Serial.print((i/linux.size())*100);
      Serial.print(" file size is ");
      Serial.print(linux.size());
      Serial.print(" bytes. I'm doing byte: ");
      Serial.println(i);
      store1(i, linux.read());
      i++;
    }
    linux.close();
  }else {
    Serial.println("Error opening Linux!");
    while(true);
  }
  
  
  uint32_t dtb_ptr = ram_amt - sizeof(default64mbdtb);
  uint32_t validram = dtb_ptr;
  for(int i = 0; i < sizeof(default64mbdtb); i++) {
    store1(dtb_ptr + i, default64mbdtb[i]);
  }

  uint32_t dtbRamValue = (validram >> 24) | (((validram >> 16) & 0xff) << 8) | (((validram >> 8) & 0xff) << 16) | ((validram & 0xff) << 24);
  MINIRV32_STORE4(dtb_ptr + 0x13c, dtbRamValue);
  Serial.println("Loaded Kernel Image and DTB into RAM!");

  core.pc = MINIRV32_RAM_IMAGE_OFFSET;
  core.regs[10] = 0x00;
  core.regs[11] = dtb_ptr ? (dtb_ptr + MINIRV32_RAM_IMAGE_OFFSET) : 0;
  core.extraflags |= 3;

  Serial.println("RV32IMA starting!");
}

uint64_t lastTime = millis();
int instrs_per_flip = 1024;
int time_divisor = 6;

void loop() {
  int ret;
  uint64_t *this_ccount = ((uint64_t*)&core.cyclel);
  uint32_t elapsedUs = millis() / time_divisor - lastTime;

  lastTime = elapsedUs;
  ret = MiniRV32IMAStep(&core, NULL, 0, elapsedUs, instrs_per_flip);
  switch (ret) {
  case 0:
    break;
  case 1:
    *this_ccount += instrs_per_flip;
    break;
  case 3:
    Serial.println("Got return 3!");
    break;
  case 0x7777:
    Serial.println("Reboot...");
  case 0x5555:
    Serial.println("POWEROFF");
    while(1);
    return;
  default:
    Serial.println("Unknown failure (%d)! Continuing...");
    break;
  }
}
