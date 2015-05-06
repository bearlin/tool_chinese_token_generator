//
// This file contains the Chinese token generator interface
//

#ifndef __XDB_GENERATOR_H__
#define __XDB_GENERATOR_H__

#include "TokenGeneratorChineseConfig.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>     /* atoi */
#include <algorithm>
#include <vector>
#include <string>

//[Normalization procedures]
//[Abandon] step 0: use xdb_dump tool to dump the original simple_export file from original raw xdb.
//[Abandon] step 1: Add/Delete/Update tokens in the original simple_export file and save to another updated simple_export file.
//step 1: use xdb_filter tool to filter, optimize and generate tt_tokens_list according to TomTom Addresses/POIs.
//step 2: use xdb_gen tool to generate a xdb(non-fuzzy) from this tt_tokens_list file.
//step 3: use xdb_dump tool to generate the non-normalized/normalized token files from this non-fuzzy xdb(with the latest fts-{tc|sc}-n.tok file), then merge them to a merge_export file.
//step 4: use xdb_gen tool generate a final normalized xdb(fuzzy) from this merge_export file.
// TODO: We can just skip step 3(skip xdb_dump), because we can just generage a normalized tt_tokens_list after step 1(we can get both tt_tokens_list(non-normailzed) and tt_tokens_list(normailzed) together).

#if 1 // traditional Chinese
#define DATA_DIR "TC/"
#if 1 //step 2: use xdb_gen tool to generate a updated xdb from this updated simple_export file.
#define SRC_PATH "Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/s06_log_tokens_full.txt" 
#define LOG_PATH "Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/log(s06_log_tokens_full.txt).txt" 
#define XDB_PATH "Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/xdb(s06_log_tokens_full.txt).xdb" 
#define LOG_REPEAT_PATH "Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/log_repeat(s06_log_tokens_full.txt).txt" 
#else //step 4: use xdb_gen tool generate a final normalized xdb from this merge_export file.
#define SRC_PATH "Normalization_20131017_v01_xdb_filter_optimized/2_merge_gen/merge_export(SILK31).txt"
#define LOG_PATH "Normalization_20131017_v01_xdb_filter_optimized/2_merge_gen/log(merge_export(SILK31).txt).txt" 
#define XDB_PATH "Normalization_20131017_v01_xdb_filter_optimized/2_merge_gen/xdb(merge_export(SILK31).txt).xdb" 
#define LOG_REPEAT_PATH "Normalization_20131017_v01_xdb_filter_optimized/2_merge_gen/log_repeat(merge_export(SILK31).txt).txt" 
#endif
#else // simplify Chinese
#define DATA_DIR "SC/"
#if 0 //step 2: use xdb_gen tool to generate a updated xdb from this updated simple_export file.
#define SRC_PATH "Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/s06_log_tokens_full.txt" 
#define LOG_PATH "Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/log(s06_log_tokens_full.txt).txt" 
#define XDB_PATH "Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/xdb(s06_log_tokens_full.txt).xdb" 
#define LOG_REPEAT_PATH "Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/log_repeat(s06_log_tokens_full.txt).txt" 
#else //step 4: use xdb_gen tool generate a final normalized xdb from this merge_export file.
#define SRC_PATH "Normalization_20131017_v01_xdb_filter_optimized/2_merge_gen/merge_export(SILK31).txt" 
#define LOG_PATH "Normalization_20131017_v01_xdb_filter_optimized/2_merge_gen/log(merge_export(SILK31).txt).txt" 
#define XDB_PATH "Normalization_20131017_v01_xdb_filter_optimized/2_merge_gen/xdb(merge_export(SILK31).txt).xdb" 
#define LOG_REPEAT_PATH "Normalization_20131017_v01_xdb_filter_optimized/2_merge_gen/log_repeat(merge_export(SILK31).txt).txt" 
#endif
#endif


#define ENABLE_LOG

/* constant var define */
#define  SCWS_WORD_FULL    0x01  // 多字: 整词
#define  SCWS_WORD_PART    0x02  // 多字: 前词段
#define  SCWS_WORD_USED    0x04  // 多字: 已使用
#define  SCWS_WORD_RULE    0x08  // 多字: 自动识别的
#define  SCWS_WORD_LONG    0x10  // 多字: 短词组成的长词

