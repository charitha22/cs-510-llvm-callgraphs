#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"

using namespace llvm;

namespace {
	class CallGraphPass : public ModulePass {
	public:
		static char ID;
	
		CallGraphPass() : ModulePass(ID) {}

		bool runOnModule(Module &m) override;
		
	}; 
} 

bool CallGraphPass::runOnModule(Module& m){
	//errs() << "Call graph for module : "<< m.getName().str().c_str() << "\n";
	
	// iterate over functions
	for(Module::iterator it =  m.begin(); it != m.end(); it++){
		
		Function& f = *it;
		errs() << "Function : " << f.getName().str() << " == > ";

		//iterate over basic blocks
		for(Function::iterator it1 = f.begin(); it1 != f.end(); it1++){
			BasicBlock& bb = *it1;

			// iterate over instructions
			for(BasicBlock::iterator it2 = bb.begin(); it2 != bb.end(); it2++){

				Instruction & inst = *it2;

				inst.dump();
			}


		}	
	}
	return false;
}


char CallGraphPass::ID = 0;
static RegisterPass<CallGraphPass> X("callgraph", "Call Grpah Pass",
	false /* Only looks at CFG */,
	false /* Analysis Pass */);

