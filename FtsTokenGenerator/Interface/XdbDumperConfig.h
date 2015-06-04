//
// This file contains the Chinese token dumper config interface
//

#ifndef XDB_DUMPER_CONFIG_H
#define XDB_DUMPER_CONFIG_H

#include "TokenGeneratorChineseConfig.h"

namespace NFtsTokenGenerator
{

// Select one of below macro:
// XDB_GEN_TOOL_EXPORT_FILE will use dict_cht.utf8.xdb as input and output all nodes to simple_export.txt.
// DETAIL_EXPORT_FILE will use dict_cht.utf8.xdb as input too, and output detail nodes information to detail_export.txt.
#define XDB_GEN_TOOL_EXPORT_FILE
//#define DETAIL_EXPORT_FILE

//CONVERT_NORMALIZE + XDB_GEN_TOOL_EXPORT_FILE will output all nodes to normal_export.txt, and its key will be normalized with char_pinyin_normalize_utf8.txt.
//#define CONVERT_NORMALIZE

//[Normalization procedures]
//[Abandon] step 0: use xdb_dump tool to dump the original simple_export file from original raw xdb.
//[Abandon] step 1: Add/Delete/Update tokens in the original simple_export file and save to another updated simple_export file.
//step 1: use xdb_filter tool to filter, optimize and generate tt_tokens_list according to TomTom Addresses/POIs.
//step 2: use xdb_gen tool to generate a xdb(non-fuzzy) from this tt_tokens_list file.
//step 3: use xdb_dump tool to generate the non-normalized/normalized token files from this non-fuzzy xdb(with the latest fts-{tc|sc}-n.tok file), then merge them to a merge_export file.
//step 4: use xdb_gen tool generate a final normalized xdb(fuzzy) from this merge_export file.
// TODO: We can just skip step 3(skip xdb_dump), because we can just generage a normalized tt_tokens_list after step 1(we can get both tt_tokens_list(non-normailzed) and tt_tokens_list(normailzed) together).

  #ifdef DETAIL_EXPORT_FILE
    #define DETAIL_EXPORT_FILENAME "detail_export.txt"  // detail export file
  #endif
  #ifdef XDB_GEN_TOOL_EXPORT_FILE
    #define SIMPLE_EXPORT_FILENAME "simple_export.txt"  // simple export file
  #endif
  #ifdef CONVERT_NORMALIZE
    #define NORMAL_EXPORT_FILENAME "normal_export.txt"  // normalize export file
    #define NORMAL_EXPORT_REPEAT_FILENAME "normal_export_repeat.txt"  // normalize export repeat file
  #endif

class CXdbDumperConfig : public CTokenGeneratorChineseConfig
{
public:
  CXdbDumperConfig();
  ~CXdbDumperConfig();

  void SetInputScwsXdb(const std::string& aInputScwsXdb);
  void SetInputNormalizeMap(const std::string& aInputNormalizeMap);
  const std::string GetOutputDumpText() const;

  std::string iInputScwsXdb;
  std::string iInputNormalizeMap;
  std::string iOutputDumpText;

};

} // namespace NFtsTokenGenerator

#endif // XDB_DUMPER_CONFIG_H
