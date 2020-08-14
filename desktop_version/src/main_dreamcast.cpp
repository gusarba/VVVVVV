#include <SDL.h>
#include "SoundSystem.h"

#include "UtilityClass.h"
#include "Game.h"
#include "Graphics.h"
#include "KeyPoll.h"
#include "Render.h"

#include "Tower.h"
#include "WarpClass.h"
#include "Labclass.h"
#include "Finalclass.h"
#include "Map.h"

#include "Screen.h"

#include "Script.h"

#include "Logic.h"

#include "Input.h"
#include "editor.h"
#include "preloader.h"

#include "FileSystemUtils.h"
#include "FileSystemUtils_dreamcast.h"
#include "Network.h"

#include <stdio.h>
#include <string.h>

extern "C"
{
#include "SDL_inprint.h"
}

#ifndef DREAMCAST
FILE _iob[] = { *stdin, *stdout, *stderr };

extern "C" FILE * __cdecl __iob_func(void)
{
  return _iob;
}
#endif

#include "memory_stats.h"

#define FS_PREFIX "/cd"
  #if defined(FS_PREFIX_PC)
  #define FS_PREFIX "/pc/VVVVVV/desktop_version/dc/romdisk"
  #elif defined(FS_PREFIX_SD)
  #define FS_PREFIX "/sd"
  #elif defined(FS_PREFIX_CD)
  #define FS_PREFIX "/cd"
  #endif

#include <errno.h>
#include <dirent.h>
#include <dc/sd.h>
#include <kos.h>
#include <fat/fs_fat.h>
#define MNT_MODE FS_FAT_MOUNT_READONLY

kos_blockdev_t sd_dev;
uint8 partition_type;
DIR* d;
struct dirent* entry;

void setup_sd() {
  if (sd_init()) {
    printf("Could not initilize SD card\n");
    return;
  }
  if (sd_blockdev_for_partition(0, &sd_dev, &partition_type)) {
    printf("Could not find the first partition on the SD card!\n");
    return;
  }
  if(fs_fat_init()) {
    printf("Could not initialize fs_fat!\n");
    return;
  }
  if(fs_fat_mount("/sd", &sd_dev, MNT_MODE)) {
    printf("Could not mount SD card\n");
    return;
  }
  printf("Listing the contents of /sd:\n"); 
  if(!(d = opendir("/sd"))) { 
    printf("Could not open /sd: %s\n", strerror(errno));
  }
  while((entry = readdir(d))) {
    printf("%s\n", entry->d_name);
  }
  if(closedir(d)) {
    printf("Could not close directory: %s\n", strerror(errno));
    return;
  }
}
void setup_pc() {
  printf("Listing the contents of " FS_PREFIX ":\n"); 
  if(!(d = opendir(FS_PREFIX))) { 
    printf("Could not open " FS_PREFIX ": %s\n", strerror(errno));
  }
  while((entry = readdir(d))) {
    printf("%s\n", entry->d_name);
  }
  if(closedir(d)) {
    printf("Could not close directory: %s\n", strerror(errno));
    return;
  }
}

// Shared variables from Music
extern int mix_next;
extern int mix_volume;

// VMU management stuff
uint8_t vmu_slots[8] = {0,0,0,0,0,0,0,0};  // A1, A2, B1 . . . D2
typedef char vmu_name_t[8];
vmu_name_t vmu_names[] = {"/vmu/a1", "/vmu/a2", "/vmu/b1", "/vmu/b2", 
                          "/vmu/c1", "/vmu/c2", "/vmu/d1", "/vmu/d2"};
vmu_name_t vmu_pretty_names[] = {"A1", "A2", "B1", "B2", 
                                 "C1", "C2", "D1", "D2"};
int8_t vmu_selected_slot = -1;  // No save
int8_t vmu_saving = 0;  // -1 = Error saving,  0 = Selecting VMU, 
                        //  1 = Saving to VMU, 2 = Saved correctly
                        //  3 = Request saving

scriptclass script;

#if !defined(NO_CUSTOM_LEVELS)
std::vector<edentities> edentity;
editorclass ed;
#endif

