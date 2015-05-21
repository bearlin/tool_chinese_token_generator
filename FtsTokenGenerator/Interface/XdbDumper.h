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

static const int KSzLogSize = 1024;
static const int KMblenTableUTF8Size = 256;
  
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
  typedef struct TXdbHeader
  {
    char tag[3];
    unsigned char ver;
    int base;
    int prime;
    unsigned int fsize;
    float check;
    char unused[12];
  } TXdbHeader;

  typedef struct TNodeContent
  {
    float tf;
    float idf;
    unsigned char flag;
    char attr[3];
  } TNodeContent;

  FILE* iLog;
  FILE* iLogDetail;

  #ifdef CONVERT_NORMALIZE
  std::map<std::string, std::string> iNormalizeHash;
  FILE* iNormalizeLog;

  // For repeat normalized tokens log.
  std::map<std::string, TNodeContent> iNormalizeMap; //this map will save normalized string and it's node_content informations.
  std::map<std::string, std::string> iFirstNormalizeToOriginalMap; //we will remember the first original_string -> normalized_string pair.
  std::vector<std::string> iNormalizeRepeatVector;
  FILE* iNormalizeRepeatLog;

  static const unsigned char iMblenTableUTF8[KMblenTableUTF8Size];
  #endif //CONVERT_NORMALIZE

  std::string iFilePath;

  unsigned long iWordCount;
  char iSzLog[KSzLogSize];
  //unsigned int XDB_MAXKLEN = 0xf0;
  int iPrime;
  int iHashBase;

  int GetIndex(const unsigned char* aKey, int aHashBase, int aHashPrime);
  void GetRecord(FILE *aFd, unsigned int aOffset, unsigned int aLength, int aDirection, int aLevel, const char* aFather);
};

#endif // __XDB_DUMPER_H__
