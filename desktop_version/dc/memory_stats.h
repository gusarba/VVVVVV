
#ifndef __MEMORY_STATS_H__
#define __MEMORY_STATS_H__ 1

void set_system_ram();
unsigned long get_system_ram();
unsigned long get_free_ram();
void print_ram_stats();
void get_ram_stats(float* sys_ram, float* free_ram, float* used_ram, float* pvr_ram = 0L);

#endif

