
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Bitcode/BitcodeWriter.h"

using namespace llvm;

int main(){
        LLVMContext context;
        Module* mod = new Module("sum.ll", context );

        SmallVector<Type*, 2> funcTypeArgs;
        funcTypeArgs.push_back( IntegerType::get(context, 32));
        funcTypeArgs.push_back( IntegerType::get(context, 32));

        FunctionType* funcType = FunctionType::get(
                IntegerType::get(context, 32),
                funcTypeArgs, false );

        Function* func = Function::Create( 
                funcType, GlobalValue::ExternalLinkage,
                "sum", mod );

        func->setCallingConv( CallingConv::C );
        
        auto iter = func->arg_begin();
        Value* lParam = &*iter;
        ++iter;
        Value* rParam = &*iter;

        lParam->setName("l_param");
        rParam->setName("r_param");
        
        BasicBlock* bb = BasicBlock::Create( context,
                                             "entry",
                                             func,
                                             0);

        AllocaInst* ptrA = new AllocaInst( IntegerType::get(context, 32),
                                           "a.addr",
                                           bb);
        ptrA->setAlignment(4);
        AllocaInst* ptrB = new AllocaInst( IntegerType::get(context, 32),
                                           "a.addr",
                                           bb);
        ptrB->setAlignment(4);

        StoreInst* storeA = new StoreInst( lParam, ptrA, false, bb );
        storeA->setAlignment(4);
        StoreInst* storeB = new StoreInst( rParam, ptrB, false, bb );
        storeB->setAlignment(4);

        LoadInst* loadA = new LoadInst( ptrA, "", false, bb);
        loadA->setAlignment(4);
        
        LoadInst* loadB = new LoadInst( ptrB, "", false, bb);
        loadB->setAlignment(4);

        BinaryOperator* add = BinaryOperator::Create( Instruction::Add,
                                                      loadA, loadB,
                                                      "add", bb);

        ReturnInst* ret = ReturnInst::Create( context, add, bb );

        (void)ret;

        WriteBitcodeToFile(mod, outs());


}
