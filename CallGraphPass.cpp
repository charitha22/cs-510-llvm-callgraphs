#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/CallSite.h"

using namespace llvm;

namespace {
	class CallGraphPass : public ModulePass {
	public:
		static char ID;

		// this map will store which variables/pointers are pointing to functions
		std::map<Value*, Function*> pointersToFuncMap_;
		std::map<Value*, Value*> valuesToPointersMap_;
	
		CallGraphPass() : ModulePass(ID) {}

		void handleStores(StoreInst* sinst);
		void handleLoads(LoadInst* linst);
		void handleCallSites(Instruction& inst);

		bool runOnModule(Module &m) override;
		
	}; 
} 

void CallGraphPass::handleCallSites(Instruction& inst){

	// manage call sites
	CallSite cs(&inst);

	if(cs.getInstruction()){
		Value * called = cs.getCalledValue()->stripPointerCasts();

		if(Function * f = dyn_cast<Function>(called) ){
			// ignoring intrinsic function
			if(!f->isIntrinsic()){
				errs()  << "calls function " << f->getName() << " \n";
			}
			
		}
		else{

			//check the value in valuesToPointersMap_
			std::map<Value*, Value*>::iterator it = valuesToPointersMap_.find(called);

			if(it != valuesToPointersMap_.end()){
				// now get the function this pointer points to
				std::map<Value*, Function*>::iterator it2 = pointersToFuncMap_.find(it->second);

				if(it2 != pointersToFuncMap_.end()){
					errs()  << "calls function " << it2->second->getName() << " \n";
				}
			}

		}
	}
	
}

void CallGraphPass::handleStores(StoreInst* sinst){
	Value* valOp = sinst->getValueOperand()->stripPointerCasts();
					
	// check if this vlaue operand is a function
	if(Function* f = dyn_cast<Function>(valOp)){

		Value* pointOp = sinst->getPointerOperand()->stripPointerCasts();
		pointersToFuncMap_.insert(std::pair<Value*, Function*>(pointOp, f)); 

	}
				
}

void CallGraphPass::handleLoads(LoadInst* linst){
	// from which address this is loading
	Value* pointerOp = linst->getPointerOperand()->stripPointerCasts();

	// result of the load instruction
	Value* loadVal = dyn_cast<Value>(linst);

	valuesToPointersMap_.insert(std::pair<Value*, Value*>(loadVal, pointerOp));
}

bool CallGraphPass::runOnModule(Module& m){
	//errs() << "Call graph for module : "<< m.getName().str().c_str() << "\n";
	
	// iterate over functions
	for(Module::iterator it =  m.begin(); it != m.end(); it++){
		
		Function& f = *it;

		if(f.isIntrinsic()){
			continue;
		}

		errs() << f.getName().str() << " : \n" ;

		//iterate over basic blocks
		for(Function::iterator it1 = f.begin(); it1 != f.end(); it1++){
			BasicBlock& bb = *it1;

			// iterate over instructions
			for(BasicBlock::iterator it2 = bb.begin(); it2 != bb.end(); it2++){

				Instruction & inst = *it2;

				// if this instruction is a store. check if its stores function pointers
				// if it does store the pointer and function in a map
				if(StoreInst * sinst = dyn_cast<StoreInst>(&inst)){
					handleStores(sinst);
				}

				// if the instruction is a load instruction put the loading pointer
				// and the value it loads to a map
				else if(LoadInst* linst = dyn_cast<LoadInst>(&inst)){
					handleLoads(linst);
				}

				//manage callsites
				handleCallSites(inst);
			}


		}	
		errs() << "\n";
	}
	return false;
}


char CallGraphPass::ID = 0;
static RegisterPass<CallGraphPass> X("callgraph", "Call Graph Pass",
	false /* Only looks at CFG */,
	false /* Analysis Pass */);

