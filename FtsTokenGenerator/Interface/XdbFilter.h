//
// This file contains the Chinese token filter interface
//

#ifndef XDB_FILTER_H
#define XDB_FILTER_H

#include "XdbFilterConfig.h"

#include <iostream>
#include <string>
#include <map>
#include <fstream>      // std::ofstream
#include <cstdio>
#include <cstring>
#include <cstdlib>     /* atoi */

#include "xtree.h"
#include "xdb.h"
#include "xdict.h"
#include "scws.h"

#if defined(WIN32)
  #define strcasecmp _stricmp
#endif

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
//step 4: use xdb_gen tool generate a final normalized xdb(fuzzy) from this merge_export file.
// TODO: We can just skip step 3(skip xdb_dump), because we can just generage a normalized tt_tokens_list after step 1(we can get both tt_tokens_list(non-normailzed) and tt_tokens_list(normailzed) together).

#define MAP_INIT  // To allow xdb_filter starting from reload existent raw map.
//#define ENABLE_USER_INTERACTIVE_CONTROL // To allow user interactive remove/add tokens.

// Enable this macro will remove all tokens end with "路".
#define REMOVE_FULL_TOKENS_WITH_SPECIAL_END  //"路"

#define MAX_LINE_SIZE (1024)

////////////////////// FILE PATH /////////////////////////////////////
#define OUT_PATH_S00_SUFFIX_TOK_MAP "s00_log_map_suffix_tokens.txt"

#define OUT_PATH_S01_NOT_CH "s01_log_tokens_not_ch.txt"
#define OUT_PATH_S01_DUPLICATE "s01_log_tokens_duplicate.txt"
#define OUT_PATH_S01_MAP_RAW "s01_log_map_raw.txt" //This is also the input file after disabled MAP_INIT.
#define OUT_PATH_S01_MAP_RELOAD "s01_log_map_raw_reload.txt"

#define OUT_PATH_S02_MAP_SUFFIX "s02_log_map_plus_suffix_tokens.txt"
#define OUT_PATH_S02_LOG_AREA_NAME_FILE "s02_log_area_name_file.txt"
#define OUT_PATH_S02_LOG_MAP_PLUS_AREA_NAME "s02_log_map_plus_area_name_tokens.txt"
#define OUT_PATH_S02_LOG_REMOVE_TOK_FILE "s02_log_remove_tokens.txt"
#define OUT_PATH_S02_LOG_MAP_AFTER_REMOVE_TOK "s02_log_map_minus_remove_tokens.txt"

#define OUT_PATH_S03_MAP_WITH_ONE_WORDS "s03_log_map_plus_one_word_tokens.txt"
#define OUT_PATH_S03_CH_ONE_WORD "s03_log_map_added_one_word_tokens.txt"

#define OUT_PATH_S04_CH_NOT_FOUND "s04_log_tokens_not_found.txt"
#define OUT_PATH_S04_CH_PART "s04_log_tokens_part.txt"
#define OUT_PATH_S04_CH_FULL "s04_log_tokens_full.txt"

#define OUT_PATH_S05_FOUND_SUFFIX "s05_log_tokens_found_suffix_tokens.txt"
#define OUT_PATH_S05_ADDED_REMOVED_SUFFIX "s05_log_tokens_added_removed_suffix.txt"
#define OUT_PATH_S05_MAP_OPTIMIZED "s05_log_map_optimized.txt"
#ifdef REMOVE_FULL_TOKENS_WITH_SPECIAL_END
#define OUT_PATH_S05_REMOVED_SPECIAL_END_TOKENS "s05_log_tokens_remove_special_end.txt"
#endif

#define OUT_PATH_S06_SUFFIX_NOT_FOUND "s06_log_tokens_not_found.txt"
#define OUT_PATH_S06_SUFFIX_PART "s06_log_tokens_part.txt"
#define OUT_PATH_S06_SUFFIX_FULL "s06_log_tokens_full.txt"

#define OUT_PATH_S07_SUFFIX_FULL_NOR "s07_log_tokens_full_normalized.txt"
#define OUT_PATH_S08_SUFFIX_FULL_FUZZY "s08_log_tokens_full_fuzzy.txt"
////////////////////// FILE PATH /////////////////////////////////////

class CXdbFilter
{
public:
  CXdbFilter();
  ~CXdbFilter();

  bool Run();
  CXdbFilterConfig& GetConfig();

private:
  CXdbFilterConfig iConfig;

  char iBuffer[MAX_LINE_SIZE];

