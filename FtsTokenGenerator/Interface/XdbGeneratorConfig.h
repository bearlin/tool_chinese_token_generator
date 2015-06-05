////////////////////////////////////////////////////////////////////////////////
// This file contains the CXdbGeneratorConfig interface
////////////////////////////////////////////////////////////////////////////////

#ifndef XDB_GENERATOR_CONFIG_H
#define XDB_GENERATOR_CONFIG_H

#include "TokenGeneratorChineseConfig.h"

namespace NFtsTokenGenerator
{

// [Normalization procedures]
// [Abandon] step 0: use xdb_dump tool to dump the original simple_export file from original raw xdb.
// [Abandon] step 1: Add/Delete/Update tokens in the original simple_export file and save to another updated simple_export file.
// step 1: use xdb_filter tool to filter, optimize and generate tt_tokens_list according to TomTom Addresses/POIs.
// step 2: use xdb_gen tool to generate a xdb(non-fuzzy) from this tt_tokens_list file.
// step 3: use xdb_dump tool to generate the non-normalized/normalized token files from this non-fuzzy xdb(with the latest fts-{tc|sc}-n.tok file), then merge them to a merge_export file.
// step 4: use xdb_gen tool generate a final normalized xdb(fuzzy) from this merge_export file.
// TODO: We can just skip step 3(skip xdb_dump), because we can just generage a normalized tt_tokens_list after step 1(we can get both tt_tokens_list(non-normailzed) and tt_tokens_list(normailzed) together).

#define ENABLE_LOG
#define LOG_FILE_NAME "log(s06_log_tokens_full.txt).txt"
#define LOG_REPEAT_FILE_NAME "log_repeat(s06_log_tokens_full.txt).txt"
#define MAX_NODE_COUNT 127
#define XDB_HASH_BASE 0xf422f // Check xtree_new function in xtree.c
#define NO_FATHER -1
#define DIR_LEFT 0
#define DIR_RIGHT 1

class CXdbGeneratorConfig : public CTokenGeneratorChineseConfig
{
public:
  CXdbGeneratorConfig();
  ~CXdbGeneratorConfig();

  void SetInputTokenList(const std::string& aInputTokenList);
  void SetOutputXdb(const std::string& aOutputXdb);
  const std::string GetOutputXdb() const;

  std::string iInputTokenList;
  std::string iOutputXdb;
};

} // namespace NFtsTokenGenerator

#endif // XDB_GENERATOR_CONFIG_H
