#include "Function.h"
#include <iostream>
NS_TASK
Function::Function()
{

}

Function::~Function()
{

}
Function* Function::Create(const std::wstring& strFunction,task_function_fn callback)
{
	Function* pFunc = new Function;
	pFunc->m_funcName = strFunction;
	pFunc->m_callback = callback;
	return pFunc;
}

const std::wstring& Function::GetFunctionName() const
{
	return m_funcName;
}

task_function_fn* Function::GetCallBack() const
{
	return m_callback;
}

NS_END