#
# VVVVVV
# (c) Terry Cavanagh
# (c) Flibit
#   

TARGET = vvvvvv.bin
TARGET_ELF = vvvvvv.elf

USE_ROMDISK=1
EMU=1


#OPTFLAGS=-DFRAME_POINTERS -fno-omit-frame-pointer -g -O0 -fpermissive
OPTFLAGS=-fomit-frame-pointer -O3 -ffast-math -std=c99

all: rm-elf $(TARGET)

ifeq ($(USE_ROMDISK),1)
    OPTFLAGS+= -DUSE_ROMDISK
endif

ifeq ($(EMU),1)
    OPTFLAGS+= -DEMU
endif

KOS_CFLAGS += -I. \
	      -I../src \
				-I../../third_party/tinyxml \
				-I../../third_party/physfs \
				-I../../third_party/lodepng \
				-I../../third_party/utfcpp/source \
				-ISDL-1.2.13/include \
				-Ialdc/include \
				-DFS_PREFIX_CD \
				-DPHYSFS_SUPPORTS_DEFAULT=0 -DPHYSFS_SUPPORTS_ZIP=1 \
	      -DDREAMCAST -ffast-math -fno-common -fno-builtin -fno-exceptions -fstrict-aliasing \
	      $(OPTFLAGS)
				#-I$(KOS_PORTS)/include/SDL \
	      #-std=c++11 \

#KOS_CPPFLAGS += -fno-operator-names -fno-exceptions -std=c++11 
KOS_CPPFLAGS += -fno-operator-names -fno-exceptions -fpermissive 

#KOS_ROMDISK_DIR = romdisk
KOS_ROMDISK_DIR = rd

#all: $(TARGET).bin

include $(KOS_BASE)/Makefile.rules


OBJS = ../../third_party/tinyxml/tinystr.o \
			../../third_party/tinyxml/tinyxml.o \
			../../third_party/tinyxml/tinyxmlerror.o \
			../../third_party/tinyxml/tinyxmlparser.o \
      ../../third_party/physfs/physfs.o \
      ../../third_party/physfs/physfs_archiver_dir.o \
      ../../third_party/physfs/physfs_archiver_unpacked.o \
			../../third_party/physfs/physfs_platform_dreamcast.o \
      ../../third_party/physfs/physfs_archiver_zip.o \
			../../third_party/physfs/physfs_unicode.o \
      ../../third_party/physfs/physfs_byteorder.o \
			../../third_party/lodepng/lodepng.o \
			../src/BinaryBlob.o \
      ../src/BlockV.o \
      ../src/DreamcastNetwork.o \
      ../src/Ent.o \
      ../src/Entity.o \
      ../src/FileSystemUtils.o \
      ../src/FileSystemUtils_dreamcast.o \
      ../src/Finalclass.o \
      ../src/Game.o \
      ../src/GOGNetwork.o \
      ../src/Graphics.o \
      ../src/GraphicsResources_dreamcast.o \
      ../src/GraphicsUtil_dreamcast.o \
      ../src/Input.o \
      ../src/KeyPoll_dreamcast.o \
      ../src/Labclass.o \
      ../src/Logic.o \
      ../src/Map.o \
      ../src/Music_dreamcast.o \
      ../src/Network.o \
      ../src/Otherlevel.o \
      ../src/Render.o \
      ../src/Screen_dreamcast.o \
      ../src/Script.o \
      ../src/Scripts.o \
      ../src/SoundSystem_dreamcast.o \
      ../src/Spacestation2.o \
      ../src/SteamNetwork.o \
      ../src/TerminalScripts.o \
      ../src/Textbox.o \
      ../src/Tower.o \
      ../src/UtilityClass.o \
      ../src/WarpClass.o \
      ../src/editor.o \
      ../src/preloader.o \
			aldc/src/alut.o \
			aldc/src/context.o \
			aldc/src/listener.o \
			aldc/src/platform.o \
			aldc/src/buffer.o \
			aldc/src/core.o \
			aldc/src/math.o \
			aldc/src/source.o \
			unistd_stub.o \
			memory_stats.o \
			inprint.o \
			vvvvvvmu.o \
      ../src/main_dreamcast.o
			#memcmp.o \
			#memcpy.o \
			#memmove.o \
			#memset.o \
			#SDL_memory.o \
			#aldc/src/alut.o \
			#aldc/src/context.o \
			#aldc/src/listener.o \
			#aldc/src/platform.o \
			#aldc/src/buffer.o \
			#aldc/src/core.o \
			#aldc/src/math.o \
			#aldc/src/source.o \
			#aldc-legacy/AL/aldc.o \
			#aldc-legacy/AL/mojoal.o \
			#aldc-legacy/AL/alut.o \


ifeq ($(USE_ROMDISK),1)
    OBJS+= romdisk.o
endif

