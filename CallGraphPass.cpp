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
		std::map<Value*, Value*> storesMap_;
		std::map<Value*, Value*> loadsMap_;
	
		CallGraphPass() : ModulePass(ID) {}

		void handleStores(StoreInst* sinst);
		void handleLoads(LoadInst* linst);
		void handleCallSites(Instruction& inst);

		bool runOnModule(Module &m) override;

		// debug
		void dumpStoresMap();
		void dumpLoadsMap();
		
	}; 
} 

void CallGraphPass::dumpStoresMap(){
	errs() << "==== Store map ====\n";
	for(std::map<Value*, Value*>::iterator it = storesMap_.begin(); it!=storesMap_.end(); it++){
		errs() << "Value : ";
		it->first->dump();
		errs() << "points to : ";
		it->second->dump();
	}
}

void CallGraphPass::dumpLoadsMap(){
	errs() << "==== Load map ==== \n";
	for(std::map<Value*, Value*>::iterator it = loadsMap_.begin(); it!=loadsMap_.end(); it++){
		errs() << "Value : ";
		it->first->dump();
		errs() << "points to : ";
		it->second->dump();
	}
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
			

			//errs() << "called value : ";

			//called->dump();

			//errs() << "called value points to : ";
			//dumpLoadsMap();
			//dumpStoresMap();

			bool addrFound = false;

			Value* curr = called;
			Value* addr = 0;

			// iterate in loadsMap
			while(!addrFound){

				// check if the current called value is in store
				std::map<Value*, Value*>::iterator it = storesMap_.find(curr);

				if(it!=storesMap_.end()){
					addrFound = true;
					addr = curr;
				}
				else{

					//curr = &(loadsMap_.at(curr));
					std::map<Value*, Value*>::iterator it1 = loadsMap_.find(curr);
					if(it1!=loadsMap_.end()){
						curr = it1->second;
					}

				}

			}

			// iterate in storesMap
			bool funcFound = false;
			while(!funcFound){
				std::map<Value*, Value*>::iterator it = storesMap_.find(addr);

				if(it!=storesMap_.end()){

					Value* storedVal = it->second;

					if(Function* f = dyn_cast<Function>(storedVal)){
						errs() << "calls function : " << f->getName() << "\n";
						funcFound = true;
					}
					else{
						addr = storedVal;
					}


				}

			}



			

		}
	}
	
}

void CallGraphPass::handleStores(StoreInst* sinst){
	Value* valOp = sinst->getValueOperand()->stripPointerCasts();
	Value* pointOp = sinst->getPointerOperand()->stripPointerCasts();
	
	storesMap_.insert(std::pair<Value*, Value*>(pointOp, valOp)); 

					
	// check if this vlaue operand is a function
	// if(Function* f = dyn_cast<Function>(valOp)){

	// 	Value* pointOp = sinst->getPointerOperand()->stripPointerCasts();
	// 	pointersToFuncMap_.insert(std::pair<Value*, Function*>(pointOp, f)); 

	// errs() << "adding store entry \n" ;
	// pointOp->dump();
	// errs() << " stores the value  ";
	// valOp->dump();


	// }
				
}

void CallGraphPass::handleLoads(LoadInst* linst){
	// from which address this is loading
	Value* pointerOp = linst->getPointerOperand()->stripPointerCasts();

	// result of the load instruction
	Value* loadVal = dyn_cast<Value>(linst);

	loadsMap_.insert(std::pair<Value*, Value*>(loadVal, pointerOp));

	// errs() << "adding load entry \n" ;
	// loadVal->dump();
	// errs() << " get loaded the value in ";
	// pointerOp->dump();


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