  ////////////////////// Search Node /////////////////////////////////////
  /* header struct */
  typedef struct TNodeInfoHeader
  {
    unsigned int l_offset;
    unsigned int l_length;
    unsigned int r_offset;
    unsigned int r_length;
    unsigned char k_length;
  } TNodeInfoHeader;

  typedef struct TNodeInfoAttr
  {
    float tf;
    float idf;
    unsigned char flag;
    char attr[3];
  } TNodeInfoAttr;

  typedef struct TPrimeNode
  {
    int offset;
    int length;
  } TPrimeNode;

  #define SEARCH_TOKEN_INFO_FOUND (0)
  #define SEARCH_TOKEN_INFO_NOT_FOUND (-1)

  int GetHashIndex( const unsigned char*  aKey,
                    int                   aHashBase,
                    int                   aHashPrime) const;
  int SearchTokenInfo(const char*     aTokenContent,
                      long            aNodeOffset,
                      long            aNodeLength,
                      int             aHashBase,
                      int             aHashPrime,
                      TNodeInfoAttr*  aAttribute,
                      FILE*           aFileXdb);
  ////////////////////// Search Node /////////////////////////////////////

  #define UTF8_CHAR_KIND_NONE (0)
  #define UTF8_CHAR_KIND_1 (1)  // ASCII(0x00~0x7F, 1 BYTE in UTF8)
  #define UTF8_CHAR_KIND_2 (2)  // Latin Characters(First byte range:0xC0-0xDF, 2 BYTE in UTF8)
  #define UTF8_CHAR_KIND_3 (3)  // CJKS  Characters(First byte range:0xE0-0xEF, 3 BYTE in UTF8)
  #define UTF8_CHAR_KIND_4 (4)  // UTF-32 Characters(First byte range:0xF0-0xF7, 4 BYTE in UTF8)
  int GetUTF8CharKind(unsigned char aChar);
  int ConvUTF8ToUTF16(unsigned char* aUtf8CodeData,
                      unsigned int* aData,
                      int aDataLength);
  int IsAllChineseToken(const char* aTokenContent,
                        int aTokenLen);
  int IsValidChineseToken(scws_res_t  aScwsCur,
                          const char* aTokenContent);
  int GetTokenTotalChineseWordCount(const char* aString);
  int GetNthChineseWordByteOffset(const char* aString,
                                  int aNthChineseWord);

  ////////////////////// SuffixTokenMap /////////////////////////////////////
  int InitSuffixTokenMap(std::map<std::string,int>& aSuffixTokenMap);
  int GetMaxSuffixTokenLength(std::map<std::string,int>& aSuffixTokenMap);
  ////////////////////// SuffixTokenMap /////////////////////////////////////

  int IsTokenEndWithIgnoredSuffix(const char* aString,
                                  int* aSuffixOffset,
                                  char* aTmpBuffer,
                                  int aMaxSuffixTokenLength,
                                  std::map<std::string,int>& aSuffixTokenMap);
  int GetOneWordInToken(const char* aString,
                        char* aTmpBuffer,
                        size_t aBufferLength,
                        int aWordIdx);

  // Length of multibyte character from first byte of Utf8
  static const unsigned char iUTF8MultibyteLengthTable[256];

  const unsigned int KNormBufUnitSize;
  const unsigned int KCJKBytes;
  std::map<std::string,std::string> iNormalizerMap;

  int CHomophoneNormalizer_Init(const char* aFile);
  size_t CHomophoneNormalizer_Normalize(const char* aSource, char* aOutput, size_t aLength);
  void CFtsTokenizerExtChinese_ReserveStringCapacity(std::string& aString, size_t aSize, size_t aUnitSize);

  std::string iFilePath;
  FILE* iFileXdb;
  xdb_header iXdbHeader;
  TNodeInfoAttr iNodeInfoAttr;

  char iTmpBuffer[MAX_LINE_SIZE];

  std::string iTokenItem;

  int iTokenIndex;
  std::map<std::string,int> iTokenMap, iTokenMapWithOneWord, iTokenMapOpt;
  std::map<std::string,int> iSuffixTokenMap;
  std::map<std::string,int> iAllAreaMap;

  std::ofstream iOutputFile;
  std::ifstream iInputFile;
  std::string iTmpLine;

  bool Init();
  bool CollectTokens();
  bool AddExtraTokens();
  bool RemoveUnwantTokens();
  bool AddMissingOneWordTokens();
  bool RemoveSpecialSuffixTokens();
  bool RetrieveTokenInfo();
  bool ConvertToNormalizedTokens();
  bool MergeToFuzzyTokens();
};

#endif // XDB_FILTER_H_
