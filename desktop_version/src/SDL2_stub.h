
#ifndef SDL2_STUB
#define SDL2_STUB 1

#include <assert.h>

#define SDL_assert(X) assert(X)


// Keyboard
typedef Sint32 SDL_Keycode;


// Joystick
/* A structure that encodes the stable unique id for a joystick device */
typedef struct {
  Uint8 data[16];
} SDL_JoystickGUID;
typedef Sint32 SDL_JoystickID;

// Uncomment these if you are using Chui's SDL port
//#define SDL_CONTROLLERBUTTONDOWN SDL_JOYBUTTONUP
//#define SDL_CONTROLLERBUTTONUP SDL_JOYBUTTONDOWN

// Uncomment these if you are using kos-ports (Bero's) SDL
#define SDL_CONTROLLERBUTTONDOWN SDL_JOYBUTTONDOWN
#define SDL_CONTROLLERBUTTONUP SDL_JOYBUTTONUP

#define SDL_CONTROLLERAXISMOTION SDL_JOYAXISMOTION
#define SDL_CONTROLLER_AXIS_LEFTX SDL_JOYSTICK_AXIS_LEFTX
#define SDL_CONTROLLER_AXIS_LEFTY SDL_JOYSTICK_AXIS_LEFTY


/* SDL_gamecontroller.h*/
#ifdef DREAMCAST
#include <stdint.h>
#include <stdlib.h>
#else
#include "SDL_stdinc.h"
#endif
#include "SDL_error.h"
#include "SDL_rwops.h"
#include "SDL_joystick.h"

//#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

  /**
  *  \file SDL_gamecontroller.h
  *
  *  In order to use these functions, SDL_Init() must have been called
  *  with the ::SDL_INIT_GAMECONTROLLER flag.  This causes SDL to scan the system
  *  for game controllers, and load appropriate drivers.
  *
  *  If you would like to receive controller updates while the application
  *  is in the background, you should set the following hint before calling
  *  SDL_Init(): SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS
  */

  /**
  * The gamecontroller structure used to identify an SDL game controller
  */
  struct _SDL_GameController;
  typedef struct _SDL_GameController SDL_GameController;

  typedef enum
  {
    SDL_CONTROLLER_TYPE_UNKNOWN = 0,
    SDL_CONTROLLER_TYPE_XBOX360,
    SDL_CONTROLLER_TYPE_XBOXONE,
    SDL_CONTROLLER_TYPE_PS3,
    SDL_CONTROLLER_TYPE_PS4,
    SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO
  } SDL_GameControllerType;

  typedef enum
  {
    SDL_CONTROLLER_BINDTYPE_NONE = 0,
    SDL_CONTROLLER_BINDTYPE_BUTTON,
    SDL_CONTROLLER_BINDTYPE_AXIS,
    SDL_CONTROLLER_BINDTYPE_HAT
  } SDL_GameControllerBindType;

  /**
  *  Get the SDL joystick layer binding for this controller button/axis mapping
  */
  typedef struct SDL_GameControllerButtonBind
  {
    SDL_GameControllerBindType bindType;
    union
    {
      int button;
      int axis;
      struct {
        int hat;
        int hat_mask;
      } hat;
    } value;

  } SDL_GameControllerButtonBind;


  /**
  *  To count the number of game controllers in the system for the following:
  *  int nJoysticks = SDL_NumJoysticks();
  *  int nGameControllers = 0;
  *  for (int i = 0; i < nJoysticks; i++) {
  *      if (SDL_IsGameController(i)) {
  *          nGameControllers++;
  *      }
  *  }
  *
  *  Using the SDL_HINT_GAMECONTROLLERCONFIG hint or the SDL_GameControllerAddMapping() you can add support for controllers SDL is unaware of or cause an existing controller to have a different binding. The format is:
  *  guid,name,mappings
  *
  *  Where GUID is the string value from SDL_JoystickGetGUIDString(), name is the human readable string for the device and mappings are controller mappings to joystick ones.
  *  Under Windows there is a reserved GUID of "xinput" that covers any XInput devices.
  *  The mapping format for joystick is:
  *      bX - a joystick button, index X
  *      hX.Y - hat X with value Y
  *      aX - axis X of the joystick
  *  Buttons can be used as a controller axis and vice versa.
  *
  *  This string shows an example of a valid mapping for a controller
  *  "03000000341a00003608000000000000,PS3 Controller,a:b1,b:b2,y:b3,x:b0,start:b9,guide:b12,back:b8,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftshoulder:b4,rightshoulder:b5,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b6,righttrigger:b7",
  *
  */

  /**
  *  Load a set of mappings from a seekable SDL data stream (memory or file), filtered by the current SDL_GetPlatform()
  *  A community sourced database of controllers is available at https://raw.github.com/gabomdq/SDL_GameControllerDB/master/gamecontrollerdb.txt
  *
  *  If \c freerw is non-zero, the stream will be closed after being read.
  *
  * \return number of mappings added, -1 on error
  */
  extern DECLSPEC int SDLCALL SDL_GameControllerAddMappingsFromRW(SDL_RWops * rw, int freerw);

  /**
  *  Load a set of mappings from a file, filtered by the current SDL_GetPlatform()
  *
  *  Convenience macro.
  */
