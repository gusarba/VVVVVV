#ifndef SOUNDSYSTEM_H
#define SOUNDSYSTEM_H

#ifndef DREAMCAST
#include <SDL_mixer.h>
#else
#include <dc/sound/sfxmgr.h>
//#include <AL/al.h>
//#include <AL/alc.h>
#endif

class MusicTrack
{
public:
	MusicTrack(const char* fileName);
#ifndef DREAMCAST
  MusicTrack(SDL_RWops *rw);
	Mix_Music *m_music;
#else
	int m_music;
#endif
	bool m_isValid;
  static int s_num_tracks;
};

class SoundTrack
{
public:
	SoundTrack(const char* fileName);
#ifndef DREAMCAST
	Mix_Chunk *sound;
  int size;
#else
  //unsigned char *sound;
  //ALuint sound;
  //ALsizei size;
  sfxhnd_t sound;
#endif
};

class SoundSystem
{
public:
	SoundSystem();
	void playMusic(MusicTrack* music);
};

#endif /* SOUNDSYSTEM_H */
