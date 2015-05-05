//
// This file contains the Chinese token generator implementation 
//

#include "TokenGeneratorChinese.h"
#include "XdbFilter.h"
#include "XdbDumper.h"
#include "XdbGenerator.h"
#include <iostream>

CTokenGeneratorChinese::CTokenGeneratorChinese() :
  iInputPath("."),
  iOutputPath(".")
{
  //std::cout << "CTokenGeneratorChinese" << std::endl;
}

CTokenGeneratorChinese::~CTokenGeneratorChinese()
{
  //std::cout << "~CTokenGeneratorChinese" << std::endl;
}

void CTokenGeneratorChinese::SetInputFilePath(std::string aInputPath)
{
  iInputPath = aInputPath;
}

void CTokenGeneratorChinese::SetOutputFilePath(std::string aOutputPath)
{
  iOutputPath = aOutputPath;
}

bool CTokenGeneratorChinese::Run()
{
  //std::cout << "Running CTokenGeneratorChinese" << std::endl;

  // run xdb_filter, xdb_gen, xdb_dump, ...
  CXdbFilter xdbFilter;
  xdbFilter.Run();

  return true;
}

