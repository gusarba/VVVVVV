
#include "FileSystemUtils_dreamcast.h"

//#include <stdlib.h>
//#include <string.h>

#include <kos.h>
#include <kos/string.h>
#include <zlib/zlib.h>

extern uint16 vvvvvvmu_icon_pal[];
extern uint8  vvvvvvmu_icon_img[];

const int vvvvvv_lcd_inv_size = 192;
const unsigned char vvvvvv_lcd_inv_data[192] ={
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xf1, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xf1, 0x8f, 
	0xff, 0xff, 0xff, 0xff, 0xf1, 0x8f, 0xff, 0xff, 
	0xff, 0xff, 0xf9, 0x9f, 0xff, 0xff, 0xff, 0xff, 
	0xf8, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xe4, 0x27, 
	0xff, 0xff, 0xff, 0xff, 0xe4, 0x27, 0xff, 0xff, 
	0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xff, 0xff, 
	0xe0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x07, 
	0xff, 0xff, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 
	0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xff, 0xff, 
	0xf0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xe7, 0x87, 
	0xff, 0xff, 0xff, 0xff, 0xef, 0xc7, 0xff, 0xff, 
	0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xff, 0xff, 
	0xe0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xec, 0xc7, 
	0xff, 0xff, 0xff, 0xff, 0xec, 0xc7, 0xff, 0xff, 
	0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xff, 0xff, 
	0xf0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	
};


//static char vmu_path[8] = {0};
static char vmu_path[8] = "\0\0\0\0\0\0\0\0";
static int vmu_port = -1;
static int vmu_slot = -1;

void PLATFORM_beepVmu(void* arg) {
  // BEEP !!!
  if (vmu_port > -1) {
    maple_device_t* dev = NULL;
    dev = maple_enum_dev(vmu_port, vmu_slot);
    if (dev) {
      vmu_beep_raw(dev, 0x000065F0);
      thd_sleep(500);
      vmu_beep_raw(dev, 0x0);
    }
  }
}

int PLATFORM_initVmu(char* vmuPath, int port, int slot) {
  if (vmuPath != NULL) {
    strncpy(vmu_path, vmuPath, 8);
    vmu_port = port;
    vmu_slot = slot;
  } else {
    memset(vmu_path, '\0', 8);
    vmu_port = -1;
    vmu_slot = -1;
  }

  printf("PLATFORM_initVmu %s %d %d\n", vmuPath, port, slot);

  return 1;
}

bool PLATFORM_saveToVmu(const char* name) {
  printf("PLATFORM_saveToVmu %s\n", name);

  if (vmu_path[0] == 0) {
    return true;
  }

  vmu_pkg_t   pkg;
  uint8       *pkg_out;
  int     pkg_size;
  file_t      f;
  file_t      fram;
  const int   save_size = 512*5;
  const int   unlock_size = 512*3;
  const int   data_size = save_size + save_size + unlock_size;  // tsave, qsave, unlock
  const int   zdata_size = 512*3;
  uint8       data[data_size];
  uint8       zdata[zdata_size];

  strcpy(pkg.desc_short, "VVVVVV");
  strcpy(pkg.desc_long, "VVVVVV");
  strcpy(pkg.app_id, "VVVVVV");
  pkg.icon_cnt = 1;
  pkg.icon_anim_speed = 0;
  pkg.eyecatch_type = VMUPKG_EC_NONE;
  pkg.data_len = zdata_size;
  pkg.data = zdata;
  memcpy(pkg.icon_pal, vvvvvvmu_icon_pal, 32);
  pkg.icon_data = vvvvvvmu_icon_img;

  memset(data, 0, data_size);
  memset(zdata, 0, zdata_size);
  fram = fs_open("/ram/tsave.vvv", O_RDONLY);
  if (fram != -1) {
    fs_read(fram, data, save_size);
    fs_close(fram);
  } else {
    printf("PLATFORM_saveToVmu: Could not open tsave.vvv\n");
  }
  fram = fs_open("/ram/qsave.vvv", O_RDONLY);
  if (fram != -1) {
    fs_read(fram, &data[save_size], save_size);
    fs_close(fram);
  } else {
    printf("PLATFORM_saveToVmu: Could not open qsave.vvv\n");
  }
  fram = fs_open("/ram/unlock.vvv", O_RDONLY);
  if (fram != -1) {
    fs_read(fram, &data[save_size*2], unlock_size);
    fs_close(fram);
  } else {
    printf("PLATFORM_saveToVmu: Could not open unlock.vvv\n");
  }
  
  //for (int i = 0; i < data_size; ++i) {printf("%c", data[i]);}
  printf("\n");

  #define CHUNK 0x4000
  unsigned char out[CHUNK];
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree  = Z_NULL;
  strm.opaque = Z_NULL;
  deflateInit(&strm, 5);
  strm.next_in = (unsigned char *) data;
  strm.avail_in = data_size;
  int temp_have = 0;
  do {
    int have;
    strm.avail_out = CHUNK;
    strm.next_out = out;
    deflate (&strm, Z_FINISH);
    have = CHUNK - strm.avail_out;
    //fwrite (out, sizeof (char), have, stdout);
    memcpy(&zdata[temp_have], out, have);
    printf("deflating: %d bytes\n", have);
    temp_have += have;
  } while (strm.avail_out == 0);
  deflateEnd(&strm);
  //cret = compress(zdata, &dest_len, data, data_size);
  printf("ZLib compressed data %d bytes\n", temp_have);
  //for (int i = 0; i < temp_have; ++i) {printf("%x ", zdata[i]);}
  printf("\n");

  vmu_pkg_build(&pkg, &pkg_out, &pkg_size);

  char tmp[16];
  memset(tmp, '\0', 16);
  strncpy(tmp, vmu_path, 8);
  strncat(tmp, "/VVVVVV\0", 8);
  fs_unlink(tmp);
  f = fs_open(tmp, O_WRONLY);

  if(!f) {
    printf("VMU ERROR: COULD NOT OPEN %s FILE\n", tmp);
    return false;
  }

  fs_write(f, pkg_out, pkg_size);
  fs_close(f);
  
  // Uncomment this for BEEP !!!
  //thd_create(0, PLATFORM_beepVmu, NULL);
    
  printf("VMU: SAVED %s FILE TO VMU %s WITH SIZE %d\n", tmp, vmu_path, pkg_size);

  return true;
}

