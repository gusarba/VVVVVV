
// DREAMCAST memory stats related code

#include "memory_stats.h"

#include <dc/pvr.h>

#include <malloc.h>
#include <stdio.h>

static unsigned long systemRam = 0x00000000;
static unsigned long elfOffset = 0x00000000;
static unsigned long stackSize = 0x00000000;

extern unsigned long end;
extern unsigned long start;

#define _end end
#define _start start

void set_system_ram()
{
  systemRam = 0x8d000000 - 0x8c000000;
  elfOffset = 0x8c000000;

  stackSize = (int)&_end - (int)&_start + ((int)&_start - elfOffset);
}

unsigned long get_system_ram()
{
  return systemRam;
}

unsigned long get_free_ram()
{
  struct mallinfo mi = mallinfo();
  return systemRam - (mi.usmblks + stackSize);
}

void print_ram_stats()
{
  float sys_ram, free_ram, used_ram, pvr_ram;
  sys_ram = (float)get_system_ram() / (float)(1024*1024);
  free_ram = (float)get_free_ram() / (float)(1024*1024);
  used_ram = (sys_ram - free_ram);
  //pvr_ram = (float)pvr_mem_available() / (float)(1024*1024);

  //printf("\n---------\nRAM stats (MB):\nTotal: %.2f, Free: %.2f, Used: %.2f, PVR: %.2f\n---------\n", sys_ram, free_ram, used_ram, pvr_ram);
  printf("\n---------\nRAM stats (MB):\nTotal: %.2f, Free: %.2f, Used: %.2f\n---------\n", sys_ram, free_ram, used_ram);
}

void get_ram_stats(float* sys_ram, float* free_ram, float* used_ram, float* pvr_ram)
{
  *sys_ram = (float)get_system_ram() / (float)(1024*1024);
  *free_ram = (float)get_free_ram() / (float)(1024*1024);
  *used_ram = (*sys_ram - *free_ram);
  if (pvr_ram) {
    *pvr_ram = (float)pvr_mem_available() / (float)(1024*1024);
  }
}

