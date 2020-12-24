#include "KeyPoll.h"
#include <stdio.h>
#include <string.h>
#include <utf8/unchecked.h>

void KeyPoll::setSensitivity(int _value)
{
	switch (_value)
	{
		case 0:
			sensitivity = 28000;
			break;
		case 1:
			sensitivity = 16000;
			break;
		case 2:
			sensitivity = 8000;
			break;
		case 3:
			sensitivity = 4000;
			break;
		case 4:
			sensitivity = 2000;
			break;
	}

}

KeyPoll::KeyPoll()
{
	xVel = 0;
	yVel = 0;
	setSensitivity(2);

	quitProgram = 0;
	textentrymode=true;
	keybuffer="";
	leftbutton=0; rightbutton=0; middlebutton=0;
	mx=0; my=0;
	resetWindow = 0;
	toggleFullscreen = false;
	pressedbackspace=false;

	useFullscreenSpaces = false;
  // GUSARBA: Disabled
  /*
	if (strcmp(SDL_GetPlatform(), "Mac OS X") == 0)
	{
		useFullscreenSpaces = true;
		const char *hint = SDL_GetHint(SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES);
		if (hint != NULL)
		{
			useFullscreenSpaces = (strcmp(hint, "1") == 0);
		}
	}
  */

	linealreadyemptykludge = false;
}

void KeyPoll::enabletextentry()
{
	keybuffer="";
	textentrymode = true;
  // GUSARBA: Disabled at the moment
	//SDL_StartTextInput();
}

void KeyPoll::disabletextentry()
{
	textentrymode = false;
  // GUSARBA: Disabled at the moment
	//SDL_StopTextInput();
}

