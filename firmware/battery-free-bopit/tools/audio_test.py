#!/usr/bin/env python3

import wave, sys

fin = wave.open("Crash-Cymbal-1.wav", 'rb')
print("fin nchannels = " + str(fin.getnchannels()))
print("fin nframes  = " + str(fin.getnframes()))
depth = fin.getsampwidth()
chans = fin.getnchannels()
clip = fin.readframes(fin.getnframes())[(depth - 1)::(depth * chans)]


fout = wave.open('sound.wav', 'wb')
fout.setnchannels(1)
fout.setsampwidth(1)
fout.setframerate(44100)

for b in clip:
    fout.writeframesraw(bytes([(b + 128) % 256]))
fout.close()
