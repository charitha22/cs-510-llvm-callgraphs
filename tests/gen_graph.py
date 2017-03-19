import sys
import os
import re

def formatName(name):
    temp1 = re.sub(r'\[', "", name)
    temp2 = re.sub(r'\]', "", temp1)

    return temp2
 
def genGraph(file):

    f = open(file, "r")
    dot_f = open(file+".dot","w")

    dot_f.write("digraph G {\n")

    lines = f.readlines()

    for l in lines:
        if(":" in l):
            parent = l.split(":")[0]
            
            children = l.split(":")[1].rstrip("\n").split(",")

            

            for ch in children:
                if(len(ch.rstrip(" "))==0):
                    continue
                dot_f.write(formatName(parent)+ " -> " + formatName(ch) + ";\n")

    dot_f.write("}\n")
    f.close()
    dot_f.close()

    # run dot
    dot_command = "dot -Tpng "+file+".dot"+" -o "+file+".png"
    rc = os.system(dot_command)
    if(rc!=0):
        print("Error running dot command (graphviz)")
        sys.exit(-1)


def main():
    for i in range(1,len(sys.argv)):
        c_file = str(sys.argv[i])
        output_log = "output_"+c_file.split(".")[0]+".log"

        # run the pass and get the output to a file
        script_command = "python3 run_test.py "+c_file + " >> " + output_log + " 2>&1"
        rc = os.system(script_command)

        if(rc!=0):
            print("Error running run_test.py")
            sys.exit(-1)

        genGraph(output_log)
        # remove log file
        rm_log_cmd = "rm "+output_log
        rc = os.system(rm_log_cmd)

        if(rc!=0):
            print("Error removing log file")
            sys.exit(-1)


if __name__ == "__main__":
    main()