bool PLATFORM_loadFromVmu(const char* name) {
  vmu_pkg_t pkg;
  uint8 *pkg_out;
  int pkg_size;
  file_t      f;
  file_t      fram;
  const int   save_size = 512*5;
  const int   unlock_size = 512*3;
  const int   data_size = save_size + save_size + unlock_size;  // tsave, qsave, unlock
  const int   zdata_size = 512*3;
  uint8       data[data_size];
  uint8       zdata[zdata_size];

  if (vmu_path[0] == 0) {
    return true;
  }

  memset(data, 0, data_size);
  memset(zdata, 0, zdata_size);

  char tmp[16];
  memset(tmp, '\0', 16);
  strncpy(tmp, vmu_path, 8);
  strncat(tmp, "/VVVVVV\0", 8);
  f = fs_open(tmp, O_RDONLY);

  if(!f) {
    printf("VMU ERROR: COULD NOT OPEN %s FILE\n", tmp);
    return false;
  }

  pkg_size = fs_total(f);
  pkg_out = (uint8_t *)malloc(pkg_size);
  int ret = fs_read(f, pkg_out, pkg_size);
  if ((ret == -1) || (ret < pkg_size)) {
    printf("VMU ERROR: ERROR READING FILE %s: %d\n", tmp, ret);
    return false;
  } else {
    printf("VMU: LOADED %s FILE FROM VMU %s WITH SIZE %d\n", tmp, vmu_path, pkg_size);
  }
  fs_close(f);

  vmu_pkg_parse(pkg_out, &pkg);
  memcpy(zdata, pkg.data, zdata_size);
  free(pkg_out);

  printf("ZLib compressed data %d bytes\n", zdata_size);
  //for (int i = 0; i < zdata_size; ++i) {printf("%x ", zdata[i]);}
  printf("\n");

  // Inflate
  #define CHUNK 0x4000
  unsigned char out[CHUNK];
  z_stream istrm;
  istrm.zalloc = Z_NULL;
  istrm.zfree  = Z_NULL;
  istrm.opaque = Z_NULL;
  inflateInit(&istrm);
  istrm.next_in = (unsigned char*) zdata;
  istrm.avail_in = zdata_size;
  int temp_have = 0;
  do {
    int have;
    istrm.avail_out = CHUNK;
    istrm.next_out = out;
    inflate (&istrm, Z_FINISH);
    have = CHUNK - istrm.avail_out;
    //fwrite (out, sizeof (char), have, stdout);
    memcpy(&data[temp_have], out, have);
    printf("inflating: %d bytes\n", have);
    temp_have += have;
  } while (istrm.avail_out == 0);
  inflateEnd(&istrm);
  printf("ZLib decompressed data %d bytes\n", temp_have);
  //for (int i = 0; i < temp_have; ++i) {printf("%c", data[i]);}
  printf("\n");

  fram = fs_open("/ram/tsave.vvv", O_WRONLY);
  if (fram != -1) {
    fs_write(fram, data, save_size);
    fs_close(fram);
  } else {
    printf("PLATFORM_loadFromVmu: Could not open tsave.vvv\n");
  }
  fram = fs_open("/ram/qsave.vvv", O_WRONLY);
  if (fram != -1) {
    fs_write(fram, &data[save_size], save_size);
    fs_close(fram);
  } else {
    printf("PLATFORM_loadFromVmu: Could not open qsave.vvv\n");
  }
  fram = fs_open("/ram/unlock.vvv", O_WRONLY);
  if (fram != -1) {
    fs_write(fram, &data[save_size*2], unlock_size);
    fs_close(fram);
  } else {
    printf("PLATFORM_loadFromVmu: Could not open unlock.vvv\n");
  }

  printf("PLATFORM_loadFromVmu finished\n");

  return true;
}

void PLATFORM_drawToVmu() {
  if (vmu_port > -1) {
    maple_device_t* dev = NULL;
    dev = maple_enum_dev(vmu_port, vmu_slot);
    if (dev) {
      int ret = vmu_draw_lcd(dev, (void*)vvvvvv_lcd_inv_data);
      if (ret == MAPLE_EOK) {
        printf("Drawn succesfully to VMU LCD %d %d\n", vmu_port, vmu_slot);
      } else {
        printf("Could not draw to VMU LCD %d %d\n", vmu_port, vmu_slot);
      }
    }
  } else {
    printf("Could not draw to VMU LCD %d %d\n", vmu_port, vmu_slot);
  }
}
