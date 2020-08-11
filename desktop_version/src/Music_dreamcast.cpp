#include <SDL.h>
#include <stdio.h>
#include "Music.h"
#include "BinaryBlob.h"
#include "Map.h"

#include <kos.h>
#include <dc/sound/sound.h>
#include "memory_stats.h"
struct SFX_t {
  //snd_stream_hnd_t shnd;
  SoundTrack* t;
  kthread_t* thread;
  int pos;
};
static SFX_t SFX[32];
#define FS_PREFIX "/cd"
  #if defined(FS_PREFIX_PC)
  #define FS_PREFIX "/pc/VVVVVV/desktop_version/dc/romdisk/"
  #elif defined(FS_PREFIX_SD)
  #define FS_PREFIX "/sd"
  #elif defined(FS_PREFIX_CD)
  #define FS_PREFIX "/cd"
  #endif


/*
* Struct that holds the RIFF data of the Wave file.
* The RIFF data is the meta data information that holds,
* the ID, size and format of the wave file
*/
struct RIFF_Header {
  char chunkID[4];
  long chunkSize;  //size not including chunkSize or chunkID
  char format[4];
};

/*
* Struct to hold fmt subchunk data for WAVE files.
*/
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
* Struct to hold the data of the wave file
*/
struct WAVE_Data {
  char subChunkID[4];  //should contain the word data
  long subChunk2Size;  //Stores the size of the data block
};

// Variables for streaming music
static file_t soundFile = NULL;

int8_t mix_fading_out = -1;  // -1 = FadingIn , +1 = Fading Out , 0 = Nothing
int mix_fading_ms = -1;
uint8_t mix_playing_music = 0;
int mix_volume = 15;
int mix_loops = -1;
int mix_next = -1;

int LoadWavFile(const char* filename,
    int* size, int* frequency) {
  WAVE_Format wave_format;
  RIFF_Header riff_header;
  WAVE_Data wave_data;

  soundFile = fs_open(filename, O_RDONLY);
  if (soundFile < 0) {
    printf("LoadWavFile: Could not open %s\n", filename);
    return 0;
  } else {
    printf("LoadWavFile: Opened %s\n", filename);
  }

  fs_read(soundFile, &riff_header, sizeof(RIFF_Header));

  if ((riff_header.chunkID[0] != 'R' ||
      riff_header.chunkID[1] != 'I' ||
      riff_header.chunkID[2] != 'F' ||
      riff_header.chunkID[3] != 'F') ||
      (riff_header.format[0] != 'W' ||
        riff_header.format[1] != 'A' ||
        riff_header.format[2] != 'V' ||
        riff_header.format[3] != 'E')) {
    fs_close(soundFile);
    return 0;
  }

  fs_read(soundFile, &wave_format, sizeof(WAVE_Format));
  if (wave_format.subChunkID[0] != 'f' ||
      wave_format.subChunkID[1] != 'm' ||
      wave_format.subChunkID[2] != 't' ||
      wave_format.subChunkID[3] != ' ') {
    fs_close(soundFile);
    return 0;
  }

  if (wave_format.subChunkSize > 16)
    fs_seek(soundFile, sizeof(short), SEEK_CUR);

  fs_read(soundFile, &wave_data, sizeof(WAVE_Data));
  if (wave_data.subChunkID[0] != 'd' ||
      wave_data.subChunkID[1] != 'a' ||
      wave_data.subChunkID[2] != 't' ||
      wave_data.subChunkID[3] != 'a') {
    fs_close(soundFile);
    printf("LoadWavFile: Error reading data %s\n", filename);
    return 0;
  }

  *size = wave_data.subChunk2Size;
  *frequency = wave_format.sampleRate;

  return 1;  
}

void *wav_callback(snd_stream_hnd_t hnd, int len, int * actual) {
  //printf("wav_callback %d %d\n", hnd, len);
  SoundTrack* t = SFX[hnd].t;
  int off = sizeof(RIFF_Header) + sizeof(WAVE_Format) + sizeof(WAVE_Data); 
  int rest = t->size - off - SFX[hnd].pos;

  if (rest <= 0) {
    // We have reached the end of the buffer
    return NULL;
  }

  // pcm is the amount of data read by this call(back)
  int pcm = len;
  // If there is less data left in the buffer than len...
  if (rest < len) pcm = rest;
  // pcm_ptr holds the current position in the buffer
  uint8_t* pcm_ptr = t->sound + off + SFX[hnd].pos;
  // Increment pos and set actual correctly
  SFX[hnd].pos += pcm;

  *actual = pcm;

  return pcm_ptr;
}

