//
// This file contains the Chinese token filter interface
//

#ifndef __XDB_FILTER_H__
#define __XDB_FILTER_H__

#include "TokenGeneratorChineseConfig.h"

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

#define MAP_INIT  // To allow xdb_filter starting from reload existent raw map.
//#define ENABLE_USER_INTERACTIVE_CONTROL // To allow user interactive remove/add tokens.
#define USE_TC

// Enable this macro will remove all tokens end with "路".
#define REMOVE_FULL_TOKENS_WITH_SPECIAL_END  //"路"

// Enable this macro will add all AreaNames tokens from file.
#define TOKENS_ADD_FROM_AREA_NAME_FILE

// Enable this macro will remove all tokens in {TC|SC}\input\_removed_tokens_table.txt.
#define TOKENS_REMOVE_FROM_FILE

#define MAX_LINE_SIZE (1024)

////////////////////// FILE PATH /////////////////////////////////////
#ifdef USE_TC
#define DATA_DIR "TC/"
#define SCWS_XDB_PATH "input/xdb/dict_cht.utf8.xdb"
#define SCWS_RULE_PATH "input/xdb/fts-tc-r.tok"
#else //USE_TC
#define DATA_DIR "SC/"
#define SCWS_XDB_PATH "input/xdb/dict.utf8.xdb"
#define SCWS_RULE_PATH "input/xdb/fts-sc-r.tok"
#endif //USE_TC

#ifdef USE_TC
#define IN_PATH_S00_SUFFIX_TABLE "input/_ignored_suffix_table_tc_utf8.txt"
#else //USE_TC
#define IN_PATH_S00_SUFFIX_TABLE "input/_ignored_suffix_table_sc_utf8.txt"
#endif //USE_TC
#define OUT_PATH_S00_SUFFIX_TOK_MAP "output_ALL/s00_log_map_suffix_tokens.txt"

#define IN_PATH_S01_SRC "input/ALL.txt"//"input/ALL.txt"
#define OUT_PATH_S01_NOT_CH "output_ALL/s01_log_tokens_not_ch.txt"
#define OUT_PATH_S01_DUPLICATE "output_ALL/s01_log_tokens_duplicate.txt"
#define OUT_PATH_S01_MAP_RAW "output_ALL/s01_log_map_raw.txt" //This is also the input file after disabled MAP_INIT.
#define OUT_PATH_S01_MAP_RELOAD "output_ALL/s01_log_map_raw_reload.txt"

#define OUT_PATH_S02_MAP_SUFFIX "output_ALL/s02_log_map_plus_suffix_tokens.txt"
#ifdef TOKENS_ADD_FROM_AREA_NAME_FILE
#define IN_PATH_S02_AREA_NAME_FILE "input/areas/05_all_area_map02.txt"
#define OUT_PATH_S02_LOG_AREA_NAME_FILE "output_ALL/s02_log_area_name_file.txt"
#define OUT_PATH_S02_LOG_MAP_PLUS_AREA_NAME "output_ALL/s02_log_map_plus_area_name_tokens.txt"
#endif
#ifdef TOKENS_REMOVE_FROM_FILE
#define IN_PATH_S02_REMOVE_TOK_FILE "input/_removed_tokens_table.txt"
#define OUT_PATH_S02_LOG_REMOVE_TOK_FILE "output_ALL/s02_log_remove_tokens.txt"
#define OUT_PATH_S02_LOG_MAP_AFTER_REMOVE_TOK "output_ALL/s02_log_map_minus_remove_tokens.txt"
#endif

#define OUT_PATH_S03_MAP_WITH_ONE_WORDS "output_ALL/s03_log_map_plus_one_word_tokens.txt"
#define OUT_PATH_S03_CH_ONE_WORD "output_ALL/s03_log_map_added_one_word_tokens.txt"

#define OUT_PATH_S04_CH_NOT_FOUND "output_ALL/s04_log_tokens_not_found.txt"
#define OUT_PATH_S04_CH_PART "output_ALL/s04_log_tokens_part.txt"
#define OUT_PATH_S04_CH_FULL "output_ALL/s04_log_tokens_full.txt"

#define OUT_PATH_S05_FOUND_SUFFIX "output_ALL/s05_log_tokens_found_suffix_tokens.txt"
#define OUT_PATH_S05_ADDED_REMOVED_SUFFIX "output_ALL/s05_log_tokens_added_removed_suffix.txt"
#define OUT_PATH_S05_MAP_OPTIMIZED "output_ALL/s05_log_map_optimized.txt"
#ifdef REMOVE_FULL_TOKENS_WITH_SPECIAL_END
#define OUT_PATH_S05_REMOVED_SPECIAL_END_TOKENS "output_ALL/s05_log_tokens_remove_special_end.txt"
#endif

#define OUT_PATH_S06_SUFFIX_NOT_FOUND "output_ALL/s06_log_tokens_not_found.txt"
#define OUT_PATH_S06_SUFFIX_PART "output_ALL/s06_log_tokens_part.txt"
#define OUT_PATH_S06_SUFFIX_FULL "output_ALL/s06_log_tokens_full.txt"