#define  SCWS_WORD_MALLOCED  0x80  // xdict_query 结果必须调用 free

#define  SCWS_ZFLAG_PUT    0x02  // 单字: 已使用
#define  SCWS_ZFLAG_N2    0x04  // 单字: 双字名词头
#define  SCWS_ZFLAG_NR2    0x08  // 单字: 词头且为双字人名
#define  SCWS_ZFLAG_WHEAD  0x10  // 单字: 词头
#define  SCWS_ZFLAG_WPART  0x20  // 单字: 词尾或词中
#define  SCWS_ZFLAG_ENGLISH  0x40  // 单字: 夹在中间的英文
#define SCWS_ZFLAG_SYMBOL   0x80    // 单字: 符号系列
#ifndef _GET_PRIME_ 
#define  SCWS_XDICT_PRIME  0x3ffd  // 词典结构树数：16381
#else
#define  SCWS_XDICT_PRIME  30000  // 词典结构树数：30000
#define CHK_MIN_PRIME 18000//16381
#define CHK_MAX_PRIME 30000
#endif

//#define MAX_NODE_COUNT    63
#define MAX_NODE_COUNT    127

#define ENABLE_LOG

/* constant var define */
#define  SCWS_WORD_FULL    0x01  // 多字: 整词
#define  SCWS_WORD_PART    0x02  // 多字: 前词段
#define  SCWS_WORD_USED    0x04  // 多字: 已使用
#define  SCWS_WORD_RULE    0x08  // 多字: 自动识别的
#define  SCWS_WORD_LONG    0x10  // 多字: 短词组成的长词

#define  SCWS_WORD_MALLOCED  0x80  // xdict_query 结果必须调用 free

#define  SCWS_ZFLAG_PUT    0x02  // 单字: 已使用
#define  SCWS_ZFLAG_N2    0x04  // 单字: 双字名词头
#define  SCWS_ZFLAG_NR2    0x08  // 单字: 词头且为双字人名
#define  SCWS_ZFLAG_WHEAD  0x10  // 单字: 词头
#define  SCWS_ZFLAG_WPART  0x20  // 单字: 词尾或词中
#define  SCWS_ZFLAG_ENGLISH  0x40  // 单字: 夹在中间的英文
#define SCWS_ZFLAG_SYMBOL   0x80    // 单字: 符号系列
#ifndef _GET_PRIME_ 
#define  SCWS_XDICT_PRIME  0x3ffd  // 词典结构树数：16381
#else
#define  SCWS_XDICT_PRIME  30000  // 词典结构树数：30000
#define CHK_MIN_PRIME 18000//16381
#define CHK_MAX_PRIME 30000
#endif

//#define MAX_NODE_COUNT    63
#define MAX_NODE_COUNT    127

class CXdbGenerator : public CTokenGeneratorChineseConfig
{
public:
  CXdbGenerator();
  ~CXdbGenerator();

  bool Run();

private:
  typedef struct _node_info_
  {
    int prime_index;
    int level;
    int father;

    unsigned int offset;
    unsigned int length;
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

  /* constant var define */
  #define  XDB_FLOAT_CHECK    (3.14)
  #define  XDB_TAGNAME      "XDB"
  #define  XDB_MAXKLEN      0xf0
  #define  XDB_VERSION      34      /* version: 3bit+5bit */
  #define XDB_BASE 0xf422f
  //#define XDB_BASE 0xf4232

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

  #define NO_FATHER      -1
  #define DIR_LEFT  0
  #define DIR_RIGHT 1

  const unsigned char _mblen_table_utf8[256];
  std::string file_path;

  int _get_hash_index(unsigned char* key, int hash_base, int hash_prime );
  static bool compare_node(const node_info s1,const node_info s2);
  void gen_btree_node_index(int start, int end, int father, int level, std::vector<node_info>::iterator iter, int dir );
  void write_sort_data_to_xdb(int start, int end, unsigned int node_offset, unsigned int father_offset, int prime, std::vector<node_info>::iterator iter, FILE *fp_xdb );

};

#endif // __XDB_GENERATOR_H__
