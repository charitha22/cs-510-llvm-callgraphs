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
	class ExtraFunc{
	public:
		Value* callValue_;
		std::vector<Value*> funcsPointedTo_;
		
		ExtraFunc(Value* calledValue) {callValue_ = calledValue;}
		void addValue(Value* val) {funcsPointedTo_.push_back(val);}
		Value* getCallValue() {return callValue_;}
		void dumpAllFuncs(std::vector<Function*>& output) {
			for(std::vector<Value*>::iterator it = funcsPointedTo_.begin(); it != funcsPointedTo_.end();
				it++){
				
				Function* f = dyn_cast<Function>(*it);
				if(f){
					//errs() << "[" << f->getName() << "]";
					output.push_back(f);
				}
				

			}
		}

	};

	class CallGraphPass : public ModulePass {
	public:
		static char ID;

		// this map will store which pointer are string which values
		std::map<Value*, Value*> storesMap_;
		// this map will store which values/pointers are loaded to which values 
		std::map<Value*, Value*> loadsMap_;

		// this map is used when a pointer variable is pointing to 
		// multiple functions. eg. tc3.c
		std::vector<ExtraFunc*> extraFuncMap_;
	
		CallGraphPass() : ModulePass(ID) {}

		void handleStores(StoreInst* sinst);
		void handleLoads(LoadInst* linst);
		void handleCallSites(Instruction& inst, std::vector<Function*>& output);

		bool runOnModule(Module &m) override;

		// for debugging
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



void CallGraphPass::handleCallSites(Instruction& inst,std::vector<Function*>& output){

	// manage call sites
	CallSite cs(&inst);

	if(cs.getInstruction()){
		Value * called = cs.getCalledValue()->stripPointerCasts();

		if(Function * f = dyn_cast<Function>(called) ){
			// ignoring intrinsic function
			if(!f->isIntrinsic()){
				//errs()  << "[" << f->getName() << "]";
				output.push_back(f);
			}
			
		}
		else{


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
						
						// NOTE tc3.c : here we need to find if the addr is pointing to multiple
						// functions in possible control flows. So check in extraFuncMap_
						bool mulFuncsFound = false;
						
						for(std::vector<ExtraFunc*>::iterator it = extraFuncMap_.begin(); it!=extraFuncMap_.end()
								;it++){
							if((*it)->getCallValue()==addr){
								mulFuncsFound = true;
								(*it)->dumpAllFuncs(output);
								break;

							}

						}

						if(!mulFuncsFound){
							//errs() << "[" << f->getName() << "]";
							output.push_back(f);
						}

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

	// tc3.c : see if this pointOp is already storing a value
	// In this case we assume valueOp is always a function and we 
	// store the additional functions in a seprate list to use later
	std::map<Value*, Value*>::iterator it = storesMap_.find(pointOp);
	if(it!=storesMap_.end()){
		//errs() << "This pointer is already assigned.\n";
		ExtraFunc* functions = new ExtraFunc(pointOp);
		functions->addValue(valOp);
		functions->addValue(it->second);
		extraFuncMap_.push_back(functions);
		
		
	}
	else{
		storesMap_.insert(std::pair<Value*, Value*>(pointOp, valOp)); 
	}
	
				
}

void CallGraphPass::handleLoads(LoadInst* linst){
	// from which address this is loading
	Value* pointerOp = linst->getPointerOperand()->stripPointerCasts();

	// result of the load instruction
	Value* loadVal = dyn_cast<Value>(linst);

	loadsMap_.insert(std::pair<Value*, Value*>(loadVal, pointerOp));


}

bool CallGraphPass::runOnModule(Module& m){
	
	// iterate over functions
	for(Module::iterator it =  m.begin(); it != m.end(); it++){
		
		Function& f = *it;

		if(f.isIntrinsic()){
			continue;
		}

		errs() << "[" << f.getName().str() << "] : " ;

		// all the functions called by this function is stored in this list
		std::vector<Function*> outputFuncs;

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
				handleCallSites(inst, outputFuncs);
			}


		}	

		// print all the functions called by this function
		int commaCount = outputFuncs.size()-1; //formatting stuff 
		for(std::vector<Function*>::iterator it3 = outputFuncs.begin(); it3!=outputFuncs.end();it3++){
			Function * f = *it3;
			errs() << "[" << f->getName() << "]" ;
			if(commaCount>0){
				errs() << "," ;
			}
			commaCount--;
		}


		errs() << "\n";
	}
	return false;
}


char CallGraphPass::ID = 0;
static RegisterPass<CallGraphPass> X("callgraph", "Call Graph Pass",
	false /* Only looks at CFG */,
	false /* Analysis Pass */);