#define _CONVERT_NORMALIZE_
#ifdef _CONVERT_NORMALIZE_
#ifdef USE_TC
#define IN_PATH_S07_NORM_MAP     "input/xdb/fts-tc-n.tok"
#else //USE_TC
#define IN_PATH_S07_NORM_MAP     "input/xdb/fts-sc-n.tok"
#endif //USE_TC
#define OUT_PATH_S07_SUFFIX_FULL_NOR "output_ALL/s07_log_tokens_full_normalized.txt"
#endif //_CONVERT_NORMALIZE_
////////////////////// FILE PATH /////////////////////////////////////

class CXdbFilter : public CTokenGeneratorChineseConfig
{
public:
  CXdbFilter();
  ~CXdbFilter();

  bool Run();

private:
  char tmp_buf1[MAX_LINE_SIZE];

  ////////////////////// Search Node /////////////////////////////////////
  /* header struct */
  typedef struct _node_info_header_
  {
    unsigned int l_offset;
    unsigned int l_length;
    unsigned int r_offset;
    unsigned int r_length;
    unsigned char k_length;
  } node_info_header;

  typedef struct _node_info_attr_
  {
    float tf;
    float idf;
    unsigned char flag;
    char attr[3];
  } node_info_attr;

  typedef struct _node_info_
  {
    unsigned int l_offset;
    unsigned int l_length;
    unsigned int r_offset;
    unsigned int r_length;
    unsigned char k_length;
    unsigned char k_data[300];

    float tf;
    float idf;
    unsigned char flag;
    char attr[3];
  } node_info;

  typedef struct _prime_node_
  {
    int offset;
    int length;
  } prime_node;

  #define SEARCH_TOKEN_INFO_FOUND       (0)
  #define SEARCH_TOKEN_INFO_NOT_FOUND   (-1)

  int _get_hash_index(unsigned char* key, int hash_base, int hash_prime );
  int searchTokenInfo( char *token_content, long node_offset, long node_length, int hash_base, int hash_prime, node_info_attr *attr, FILE* fp_xdb  );
  ////////////////////// Search Node /////////////////////////////////////

  #define UTF8_CHAR_KIND_NONE    (0)
  #define UTF8_CHAR_KIND_1    (1)  // ASCII(0x00~0x7F, 1 BYTE in UTF8)
  #define UTF8_CHAR_KIND_2    (2)  // Latin Characters(First byte range:0xC0-0xDF, 2 BYTE in UTF8)
  #define UTF8_CHAR_KIND_3    (3)  // CJKS  Characters(First byte range:0xE0-0xEF, 3 BYTE in UTF8)
  #define UTF8_CHAR_KIND_4    (4)  // UTF-32 Characters(First byte range:0xF0-0xF7, 4 BYTE in UTF8)
  int getUTF8CharKind(unsigned char x);
  int convUTF8ToUTF16(unsigned char* utf8_code_data, unsigned int* data, int data_length);
  int isAllChineseToken(char* token_content, int token_len);
  int isValidChineseToken(scws_res_t cur, char* token_content);
  int TokenTotalChineseWordCountGet(const char* str);
  int NthChineseWordByteOffsetGet(const char* str, int NthChineseWord);

  ////////////////////// SuffixTokenMap /////////////////////////////////////
  int SuffixTokenMapInit(std::map<std::string,int> &suffix_token_map);
  int MaxSuffixTokenLengthGet(std::map<std::string,int> &suffix_token_map);
  ////////////////////// SuffixTokenMap /////////////////////////////////////

  int isTokenEndWithIgnoredSuffix(const char* str, int* pSuffixOff, char* tmp_buf, int MaxSuffixTokenLength, std::map<std::string,int> &suffix_token_map);
  int TokenGetOneWord(const char* str, char* tmp_buf, size_t aBufferLength, int wordIdx);

  #ifdef _CONVERT_NORMALIZE_
  // Length of multibyte character from first byte of Utf8
  const unsigned char g_mblen_table_utf8[256];

  const unsigned int KNormBufUnitSize;
  const unsigned int KCJKBytes;
  std::map<std::string,std::string> iMap;
  std::map<std::string,std::string>::iterator iMap_it;

  int CHomophoneNormalizer_Init(const char* aFile);
  size_t CHomophoneNormalizer_Normalize(const char* aSource, char* aOutput, size_t aLength);
  void CFtsTokenizerExtChinese_ReserveStringCapacity(std::string& aString, size_t aSize, size_t aUnitSize);
  #endif //_CONVERT_NORMALIZE_

  std::string file_path;
  FILE *fp_xdb;
  xdb_header header;
  node_info_attr node_ia;
  
  char tmp_buf[MAX_LINE_SIZE];

  std::string token_item;
  //std::vector<std::string> token_items;

  std::map<std::string,int> token_map, token_map_with_one_word, token_map_opt;
  std::map<std::string,int> suffix_token_map;
  int token_idx;
  
  int line_no;
  char* pline;

  std::ofstream ofs;
  std::ifstream ifs;
  std::string tmp_line;

  std::size_t pos;
  std::string str1;
  std::string str2;
  int idx_tmp;
  int idx_max;

  size_t readSize;

  std::map<std::string,int> all_area_map;
  std::map<std::string,int>::iterator all_area_iter;

  bool Init();
  bool CollectTokens();
  bool AddExtraTokens();
  bool RemoveUnwantTokens();
  bool AddMissingOneWordTokens();
  bool RemoveSpecialSuffixTokens();
  bool RetrieveTokenInfo();
  bool ConvertToNormalizedTokens();
};

#endif // __XDB_FILTER_H__
