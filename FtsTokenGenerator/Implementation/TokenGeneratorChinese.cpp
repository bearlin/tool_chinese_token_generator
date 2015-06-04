//
// This file contains the Chinese token generator implementation 
//

#include "TokenGeneratorChinese.h"
#include "XdbFilter.h"
#include "XdbDumper.h"
#include "XdbGenerator.h"
#include <iostream>

#include "TTLog.h"
DEFINE_LOGGER(gLogCTokenGeneratorChinese, "CTokenGeneratorChinese")

namespace NFtsTokenGenerator
{

#define ENABLE_XDBFILTER
#define ENABLE_XDBGENERATOR
//#define ENABLE_XDBDUMPER

#define TRANDITIONAL_CHINESE
#ifdef TRANDITIONAL_CHINESE
  #define CHINESE_FOLDER "TC"
#else
  #define CHINESE_FOLDER "SC"
#endif //TRANDITIONAL_CHINESE

CTokenGeneratorChinese::CTokenGeneratorChinese()
{
}

CTokenGeneratorChinese::~CTokenGeneratorChinese()
{
}

bool CTokenGeneratorChinese::Run()
{
  bool runSuccess = false;

  LOG_INFO(gLogCTokenGeneratorChinese, "Running CTokenGeneratorChinese...\n");
#ifdef ENABLE_XDBFILTER
  // ============================ CXdbFilter ============================
  // Set CXdbFilter In/Out/Log path
  CXdbFilter xdbFilter;
  xdbFilter.GetConfig().SetInputPath("../config/XdbFilter/" CHINESE_FOLDER "/input/");
  xdbFilter.GetConfig().SetOutputPath("../config/XdbFilter/" CHINESE_FOLDER "/output/");
  xdbFilter.GetConfig().SetLogPath("../config/XdbFilter/" CHINESE_FOLDER "/log/");

  // Set I/O path
#ifdef TRANDITIONAL_CHINESE
  xdbFilter.GetConfig().SetInputScwsXdb("xdb/dict_cht.utf8.xdb");
  xdbFilter.GetConfig().SetInputScwsRule("xdb/fts-tc-r.tok");
  xdbFilter.GetConfig().SetInputNormalizeMap("xdb/fts-tc-n.tok");
  xdbFilter.GetConfig().SetInputSpecialSuffixTable("_ignored_suffix_table_tc_utf8.txt");
#else
  xdbFilter.GetConfig().SetInputScwsXdb("xdb/dict.utf8.xdb");
  xdbFilter.GetConfig().SetInputScwsRule("xdb/fts-sc-r.tok");
  xdbFilter.GetConfig().SetInputNormalizeMap("xdb/fts-sc-n.tok");
  xdbFilter.GetConfig().SetInputSpecialSuffixTable("_ignored_suffix_table_sc_utf8.txt");
#endif //TRANDITIONAL_CHINESE

  xdbFilter.GetConfig().SetInputAreaName("areas/05_all_area_map02.txt");
  xdbFilter.GetConfig().SetInputRemoveToken("_removed_tokens_table.txt");
  xdbFilter.GetConfig().SetInputSourceData("ALL.txt");

  runSuccess = xdbFilter.Run();
  if (false == runSuccess)
  {
    LOG_INFO(gLogCTokenGeneratorChinese, "xdbFilter.Run() error!!!\n");
    return false;
  }

  // Output results token list path
  LOG_INFO(gLogCTokenGeneratorChinese, "xdbFilter OutputTokenList path:%s\n", (xdbFilter.GetConfig().GetOutputPath() + xdbFilter.GetConfig().GetOutputTokenList()).c_str());
  LOG_INFO(gLogCTokenGeneratorChinese, "xdbFilter OutputTokenListNormalized path:%s\n", (xdbFilter.GetConfig().GetOutputPath() + xdbFilter.GetConfig().GetOutputTokenListNormalized()).c_str());
  LOG_INFO(gLogCTokenGeneratorChinese, "xdbFilter OutputTokenListFuzzy path:%s\n", (xdbFilter.GetConfig().GetOutputPath() + xdbFilter.GetConfig().GetOutputTokenListFuzzy()).c_str());
  // ====================================================================
#endif // ENABLE_XDBFILTER

#ifdef ENABLE_XDBGENERATOR
  // ============================ CXdbGenerator ============================
  // Set CXdbGenerator In/Out/Log path
  CXdbGenerator xdbGenerator;
  xdbGenerator.GetConfig().SetInputPath(xdbFilter.GetConfig().GetOutputPath());
  //xdbGenerator.GetConfig().SetInputPath("../config/XdbGenerator/" CHINESE_FOLDER "/input/");
  xdbGenerator.GetConfig().SetOutputPath("../config/XdbGenerator/" CHINESE_FOLDER "/output/");
  xdbGenerator.GetConfig().SetLogPath("../config/XdbGenerator/" CHINESE_FOLDER "/log/");

  // Set I/O path of fuzzy xdb
  xdbGenerator.GetConfig().SetInputTokenList(xdbFilter.GetConfig().GetOutputTokenListFuzzy());
  xdbGenerator.GetConfig().SetOutputXdb("xdb_fuzzy.xdb");
  //xdbGenerator.GetConfig().SetInputTokenList("s06_log_tokens_full.txt");
  //xdbGenerator.GetConfig().SetOutputXdb("xdb(s06_log_tokens_full.txt).xdb");
  runSuccess = xdbGenerator.Run();
  if (false == runSuccess)
  {
    LOG_INFO(gLogCTokenGeneratorChinese, "xdbGenerator.Run() for fuzzy xdb error!!!\n");
    return false;
  }

  // Output results xdb path
  LOG_INFO(gLogCTokenGeneratorChinese, "xdbGenerator fuzzy xdb path:%s\n", (xdbGenerator.GetConfig().GetOutputPath() + xdbGenerator.GetConfig().GetOutputXdb()).c_str());

  // Set I/O path of non-fuzzy xdb
  xdbGenerator.GetConfig().SetInputTokenList(xdbFilter.GetConfig().GetOutputTokenList());
  xdbGenerator.GetConfig().SetOutputXdb("xdb_non_fuzzy.xdb");
  runSuccess = xdbGenerator.Run();
  if (false == runSuccess)
  {
    LOG_INFO(gLogCTokenGeneratorChinese, "xdbGenerator.Run() for non-fuzzy xdb error!!!\n");
    return false;
  }

  // Output results xdb path
  LOG_INFO(gLogCTokenGeneratorChinese, "xdbGenerator non-fuzzy xdb path:%s\n", (xdbGenerator.GetConfig().GetOutputPath() + xdbGenerator.GetConfig().GetOutputXdb()).c_str());
  // =============================================================
#endif // ENABLE_XDBGENERATOR

#ifdef ENABLE_XDBDUMPER
  // ============================ CXdbDumper ===========================
  CXdbDumper xdbDumper;
  xdbDumper.GetConfig().SetInputPath("../config/XdbDumper/" CHINESE_FOLDER "/input/");
  xdbDumper.GetConfig().SetOutputPath("../config/XdbDumper/" CHINESE_FOLDER "/output/");
  xdbDumper.GetConfig().SetLogPath("../config/XdbDumper/" CHINESE_FOLDER "/log/");

  xdbDumper.GetConfig().SetInputScwsXdb("xdb(s06_log_tokens_full.txt).xdb");
#ifdef TRANDITIONAL_CHINESE
  xdbDumper.GetConfig().SetInputNormalizeMap("fts-tc-n.tok");
#else
  xdbDumper.GetConfig().SetInputNormalizeMap("fts-sc-n.tok");
#endif

  runSuccess = xdbDumper.Run();
  if (false == runSuccess)
  {
    LOG_INFO(gLogCTokenGeneratorChinese, "xdbDumper.Run() error!!!\n");
    return false;
  }

  // Output results token list path
  LOG_INFO(gLogCTokenGeneratorChinese, "xdbDumper OutputTokenList path:%s\n", (xdbDumper.GetConfig().GetOutputPath() + xdbDumper.GetConfig().GetOutputDumpText()).c_str());
  // ===================================================================
#endif // ENABLE_XDBDUMPER

  return true;
}

} // namespace NFtsTokenGenerator