UtilityClass help;
Graphics graphics;
musicclass music;
Game game;
KeyPoll key;
mapclass map;
entityclass obj;

void arrayslot2portslot(int arrayslot, int* port, int* slot) {
  if ((arrayslot < 0) || (arrayslot > 7)) {
    *port = -1; *slot = -1;
  } else {
    *port = arrayslot / 2;
    *slot = (arrayslot % 2) + 1;
  }

  printf("arrayslot %d to port %d slot %d\n", arrayslot, *port, *slot);
}

void updatevmus() {
  maple_device_t *vmu;

  for (int8_t port = 0; port < 4; ++port) {
    for (int8_t slot = 1; slot < 3; ++slot) {
      int idx = port*2 + (slot-1);
      vmu_slots[idx] = 0;
      vmu = maple_enum_dev(port, slot);
      if (vmu != 0) {
        if ((vmu->valid) && (vmu->info.functions & MAPLE_FUNC_MEMCARD)) {
          vmu_slots[idx] = 1;
        }
      }
    }
  }

  for (int idx = 0; idx < 8; ++idx) {
    if (vmu_slots[idx] == 1) {
      printf("VMU SLOT %s AVAILABLE\n", vmu_names[idx]);
    }
  }
}

void vmuselectdelay() {
  if ((vmu_saving == -1) || (vmu_saving == 2)) {
    SDL_Delay(2000);
  }
}

void vmuselectinput() {
  static int prev_key = -1;
  if (vmu_saving == 0 ) {
    if (key.isDown(KEYBOARD_UP) || key.controllerWantsLeft(true)) {
      if (prev_key != KEYBOARD_UP) {
        uint8_t done = 0;
        while (done == 0) {
          --vmu_selected_slot;
          if (vmu_selected_slot <= -1) {
            vmu_selected_slot = -1;
            done = 1;
          } else {
            if (vmu_slots[vmu_selected_slot] == 1) {
              done = 1;
            }
          }
        }
      }
      prev_key = KEYBOARD_UP;
    } else if (key.isDown(KEYBOARD_DOWN) || key.controllerWantsRight(true)) {
      if (prev_key != KEYBOARD_DOWN) {
        uint8_t done = 0;
        int8_t prev = vmu_selected_slot;
        while (done == 0) {
          ++vmu_selected_slot;
          if (vmu_selected_slot >= 8) {
            vmu_selected_slot = prev;
            done = 1;
          } else {
            if (vmu_slots[vmu_selected_slot] == 1) {
              done = 1;
            }
          }
        }
      }
      prev_key = KEYBOARD_DOWN;
    } else if (key.controllerButtonDown()) {
      vmu_saving = 3;  
    } else {
      prev_key = -1;
    }
  }
  key.keymap.clear();
}

int vmuselectlogic() {
  static Uint32 ticks = 0;
  static Uint32 last = SDL_GetTicks();
  ticks += (SDL_GetTicks() - last);
  last = SDL_GetTicks();

  // Check if we need to save
  if (vmu_saving == -1) {
    // Dramatic pause
    //SDL_Delay(3000);
    vmu_saving = 0;
  } else if (vmu_saving == 1) {
    // Do we have a previous savefile?
    bool ret = PLATFORM_loadFromVmu("unlock.vvv");
    //game.loadstats();
    if (ret == true) {
      vmu_saving = 2;
      // We have loaded a previous save succesfully
      return 1;
    }
    
    // We don't have a previous savefile. Can we make a new valid savefile?
    //game.savestats();
    ret = PLATFORM_saveToVmu("unlock.vvv");
    if (ret == true) {
      // We have saved succesfully
      vmu_saving = 2;
    } else {
      vmu_saving = -1;
    }
  } else if (vmu_saving == 2) {
      // We have saved succesfully
      return 1;
  } else if (vmu_saving == 3) {
    // Saving was requested, check if "Continue without saving" was chosen
    if (vmu_selected_slot <= -1) {
      PLATFORM_initVmu(NULL, -1, -1);
      vmu_saving = 2;
      return 1;
    } else {
      // Perform VMU access on next frame
      int port = -1;
      int slot = -1;
      arrayslot2portslot(vmu_selected_slot, &port, &slot);
      PLATFORM_initVmu(vmu_names[vmu_selected_slot], port, slot);
      vmu_saving = 1;
    }
  }

  if (ticks >= 2000) {
    updatevmus();
    if (vmu_slots[vmu_selected_slot] == 0) vmu_selected_slot = -1;
    ticks = 0;
  }

  return 0;
}