int stream_setup(SoundTrack* t) {
  snd_stream_hnd_t shnd = snd_stream_alloc(&wav_callback, SND_STREAM_BUFFER_MAX);
  if (shnd == SND_STREAM_INVALID) {
    printf("stream_setup: SND_STREAM_INVALID %p\n", t);
    return -1;
  }

  RIFF_Header riff_header;
  WAVE_Format wave_format;

  memcpy(&riff_header, t->sound, sizeof(RIFF_Header));
  memcpy(&wave_format, t->sound + sizeof(RIFF_Header), sizeof(WAVE_Format));

  printf("WAV frequency %d channels %d\n", wave_format.sampleRate, wave_format.numChannels);

  SFX[shnd].t = t;
  SFX[shnd].thread = thd_get_current();
  SFX[shnd].pos = sizeof(RIFF_Header) + sizeof(WAVE_Format) + sizeof(WAVE_Data); 
  snd_stream_start(shnd, wave_format.sampleRate, wave_format.numChannels-1);
}
int stream_loop() {
  for (int shnd = 0; shnd < 32; ++shnd) {
    if (SFX[shnd].t != NULL) {
      int ret = snd_stream_poll(shnd);
      if(ret != 0) {
        printf("stream_loop: Stream ended %d\n", shnd);
        snd_stream_stop(shnd);
        snd_stream_destroy(shnd);

        SFX[shnd].t = NULL;
        SFX[shnd].thread = NULL;
        SFX[shnd].pos = 0;
      }
    }
  }
  
  return 0;
}

void musicclass::init()
{
  int rc = snd_stream_init();
  if(rc != 0) {
    printf("musicclass: Error initializing the stream!!\n");
  }
  
	soundTracks.push_back(SoundTrack( "sounds/jump.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/jump2.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/hurt.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/souleyeminijingle.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/coin.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/save.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/crumble.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/vanish.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/blip.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/preteleport.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/teleport.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/crew1.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/crew2.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/crew3.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/crew4.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/crew5.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/crew6.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/terminal.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/gamesaved.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/crashing.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/blip2.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/countdown.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/go.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/crash.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/combine.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/newrecord.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/trophy.wav" ));
	soundTracks.push_back(SoundTrack( "sounds/rescue.wav" ));
  
    
  print_ram_stats();

#ifdef VVV_COMPILEMUSIC
	binaryBlob musicWriteBlob;
	musicWriteBlob.AddFileToBinaryBlob("data/music/0levelcomplete.ogg");
	musicWriteBlob.AddFileToBinaryBlob("data/music/1pushingonwards.ogg");
	musicWriteBlob.AddFileToBinaryBlob("data/music/2positiveforce.ogg");
	musicWriteBlob.AddFileToBinaryBlob("data/music/3potentialforanything.ogg");
	musicWriteBlob.AddFileToBinaryBlob("data/music/4passionforexploring.ogg");
	musicWriteBlob.AddFileToBinaryBlob("data/music/5intermission.ogg");
	musicWriteBlob.AddFileToBinaryBlob("data/music/6presentingvvvvvv.ogg");
	musicWriteBlob.AddFileToBinaryBlob("data/music/7gamecomplete.ogg");
	musicWriteBlob.AddFileToBinaryBlob("data/music/8predestinedfate.ogg");
	musicWriteBlob.AddFileToBinaryBlob("data/music/9positiveforcereversed.ogg");
	musicWriteBlob.AddFileToBinaryBlob("data/music/10popularpotpourri.ogg");
	musicWriteBlob.AddFileToBinaryBlob("data/music/11pipedream.ogg");
	musicWriteBlob.AddFileToBinaryBlob("data/music/12pressurecooker.ogg");
	musicWriteBlob.AddFileToBinaryBlob("data/music/13pacedenergy.ogg");
	musicWriteBlob.AddFileToBinaryBlob("data/music/14piercingthesky.ogg");
	musicWriteBlob.AddFileToBinaryBlob("data/music/predestinedfatefinallevel.ogg");

	musicWriteBlob.writeBinaryBlob("data/BinaryMusic.vvv");
#endif

	binaryBlob musicReadBlob;

  print_ram_stats();

  for (int i = 0; i < 32; ++i) {
    SFX[i].t = NULL;
    SFX[i].thread = NULL;
    SFX[i].pos = 0;
  }

	safeToProcessMusic= false;
	m_doFadeInVol = false;
	musicVolume = 128;
	FadeVolAmountPerFrame = 0;

	custompd = false;

	currentsong = 0;
	musicfade = 0;
	musicfadein = 0;
	nicechange = 0;
	nicefade = 0;
	resumesong = 0;
	volume = 0.0f;
	fadeoutqueuesong = -1;
	dontquickfade = false;
}

