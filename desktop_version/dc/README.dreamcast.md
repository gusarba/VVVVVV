Quick port of [Terry Cavanagh's VVVVVV](https://thelettervsixtim.es/) to the Sega Dreamcast game console. This repository is a fork of [the official one](https://github.com/TerryCavanagh/VVVVVV), which was uploaded to celebrate the game's 10th anniversary and contains the source code for the computer and mobile phone versions of the game. The fork was created on April 6th 2020, so newer additions and improvements to the original source are still not reflected here.

The port is far from perfect and there's still a lot of room for improvement, but it is very playable. To develop it, some fundamental changes needed to be made to the original source files. The original structure and code path has been preserved wherever possible, opting to create Dreamcast-specific source files (e.g: main_dreamcast.cpp) where it made more sense, while in other cases opting for a simple ```#ifdef/#endif``` code block. Some characteristics of the port:

- Backported all the SDL calls from the 2.0 API to the old 1.2 API. It incluides a simple "SDL2_stub.h" header file with some glue code.
- Instead of alpha-blending the SDL surfaces, color keying is used, which had a dramatic impact on performance, since all the blitting is done in software by the Dreamcast's SH4 processor.
- The game runs at around 24/30 FPS most of the time.
- Saving and loading to the VMU is implemented, as well as a new small menu screen when booting the game to select which VMU to use (or to not save at all).
- All the music system has been changed to use CDDA audio tracks instead of a binary blob file (which cointains Ogg/Vorbis files). Numerous tests were performed streaming and decoding music from the blob, but it was too slow and choppy.
- The code also includes a quick port to the Dreamcast of the [PhysicsFS library](https://www.icculus.org/physfs/) which the games uses as its file system abstraction.

How to Build
------------
To build the game you need a working [KallistiOS (KOS)](http://gamedev.allusion.net/softprj/kos/) development environment with some extra tools for development on the Dreamcast, namely [IMG4DC](https://github.com/sizious/img4dc). Under Windows, probably the easiest way to set this up is [DreamSDK](https://www.dreamsdk.org/). You will also need Python 3 and PyOgg to run the script which extracts the audio tracks. Normally, you can use pip to install PyOgg:

```
pip3 install PyOgg
```

Once you have cloned the repository, follow these steps:

- Download the data.zip file from the [Make and Play](https://thelettervsixtim.es/makeandplay/) edition of the game.
- Extract the contents of the data.zip file into the _dc/romdisk_ directory.
- Move the _dc/romdisk/vvvvvvmusic.vvv_ file into the _dc/ogg_ directory.
- Run the _vvvvvv_music_unpacker.py_ script in the _dc/ogg_ directory. It should unpack the music files into the original Ogg/Vorbis files and convert them into RAW files for the CDDA audio tracks. As a reference, here are the md5 checksums of the RAW files:
```
3e4d835cfea1771416c96bc69b3bf1d4 *0.raw
633a7bbb0ac111284b6825365b9a9eba *1.raw
3142b4e06a401b0277abcbcef3470cc9 *2.raw
b5885c655a60bde4c76212a4cf104f68 *3.raw
10604a765c07a2501967493a676ccc8c *4.raw
6723db34188a6e7e972444caa8f9824a *5.raw
8834b332538adda1eb229e5c1e604619 *6.raw
aadeda3846fd2689b807fc7611ac0361 *7.raw
53a63bf3a50f53b2bff483747b2036f6 *8.raw
b5f413bccb4e0aad16c4856207bcbca8 *9.raw
67aed46a4a331c2e98270fcc18f37939 *10.raw
3b4598404b7f140ccd5d10dd08494479 *11.raw
02b605e02ce33734a549b4c900be8c72 *12.raw
7764c31de5c2afdd176ab90580c56f7d *13.raw
221f1b67c0d8c017ab25cb5b00ca397b *14.raw
04e919a31f8959f0514faf56697500c2 *15.raw
```
- Go to the _dc_ directory and run ```make``` to build the game binaries.
- Run ```make fullmds``` to build the CD image file that you can burn to a disk or use in an emulator.