void vmuselectrender() {
  if (vmu_saving == -1) {
    graphics.Print(10, 60, "There was an error acessing VMU!",
                   255, 255, 255);
  } else if (vmu_saving == 0) {
    graphics.Print(10, 60, "Please select a VMU for saving",
        255, 255, 255);
    graphics.Print(10, 70, "with at least 5 free blocks",
        255, 255, 255);
    graphics.Print(10, 80, "or an existing VVVVVV save file",
        255, 255, 255);
    graphics.Print(30, 100, "Continue without saving",
        255, 255, 255);
    if (vmu_selected_slot == -1) {
      graphics.Print(10, 100, ">>",
          255, 255, 255);
    }

    for (int i = 0; i < 8; ++i) {
      if (vmu_slots[i] == 1) {
        graphics.Print(30, 110 + 10*i, vmu_pretty_names[i],
            255, 255, 255);
        if (vmu_selected_slot == i) {
          graphics.Print(10, 110 + 10*i, ">>",
              255, 255, 255);
        }
      }
    }
  } else if ((vmu_saving == 1) || (vmu_saving == 3)) {
    graphics.Print(10, 60, "Accessing...",
                   255, 255, 255);
    if (vmu_selected_slot > -1) {
      graphics.Print(120, 60, vmu_pretty_names[vmu_selected_slot],
                     255, 255, 255);
    }
  } else if (vmu_saving == 2) {
    graphics.Print(10, 60, "Success",
                   255, 255, 255);
  }

  graphics.render();
}

