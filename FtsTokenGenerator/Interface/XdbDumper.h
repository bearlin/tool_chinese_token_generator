//
// This file contains the Chinese token dumper interface
//

#ifndef __XDB_DUMPER_H__
#define __XDB_DUMPER_H__

#include "XdbDumperConfig.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>     /* atoi */

#include <string>
#include <vector>
#include <map>
#include <algorithm>    // std::find std::sort

// Select one of below macro:
// _XDB_GEN_TOOL_EXPORT_ will use dict_cht.utf8.xdb as input and output all nodes to simple_export.txt.
// _DETAIL_EXPORT_ will use dict_cht.utf8.xdb as input too, and output detail nodes information to detail_export.txt.
#define _XDB_GEN_TOOL_EXPORT_
//#define _DETAIL_EXPORT_

//_CONVERT_NORMALIZE_ + _XDB_GEN_TOOL_EXPORT_ will output all nodes to normal_export.txt, and its key will be normalized with char_pinyin_normalize_utf8.txt.
//#define _CONVERT_NORMALIZE_

//[Normalization procedures]
//[Abandon] step 0: use xdb_dump tool to dump the original simple_export file from original raw xdb.
//[Abandon] step 1: Add/Delete/Update tokens in the original simple_export file and save to another updated simple_export file.
//step 1: use xdb_filter tool to filter, optimize and generate tt_tokens_list according to TomTom Addresses/POIs.
//step 2: use xdb_gen tool to generate a xdb(non-fuzzy) from this tt_tokens_list file.
//step 3: use xdb_dump tool to generate the non-normalized/normalized token files from this non-fuzzy xdb(with the latest fts-{tc|sc}-n.tok file), then merge them to a merge_export file.
//step 4: use xdb_gen tool generate a final normalized xdb(fuzzy) from this merge_export file.
// TODO: We can just skip step 3(skip xdb_dump), because we can just generage a normalized tt_tokens_list after step 1(we can get both tt_tokens_list(non-normailzed) and tt_tokens_list(normailzed) together).

  #ifdef _DETAIL_EXPORT_
    #define DETAIL_EXP_FILE "detail_export.txt"  // detail export file
  #endif
  #ifdef _XDB_GEN_TOOL_EXPORT_
    #define SIMPLE_EXP_FILE "simple_export.txt"  // simple export file
  #endif
  #ifdef _CONVERT_NORMALIZE_
    #define NORMAL_EXP_FILE "normal_export.txt"  // normalize export file
    #define NORMAL_EXP_REPEAT "normal_export_repeat.txt"  // normalize export repeat file
  #endif

class CXdbDumper
{
public:
  CXdbDumper();
  ~CXdbDumper();

  bool Run();
  CXdbDumperConfig& GetConfig();

private:
  CXdbDumperConfig iConfig;

  /* header struct */
  struct xdb_header
  {
    char tag[3];
    unsigned char ver;
    int base;
    int prime;
    unsigned int fsize;
    float check;
    char unused[12];
  };

  typedef struct _node_content
  {
    float tf;
    float idf;
    unsigned char flag;
    char attr[3];
  } node_content;

  FILE *gLog;
  FILE *gLogDetail;

  #ifdef _CONVERT_NORMALIZE_
  std::map<std::string, std::string>  g_normalize_hash;
  FILE *gNormalizeLog;

  // For repeat normalized tokens log.
  std::map<std::string,node_content> nor_map; //this map will save normalized string and it's node_content informations.
  std::map<std::string,std::string> first_nor_to_ori_map; //we will remember the first original_string -> normalized_string pair.
  std::vector<std::string> nor_repeat_vector;
  FILE *gNormalizeRepeatLog;

  const unsigned char _mblen_table_utf8[256];
  #endif //_CONVERT_NORMALIZE_

  std::string file_path;

  unsigned long word_count;
  char szLog[1024];
  //unsigned int XDB_MAXKLEN = 0xf0;
  int gPrime,gHashBase;

  int _get_index(unsigned char* key, int hash_base, int hash_prime );
  void get_record(FILE *fd, unsigned int off, unsigned int len, int direction, int level, const char* father );
};

#endif // __XDB_DUMPER_H__
