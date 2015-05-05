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
  xdbFilter.SetInputPath("../config/XdbFilter/TC/input/");
  xdbFilter.SetOutputPath("../config/XdbFilter/TC/output_ALL//");
  xdbFilter.SetLogPath("../config/XdbFilter/TC/output_ALL//");
  runSuccess = xdbFilter.Run();
  if (false == runSuccess)
    return false;

  //CXdbGenerator xdbGenerator;
  //xdbGenerator.SetInputPath("../config/xdbGenerator/TC/input/");
  //xdbGenerator.SetOutputPath("../config/xdbGenerator/TC/output_ALL//");
  //xdbGenerator.SetLogPath("../config/xdbGenerator/TC/output_ALL//");
  //runSuccess = xdbGenerator.Run();
  //if (false == runSuccess)
  //  return false;

  //CXdbDumper xdbDumper;
  //xdbDumper.SetInputPath("../config/xdbDumper/TC/input/");
  //xdbDumper.SetOutputPath("../config/xdbDumper/TC/output_ALL//");
  //xdbDumper.SetLogPath("../config/xdbDumper/TC/output_ALL//");
  //runSuccess = xdbDumper.Run();
  //if (false == runSuccess)
  //  return false;

  return true;
}

