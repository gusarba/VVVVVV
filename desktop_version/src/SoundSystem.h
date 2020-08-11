#ifndef SOUNDSYSTEM_H
#define SOUNDSYSTEM_H

#ifndef DREAMCAST
#include <SDL_mixer.h>
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
#else
  unsigned char *sound;
#endif
  int size;
};

class SoundSystem
{
public:
	SoundSystem();
	void playMusic(MusicTrack* music);
};

#endif /* SOUNDSYSTEM_H */
