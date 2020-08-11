#include <SDL.h>
#include "SoundSystem.h"
#include "FileSystemUtils.h"

#include "SDL2_stub.h"

#include <kos.h>
#define FS_PREFIX "/cd"
  #if defined(FS_PREFIX_PC)
  #define FS_PREFIX "/pc/VVVVVV/desktop_version/dc/romdisk/"
  #elif defined(FS_PREFIX_SD)
  #define FS_PREFIX "/sd"
  #elif defined(FS_PREFIX_CD)
  #define FS_PREFIX "/cd"
  #endif

int MusicTrack::s_num_tracks = 0;

MusicTrack::MusicTrack(const char* fileName)
{
  m_music = s_num_tracks;
  ++s_num_tracks;
  m_isValid = true;
}

SoundTrack::SoundTrack(const char* fileName)
{
	sound = NULL;

  char tmp[256];
  memset(tmp, '\0', 256);
  strcpy(tmp, FS_PREFIX);
  strcat(tmp, "/");
  strcat(tmp, fileName);
  size = fs_load(tmp, &sound);
  if (size <= 0)
	{
		fprintf(stderr, "Unable to load WAV file: %s\n", fileName);
	} else {
    printf("Loaded WAV file: %s %d bytes\n", fileName, size);
  }

}

SoundSystem::SoundSystem()
{
  return;
}

void SoundSystem::playMusic(MusicTrack* music)
{
	if(!music->m_isValid)
	{
		fprintf(stderr, "Invalid music track specified: %p\n", music->m_music);
	}
	//if(Mix_PlayMusic(music->m_music, 0) == -1)
	if(cdrom_cdda_play(music->m_music, music->m_music, 15, CDDA_TRACKS) != ERR_OK)
	{
		//fprintf(stderr, "Unable to play Ogg file: %s\n", Mix_GetError());
		fprintf(stderr, "Unable to play track: %d\n", music->m_music);
	}
}
