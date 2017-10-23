#!/usr/bin/env python2

import sys
import os

if len(sys.argv) < 3:
    sys.stderr.write("2 arguments require\n")
    sys.exit()

inputFile = sys.argv[1]
outputFile = sys.argv[2]

os.chdir("build/bin")
os.system("./align ../../" + inputFile + " ../../" + outputFile + " ../../log.txt --filter")
