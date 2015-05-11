//
// This file contains the Chinese token generator implementation 
//

#include "TokenGeneratorChinese.h"
#include "XdbFilter.h"
#include "XdbDumper.h"
#include "XdbGenerator.h"
#include <iostream>

#define ENABLE_XDBFILTER
#define ENABLE_XDBGENERATOR
//#define ENABLE_XDBDUMPER

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
  bool runSuccess = false;

#ifdef ENABLE_XDBFILTER
  // ============================ CXdbFilter ============================
  // Set CXdbFilter In/Out/Log path
  CXdbFilter xdbFilter;
  xdbFilter.GetConfig().SetInputPath("../config/XdbFilter/TC/input/");
  xdbFilter.GetConfig().SetOutputPath("../config/XdbFilter/TC/output_ALL/");
  xdbFilter.GetConfig().SetLogPath("../config/XdbFilter/TC/output_ALL/");

  // Set I/O path
  xdbFilter.GetConfig().SetInputScwsXdb("xdb/dict_cht.utf8.xdb");
  //xdbFilter.GetConfig().SetInputScwsXdb("xdb/dict.utf8.xdb");
  xdbFilter.GetConfig().SetInputScwsRule("xdb/fts-tc-r.tok");
  //xdbFilter.GetConfig().SetInputScwsRule("xdb/fts-sc-r.tok");
  xdbFilter.GetConfig().SetInputNormalizeMap("xdb/fts-tc-n.tok");
  //xdbFilter.GetConfig().SetInputNormalizeMap("xdb/fts-sc-n.tok");
  xdbFilter.GetConfig().SetInputSpecialSuffixTable("_ignored_suffix_table_tc_utf8.txt");
  //xdbFilter.GetConfig().SetInputSpecialSuffixTable("_ignored_suffix_table_sc_utf8.txt");
  xdbFilter.GetConfig().SetInputAreaName("areas/05_all_area_map02.txt");
  xdbFilter.GetConfig().SetInputRemoveToken("_removed_tokens_table.txt");
  xdbFilter.GetConfig().SetInputSourceData("ALL.txt");
  //xdbFilter.GetConfig().SetInputSourceData("test.txt");

  runSuccess = xdbFilter.Run();
  if (false == runSuccess)
  {
    std::cout << "xdbFilter.Run() error!!!" << std::endl;
    return false;
  }

  // Output results token list path
  std::cout << "xdbFilter OutputTokenList path:" << xdbFilter.GetConfig().GetOutputPath() + xdbFilter.GetConfig().GetOutputTokenList()<< std::endl;
  std::cout << "xdbFilter OutputTokenListNormalized path:" << xdbFilter.GetConfig().GetOutputPath() + xdbFilter.GetConfig().GetOutputTokenListNormalized()<< std::endl;
  std::cout << "xdbFilter OutputTokenListFuzzy path:" << xdbFilter.GetConfig().GetOutputPath() + xdbFilter.GetConfig().GetOutputTokenListFuzzy()<< std::endl;
  // ====================================================================
#endif // ENABLE_XDBFILTER

#ifdef ENABLE_XDBGENERATOR
  // ============================ CXdbGenerator ============================
  // Set CXdbGenerator In/Out/Log path
  CXdbGenerator xdbGenerator;
  xdbGenerator.GetConfig().SetInputPath(xdbFilter.GetConfig().GetOutputPath());
  //xdbGenerator.GetConfig().SetInputPath("../config/XdbGenerator/TC/Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/");
  xdbGenerator.GetConfig().SetOutputPath("../config/XdbGenerator/TC/Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/");
  xdbGenerator.GetConfig().SetLogPath("../config/XdbGenerator/TC/Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/");

  // Set I/O path of fuzzy xdb
  xdbGenerator.GetConfig().SetInputTokenList(xdbFilter.GetConfig().GetOutputTokenListFuzzy());
  xdbGenerator.GetConfig().SetOutputXdb("xdb_fuzzy.xdb");
  //xdbGenerator.GetConfig().SetInputTokenList("s06_log_tokens_full.txt");
  //xdbGenerator.GetConfig().SetOutputXdb("xdb(s06_log_tokens_full.txt).xdb");
  runSuccess = xdbGenerator.Run();
  if (false == runSuccess)
  {
    std::cout << "xdbGenerator.Run() for fuzzy xdb error!!!" << std::endl;
    return false;
  }

  // Output results xdb path
  std::cout << "xdbGenerator fuzzy xdb path:" <<xdbGenerator.GetConfig().GetOutputPath() + xdbGenerator.GetConfig().GetOutputXdb() << std::endl;

  // Set I/O path of non-fuzzy xdb
  xdbGenerator.GetConfig().SetInputTokenList(xdbFilter.GetConfig().GetOutputTokenList());
  xdbGenerator.GetConfig().SetOutputXdb("xdb_non_fuzzy.xdb");
  runSuccess = xdbGenerator.Run();
  if (false == runSuccess)
  {
    std::cout << "xdbGenerator.Run() for non-fuzzy xdb error!!!" << std::endl;
    return false;
  }

  // Output results xdb path
  std::cout << "xdbGenerator non-fuzzy xdb path:" <<xdbGenerator.GetConfig().GetOutputPath() + xdbGenerator.GetConfig().GetOutputXdb() << std::endl;
  // =============================================================
#endif // ENABLE_XDBGENERATOR

#ifdef ENABLE_XDBDUMPER
  // ============================ CXdbDumper ===========================
  CXdbDumper xdbDumper;
  xdbDumper.GetConfig().SetInputPath("../config/XdbDumper/TC/Normalization_20131017_v01_xdb_filter_optimized/");
  xdbDumper.GetConfig().SetOutputPath("../config/XdbDumper/TC/Normalization_20131017_v01_xdb_filter_optimized/");
  xdbDumper.GetConfig().SetLogPath("../config/XdbDumper/TC/Normalization_20131017_v01_xdb_filter_optimized/");

  xdbDumper.GetConfig().SetInputScwsXdb("xdb(s06_log_tokens_full.txt).xdb");
  //xdbDumper.GetConfig().SetInputScwsXdb("xdb/dict.utf8.xdb");
  xdbDumper.GetConfig().SetInputNormalizeMap("fts-tc-n.tok");
  //xdbDumper.GetConfig().SetInputNormalizeMap("fts-sc-n.tok");

  runSuccess = xdbDumper.Run();
  if (false == runSuccess)
    return false;

  // Output results token list path
  std::cout << "xdbDumper OutputTokenList path:" << xdbDumper.GetConfig().GetOutputPath() + xdbDumper.GetConfig().GetOutputDumpText()<< std::endl;
  // ===================================================================
#endif // ENABLE_XDBDUMPER

  return true;
}

