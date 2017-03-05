import os
import sys



def main():
    clang_path = "/home/chartha/Installs/LLVM/build/bin/clang-4.0" 
    opt_path = "/home/chartha/Installs/LLVM/build/bin/opt"
    dis_location = "/home/chartha/Installs/LLVM/build/bin/llvm-dis"
    so_location = "/home/chartha/Installs/LLVM/build/lib/LLVMCallGraph.so"

    # run clang and pass on all the input c files
    for i in range(1, len(sys.argv)):
        #print(sys.argv[i])
        clang_command = clang_path + " -O0  -g -c -emit-llvm " +sys.argv[i]
        rc = os.system(clang_command)

        if(rc!=0):
            print("Error running clang command.")
            sys.exit(-1)

        # generate the .ll file for human readable bitcode
        dis_command = dis_location + " " + sys.argv[i].split(".")[0] + ".bc"

        rc = os.system(dis_command)

        if(rc!=0):
            print("Error running llvm-dis command")
            sys.exit(-1) 


        opt_command = opt_path + " -load " +so_location + " -callgraph < " + sys.argv[i].split(".")[0] + ".bc > /dev/null"
        print("Running "+opt_command)

        rc = os.system(opt_command)

        if(rc!=0):
            print("Error running opt command.")
            sys.exit(-1)

if __name__ == "__main__":
    main()