void KeyPoll::Poll()
{
	SDL_Event evt;
	while (SDL_PollEvent(&evt))
	{
		/* Keyboard Input */
		if (evt.type == SDL_KEYDOWN)
		{
			keymap[evt.key.keysym.sym] = true;

			if (evt.key.keysym.sym == SDLK_BACKSPACE)
			{
				pressedbackspace = true;
			}

#ifdef __APPLE__ /* OSX prefers the command keys over the alt keys. -flibit */
			bool altpressed = keymap[SDLK_LGUI] || keymap[SDLK_RGUI];
#else
			bool altpressed = keymap[SDLK_LALT] || keymap[SDLK_RALT];
#endif
			bool returnpressed = evt.key.keysym.sym == SDLK_RETURN;
			bool fpressed = evt.key.keysym.sym == SDLK_f;
			bool f11pressed = evt.key.keysym.sym == SDLK_F11;
			if ((altpressed && (returnpressed || fpressed)) || f11pressed)
			{
				toggleFullscreen = true;
			}

			if (textentrymode)
			{
				if (evt.key.keysym.sym == SDLK_BACKSPACE && !keybuffer.empty())
				{
					std::string::iterator iter = keybuffer.end();
					utf8::unchecked::prior(iter);
					keybuffer = keybuffer.substr(0, iter - keybuffer.begin());
					if (keybuffer.empty())
					{
						linealreadyemptykludge = true;
					}
				}
				else if (	evt.key.keysym.sym == SDLK_v &&
						keymap[SDLK_LCTRL]	)
				{
          // GUSARBA: Disabled at the moment
					//keybuffer += SDL_GetClipboardText();
				}
			}
		}
		else if (evt.type == SDL_KEYUP)
		{
			keymap[evt.key.keysym.sym] = false;
			if (evt.key.keysym.sym == SDLK_BACKSPACE)
			{
				pressedbackspace = false;
			}
		}
    // GUSARBA: Disabled at the moment
		/* else if (evt.type == SDL_TEXTINPUT)
		{
			keybuffer += evt.text.text;
		} */

		/* Mouse Input */
		else if (evt.type == SDL_MOUSEMOTION)
		{
			mx = evt.motion.x;
			my = evt.motion.y;
		}
		else if (evt.type == SDL_MOUSEBUTTONDOWN)
		{
			if (evt.button.button == SDL_BUTTON_LEFT)
			{
				mx = evt.button.x;
				my = evt.button.y;
				leftbutton = 1;
			}
			else if (evt.button.button == SDL_BUTTON_RIGHT)
			{
				mx = evt.button.x;
				my = evt.button.y;
				rightbutton = 1;
			}
			else if (evt.button.button == SDL_BUTTON_MIDDLE)
			{
				mx = evt.button.x;
				my = evt.button.y;
				middlebutton = 1;
			}
		}
		else if (evt.type == SDL_MOUSEBUTTONUP)
		{
			if (evt.button.button == SDL_BUTTON_LEFT)
			{
				mx = evt.button.x;
				my = evt.button.y;
				leftbutton=0;
			}
			else if (evt.button.button == SDL_BUTTON_RIGHT)
			{
				mx = evt.button.x;
				my = evt.button.y;
				rightbutton=0;
			}
			else if (evt.button.button == SDL_BUTTON_MIDDLE)
			{
				mx = evt.button.x;
				my = evt.button.y;
				middlebutton=0;
			}
		}

    // GUSARBA: Joystick Hat
    else if (evt.type == SDL_JOYHATMOTION) {
      printf("HAT %x", evt.jhat.value); 
      // Uncomment this if you are using Chui's SDL-1.2.13
      //evt.jhat.value = ~evt.jhat.value;
      printf("HAT %x", evt.jhat.value); 

      printf("Joystick %d hat %d value:", evt.jhat.which, evt.jhat.hat);
      if (evt.jhat.value == SDL_HAT_CENTERED) {
        printf(" centered");
        buttonmap[SDL_CONTROLLER_BUTTON_DPAD_UP] = false;
        buttonmap[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = false;
        buttonmap[SDL_CONTROLLER_BUTTON_DPAD_DOWN] = false;
        buttonmap[SDL_CONTROLLER_BUTTON_DPAD_LEFT] = false;
        // GUSARBA: Ugly hack to not mess around too much with Game.cpp
        keymap[SDLK_UP] = false;
        keymap[SDLK_DOWN] = false;
      }
      else if (evt.jhat.value & SDL_HAT_UP) {
        printf(" up");
        buttonmap[SDL_CONTROLLER_BUTTON_DPAD_UP] = true;
        // GUSARBA: Ugly hack to not mess around too much with Game.cpp
        keymap[SDLK_UP] = true;
      }
      else if (evt.jhat.value & SDL_HAT_RIGHT) {
        printf(" right");
        buttonmap[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = true;
      }
      else if (evt.jhat.value & SDL_HAT_DOWN) {
        printf(" down");
        buttonmap[SDL_CONTROLLER_BUTTON_DPAD_DOWN] = true;
        // GUSARBA: Ugly hack to not mess around too much with Game.cpp
        keymap[SDLK_DOWN] = true;
      }
      else if (evt.jhat.value & SDL_HAT_LEFT) {
        printf(" left");
        buttonmap[SDL_CONTROLLER_BUTTON_DPAD_LEFT] = true;
      }
      printf("\n");
    }

		/* Controller Input */
		else if (evt.type == SDL_CONTROLLERBUTTONDOWN)
		{
			//buttonmap[(SDL_GameControllerButton) evt.cbutton.button] = true;
      buttonmap[(SDL_GameControllerButton)evt.jbutton.button] = true;
      printf("SDL_CONTROLLERBUTTONDOWN, %d\n", evt.jbutton.button);
      
      // GUSARBA: Uncomment this to abruptly quit the game when pressing the 
      // START button. Useful to quickly test things or go back to dcload
      /*
      if (evt.jbutton.button == SDL_CONTROLLER_BUTTON_START) {
        exit(-1);
      }
      */
		}
		else if (evt.type == SDL_CONTROLLERBUTTONUP)
		{
			//buttonmap[(SDL_GameControllerButton) evt.cbutton.button] = false;
      buttonmap[(SDL_GameControllerButton)evt.jbutton.button] = false;
      printf("SDL_CONTROLLERBUTTONUP, %d\n", evt.jbutton.button);
		}
    else if (evt.type == SDL_CONTROLLERAXISMOTION)
		{
      // GUSARBA: Standard SDL maps axis values from -32768 to 32767,
      // but the SDL kos-port ranges them from -128 to 127,
      // so we need to amplify the value 256 times
      evt.jaxis.value *= 256;
      if (evt.jaxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
      {
        printf("SDL_CONTROLLER_AXIS_LEFTX: %d %d\n", evt.jaxis.axis, evt.jaxis.value);
        if (evt.jaxis.value > -sensitivity &&
          evt.jaxis.value < sensitivity)
        {
          xVel = 0;
        } else
        {
          xVel = (evt.jaxis.value > 0) ? 1 : -1;
          printf("xVel %d\n", xVel);
        }
      }
      if (evt.jaxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
      {
        if (evt.jaxis.value > -sensitivity &&
          evt.jaxis.value < sensitivity)
        {
          yVel = 0;
        } else
        {
          yVel = (evt.jaxis.value > 0) ? 1 : -1;
          printf("yVel %d\n", yVel);
        }
      }
		}
    //GUSARBA: Disabled at the moment
    /*else if (evt.type == SDL_CONTROLLERDEVICEADDED)
		{
			SDL_GameController *toOpen = SDL_GameControllerOpen(evt.cdevice.which);
			printf(
				"Opened SDL_GameController ID #%i, %s\n",
				evt.cdevice.which,
				SDL_GameControllerName(toOpen)
			);
			controllers[SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(toOpen))] = toOpen;
		}
		else if (evt.type == SDL_CONTROLLERDEVICEREMOVED)
		{
			SDL_GameController *toClose = controllers[evt.cdevice.which];
			controllers.erase(evt.cdevice.which);
			printf("Closing %s\n", SDL_GameControllerName(toClose));
			SDL_GameControllerClose(toClose);
		}*/

		/* Window Events */
		//else if (evt.type == SDL_WINDOWEVENT)SDL_SYSWMEVENT
    else if ((evt.type == SDL_ACTIVEEVENT) || (evt.type == SDL_SYSWMEVENT) || (evt.type == SDL_VIDEORESIZE))
		{
			/* Window Resize */
			//if (evt.window.event == SDL_WINDOWEVENT_RESIZED)
      if (evt.type == SDL_VIDEORESIZE)
			{
				resetWindow = true;
			}

			/* Window Focus */
			//else if (evt.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
      else if ((evt.type == SDL_ACTIVEEVENT) && (evt.active.gain == 1))
			{				
        // GUSARBA: Disabled at the moment
        /*
        isActive = true;
				if (!useFullscreenSpaces)
				{
					SDL_Window *window = SDL_GetWindowFromID(evt.window.windowID);
					wasFullscreen = SDL_GetWindowFlags(window);
					SDL_SetWindowFullscreen(window, 0);
				}
				SDL_DisableScreenSaver();
        */
				resetWindow = true;
			}
			//else if (evt.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
      else if ((evt.type == SDL_ACTIVEEVENT) && (evt.active.gain == 0))
			{				
        // GUSARBA: Disabled at the moment
        /*
        isActive = false;
				if (!useFullscreenSpaces)
				{
					SDL_SetWindowFullscreen(
						SDL_GetWindowFromID(evt.window.windowID),
						wasFullscreen
					);
				}
				SDL_EnableScreenSaver();
        */
				resetWindow = true;
			}

			/* Mouse Focus */
      /*
			else if (evt.window.event == SDL_WINDOWEVENT_ENTER)
			{
				SDL_DisableScreenSaver();
				resetWindow = true;
			}
			else if (evt.window.event == SDL_WINDOWEVENT_LEAVE)
			{
				SDL_EnableScreenSaver();
				resetWindow = true;
			}
      */
		}

		/* Quit Event */
		else if (evt.type == SDL_QUIT)
		{
			quitProgram = true;
		}
	}
}

bool KeyPoll::isDown(SDL_Keycode key)
{
	return keymap[key];
}

bool KeyPoll::isUp(SDL_Keycode key)
{
	return !keymap[key];
}

bool KeyPoll::isDown(std::vector<SDL_GameControllerButton> buttons)
{
	for (size_t i = 0; i < buttons.size(); i += 1)
	{
		if (buttonmap[buttons[i]])
		{
			return true;
		}
	}
	return false;
}

bool KeyPoll::isDown(SDL_GameControllerButton button)
{
	return buttonmap[button];
}

bool KeyPoll::controllerButtonDown()
{
	for (
		SDL_GameControllerButton button = SDL_CONTROLLER_BUTTON_A;
		button < SDL_CONTROLLER_BUTTON_DPAD_UP;
		button = (SDL_GameControllerButton) (button + 1)
	) {
		if (isDown(button))
		{
			return true;
		}
	}
	return false;
}

bool KeyPoll::controllerWantsLeft(bool includeVert)
{
	return (	buttonmap[SDL_CONTROLLER_BUTTON_DPAD_LEFT] ||
			xVel < 0 ||
			(	includeVert &&
				(	buttonmap[SDL_CONTROLLER_BUTTON_DPAD_UP] ||
					yVel < 0	)	)	);
}

bool KeyPoll::controllerWantsRight(bool includeVert)
{
	return (	buttonmap[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] ||
			xVel > 0 ||
			(	includeVert &&
				(	buttonmap[SDL_CONTROLLER_BUTTON_DPAD_DOWN] ||
					yVel > 0	)	)	);
}