#define SDL_GameControllerAddMappingsFromFile(file)   SDL_GameControllerAddMappingsFromRW(SDL_RWFromFile(file, "rb"), 1)

  /**
  *  Add or update an existing mapping configuration
  *
  * \return 1 if mapping is added, 0 if updated, -1 on error
  */
  extern DECLSPEC int SDLCALL SDL_GameControllerAddMapping(const char* mappingString);

  /**
  *  Get the number of mappings installed
  *
  *  \return the number of mappings
  */
  extern DECLSPEC int SDLCALL SDL_GameControllerNumMappings(void);

  /**
  *  Get the mapping at a particular index.
  *
  *  \return the mapping string.  Must be freed with SDL_free().  Returns NULL if the index is out of range.
  */
  extern DECLSPEC char * SDLCALL SDL_GameControllerMappingForIndex(int mapping_index);

  /**
  *  Get a mapping string for a GUID
  *
  *  \return the mapping string.  Must be freed with SDL_free().  Returns NULL if no mapping is available
  */
  extern DECLSPEC char * SDLCALL SDL_GameControllerMappingForGUID(SDL_JoystickGUID guid);

  /**
  *  Get a mapping string for an open GameController
  *
  *  \return the mapping string.  Must be freed with SDL_free().  Returns NULL if no mapping is available
  */
  extern DECLSPEC char * SDLCALL SDL_GameControllerMapping(SDL_GameController * gamecontroller);

  /**
  *  Is the joystick on this index supported by the game controller interface?
  */
  extern DECLSPEC SDL_bool SDLCALL SDL_IsGameController(int joystick_index);

  /**
  *  Get the implementation dependent name of a game controller.
  *  This can be called before any controllers are opened.
  *  If no name can be found, this function returns NULL.
  */
  extern DECLSPEC const char *SDLCALL SDL_GameControllerNameForIndex(int joystick_index);

  /**
  *  Get the type of a game controller.
  *  This can be called before any controllers are opened.
  */
  extern DECLSPEC SDL_GameControllerType SDLCALL SDL_GameControllerTypeForIndex(int joystick_index);

  /**
  *  Get the mapping of a game controller.
  *  This can be called before any controllers are opened.
  *
  *  \return the mapping string.  Must be freed with SDL_free().  Returns NULL if no mapping is available
  */
  extern DECLSPEC char *SDLCALL SDL_GameControllerMappingForDeviceIndex(int joystick_index);

  /**
  *  Open a game controller for use.
  *  The index passed as an argument refers to the N'th game controller on the system.
  *  This index is not the value which will identify this controller in future
  *  controller events.  The joystick's instance id (::SDL_JoystickID) will be
  *  used there instead.
  *
  *  \return A controller identifier, or NULL if an error occurred.
  */
  extern DECLSPEC SDL_GameController *SDLCALL SDL_GameControllerOpen(int joystick_index);

  /**
  * Return the SDL_GameController associated with an instance id.
  */
  extern DECLSPEC SDL_GameController *SDLCALL SDL_GameControllerFromInstanceID(SDL_JoystickID joyid);

  /**
  * Return the SDL_GameController associated with a player index.
  */
  extern DECLSPEC SDL_GameController *SDLCALL SDL_GameControllerFromPlayerIndex(int player_index);

  /**
  *  Return the name for this currently opened controller
  */
  extern DECLSPEC const char *SDLCALL SDL_GameControllerName(SDL_GameController *gamecontroller);

  /**
  *  Return the type of this currently opened controller
  */
  extern DECLSPEC SDL_GameControllerType SDLCALL SDL_GameControllerGetType(SDL_GameController *gamecontroller);

  /**
  *  Get the player index of an opened game controller, or -1 if it's not available
  *
  *  For XInput controllers this returns the XInput user index.
  */
  extern DECLSPEC int SDLCALL SDL_GameControllerGetPlayerIndex(SDL_GameController *gamecontroller);

  /**
  *  Set the player index of an opened game controller
  */
  extern DECLSPEC void SDLCALL SDL_GameControllerSetPlayerIndex(SDL_GameController *gamecontroller, int player_index);

  /**
  *  Get the USB vendor ID of an opened controller, if available.
  *  If the vendor ID isn't available this function returns 0.
  */
  extern DECLSPEC Uint16 SDLCALL SDL_GameControllerGetVendor(SDL_GameController * gamecontroller);

  /**
  *  Get the USB product ID of an opened controller, if available.
  *  If the product ID isn't available this function returns 0.
  */
  extern DECLSPEC Uint16 SDLCALL SDL_GameControllerGetProduct(SDL_GameController * gamecontroller);

  /**
  *  Get the product version of an opened controller, if available.
  *  If the product version isn't available this function returns 0.
  */
  extern DECLSPEC Uint16 SDLCALL SDL_GameControllerGetProductVersion(SDL_GameController * gamecontroller);

  /**
  *  Returns SDL_TRUE if the controller has been opened and currently connected,
  *  or SDL_FALSE if it has not.
  */
  extern DECLSPEC SDL_bool SDLCALL SDL_GameControllerGetAttached(SDL_GameController *gamecontroller);

  /**
  *  Get the underlying joystick object used by a controller
  */
  extern DECLSPEC SDL_Joystick *SDLCALL SDL_GameControllerGetJoystick(SDL_GameController *gamecontroller);

  /**
  *  Enable/disable controller event polling.
  *
  *  If controller events are disabled, you must call SDL_GameControllerUpdate()
  *  yourself and check the state of the controller when you want controller
  *  information.
  *
  *  The state can be one of ::SDL_QUERY, ::SDL_ENABLE or ::SDL_IGNORE.
  */
  extern DECLSPEC int SDLCALL SDL_GameControllerEventState(int state);

  /**
  *  Update the current state of the open game controllers.
  *
  *  This is called automatically by the event loop if any game controller
  *  events are enabled.
  */
  extern DECLSPEC void SDLCALL SDL_GameControllerUpdate(void);


  /**
  *  The list of axes available from a controller
  *
  *  Thumbstick axis values range from SDL_JOYSTICK_AXIS_MIN to SDL_JOYSTICK_AXIS_MAX,
  *  and are centered within ~8000 of zero, though advanced UI will allow users to set
  *  or autodetect the dead zone, which varies between controllers.
  *
  *  Trigger axis values range from 0 to SDL_JOYSTICK_AXIS_MAX.
  */
  typedef enum
  {
    SDL_CONTROLLER_AXIS_INVALID = -1,
    SDL_CONTROLLER_AXIS_LEFTX,
    SDL_CONTROLLER_AXIS_LEFTY,
    SDL_CONTROLLER_AXIS_RIGHTX,
    SDL_CONTROLLER_AXIS_RIGHTY,
    SDL_CONTROLLER_AXIS_TRIGGERLEFT,
    SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
    SDL_CONTROLLER_AXIS_MAX
  } SDL_GameControllerAxis;

  /**
  *  turn this string into a axis mapping
  */
  extern DECLSPEC SDL_GameControllerAxis SDLCALL SDL_GameControllerGetAxisFromString(const char *pchString);

  /**
  *  turn this axis enum into a string mapping
  */
  extern DECLSPEC const char* SDLCALL SDL_GameControllerGetStringForAxis(SDL_GameControllerAxis axis);

  /**
  *  Get the SDL joystick layer binding for this controller button mapping
  */
  extern DECLSPEC SDL_GameControllerButtonBind SDLCALL
    SDL_GameControllerGetBindForAxis(SDL_GameController *gamecontroller,
      SDL_GameControllerAxis axis);

  /**
  *  Get the current state of an axis control on a game controller.
  *
  *  The state is a value ranging from -32768 to 32767 (except for the triggers,
  *  which range from 0 to 32767).
  *
  *  The axis indices start at index 0.
  */
  extern DECLSPEC Sint16 SDLCALL
    SDL_GameControllerGetAxis(SDL_GameController *gamecontroller,
      SDL_GameControllerAxis axis);

  /**
  *  The list of buttons available from a controller
  */
  typedef enum
  {
    // Uncomment these if you are using kos-ports (Bero's) SDL    
    SDL_CONTROLLER_BUTTON_INVALID = -1,
    SDL_CONTROLLER_BUTTON_A,
    SDL_CONTROLLER_BUTTON_B,
    SDL_CONTROLLER_BUTTON_X,
    SDL_CONTROLLER_BUTTON_Y,    
    SDL_CONTROLLER_BUTTON_START,    
    SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
    SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,    
    SDL_CONTROLLER_BUTTON_GUIDE,
    SDL_CONTROLLER_BUTTON_LEFTSTICK,
    SDL_CONTROLLER_BUTTON_RIGHTSTICK,
    SDL_CONTROLLER_BUTTON_BACK,
    SDL_CONTROLLER_BUTTON_DPAD_UP = 12,
    SDL_CONTROLLER_BUTTON_DPAD_RIGHT,    
    SDL_CONTROLLER_BUTTON_DPAD_DOWN,
    SDL_CONTROLLER_BUTTON_DPAD_LEFT,    
    SDL_CONTROLLER_BUTTON_MAX
    
    // Uncomment these if you are using Chui's SDL port
    /*
    SDL_CONTROLLER_BUTTON_INVALID = -1,
    SDL_CONTROLLER_BUTTON_GUIDE,
    SDL_CONTROLLER_BUTTON_B = 1,
    SDL_CONTROLLER_BUTTON_A = 2,
    SDL_CONTROLLER_BUTTON_START = 3,    
    SDL_CONTROLLER_BUTTON_BACK,
    SDL_CONTROLLER_BUTTON_Y = 5,    
    SDL_CONTROLLER_BUTTON_X = 6,
    SDL_CONTROLLER_BUTTON_LEFTSHOULDER = 7,
    SDL_CONTROLLER_BUTTON_RIGHTSHOULDER = 8,    
    SDL_CONTROLLER_BUTTON_DPAD_UP = 9,
    SDL_CONTROLLER_BUTTON_DPAD_DOWN = 10,
    SDL_CONTROLLER_BUTTON_DPAD_LEFT = 11,    
    SDL_CONTROLLER_BUTTON_DPAD_RIGHT = 12,    
    SDL_CONTROLLER_BUTTON_LEFTSTICK,
    SDL_CONTROLLER_BUTTON_RIGHTSTICK,
    SDL_CONTROLLER_BUTTON_MAX
    */
  } SDL_GameControllerButton;

  /**
  *  turn this string into a button mapping
  */
  extern DECLSPEC SDL_GameControllerButton SDLCALL SDL_GameControllerGetButtonFromString(const char *pchString);

  /**
  *  turn this button enum into a string mapping
  */
  extern DECLSPEC const char* SDLCALL SDL_GameControllerGetStringForButton(SDL_GameControllerButton button);

  /**
  *  Get the SDL joystick layer binding for this controller button mapping
  */
  extern DECLSPEC SDL_GameControllerButtonBind SDLCALL
    SDL_GameControllerGetBindForButton(SDL_GameController *gamecontroller,
      SDL_GameControllerButton button);


  /**
  *  Get the current state of a button on a game controller.
  *
  *  The button indices start at index 0.
  */
  extern DECLSPEC Uint8 SDLCALL SDL_GameControllerGetButton(SDL_GameController *gamecontroller,
    SDL_GameControllerButton button);

  /**
  *  Trigger a rumble effect
  *  Each call to this function cancels any previous rumble effect, and calling it with 0 intensity stops any rumbling.
  *
  *  \param gamecontroller The controller to vibrate
  *  \param low_frequency_rumble The intensity of the low frequency (left) rumble motor, from 0 to 0xFFFF
  *  \param high_frequency_rumble The intensity of the high frequency (right) rumble motor, from 0 to 0xFFFF
  *  \param duration_ms The duration of the rumble effect, in milliseconds
  *
  *  \return 0, or -1 if rumble isn't supported on this joystick
  */
  extern DECLSPEC int SDLCALL SDL_GameControllerRumble(SDL_GameController *gamecontroller, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble, Uint32 duration_ms);

  /**
  *  Close a controller previously opened with SDL_GameControllerOpen().
  */
  extern DECLSPEC void SDLCALL SDL_GameControllerClose(SDL_GameController *gamecontroller);


  /* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
/* END SDL_gamecontroller.h*/

