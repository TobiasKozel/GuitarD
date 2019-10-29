import wave, struct, ntpath

files = ["src/graph/nodes/simple_cab/drive.wav", "src/graph/nodes/simple_cab/clean.wav", "src/graph/nodes/simple_cab/stack.wav"]

for f in files:
    wav = wave.open(f, "rb")
    length = wav.getnframes()
    headerFile = open(f.replace(".wav", ".h"), "w+")
    out ="""#pragma once
float """ + ntpath.basename(f).replace(".wav", "IR") + "[] = {"
    for s in range(0, length):
        waveData = wav.readframes(1)
        data = struct.unpack("h", waveData)[0] / 32768.0
        out += str(data)
        if s != length - 1:
            out += ","
    out += "};"
    headerFile.write(out)
    headerFile.close() 