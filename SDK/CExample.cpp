#include "CExample.h"

//DONT USE THIS METHOD, ITS A TEMPLATE FOR METHODS YOU CAN CREATE BY CREATING AN EXAMPLE CLASS

void CExample::ExampleMethod() {
	typedef void (*CExampleExampleMethod_t)(CExample* __this); //typedef Definition
	CExampleExampleMethod_t CExampleExampleMethod = (CExampleExampleMethod_t)reinterpret_cast<void*>(gAssm /*gAssm is the base of the dll (GameAssembly.dll)*/ + CEXAMPLE_EXAMPLECALL_ADDR);
	CExampleExampleMethod(this); //calling by RVA 
}
