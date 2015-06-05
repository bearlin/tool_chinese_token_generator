////////////////////////////////////////////////////////////////////////////////
// This file contains the CXdbFilterConfig interface
////////////////////////////////////////////////////////////////////////////////

#ifndef XDB_FILTER_CONFIG_H
#define XDB_FILTER_CONFIG_H

#include "TokenGeneratorChineseConfig.h"

namespace NFtsTokenGenerator
{

// Use xdb_filter tool to filter, optimize and generate tokens list according to TomTom Addresses/POIs, steps: 
// 1. Dump all Addresses/POIs from TomTom map(Taiwan or China).
// 2. Use xdb_filter tool, xdb_filter will send these Addresses/POIs to SCWS to get basic TomTom tokens list, 
//    before using SCWS, the .xdb and .ini files used by SCWS need to be adjusted as we want(ex: remove all prefix/suffix in the .ini file).
// 3. Add one word tokens according to TomTom tokens(see codes around GetOneWordInToken()).
// 4. Remove/Add TomTom tokens from list according to specific rules(ex: We may remove tokens with '縣','市','鎮','村','鄉','區','特區','園區','新村' suffix), 
//    and user interactive remove/add tokens can be enabled(see ENABLE_USER_INTERACTIVE_CONTROL).
// 5. Generate new fuzzy .xdb(by using xdb_gen and xdb_dump tools) and fuzzy map(by using MTC asia branch) for testing.

//[Normalization procedures]
//[Abandon] step 0: use xdb_dump tool to dump the original simple_export file from original raw xdb.
//[Abandon] step 1: Add/Delete/Update tokens in the original simple_export file and save to another updated simple_export file.
//step 1: use xdb_filter tool to filter, optimize and generate tt_tokens_list according to TomTom Addresses/POIs.
//step 2: use xdb_gen tool to generate a xdb(non-fuzzy) from this tt_tokens_list file.
//step 3: use xdb_dump tool to generate the non-normalized/normalized token files from this non-fuzzy xdb(with the latest fts-{tc|sc}-n.tok file), then merge them to a merge_export file.
//step 4: use xdb_gen tool generate a final normalized xdb(fuzzy) from this merge_export file

#define MAP_INIT  // To allow xdb_filter starting from reload existent raw map tokens
//#define ENABLE_USER_INTERACTIVE_CONTROL // To allow user interactively remove/add tokens

// Enable this macro will remove all tokens which end with "road" in Chinese
#define REMOVE_FULL_TOKENS_WITH_SPECIAL_END

#define MAX_LINE_SIZE (1024)
#define MAX_TOKEN_SIZE (300)
#define MAX_UTF8_CHARACTER_LENGTH (7)
#define MAX_UTF16_TOKEN_LENGTH (10)

#define OUT_FILE_S00_SUFFIX_TOK_MAP "s00_log_map_suffix_tokens.txt"
#define OUT_FILE_S01_NOT_CH "s01_log_tokens_not_ch.txt"
#define OUT_FILE_S01_DUPLICATE "s01_log_tokens_duplicate.txt"
#define OUT_FILE_S01_MAP_RAW "s01_log_map_raw.txt" //This is also the input file after disabled MAP_INIT
#define OUT_FILE_S01_MAP_RELOAD "s01_log_map_raw_reload.txt"
#define OUT_FILE_S02_MAP_SUFFIX "s02_log_map_plus_suffix_tokens.txt"
#define OUT_FILE_S02_LOG_AREA_NAME_FILE "s02_log_area_name_file.txt"
#define OUT_FILE_S02_LOG_MAP_PLUS_AREA_NAME "s02_log_map_plus_area_name_tokens.txt"
#define OUT_FILE_S02_LOG_REMOVE_TOK_FILE "s02_log_remove_tokens.txt"
#define OUT_FILE_S02_LOG_MAP_AFTER_REMOVE_TOK "s02_log_map_minus_remove_tokens.txt"
#define OUT_FILE_S03_MAP_WITH_ONE_WORDS "s03_log_map_plus_one_word_tokens.txt"
#define OUT_FILE_S03_CH_ONE_WORD "s03_log_map_added_one_word_tokens.txt"
#define OUT_FILE_S04_CH_NOT_FOUND "s04_log_tokens_not_found.txt"
#define OUT_FILE_S04_CH_PART "s04_log_tokens_part.txt"
#define OUT_FILE_S04_CH_FULL "s04_log_tokens_full.txt"
#define OUT_FILE_S05_FOUND_SUFFIX "s05_log_tokens_found_suffix_tokens.txt"
#define OUT_FILE_S05_ADDED_REMOVED_SUFFIX "s05_log_tokens_added_removed_suffix.txt"
#define OUT_FILE_S05_MAP_OPTIMIZED "s05_log_map_optimized.txt"
#ifdef REMOVE_FULL_TOKENS_WITH_SPECIAL_END
#define OUT_FILE_S05_REMOVED_SPECIAL_END_TOKENS "s05_log_tokens_remove_special_end.txt"
#endif
#define OUT_FILE_S06_SUFFIX_NOT_FOUND "s06_log_tokens_not_found.txt"
#define OUT_FILE_S06_SUFFIX_PART "s06_log_tokens_part.txt"
#define OUT_FILE_S06_SUFFIX_FULL "s06_log_tokens_full.txt"
#define OUT_FILE_S07_SUFFIX_FULL_NOR "s07_log_tokens_full_normalized.txt"
#define OUT_FILE_S08_SUFFIX_FULL_FUZZY "s08_log_tokens_full_fuzzy.txt"

class CXdbFilterConfig : public CTokenGeneratorChineseConfig
{
public:
  CXdbFilterConfig();
  ~CXdbFilterConfig();

  void SetInputScwsXdb(const std::string& aInputScwsXdb);
  void SetInputScwsRule(const std::string& aInputScwsRule);
  void SetInputSourceData(const std::string& aInputSourceData);
  void SetInputNormalizeMap(const std::string& aInputNormalizeMap);
  void SetInputSpecialSuffixTable(const std::string& aInputSpecialSuffixTable);
  void SetInputAreaName(const std::string& aInputAreaName);
  void SetInputRemoveToken(const std::string& aInputRemoveToken);
  const std::string GetOutputTokenList() const;
  const std::string GetOutputTokenListNormalized() const;
  const std::string GetOutputTokenListFuzzy() const;

  std::string iInputScwsXdb;
  std::string iInputScwsRule;
  std::string iInputNormalizeMap;
  std::string iInputSpecialSuffixTable;
  std::string iInputAreaName;
  std::string iInputRemoveToken;
  std::string iInputSourceData;
  std::string iOutputTokenList;
  std::string iOutputTokenListNormalized;
  std::string iOutputTokenListFuzzy;
};

} // namespace NFtsTokenGenerator

#endif // XDB_FILTER_CONFIG_H