/* SDL_gamecontroller.c */
//#include "../SDL_internal.h"
#include "SDL.h"

/* This is the game controller API for Simple DirectMedia Layer */

#include "SDL_events.h"
//#include "SDL_assert.h"
//#include "SDL_hints.h"
#include "SDL_timer.h"
//#include "SDL_sysjoystick.h"
//#include "SDL_joystick_c.h"
#include "SDL_joystick.h"
//#include "SDL_gamecontrollerdb.h"

#if !SDL_EVENTS_DISABLED
//#include "../events/SDL_events_c.h"
#include "SDL_events.h"
#endif

/* Many controllers turn the center button into an instantaneous button press */
#define SDL_MINIMUM_GUIDE_BUTTON_DELAY_MS   250

#define SDL_CONTROLLER_PLATFORM_FIELD   "platform:"
#define SDL_CONTROLLER_HINT_FIELD       "hint:"
#define SDL_CONTROLLER_SDKGE_FIELD      "sdk>=:"
#define SDL_CONTROLLER_SDKLE_FIELD      "sdk<=:"

struct _SDL_GameController;
typedef struct _SDL_GameController SDL_GameController;

/* a list of currently opened game controllers */
static SDL_GameController *SDL_gamecontrollers = NULL;

typedef struct
{
  SDL_GameControllerBindType inputType;
  union
  {
    int button;

    struct {
      int axis;
      int axis_min;
      int axis_max;
    } axis;

    struct {
      int hat;
      int hat_mask;
    } hat;

  } input;

  SDL_GameControllerBindType outputType;
  union
  {
    SDL_GameControllerButton button;

    struct {
      SDL_GameControllerAxis axis;
      int axis_min;
      int axis_max;
    } axis;

  } output;

} SDL_ExtendedGameControllerBind;