int main(int argc, char *argv[])
{
    char* baseDir = NULL;
    char* assetsPath = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-renderer") == 0) {
            ++i;
            SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, argv[i], SDL_HINT_OVERRIDE);
        } else if (strcmp(argv[i], "-basedir") == 0) {
            ++i;
            baseDir = argv[i];
        } else if (strcmp(argv[i], "-assets") == 0) {
            ++i;
            assetsPath = argv[i];
        }
    }

    set_system_ram();
    print_ram_stats();

    //setup_pc();
    //setup_sd();
    
    char baseDirDC[64] = "/ram/\0";
    // You have to decompress the contents of data.zip into the root
    // folder of the cd or other media
    char assetsPathDC[64] = FS_PREFIX "/\0";
    baseDir = baseDirDC;
    assetsPath = assetsPathDC;

    if(!FILESYSTEM_init("/cd/1ST_READ.BIN", baseDir, assetsPath))
    {
        return 1;
    }

    SDL_Init(
      SDL_INIT_VIDEO |
      //SDL_INIT_AUDIO |
      SDL_INIT_CDROM |
      SDL_INIT_JOYSTICK); /* |
        SDL_INIT_GAMECONTROLLER
    );*/
    
    print_ram_stats();

    //NETWORK_init();

    // Initialize joysticks
    const char *name;
    int i;
    SDL_Joystick *joystick;
    printf("There are %d joysticks attached\n", SDL_NumJoysticks());
    for (i = 0; i<SDL_NumJoysticks(); ++i) {
      name = SDL_JoystickName(i);
      printf("Joystick %d: %s\n", i, name ? name : "Unknown Joystick");
      joystick = SDL_JoystickOpen(i);
      if (joystick == NULL) {
        fprintf(stderr, "SDL_JoystickOpen(%d) failed: %s\n", i, SDL_GetError());

      } else {
        printf("       axes: %d\n", SDL_JoystickNumAxes(joystick));
        printf("      balls: %d\n", SDL_JoystickNumBalls(joystick));
        printf("       hats: %d\n", SDL_JoystickNumHats(joystick));
        printf("    buttons: %d\n", SDL_JoystickNumButtons(joystick));
      }
    }


    // VMU preparation
    updatevmus();
    for (int idx = 0; idx < 8; ++idx) {
      if (vmu_slots[idx] == 1) {
        printf("VMU SLOT %s AVAILABLE\n", vmu_names[idx]);
      }
    }

    printf("Creating gameScreen\n");
    Screen gameScreen;

    printf("\t\t\n");
    printf("\t\t\n");
    printf("\t\t       VVVVVV\n");
    printf("\t\t\n");
    printf("\t\t\n");
    printf("\t\t  8888888888888888  \n");
    printf("\t\t88888888888888888888\n");
    printf("\t\t888888    8888    88\n");
    printf("\t\t888888    8888    88\n");
    printf("\t\t88888888888888888888\n");
    printf("\t\t88888888888888888888\n");
    printf("\t\t888888            88\n");
    printf("\t\t88888888        8888\n");
    printf("\t\t  8888888888888888  \n");
    printf("\t\t      88888888      \n");
    printf("\t\t  8888888888888888  \n");
    printf("\t\t88888888888888888888\n");
    printf("\t\t88888888888888888888\n");
    printf("\t\t88888888888888888888\n");
    printf("\t\t8888  88888888  8888\n");
    printf("\t\t8888  88888888  8888\n");
    printf("\t\t    888888888888    \n");
    printf("\t\t    8888    8888    \n");
    printf("\t\t  888888    888888  \n");
    printf("\t\t  888888    888888  \n");
    printf("\t\t  888888    888888  \n");
    printf("\t\t\n");
    printf("\t\t\n");

    //Set up screen


    
    print_ram_stats();


    // Load Ini

    graphics.init();
    
    print_ram_stats();



    music.init();
    //game.init();
    //game.infocus = true;

    graphics.MakeTileArray();
    graphics.MakeSpriteArray();
    graphics.maketelearray();


    graphics.images.push_back(graphics.grphx.im_image0);
    graphics.images.push_back(graphics.grphx.im_image1);
    graphics.images.push_back(graphics.grphx.im_image2);
    graphics.images.push_back(graphics.grphx.im_image3);
    graphics.images.push_back(graphics.grphx.im_image4);
    graphics.images.push_back(graphics.grphx.im_image5);
    graphics.images.push_back(graphics.grphx.im_image6);

    graphics.images.push_back(graphics.grphx.im_image7);
    graphics.images.push_back(graphics.grphx.im_image8);
    graphics.images.push_back(graphics.grphx.im_image9);
    graphics.images.push_back(graphics.grphx.im_image10);
    graphics.images.push_back(graphics.grphx.im_image11);
    graphics.images.push_back(graphics.grphx.im_image12);
    
    const SDL_PixelFormat* fmt = gameScreen.GetFormat();    
    graphics.backBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    SDL_SetSurfaceBlendMode(graphics.backBuffer, SDL_BLENDMODE_NONE);
    graphics.footerbuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 10, fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    SDL_SetSurfaceBlendMode(graphics.footerbuffer, SDL_BLENDMODE_BLEND);

    //SDL_SetSurfaceAlphaMod(graphics.footerbuffer, 127);
    //SDL_SetAlpha(graphics.footerbuffer, SDL_SRCALPHA | SDL_RLEACCEL, 127);
    SDL_SetColorKey(graphics.footerbuffer, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(graphics.footerbuffer->format, 0, 0, 0));

    FillRect(graphics.footerbuffer, SDL_MapRGB(fmt, 0, 0, 0));
    graphics.Makebfont();


    graphics.foregroundBuffer =  SDL_CreateRGBSurface(SDL_SWSURFACE ,320 ,240 ,fmt->BitsPerPixel,fmt->Rmask,fmt->Gmask,fmt->Bmask,fmt->Amask  );
    SDL_SetSurfaceBlendMode(graphics.foregroundBuffer, SDL_BLENDMODE_NONE);

    graphics.screenbuffer = &gameScreen;

    graphics.menubuffer = SDL_CreateRGBSurface(SDL_SWSURFACE ,320 ,240 ,fmt->BitsPerPixel,fmt->Rmask,fmt->Gmask,fmt->Bmask,fmt->Amask );
    SDL_SetSurfaceBlendMode(graphics.menubuffer, SDL_BLENDMODE_NONE);

    graphics.towerbuffer =  SDL_CreateRGBSurface(SDL_SWSURFACE ,320 ,240 ,fmt->BitsPerPixel,fmt->Rmask,fmt->Gmask,fmt->Bmask,fmt->Amask  );
    SDL_SetSurfaceBlendMode(graphics.towerbuffer, SDL_BLENDMODE_NONE);

    graphics.tempBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE ,320 ,240 ,fmt->BitsPerPixel,fmt->Rmask,fmt->Gmask,fmt->Bmask,fmt->Amask  );
    SDL_SetSurfaceBlendMode(graphics.tempBuffer, SDL_BLENDMODE_NONE);

    int vmu_ret = 0;
    while(vmu_ret == 0) {
      vmuselectdelay();
      key.Poll();
      vmuselectinput();
      key.keymap.clear();
      FillRect(graphics.backBuffer, 0x00000000);
      vmuselectrender();
      vmu_ret = vmuselectlogic();
      gameScreen.FlipScreen();
    }
    
    // Draw to VMU LCD screen
    PLATFORM_drawToVmu();

    FillRect(graphics.backBuffer, 0x00000000);
    gameScreen.FlipScreen();

    game.init();
    game.infocus = true;

    game.gamestate = PRELOADER;

    game.menustart = false;
    game.mainmenu = 0;

    map.ypos = (700-29) * 8;
    map.bypos = map.ypos / 2;

    // VMU selection "scene"


    //Moved screensetting init here from main menu V2.1
    game.loadstats();
    if (game.skipfakeload)
        game.gamestate = TITLEMODE;
    if(game.usingmmmmmm==0) music.usingmmmmmm=false;
    if(game.usingmmmmmm==1) music.usingmmmmmm=true;
    if (game.slowdown == 0) game.slowdown = 30;

    switch(game.slowdown){
        case 30: game.gameframerate=34; break;
        case 24: game.gameframerate=41; break;
        case 18: game.gameframerate=55; break;
        case 12: game.gameframerate=83; break;
        default: game.gameframerate=34; break;
    }

