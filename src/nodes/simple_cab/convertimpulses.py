import wave, struct, ntpath

files = ["src/nodes/simple_cab/clean.wav", "src/nodes/simple_cab/long.wav"]

for f in files:
    wav = wave.open(f, "rb")
    length = wav.getnframes()
    headerFile = open(f.replace(".wav", ".h"), "w+")
    out ="""#pragma once
int """ + ntpath.basename(f).replace(".wav", "IR") + """Length = """ + str(length) + """;
WDL_RESAMPLE_TYPE """ + ntpath.basename(f).replace(".wav", "IR") + "[] = {"
    for s in range(0, length):
        waveData = wav.readframes(1)
        data = struct.unpack("h", waveData)[0] / 32768.0
        out += str(data)
        if s != length - 1:
            out += ","
    out += "};"
    headerFile.write(out)
    headerFile.close() 