void musicclass::play(int t)
{
  printf("#####\nmusicclass::play %d\n#####\n", t);
	t = (t % 16);

	if(mmmmmm)
	{
		if(!usingmmmmmm)
		{
			t += 16;
		}
	}
	safeToProcessMusic = true;
	//Mix_VolumeMusic(128);
  mix_volume = 15;
  spu_cdda_volume(15, 15);
	if (currentsong !=t)
	{
		if (t != -1)
		{
			currentsong = t;
			if (currentsong == 0 || currentsong == 7 || (!map.custommode && (currentsong == 16 || currentsong == 23)))
			{
				// Level Complete theme, no fade in or repeat
        mix_next = t;
        mix_loops = 0;
        mix_fading_ms = 0;
        /*
				if(Mix_FadeInMusic(musicTracks[t].m_music, 0, 0)==-1)
				{
					printf("Mix_PlayMusic: %s\n", Mix_GetError());
				}
        */
        mix_playing_music = 1;
        printf("Playing CD track %d\n", mix_next+1);
        if (cdrom_cdda_play(mix_next+1, mix_next+1, 15, CDDA_TRACKS) != ERR_OK) 
        {
					printf("cdrom_cdda_play failed: %d\n", mix_next+1);
          mix_playing_music = 0;
        }
			}
			else
			{
        if (mix_fading_out == 1) {
				//if (Mix_FadingMusic() == MIX_FADING_OUT) {
					// We're already fading out
					fadeoutqueuesong = t;
					currentsong = -1;
					if (!dontquickfade) {
						//Mix_FadeOutMusic(500); // fade out quicker
            mix_fading_out = 1;
            mix_fading_ms = 500;
          } else
						dontquickfade = false;
				}
				//else if(Mix_FadeInMusic(musicTracks[t].m_music, -1, 3000)==-1)
				else 
				{
					//printf("Mix_FadeInMusic: %s\n", Mix_GetError());
          mix_fading_out = -1;
          mix_next = t;
          mix_loops = -1;
          mix_fading_ms = 3000;
          mix_playing_music = 1;
          printf("Playing CD track %d\n", mix_next+1);
				  if (cdrom_cdda_play(mix_next+1, mix_next+1, 15, CDDA_TRACKS) != ERR_OK)
          {
					  printf("cdrom_cdda_play failed: %d\n", mix_next+1);
            mix_playing_music = 0;
          } 
				}
			}
		}
		else
		{
			currentsong = -1;
		}
	}
}

void musicclass::haltdasmusik()
{
  printf("musicclass::haltdasmusik\n");
  resumesong = currentsong;
	//Mix_HaltMusic();
  cdrom_cdda_pause();
	currentsong = -1;
}

void musicclass::silencedasmusik()
{
  printf("musicclass::silencedasmusik\n");
	//Mix_VolumeMusic(0) ;
  mix_volume = 0;
  spu_cdda_volume(0, 0);
	musicVolume = 0;
}

void musicclass::fadeMusicVolumeIn(int ms)
{
	m_doFadeInVol = true;
	//FadeVolAmountPerFrame =  MIX_MAX_VOLUME / (ms / 33);
	FadeVolAmountPerFrame =  15 / (ms / 33);
  mix_volume = 15;
  spu_cdda_volume(15, 15);
}