#ifndef DREAMCAST
    //Check to see if you've already unlocked some achievements here from before the update
    if (game.swnbestrank > 0){
        if(game.swnbestrank >= 1) NETWORK_unlockAchievement("vvvvvvsupgrav5");
        if(game.swnbestrank >= 2) NETWORK_unlockAchievement("vvvvvvsupgrav10");
        if(game.swnbestrank >= 3) NETWORK_unlockAchievement("vvvvvvsupgrav15");
        if(game.swnbestrank >= 4) NETWORK_unlockAchievement("vvvvvvsupgrav20");
        if(game.swnbestrank >= 5) NETWORK_unlockAchievement("vvvvvvsupgrav30");
        if(game.swnbestrank >= 6) NETWORK_unlockAchievement("vvvvvvsupgrav60");
    }

    if(game.unlock[5]) NETWORK_unlockAchievement("vvvvvvgamecomplete");
    if(game.unlock[19]) NETWORK_unlockAchievement("vvvvvvgamecompleteflip");
    if(game.unlock[20]) NETWORK_unlockAchievement("vvvvvvmaster");

    if (game.bestgamedeaths > -1) {
        if (game.bestgamedeaths <= 500) {
            NETWORK_unlockAchievement("vvvvvvcomplete500");
        }
        if (game.bestgamedeaths <= 250) {
            NETWORK_unlockAchievement("vvvvvvcomplete250");
        }
        if (game.bestgamedeaths <= 100) {
            NETWORK_unlockAchievement("vvvvvvcomplete100");
        }
        if (game.bestgamedeaths <= 50) {
            NETWORK_unlockAchievement("vvvvvvcomplete50");
        }
    }

    if(game.bestrank[0]>=3) NETWORK_unlockAchievement("vvvvvvtimetrial_station1_fixed");
    if(game.bestrank[1]>=3) NETWORK_unlockAchievement("vvvvvvtimetrial_lab_fixed");
    if(game.bestrank[2]>=3) NETWORK_unlockAchievement("vvvvvvtimetrial_tower_fixed");
    if(game.bestrank[3]>=3) NETWORK_unlockAchievement("vvvvvvtimetrial_station2_fixed");
    if(game.bestrank[4]>=3) NETWORK_unlockAchievement("vvvvvvtimetrial_warp_fixed");
    if(game.bestrank[5]>=3) NETWORK_unlockAchievement("vvvvvvtimetrial_final_fixed");
