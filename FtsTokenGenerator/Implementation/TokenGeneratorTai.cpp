//
// This file contains the Tai token generator implementation 
//

#include "TokenGeneratorTai.h"
#include <iostream>

CTokenGeneratorTai::CTokenGeneratorTai() :
  iInputPath("."),
  iOutputPath(".")
{
  std::cout << "CTokenGeneratorTai" << std::endl;
}

CTokenGeneratorTai::~CTokenGeneratorTai()
{
  std::cout << "~CTokenGeneratorTai" << std::endl;
}

void CTokenGeneratorTai::SetInputFilePath(std::string aInputPath)
{
  iInputPath = aInputPath;
}

void CTokenGeneratorTai::SetOutputFilePath(std::string aOutputPath)
{
  iOutputPath = aOutputPath;
}

bool CTokenGeneratorTai::Run()
{
  std::cout << "Running CTokenGeneratorTai" << std::endl;

  // run tai tokenizer engine...
  // TODO.

  return true;
}

