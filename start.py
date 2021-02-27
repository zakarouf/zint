#!/usr/bin/env python3
import os, sys, fnmatch
from subprocess import Popen

folderIgnore={"zcon"}
IgnoreNames={}

CC="clang"
CFLAGS=["-std=c99", "-ffunction-sections", "-fdata-sections", "-Os", "-O2"]
LDFLAGS=""
OUTEXE="zint"

def run(commands):
    Popen(commands).wait()

def checkIfNameFound(name, nArray):
    for i in nArray:
        if fnmatch.fnmatch(name, "*"+i):
            return 1
    return 0

def compile(cfiles):

    cf = cfiles.split(" ")

    run_command = [ CC ]
    run_command += CFLAGS
    for i in cf:
        run_command.append(i)
    run_command.append('-o')
    run_command.append(OUTEXE)

    run(run_command)

if __name__ == '__main__':

    dirName = sys.argv[1];
    
    # Get the list of all files in directory tree at given path
    listOfFiles = list()
    for (dirpath, dirnames, filenames) in os.walk(dirName):
        dirnames[:] = [d for d in dirnames if d not in folderIgnore]
        listOfFiles += [os.path.join(dirpath, file) for file in filenames]

    outstr = str()

    ignorePatternFront = ["."]
    ignorePatternEnd = []
    onlyTakePatternEnd = [".c"]

    # Print the files    
    for elem in listOfFiles:
        if elem[2] != ".":
            if elem[-1] == "c":
                if checkIfNameFound(elem, IgnoreNames) != 1:
                    outstr += elem + " "

    print(outstr)
    compile(outstr)