#else
    // TODO
#endif

    obj.init();

    volatile Uint32 time, timePrev = 0;
    game.infocus = true;
    key.isActive = true;

    printf("Main loop starting...\n");

    float render_time = 0.0f;
    float logic_time = 0.0f;
    float input_time = 0.0f;

    while(!key.quitProgram)
    {
      static Uint32 framecounter = 0;
      static Uint32 last_frame = 1;
      static Uint32 elapsed = 0;
      ++framecounter;      

        time = SDL_GetTicks();
        //printf("Tick...\n");

        // Update network per frame.
        NETWORK_update();

        //framerate limit to 30
        Uint32 timetaken = time - timePrev;
        if(game.gamestate==EDITORMODE)
        {
            if (timetaken < 24)
            {
                volatile Uint32 delay = 24 - timetaken;
                SDL_Delay( delay );
                time = SDL_GetTicks();
            }
            timePrev = time;

        }else{
            if (timetaken < game.gameframerate)
            {
                volatile Uint32 delay = game.gameframerate - timetaken;                
                SDL_Delay( delay );
                time = SDL_GetTicks();
            }
            timePrev = time;

        }

        elapsed += timetaken;
        static char tmpfps[32] = {"\0"};
        if (elapsed >= 1000) {
          float fps = ((float)(framecounter - last_frame) / (float)elapsed) * 1000.0f;
          printf("FPS = %f\n", fps);
          sprintf(tmpfps, "FPS = %f\0", fps);
          elapsed = 0;
          last_frame = framecounter;
        }

        key.Poll();
        if(key.toggleFullscreen)
        {
            if(!gameScreen.isWindowed)
            {
                SDL_ShowCursor(SDL_DISABLE);
                SDL_ShowCursor(SDL_ENABLE);
            }
            else
            {
                SDL_ShowCursor(SDL_ENABLE);
            }


            if(game.gamestate == EDITORMODE)
            {
                SDL_ShowCursor(SDL_ENABLE);
            }

            gameScreen.toggleFullScreen();
            game.fullscreen = !game.fullscreen;
            key.toggleFullscreen = false;

            key.keymap.clear(); //we lost the input due to a new window.
            game.press_left = false;
            game.press_right = false;
            game.press_action = true;
            game.press_map = false;
        }

        game.infocus = key.isActive;
        if(!game.infocus)
        {
            if(game.getGlobalSoundVol()> 0)
            {
                game.setGlobalSoundVol(0);
            }
            FillRect(graphics.backBuffer, 0x00000000);
            graphics.bprint(5, 110, "Game paused", 196 - help.glow, 255 - help.glow, 196 - help.glow, true);
            graphics.bprint(5, 120, "[click to resume]", 196 - help.glow, 255 - help.glow, 196 - help.glow, true);
            graphics.bprint(5, 230, "Press M to mute in game", 164 - help.glow, 196 - help.glow, 164 - help.glow, true);
            graphics.render();
            //We are minimised, so lets put a bit of a delay to save CPU
            SDL_Delay(100);
        }
        else
        {
            switch(game.gamestate)
            {
            case PRELOADER:
                //Render
                preloaderrender();
                break;
#if !defined(NO_CUSTOM_LEVELS)
            case EDITORMODE:
                graphics.flipmode = false;
                //Input
                editorinput();
                //Render
                editorrender();
                ////Logic
                editorlogic();
                break;
#endif
            case TITLEMODE:
                //Input
                titleinput();
                //Render
                titlerender();
                ////Logic
                titlelogic();
                break;
            case GAMEMODE:
                if (map.towermode)
                {
                    gameinput();
                    towerrender();
                    towerlogic();

                }
                else
                {

                    if (script.running)
                    {
                        script.run();
                    }

                    input_time = SDL_GetTicks();
                    gameinput();
                    input_time = SDL_GetTicks() - input_time;
                    render_time = SDL_GetTicks();
                    gamerender();
                    render_time = SDL_GetTicks() - render_time;
                    logic_time = SDL_GetTicks();
                    gamelogic();
                    logic_time = SDL_GetTicks() - logic_time;


                    break;
                case MAPMODE:
                    maprender();
                    mapinput();
                    maplogic();
                    break;
                case TELEPORTERMODE:
                    teleporterrender();
                    if(game.useteleporter)
                    {
                        teleporterinput();
                    }
                    else
                    {
                        if (script.running)
                        {
                            script.run();
                        }
                        gameinput();
                    }
                    maplogic();
                    break;
                case GAMECOMPLETE:
                    gamecompleterender();
                    //Input
                    gamecompleteinput();
                    //Logic
                    gamecompletelogic();
                    break;
                case GAMECOMPLETE2:
                    gamecompleterender2();
                    //Input
                    gamecompleteinput2();
                    //Logic
                    gamecompletelogic2();
                    break;
                case CLICKTOSTART:
                    help.updateglow();
                    break;
                default:

                break;
                }

            }

        }

        //We did editorinput, now it's safe to turn this off
        key.linealreadyemptykludge = false;

        if (game.savemystats)
        {
            game.savemystats = false;
            game.savestats();
        }

        //Mute button
#if !defined(NO_CUSTOM_LEVELS)
        bool inEditor = ed.textentry || ed.scripthelppage == 1;
#else
        bool inEditor = false;
#endif
        if (key.isDown(KEYBOARD_m) && game.mutebutton<=0 && !inEditor)
        {
            game.mutebutton = 8;
            if (game.muted)
            {
                game.muted = false;
            }
            else
            {
                game.muted = true;
            }
        }
        if(game.mutebutton>0)
        {
            game.mutebutton--;
        }

        if (game.muted)
        {
            game.globalsound = 0;
            spu_cdda_volume(0, 0);
            mix_volume = 0;
        }

        if (!game.muted && game.globalsound == 0)
        {
            game.globalsound = 1;
            spu_cdda_volume(15, 15);
            mix_volume = 15;
        }

        if (key.resetWindow)
        {
            key.resetWindow = false;
            gameScreen.ResizeScreen(-1, -1);
        }

        music.processmusic();
        graphics.processfade();
        game.gameclock();

        // GUSARBA: Uncomment these to get performance debug prints
        /*
        prepare_inline_font();
        incolor(0xFF0000, 0x333333);
        inprint(gameScreen.m_window, tmpfps, 10, 18);
        char tmprt[80] = {0};
        sprintf(tmprt, "it %f rt %f lt %f\0", input_time, render_time, logic_time);
        inprint(gameScreen.m_window, tmprt, 10, 26);
        char tmpmus[80] = {0};
        sprintf(tmpmus, "mix_next %d mix_volume %d\0", mix_next, mix_volume);
        inprint(gameScreen.m_window, tmpmus, 10, 34);
        */

        gameScreen.FlipScreen();
    }

    // Close joysticks
    for (i = 0; i < SDL_NumJoysticks(); ++i) {
      SDL_Joystick *joystick;
      joystick = SDL_JoystickOpen(i);
      if (joystick == NULL) {
        fprintf(stderr, "SDL_JoystickOpen(%d) failed: %s\n", i, SDL_GetError());
      } else {
        SDL_JoystickClose(joystick);
      }
    }

    game.savestats();
    NETWORK_shutdown();
    SDL_Quit();

    //fs_fat_unmount("/sd");
    //fs_fat_shutdown();
    //sd_shutdown();

    FILESYSTEM_deinit();

    return 0;
}
