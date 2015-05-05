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

  // run xdb_filter, xdb_gen, xdb_dump, ...
  CXdbFilter xdbFilter;
  xdbFilter.SetInputPath("../config/XdbFilter/");
  xdbFilter.SetOutputPath("../config/XdbFilter/");
  xdbFilter.SetLogPath("../config/XdbFilter/");
  runSuccess = xdbFilter.Run();
  if (false == runSuccess)
    return false;

  //CXdbGenerator xdbGenerator;
  //xdbGenerator.SetInputPath("../config/xdbGenerator/");
  //xdbGenerator.SetOutputPath("../config/xdbGenerator/");
  //xdbGenerator.SetLogPath("../config/xdbGenerator/");
  //runSuccess = xdbGenerator.Run();
  //if (false == runSuccess)
  //  return false;

  //CXdbDumper xdbDumper;
  //xdbDumper.SetInputPath("../config/xdbDumper/");
  //xdbDumper.SetOutputPath("../config/xdbDumper/");
  //xdbDumper.SetLogPath("../config/xdbDumper/");
  //runSuccess = xdbDumper.Run();
  //if (false == runSuccess)
  //  return false;

  return true;
}

