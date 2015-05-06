//
// This file contains the Chinese token generator implementation 
//

#include "TokenGeneratorChinese.h"
#include "XdbFilter.h"
#include "XdbDumper.h"
#include "XdbGenerator.h"
#include <iostream>

CTokenGeneratorChinese::CTokenGeneratorChinese()
{
  //std::cout << "CTokenGeneratorChinese" << std::endl;
}

CTokenGeneratorChinese::~CTokenGeneratorChinese()
{
  //std::cout << "~CTokenGeneratorChinese" << std::endl;
}

bool CTokenGeneratorChinese::Run()
{
  //std::cout << "Running CTokenGeneratorChinese" << std::endl;
  bool runSuccess = false;

  // Run XdbFilter, XdbGenerator and XdbDumper...
#if 0
  CXdbFilter xdbFilter;
  xdbFilter.SetInputPath("../config/XdbFilter/");
  xdbFilter.SetOutputPath("../config/XdbFilter/");
  xdbFilter.SetLogPath("../config/XdbFilter/");
  runSuccess = xdbFilter.Run();
  if (false == runSuccess)
    return false;
#endif

#if 0
  CXdbGenerator xdbGenerator;
  xdbGenerator.SetInputPath("../config/XdbGenerator/");
  xdbGenerator.SetOutputPath("../config/XdbGenerator/");
  xdbGenerator.SetLogPath("../config/XdbGenerator/");
  runSuccess = xdbGenerator.Run();
  if (false == runSuccess)
    return false;
#endif

#if 1
  CXdbDumper xdbDumper;
  xdbDumper.SetInputPath("../config/XdbDumper/");
  xdbDumper.SetOutputPath("../config/XdbDumper/");
  xdbDumper.SetLogPath("../config/XdbDumper/");
  runSuccess = xdbDumper.Run();
  if (false == runSuccess)
    return false;
#endif

  return true;
}