/* our hard coded list of mapping support */
typedef enum
{
  SDL_CONTROLLER_MAPPING_PRIORITY_DEFAULT,
  SDL_CONTROLLER_MAPPING_PRIORITY_API,
  SDL_CONTROLLER_MAPPING_PRIORITY_USER,
} SDL_ControllerMappingPriority;

typedef struct _ControllerMapping_t
{
  SDL_JoystickGUID guid;
  char *name;
  char *mapping;
  SDL_ControllerMappingPriority priority;
  struct _ControllerMapping_t *next;
} ControllerMapping_t;

static SDL_JoystickGUID s_zeroGUID;
static ControllerMapping_t *s_pSupportedControllers = NULL;
static ControllerMapping_t *s_pDefaultMapping = NULL;
static ControllerMapping_t *s_pHIDAPIMapping = NULL;
static ControllerMapping_t *s_pXInputMapping = NULL;

/* The SDL game controller structure */
struct _SDL_GameController
{
  SDL_Joystick *joystick; /* underlying joystick device */
  int ref_count;

  const char *name;
  int num_bindings;
  SDL_ExtendedGameControllerBind *bindings;
  SDL_ExtendedGameControllerBind **last_match_axis;
  Uint8 *last_hat_mask;
  Uint32 guide_button_down;

