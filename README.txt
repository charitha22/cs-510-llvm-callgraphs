Environment
============

I used the following environment for buidling LLVM and running my analysis pass.

Architecture:          x86_64 
CPU op-mode(s):        32-bit, 64-bit
Model name:            Intel(R) Core(TM) i7-6700HQ CPU @ 2.60GHz

LLVM version Information:   

Release version : 3.9.0
Date : 02 Sep 2016  

How to build the pass?
=======================
Assuming you have downloaded and built LLVM.

1. cd <LLVM SRC directory>/lib/Transforms/
2. mkdir cs-510-llvm-callgraphs
3. Copy the code files to <LLVM SRC directory>/lib/Transforms/cs-510-llvm-callgraphs
4. cd <LLVM SRC directory>/lib/Transforms/
5. open CMakeLists.txt and add the following line

    add_subdirectory(cs-510-llvm-callgraphs)

6. cd <LLVM BUILD directory>/lib/Transforms
7. make

This should build the pass and create a shared object in <LLVM BUILD directory>/lib

How to run the pass?
====================
For this you can use 2 methods

[1]. using manual method

1. cd <LLVM SRC directory>/lib/Transforms/cs-510-llvm-callgraphs/tests
2. Run  clang using,
    <LLVM BUILD directory>/bin/clang -O0  -g -c -emit-llvm tc1.c
3. run the pass using,
    <LLVM BUILD directory>/bin/opt -load <LLVM BUILD directory>/lib/LLVMCallGraph.so -callgraph < tc1.bc > /dev/null


[2]. Using python script

1. cd <LLVM SRC directory>/lib/Transforms/cs-510-llvm-callgraphs/tests
2. change the follwing lines in run_test.py

    clang_path = "<LLVM BUILD directory>/bin/clang"
    opt_path = "<LLVM BUILD directory>/bin/opt"
    dis_location = "<LLVM BUILD directory>/bin/llvm-dis"
    so_location = "<LLVM BUILD directory>/lib/LLVMCallGraph.so"

3. Now test cases can be run using,
    
    python3 run_test.py tc1.c


Note : replace tc1.c with the c file you want

Generating Graphical Representation of Call Graph
================================================

This is done using Graphviz visualization library

1. Install graphviz using,
    
    sudo apt-get install graphviz

2. To verify the installation do this,
    
    $ dot -v
    dot - graphviz version 2.38.0 (20140413.2041)
    .....

3. cd <LLVM SRC directory>/lib/Transforms/cs-510-llvm-callgraphs/tests
4. python3 gen_graph.py tc1.c

This should generate a .png file with the visual representation of call graph

NOTE : This is not fully tested. sorry for the bugs if any!


GitHub Link for the Project
===========================

https://github.com/charitha22/cs-510-llvm-callgraphs

