#!/usr/bin/env python3

# This tool takes a variadic number of .wav files as input and outputs c source and header files
# containing the wav files. The audio depth is 8 bits.
import wave, sys, textwrap

if __name__ == "__main__":
    if (len(sys.argv) == 1):
        print("Some file arguments are required.");
        quit(0);

    header = open("audio_samples.h", "w")
    src    = open("audio_samples.c", "w")
    header.write("#ifndef _AUDIO_SAMPLES_H\n#define _AUDIO_SAMPLES_H\n\n#include <stdint.h>\n#include<stddef.h>\n\n")
    src.write("#include \"audio_samples.h\"\n\n")

    for filename in sys.argv[1:]:
        print("Processing file " + filename)
        f = wave.open(filename, 'rb')
        depth = f.getsampwidth()
        chans = f.getnchannels()
        samps = f.readframes(f.getnframes())[(depth - 1)::(depth * chans)]
        f.close()

        # Convert from 2's complement to unsigned
        samps = [((x + 128) % 256) for x in samps]
        if (len(samps) > 20000):
            samps = samps[:20000]

        # Zero out last sample. Should be 128 as this is "DC" in our system.
        # but... maybe it would be a good idea to ramp it down from 0x80 to 0x00 over like 20
        # samples to avoid extra power dissipation from having the timer on all the time? TODO later.
        samps[-1] = 128;

        an = "".join([c if c.isalnum() else "_" for c in filename])
        header.write("extern const uint8_t " + an + "[];\n")
        header.write("extern const size_t " + an + "_size;\n\n")

        src.write("const uint8_t " + an + "[] __attribute__((section(\".upper.rodata\"))) = \n{\n    ")
        src.write("\n    ".join(textwrap.wrap(", ".join([hex(b) for b in samps]))))
        src.write("\n};\n")
        src.write("const size_t " + an + "_size = sizeof(" + an + ");\n\n")

    header.write("#endif")
    header.close()
    src.close()