  struct _SDL_GameController *next; /* pointer to next game controller we have allocated */
};
/* END SDL_gamecontroller.c */





// From SDL2
inline SDL_bool SDL_RectEmpty(const SDL_Rect *r)
{
  return ((!r) || (r->w <= 0) || (r->h <= 0)) ? SDL_TRUE : SDL_FALSE;
}

// From SDL2
inline SDL_bool SDL_HasIntersection(const SDL_Rect * A, const SDL_Rect * B)
{
  int Amin, Amax, Bmin, Bmax;

  if (!A) {
    //SDL_InvalidParamError("A");
    return SDL_FALSE;
  }

  if (!B) {
    //SDL_InvalidParamError("B");
    return SDL_FALSE;
  }

  /* Special cases for empty rects */
  if (SDL_RectEmpty(A) || SDL_RectEmpty(B)) {
    return SDL_FALSE;
  }

  /* Horizontal intersection */
  Amin = A->x;
  Amax = Amin + A->w;
  Bmin = B->x;
  Bmax = Bmin + B->w;
  if (Bmin > Amin)
    Amin = Bmin;
  if (Bmax < Amax)
    Amax = Bmax;
  if (Amax <= Amin)
    return SDL_FALSE;

  /* Vertical intersection */
  Amin = A->y;
  Amax = Amin + A->h;
  Bmin = B->y;
  Bmax = Bmin + B->h;
  if (Bmin > Amin)
    Amin = Bmin;
  if (Bmax < Amax)
    Amax = Bmax;
  if (Amax <= Amin)
    return SDL_FALSE;

  return SDL_TRUE;
}


