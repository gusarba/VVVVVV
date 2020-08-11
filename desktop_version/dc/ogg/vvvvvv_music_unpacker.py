#! python

###
### Script to extract the VVVVVV music files from the vvvvvvmusic.vvv file,
### convert them into OGG/Vorbis files and then convert them to RAW audio
### files to be added as CDDA audio tracks
###
### The MD5 checksums for the RAW files should be:
### 3e4d835cfea1771416c96bc69b3bf1d4 *0.raw
### 633a7bbb0ac111284b6825365b9a9eba *1.raw
### 3142b4e06a401b0277abcbcef3470cc9 *2.raw
### b5885c655a60bde4c76212a4cf104f68 *3.raw
### 10604a765c07a2501967493a676ccc8c *4.raw
### 6723db34188a6e7e972444caa8f9824a *5.raw
### 8834b332538adda1eb229e5c1e604619 *6.raw
### aadeda3846fd2689b807fc7611ac0361 *7.raw
### 53a63bf3a50f53b2bff483747b2036f6 *8.raw
### b5f413bccb4e0aad16c4856207bcbca8 *9.raw
### 67aed46a4a331c2e98270fcc18f37939 *10.raw
### 3b4598404b7f140ccd5d10dd08494479 *11.raw
### 02b605e02ce33734a549b4c900be8c72 *12.raw
### 7764c31de5c2afdd176ab90580c56f7d *13.raw
### 221f1b67c0d8c017ab25cb5b00ca397b *14.raw
### 04e919a31f8959f0514faf56697500c2 *15.raw
###

import pyogg
import os

f = open("vvvvvvmusic.vvv", 'rb')
q = f.read()

FILE_NAMES = ['0levelcomplete.ogg','1pushingonwards.ogg','2positiveforce.ogg','3potentialforanything.ogg','4passionforexploring.ogg','5intermission.ogg','6presentingvvvvvv.ogg','7gamecomplete.ogg','8predestinedfate.ogg','9positiveforcereversed.ogg','10popularpotpourri.ogg','11pipedream.ogg','12pressurecooker.ogg','13pacedenergy.ogg','14piercingthesky.ogg','predestinedfatefinallevel.ogg']

startAt = endAt = -1
musStartAt = musEndAt = -1
currentMus = 0
while True:
    oldStartAt = startAt
    startAt = q.find(b"OggS", oldStartAt + 1)
    endAt = q.find(b"OggS", startAt + 1) - 1
    if oldStartAt >= startAt:
        break
    if endAt == -2:
        endAt = len(q) - 1
    #sB = ord(str(q[startAt+5]))
    sB = q[startAt+5]
    #print("startAt:", startAt, "endAt:", endAt, "sB:", sB)
    if sB == 2:
        musStartAt = startAt
    elif sB == 4:
        musEndAt = endAt
        print("Found entire Ogg between",musStartAt,musEndAt)
        print("Filename: ",FILE_NAMES[currentMus])
        f2 = open(FILE_NAMES[currentMus], 'wb')
        f2.write(q[musStartAt:musEndAt])
        f2.close()
        # Decode vorbis to raw audio data
        vf = pyogg.VorbisFile(FILE_NAMES[currentMus])
        fout_name = str(currentMus) + ".raw"
        f3 = open(fout_name, "wb")
        f3.write(vf.buffer)
        f3.close()
        print("Converted to", fout_name)
        currentMus += 1
    #print("Found OggS at",startAt,"-",endAt)


    