# Calculate LBA number for mds file
LBA := $(shell lbacalc ogg/0.raw ogg/1.raw ogg/2.raw ogg/3.raw ogg/4.raw ogg/5.raw ogg/6.raw ogg/7.raw ogg/8.raw ogg/9.raw ogg/10.raw ogg/11.raw ogg/12.raw ogg/13.raw ogg/14.raw ogg/15.raw)

clean:
	-rm -f $(TARGET_ELF) $(TARGET) $(OBJS) romdisk.img romdisk/1ST_READ.BIN tmp.iso

rm-elf:
	-rm -f $(TARGET_ELF)
	-rm -f $(TARGET)

#romdisk.img:
#	$(KOS_GENROMFS) -f $@ -d romdisk -v

#romdisk.o: romdisk.img
#	$(KOS_BASE)/utils/bin2o/bin2o $< romdisk $@

$(TARGET_ELF): $(OBJS)
	#kos-c++ -o $(TARGET_ELF) $(OBJS) -lSDL -lm -lkosfat -lz $(OBJEXTRA) $(KOS_LIBS)
	kos-c++ -o $(TARGET_ELF) $(OBJS) -lSDL_1213 -lm -lkosfat -lz $(OBJEXTRA) $(KOS_LIBS)

$(TARGET): $(TARGET_ELF)
	$(KOS_OBJCOPY) -R .stack -O binary $(TARGET_ELF) $(TARGET)

run: $(TARGET)
	$(KOS_LOADER) $(TARGET)

dist:
	rm -f $(OBJS)
	$(KOS_STRIP) $(TARGET)

sdiso: $(TARGET)
	# From https://github.com/nold360/mksdiso
	#cp $(TARGET) romdisk/1ST_READ.BIN
	$(KOS_BASE)/utils/scramble/scramble $(TARGET) romdisk/1ST_READ.BIN
	#xorrisofs -V VVVVVV -G /etc/mksdiso/homebrew/IP.BIN -joliet -rock -l -o sd.iso romdisk/
	rm sd_homeb.iso
	cp ~/.mksdiso/homebrew/IP.BIN romdisk
	mksdiso -h romdisk sd_homeb.iso
	rm romdisk/1ST_READ.BIN

iso: $(TARGET)
	#$(KOS_BASE)/utils/scramble/scramble $(TARGET) romdisk/1ST_READ.BIN
	scramble $(TARGET) romdisk/1ST_READ.BIN
	#xorrisofs -V VVVVVV -G IP.BIN -joliet -rock -l -o tmp.iso romdisk/
	#wine cdi4dc.exe tmp.iso vvvvvv.cdi -d
	#mkisofs -C 0,11702 -V VVVVVV -G IP.BIN -joliet -rock -l -o tmp.iso romdisk/
	#genisoimage -C 0,11702 -V VVVVVV -G IP.BIN -joliet -rock -l -o tmp.iso romdisk/
	mkisofs -C 0,11702 -V VVVVVV -G IP.BIN -joliet -rock -l -o tmp.iso romdisk/
	mkisofs -V VVVVVV -joliet -rock -l -o mount.iso romdisk/
	#wine cdi4dc.exe tmp.iso vvvvvv.cdi
	cdi4dc tmp.iso vvvvvv.cdi 
	rm romdisk/1ST_READ.BIN

fullmds: $(TARGET)
	scramble $(TARGET) romdisk/1ST_READ.BIN
	rm -f tmp.iso mount.iso
	mkisofs -C 0,$(LBA) -V VVVVVV -G IP.BIN -joliet -rock -l -o tmp.iso romdisk/
	mkisofs -V VVVVVV -joliet -rock -l -o mount.iso romdisk/
	mds4dc -c vvvvvv.mds tmp.iso ogg/0.raw ogg/1.raw ogg/2.raw ogg/3.raw ogg/4.raw ogg/5.raw ogg/6.raw ogg/7.raw ogg/8.raw ogg/9.raw ogg/10.raw ogg/11.raw ogg/12.raw ogg/13.raw ogg/14.raw ogg/15.raw
	rm romdisk/1ST_READ.BIN

loader: $(TARGET)
	cp dcload/1st_read.bin romdisk/1ST_READ.BIN
	mkisofs -C 0,11702 -V VVVVVV -G dcload/IP.BIN -joliet -l -rock -o tmp.iso romdisk/
	cdi4dc tmp.iso loader_vvvvvv.cdi
	rm romdisk/1ST_READ.BIN

loadermds: $(TARGET)
	cp dcload/1st_read.bin romdisk/1ST_READ.BIN
	rm -f tmp.iso
	mkisofs -C 0,$(LBA) -V VVVVVV -G IP.BIN -joliet -rock -l -o tmp.iso romdisk/
	mds4dc -c loader_vvvvvv.mds tmp.iso ogg/0.raw ogg/1.raw ogg/2.raw ogg/3.raw ogg/4.raw ogg/5.raw ogg/6.raw ogg/7.raw ogg/8.raw ogg/9.raw ogg/10.raw ogg/11.raw ogg/12.raw ogg/13.raw ogg/14.raw ogg/15.raw
	rm romdisk/1ST_READ.BIN

