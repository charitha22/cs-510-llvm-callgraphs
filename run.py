#!/usr/bin/python3
import os 
import sys

def main():
    if(len(sys.argv)<=1):
        print("Specify the path to .bc file ")
        return

    opt_location = "/home/chartha/Installs/LLVM/build/bin/opt"
    pass_so_location = "/home/chartha/Installs/LLVM/build/lib/LLVMCallGraph.so"
    test_file = sys.argv[1]

    command = opt_location+" -load "+pass_so_location+" -callgraph < "+test_file+ " > /dev/null"
    rc = os.system(command)

    if(rc!=0):
        print("Running the pass failed")


if __name__ == "__main__":
    main()
