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

/*
 * * Struct that holds the RIFF data of the Wave file.
 * * The RIFF data is the meta data information that holds,
 * * the ID, size and format of the wave file
 * */
struct RIFF_Header {
  char chunkID[4];
  long chunkSize;  //size not including chunkSize or chunkID
  char format[4];
};

/*
 * * Struct to hold fmt subchunk data for WAVE files.
 * */
struct WAVE_Format {
  char subChunkID[4];
  long subChunkSize;
  short audioFormat;
  short numChannels;
  long sampleRate;
  long byteRate;
  short blockAlign;
  short bitsPerSample;
};

/*
 * * Struct to hold the data of the wave file
 * */
struct WAVE_Data {
  char subChunkID[4];  //should contain the word data
  long subChunk2Size;  //Stores the size of the data block
};

#if 0
bool LoadWavFileAL(const std::string filename, ALuint* buffer,
                 ALsizei* size, ALsizei* frequency,
                 ALenum* format) {
  //Local Declarations
  file_t soundFile = NULL;
  WAVE_Format wave_format;
  RIFF_Header riff_header;
  WAVE_Data wave_data;
  unsigned char* data;

  soundFile = fs_open(filename.c_str(), O_RDONLY);
  if (soundFile < 0) {
    printf("LoadWavFileAL: Could not open file %s\n", filename.c_str());
    return false;
  }  

  // Read in the first chunk into the struct
  fs_read(soundFile, &riff_header, sizeof(RIFF_Header));

  //check for RIFF and WAVE tag in memeory
  if ((riff_header.chunkID[0] != 'R' ||
        riff_header.chunkID[1] != 'I' ||
        riff_header.chunkID[2] != 'F' ||
        riff_header.chunkID[3] != 'F') ||
      (riff_header.format[0] != 'W' ||
       riff_header.format[1] != 'A' ||
       riff_header.format[2] != 'V' ||
       riff_header.format[3] != 'E')) {
    fs_close(soundFile);
    return false;
  }

  //Read in the 2nd chunk for the wave info
  fs_read(soundFile, &wave_format, sizeof(WAVE_Format));
  //check for fmt tag in memory
  if (wave_format.subChunkID[0] != 'f' ||
      wave_format.subChunkID[1] != 'm' ||
      wave_format.subChunkID[2] != 't' ||
      wave_format.subChunkID[3] != ' ') {
    fs_close(soundFile);
    return false;
  }

  //check for extra parameters;
  if (wave_format.subChunkSize > 16)
    fs_seek(soundFile, sizeof(short), SEEK_CUR);

  //Read in the the last byte of data before the sound file
  fs_read(soundFile, &wave_data, sizeof(WAVE_Data));
  //check for data tag in memory
  if (wave_data.subChunkID[0] != 'd' ||
      wave_data.subChunkID[1] != 'a' ||
      wave_data.subChunkID[2] != 't' ||
      wave_data.subChunkID[3] != 'a') {
    fs_close(soundFile);
    return false;
  }

  //Allocate memory for data
  data = new unsigned char[wave_data.subChunk2Size];

  // Read in the sound data into the soundData variable
  if (!fs_read(soundFile, data, wave_data.subChunk2Size)) {
    fs_close(soundFile);
    printf("LoadWavFileAL: Could not read data from file %s\n", filename.c_str());
    return false;
  }

  //Now we set the variables that we passed in with the
  //data from the structs
  *size = wave_data.subChunk2Size;
  *frequency = wave_format.sampleRate;
  //The format is worked out by looking at the number of
  //channels and the bits per sample.
  if (wave_format.numChannels == 1) {
    if (wave_format.bitsPerSample == 8)
      *format = AL_FORMAT_MONO8;
    else if (wave_format.bitsPerSample == 16)
      *format = AL_FORMAT_MONO16;
  }
  else if (wave_format.numChannels == 2) {
    if (wave_format.bitsPerSample == 8)
      *format = AL_FORMAT_STEREO8;
    else if (wave_format.bitsPerSample == 16)
      *format = AL_FORMAT_STEREO16;
  }
  //create our openAL buffer and check for success
  alGenBuffers(1, buffer);

  //now we put our data into the openAL buffer and
  //check for success
  alBufferData(*buffer, *format, (void*)data,
      *size, *frequency);

  //clean up and return true if successful
  fs_close(soundFile);
  delete[] data;  // Necessary on ALdc??
  return true;  
}
#endif

MusicTrack::MusicTrack(const char* fileName)
{
  m_music = s_num_tracks;
  ++s_num_tracks;
  m_isValid = true;
}

SoundTrack::SoundTrack(const char* fileName)
{
	sound = 0;
  char tmp[256];
  memset(tmp, '\0', 256);
  // Is it an absolute path (e.g: /rd/rescueA.wav)?
  if (fileName[0] == '/') {
    strncpy(tmp, fileName, 256);
  } else {
    strcpy(tmp, FS_PREFIX);
    strcat(tmp, "/");
    strcat(tmp, fileName);
  }
  /*
  bool ret = LoadWavFileAL(tmp, &sound, &size, &frequency, &format);
  if (ret == false) {
		fprintf(stderr, "Unable to load WAV file: %s\n", fileName);
	} else {
    printf("Loaded WAV file: %s %d bytes %d Hz %d format\n", fileName, size,
           frequency, format);
  }
  */

  sound = snd_sfx_load(tmp);
  if (sound == SFXHND_INVALID) {
		printf("Unable to load WAV file: %s\n", fileName);
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
		fprintf(stderr, "Invalid music track specified: %d\n", music->m_music);
	}
	//if(Mix_PlayMusic(music->m_music, 0) == -1)
	if(cdrom_cdda_play(music->m_music, music->m_music, 15, CDDA_TRACKS) != ERR_OK)
	{
		//fprintf(stderr, "Unable to play Ogg file: %s\n", Mix_GetError());
		fprintf(stderr, "Unable to play track: %d\n", music->m_music);
	}
}
