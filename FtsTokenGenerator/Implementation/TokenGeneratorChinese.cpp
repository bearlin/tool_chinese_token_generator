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
#if 1
  CXdbFilter xdbFilter;
  xdbFilter.SetInputPath("../config/XdbFilter/");
  xdbFilter.SetOutputPath("../config/XdbFilter/");
  xdbFilter.SetLogPath("../config/XdbFilter/");

  // Set I/O path...
  xdbFilter.SetInputDataDir("TC/");
  xdbFilter.SetInputScwsXdb("input/xdb/dict_cht.utf8.xdb");
  xdbFilter.SetInputScwsRule("input/xdb/fts-tc-r.tok");
  //xdbFilter.SetInputDataDir("SC/");
  //xdbFilter.SetInputScwsXdb("input/xdb/dict.utf8.xdb");
  //xdbFilter.SetInputScwsRule("input/xdb/fts-sc-r.tok");
  xdbFilter.SetInputSourceData("input/ALL.txt");
  //xdbFilter.SetInputSourceData("input/test.txt");
  runSuccess = xdbFilter.Run();
  if (false == runSuccess)
    return false;

  std::cout << "xdbFilter.GetOutputTokenList():" <<xdbFilter.GetOutputTokenList()<< std::endl;
  std::cout << "xdbFilter.GetOutputTokenListNormalized():" <<xdbFilter.GetOutputTokenListNormalized()<< std::endl;
  std::cout << "xdbFilter.GetOutputTokenListFuzzy():" <<xdbFilter.GetOutputTokenListFuzzy()<< std::endl;
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

#if 0
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

