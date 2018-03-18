#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "KaleidoscopeJIT.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>


using namespace llvm;
using namespace llvm::orc;

void Expr0(){

        LLVMContext TheContext;
        auto TheModule = llvm::make_unique<Module>("my cool jit", TheContext);


        // create anon function prototype
        std::vector<Type *> Doubles;
        FunctionType *FT =
                FunctionType::get(Type::getDoubleTy(TheContext), Doubles, false);

        Function *F =
                Function::Create(FT, Function::ExternalLinkage, "__main__", TheModule.get());


        BasicBlock *BB = BasicBlock::Create(TheContext, "entry", F);

        IRBuilder<> Builder(TheContext);
        Builder.SetInsertPoint(BB);
        Value* mul = Builder.CreateFMul(
                ConstantFP::get(TheContext, APFloat(2.)),
                ConstantFP::get(TheContext, APFloat(3.)),
                "mul");
        Value* add = Builder.CreateFAdd(
                mul,
                ConstantFP::get(TheContext, APFloat(1.0)),
                "add");

        Builder.CreateRet(add);
        verifyFunction(*F);

        F->print(errs());

}


void Expr1(){
  
        LLVMContext TheContext;

        auto TheJIT = llvm::make_unique<KaleidoscopeJIT>();

        auto TheModule = llvm::make_unique<Module>("my cool jit", TheContext);
        TheModule->setDataLayout(TheJIT->getTargetMachine().createDataLayout());



        // create anon function prototype
        std::vector<Type *> Doubles;
        FunctionType *FT =
                FunctionType::get(Type::getDoubleTy(TheContext), Doubles, false);

        Function *F =
                Function::Create(FT, Function::ExternalLinkage, "__main__", TheModule.get());


        BasicBlock *BB = BasicBlock::Create(TheContext, "entry", F);

        IRBuilder<> Builder(TheContext);
        Value* mul = Builder.CreateFMul(
                ConstantFP::get(TheContext, APFloat(2.)),
                ConstantFP::get(TheContext, APFloat(3.)),
                "mul");
        Value* add = Builder.CreateFAdd(
                mul,
                ConstantFP::get(TheContext, APFloat(1.0)),
                "add");
        
        Builder.SetInsertPoint(BB);
        Builder.CreateRet(add);
        verifyFunction(*F);

        F->print(errs());
        
        auto H = TheJIT->addModule(std::move(TheModule));
      
        auto ExprSymbol = TheJIT->findSymbol("__main__");
        assert(ExprSymbol && "Function not found");

        // Get the symbol's address and cast it to the right type (takes no
        // arguments, returns a double) so we can call it as a native function.
        double (*FP)() = (double (*)())(intptr_t)cantFail(ExprSymbol.getAddress());
        fprintf(stderr, "Evaluated to %f\n", FP());

        TheJIT->removeModule(H);
}

void Expr2(){
  
        LLVMContext TheContext;

        auto TheJIT = llvm::make_unique<KaleidoscopeJIT>();

        auto TheModule = llvm::make_unique<Module>("my cool jit", TheContext);
        TheModule->setDataLayout(TheJIT->getTargetMachine().createDataLayout());

        auto double_ = Type::getDoubleTy(TheContext);


        Function* times_two = nullptr;

        /*
                double __main__(){
                        return times_two(23.0);
                }
         */
        auto make_main = [&](){
                // create anon function prototype
                FunctionType *FT_D_ = FunctionType::get(double_,  std::vector<Type *>{}, false);

                Function *F = Function::Create(FT_D_, Function::ExternalLinkage, "__main__", TheModule.get());


                BasicBlock *BB = BasicBlock::Create(TheContext, "entry", F);

                IRBuilder<> Builder(TheContext);
                std::vector<Value *> args;
                args.push_back( ConstantFP::get(TheContext, APFloat(23.)) );
                
                Builder.SetInsertPoint(BB);
                auto ret = Builder.CreateCall(times_two, args, "ret");
                Builder.CreateRet(ret);

                verifyFunction(*F);

                return F;

        };

        /*
                double times_two(double x){
                        return x * 2;
                }
         */
        auto make_times_two = [&](){
                // create anon function prototype
                FunctionType *FT_D_D = FunctionType::get(double_,  std::vector<Type *>{double_}, false);

                Function *F = Function::Create(FT_D_D, Function::ExternalLinkage, "times_two", TheModule.get());

                BasicBlock *BB = BasicBlock::Create(TheContext, "entry", F);

                IRBuilder<> Builder(TheContext);
                Value* arg0 = (Value*)F->arg_begin();
                
                Builder.SetInsertPoint(BB);
                Value* result = Builder.CreateFMul(
                        arg0,
                        ConstantFP::get(TheContext, APFloat(2.)),
                        "result");
                Builder.CreateRet(result);
                verifyFunction(*F);

                return F;
        };

        times_two = make_times_two();

        auto main_ = make_main();

        times_two->print(errs());
        main_->print(errs());
        
        auto H = TheJIT->addModule(std::move(TheModule));
      
        auto ExprSymbol = TheJIT->findSymbol("__main__");
        assert(ExprSymbol && "Function not found");

        // Get the symbol's address and cast it to the right type (takes no
        // arguments, returns a double) so we can call it as a native function.
        double (*FP)() = (double (*)())(intptr_t)cantFail(ExprSymbol.getAddress());
        fprintf(stderr, "Evaluated to %f\n", FP());

        TheJIT->removeModule(H);
}

int main(){

        InitializeNativeTarget();
        InitializeNativeTargetAsmPrinter();
        InitializeNativeTargetAsmParser();

        Expr0();
        Expr1();
        Expr2();
}