void musicclass::fadeout()
{
	//Mix_FadeOutMusic(2000);
  mix_playing_music = 0;
  cdrom_cdda_pause();
  mix_fading_out = 1;
  mix_fading_ms = 2000;
	currentsong = -1;
}

void musicclass::processmusicfadein()
{
	musicVolume += FadeVolAmountPerFrame;
	//Mix_VolumeMusic(musicVolume);
  
  //mix_volume = musicVolume;
  //spu_cdda_volume(mix_volume, mix_volume);

	//if (musicVolume >= MIX_MAX_VOLUME)
	if (musicVolume >= 15)
	{
		m_doFadeInVol = false;
	}
}

void musicclass::processmusic()
{
	if(!safeToProcessMusic)
	{
		return;
	}

	//if (fadeoutqueuesong != -1 && Mix_PlayingMusic() == 0) {
	if (fadeoutqueuesong != -1 && mix_playing_music == 0) {
		play(fadeoutqueuesong);
		fadeoutqueuesong = -1;
	}

	//if (nicefade == 1 && Mix_PlayingMusic() == 0)
	if (nicefade == 1 && mix_playing_music == 0)
	{
		play(nicechange);
		nicechange = -1; nicefade = 0;
	}

	if(m_doFadeInVol)
	{
		processmusicfadein();
	}

  // Actual processing of mix variables
  if (mix_fading_out == 1) {  // FadingOut
	  /*
    int fade_amount =  mix_volume / (mix_fading_ms / 33);
    mix_volume -= fade_amount;
    if (mix_volume < 0) mix_volume = 0;
    spu_cdda_volume(mix_volume, mix_volume);
    mix_fading_ms -= 33;
    if (mix_fading_ms <= 0) {
       mix_fading_ms = 0;
       mix_fading_out = 0;
    }
    */
    mix_playing_music = 0;
    cdrom_cdda_pause();
    mix_fading_out = 0;
    if (mix_next > -1) {
      // Music CD tracks start in 1
      mix_playing_music = 1;
      cdrom_cdda_play(mix_next+1, mix_next+1, 15, CDDA_TRACKS);
      printf("Playing CD track %d\n", mix_next+1);
      printf("Playing CD track %d\n", mix_next+1);
      printf("Playing CD track %d\n", mix_next+1);
    }
  } else if (mix_fading_out == -1) {  // FadingIn
    /*
	  int fade_amount =  mix_volume / (mix_fading_ms / 33);
    mix_volume += fade_amount;
    if (mix_volume > 15) mix_volume = 15;
    spu_cdda_volume(mix_volume, mix_volume);
    mix_fading_ms -= 33;
    if (mix_fading_ms <= 0) {
       mix_fading_ms = 0;
       mix_fading_out = 0;
    }
    */
    mix_volume = 15;
    spu_cdda_volume(mix_volume, mix_volume);
    mix_fading_out = 0;
  }

  stream_loop();
}


void musicclass::niceplay(int t)
{
	// important: do nothing if the correct song is playing!
	//if((!mmmmmm && currentsong!=t) || (mmmmmm && usingmmmmmm && currentsong!=t) || (mmmmmm && !usingmmmmmm && currentsong!=t+16))
	if(currentsong!=t)
	{
		if(currentsong!=-1)
		{
			dontquickfade = true;
			fadeout();
		}
		nicefade = 1;
		nicechange = t;
	}
}

void musicclass::changemusicarea(int x, int y)
{
	switch(musicroom(x, y))
	{
	case musicroom(11, 4):
		niceplay(2);
		break;

	case musicroom(2, 4):
	case musicroom(7, 15):
		niceplay(3);
		break;

	case musicroom(18, 1):
	case musicroom(15, 0):
		niceplay(12);
		break;

	case musicroom(0, 0):
	case musicroom(0, 16):
	case musicroom(2, 11):
	case musicroom(7, 9):
	case musicroom(8, 11):
	case musicroom(13, 2):
	case musicroom(17, 12):
	case musicroom(14, 19):
	case musicroom(17, 17):
		niceplay(4);
		break;

	default:
		niceplay(1);
		break;
	}
}

void musicclass::playef(int t)
{
  stream_setup((void*)&soundTracks[t]);
}
