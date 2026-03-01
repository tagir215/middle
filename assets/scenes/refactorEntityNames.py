import os
import sys

def refactor(filename):
    processedLines = []
    vectorMode = False
    with open(filename, 'r') as f:
        lines = f.readlines()
        for line in lines:

            # vector mode
            if vectorMode and line.strip().isdigit():
                num_part = line.strip()
                line = "q  " + num_part + "_0\n"
                processedLines.append(line)
                continue
            else:
                vectorMode = False

            # non vector mode
            if "__" in line:
                line = line.strip()
                num_part = line.replace("__", "")
                line = "__q  " + num_part + "_0\n"
            elif "q" in line:
                line = line.strip() + "_0\n"
            elif "p" in line:
                vectorMode = True
            processedLines.append(line)

    with open(filename, 'w') as f:
        f.writelines(processedLines)

if __name__ == '__main__':
    arg = sys.argv[1]
    refactor(arg)