// Alpha blending
/**
*  \brief The blend mode used in SDL_RenderCopy() and drawing operations.
*/
typedef enum
{
  SDL_BLENDMODE_NONE = 0x00000000,     /**< no blending
                                       dstRGBA = srcRGBA */
  SDL_BLENDMODE_BLEND = 0x00000001,    /**< alpha blending
                                       dstRGB = (srcRGB * srcA) + (dstRGB * (1-srcA))
                                       dstA = srcA + (dstA * (1-srcA)) */
  SDL_BLENDMODE_ADD = 0x00000002,      /**< additive blending
                                       dstRGB = (srcRGB * srcA) + dstRGB
                                       dstA = dstA */
  SDL_BLENDMODE_MOD = 0x00000004,      /**< color modulate
                                       dstRGB = srcRGB * dstRGB
                                       dstA = dstA */
  SDL_BLENDMODE_MUL = 0x00000008,      /**< color multiply
                                       dstRGB = (srcRGB * dstRGB) + (dstRGB * (1-srcA))
                                       dstA = (srcA * dstA) + (dstA * (1-srcA)) */
  SDL_BLENDMODE_INVALID = 0x7FFFFFFF

  /* Additional custom blend modes can be returned by SDL_ComposeCustomBlendMode() */

} SDL_BlendMode;
static int SDL_SetSurfaceBlendMode(SDL_Surface* surface, SDL_BlendMode mode) {
  switch (mode) {
    case SDL_BLENDMODE_NONE:      
      //return SDL_SetAlpha(surface, 0, 255);
      return SDL_SetColorKey(surface, 0, 0);
      break;
    case SDL_BLENDMODE_BLEND:      
      //return SDL_SetAlpha(surface, SDL_SRCALPHA | SDL_RLEACCEL, 255);            
      return SDL_SetColorKey(surface, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(surface->format, 0, 0, 0));
      break;
    default:
      puts("Unsupported blend mode\n");
      return -1;
      break;
  }
}


// Hint
#define SDL_HINT_RENDER_SCALE_QUALITY 0x00000000
#define SDL_HINT_RENDER_DRIVER        0x00000000
#define SDL_HINT_OVERRIDE             0x00000000
static int SDL_SetHint(Uint32 hint, const char* interpolation) { return 0; } //SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
static int SDL_SetHintWithPriority(Uint32 hint, char* argv, Uint32 override) { return 0; }


// Fullscreen
#define SDL_Window SDL_Surface
#define SDL_WINDOW_FULLSCREEN_DESKTOP 0x00000001
static int SDL_SetWindowFullscreen(SDL_Surface* window, Uint32 unused_flags) {
  return SDL_WM_ToggleFullScreen(window);
};
static void SDL_GetWindowSize(SDL_Surface* window, int* x, int* y) {
  *x = window->w;
  *y = window->h;
}

#endif
