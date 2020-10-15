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

        # add 128
        samps = [((x + 128) % 256) for x in samps]

        an = "".join([c if c.isalnum() else "_" for c in filename])
        header.write("extern const uint8_t " + an + "[];\n")
        header.write("extern const size_t " + an + "_size;\n\n")

        src.write("static const uint8_t " + an + "[] = \n{\n    ")
        src.write("\n    ".join(textwrap.wrap(", ".join([hex(b) for b in samps]))))
        src.write("\n};\n")
        src.write("static const size_t " + an + "_size = sizeof(" + an + ");\n\n")

    header.write("#endif")
    header.close()
    src.close()
