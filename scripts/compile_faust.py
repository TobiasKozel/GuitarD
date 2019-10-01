import os
cwd = os.getcwd()
outFolder = os.path.join(cwd, "src/faust/generated")
rootFolder = "guitard"
baseCommand = "faust -i -double -scn FaustHeadlessDsp -a "
baseCommand += os.path.join(cwd, "src/faust/FaustArchitecture.cpp")

for root, dirs, files in os.walk(cwd):
    for file in files:
        if file.endswith(".dsp"):
            name = file[:-4]
            command = baseCommand + " -cn " + name + " -o "
            command +=  os.path.join(outFolder, name + ".h") + " "
            command += os.path.join(root, file)
            os.system(command)