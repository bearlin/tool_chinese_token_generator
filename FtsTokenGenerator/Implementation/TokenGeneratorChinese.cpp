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
  xdbFilter.SetInputPath("../config/XdbFilter/TC/input/");
  xdbFilter.SetOutputPath("../config/XdbFilter/TC/output_ALL/");
  xdbFilter.SetLogPath("../config/XdbFilter/TC/output_ALL/");

  // Set I/O path...
  xdbFilter.SetInputScwsXdb("xdb/dict_cht.utf8.xdb");
  //xdbFilter.SetInputScwsXdb("xdb/dict.utf8.xdb");
  xdbFilter.SetInputScwsRule("xdb/fts-tc-r.tok");
  //xdbFilter.SetInputScwsRule("xdb/fts-sc-r.tok");
  xdbFilter.SetInputNormalizeMap("xdb/fts-tc-n.tok");
  //xdbFilter.SetInputNormalizeMap("xdb/fts-sc-n.tok");
  xdbFilter.SetInputSpecialSuffixTable("_ignored_suffix_table_tc_utf8.txt");
  //xdbFilter.SetInputSpecialSuffixTable("_ignored_suffix_table_sc_utf8.txt");
  xdbFilter.SetInputAreaName("areas/05_all_area_map02.txt");
  xdbFilter.SetInputRemoveToken("_removed_tokens_table.txt");
  //xdbFilter.SetInputSourceData("ALL.txt");
  xdbFilter.SetInputSourceData("test.txt");

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

   // Set I/O path...
  xdbGenerator.SetDataDir("TC/Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/");
  xdbGenerator.SetInputTokenList("s06_log_tokens_full.txt");
  xdbGenerator.SetOutputXdb("xdb(s06_log_tokens_full.txt).xdb");

  runSuccess = xdbGenerator.Run();
  if (false == runSuccess)
    return false;

  std::cout << "xdbGenerator.GetOutputXdbPath():" <<xdbGenerator.GetOutputXdbPath()<< std::endl;
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

