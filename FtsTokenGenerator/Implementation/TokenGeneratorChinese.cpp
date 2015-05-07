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
  // Set In/Out/Log path
  CXdbFilter xdbFilter;
  xdbFilter.SetInputPath("../config/XdbFilter/TC/input/");
  xdbFilter.SetOutputPath("../config/XdbFilter/TC/output_ALL/");
  xdbFilter.SetLogPath("../config/XdbFilter/TC/output_ALL/");

  // Set I/O path
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

  // Output results token list path
  std::cout << "xdbFilter OutputTokenList path:" << xdbFilter.GetOutputPath() + xdbFilter.GetOutputTokenList()<< std::endl;
  std::cout << "xdbFilter OutputTokenListNormalized path:" << xdbFilter.GetOutputPath() + xdbFilter.GetOutputTokenListNormalized()<< std::endl;
  std::cout << "xdbFilter OutputTokenListFuzzy path:" << xdbFilter.GetOutputPath() + xdbFilter.GetOutputTokenListFuzzy()<< std::endl;
#endif

#if 1
  #if 1
  // Set In/Out/Log path
  CXdbGenerator xdbGenerator;
  xdbGenerator.SetInputPath(xdbFilter.GetOutputPath());
  //xdbGenerator.SetInputPath("../config/XdbGenerator/TC/Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/");
  xdbGenerator.SetOutputPath("../config/XdbGenerator/TC/Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/");
  xdbGenerator.SetLogPath("../config/XdbGenerator/TC/Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/");

  // Set I/O path of fuzzy xdb
  xdbGenerator.SetInputTokenList(xdbFilter.GetOutputTokenListFuzzy());
  xdbGenerator.SetOutputXdb("xdb_fuzzy.xdb");
  //xdbGenerator.SetInputTokenList("s06_log_tokens_full.txt");
  //xdbGenerator.SetOutputXdb("xdb(s06_log_tokens_full.txt).xdb");
  runSuccess = xdbGenerator.Run();
  if (false == runSuccess)
    return false;
  // Output results xdb path
  std::cout << "xdbGenerator OutputXdb path:" <<xdbGenerator.GetOutputPath() + xdbGenerator.GetOutputXdb() << std::endl;

  // Set I/O path of non-fuzzy xdb
  xdbGenerator.SetInputTokenList(xdbFilter.GetOutputTokenList());
  xdbGenerator.SetOutputXdb("xdb_non_fuzzy.xdb");
  runSuccess = xdbGenerator.Run();
  if (false == runSuccess)
    return false;
  // Output results xdb path
  std::cout << "xdbGenerator OutputXdb path:" <<xdbGenerator.GetOutputPath() + xdbGenerator.GetOutputXdb() << std::endl;
  #else
  // Test original manually merged fuzzy token list
  // Set In/Out/Log path
  CXdbGenerator xdbGenerator;
  xdbGenerator.SetInputPath("../config/XdbGenerator/TC/Normalization_20131017_v01_xdb_filter_optimized/2_merge_gen/");
  xdbGenerator.SetOutputPath("../config/XdbGenerator/TC/Normalization_20131017_v01_xdb_filter_optimized/2_merge_gen/");
  xdbGenerator.SetLogPath("../config/XdbGenerator/TC/Normalization_20131017_v01_xdb_filter_optimized/2_merge_gen/");

  // Set I/O path of original manually merged fuzzy token list
  xdbGenerator.SetInputTokenList("merge_export(SILK31).txt");
  xdbGenerator.SetOutputXdb("xdb(merge_export(SILK31).txt).xdb");
  runSuccess = xdbGenerator.Run();
  if (false == runSuccess)
    return false;
  // Output results xdb path
  std::cout << "xdbGenerator OutputXdb path:" <<xdbGenerator.GetOutputPath() + xdbGenerator.GetOutputXdb() << std::endl;
  #endif
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

