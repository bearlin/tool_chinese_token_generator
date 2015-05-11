//
// This file contains the Chinese token generator interface
//

#ifndef XDB_GENERATOR_H
#define XDB_GENERATOR_H

#include "XdbGeneratorConfig.h"

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

#define LOG_PATH "log(s06_log_tokens_full.txt).txt" 
#define LOG_REPEAT_PATH "log_repeat(s06_log_tokens_full.txt).txt" 

#define ENABLE_LOG

/* constant var define */
#define SCWS_WORD_FULL 0x01
#define SCWS_WORD_PART 0x02
#define SCWS_WORD_USED 0x04
#define SCWS_WORD_RULE 0x08
#define SCWS_WORD_LONG 0x10

#define SCWS_WORD_MALLOCED 0x80

#define SCWS_ZFLAG_PUT 0x02
#define SCWS_ZFLAG_N2 0x04
#define SCWS_ZFLAG_NR2 0x08
#define SCWS_ZFLAG_WHEAD 0x10
#define SCWS_ZFLAG_WPART 0x20
#define SCWS_ZFLAG_ENGLISH 0x40
#define SCWS_ZFLAG_SYMBOL 0x80

#ifndef GET_PRIME
  #define SCWS_XDICT_PRIME 0x3ffd
#else
  #define SCWS_XDICT_PRIME 30000
  #define CHK_MIN_PRIME 18000
  #define CHK_MAX_PRIME 30000
#endif //GET_PRIME

//#define MAX_NODE_COUNT    63
#define MAX_NODE_COUNT    127


/* constant var define */
#define XDB_FLOAT_CHECK (3.14)
#define XDB_TAGNAME "XDB"
#define XDB_MAXKLEN 0xf0
#define XDB_VERSION 34      /* version: 3bit+5bit */
#define XDB_BASE 0xf422f
//#define XDB_BASE 0xf4232


#define NO_FATHER -1
#define DIR_LEFT 0
#define DIR_RIGHT 1

/* header struct */
typedef struct TXdb_header
{
  char tag[3];
  unsigned char ver;
  int base;
  int prime;
  unsigned int fsize;
  float check;
  char unused[12];
}TXdb_header;

typedef struct TNodeInfo
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
} TNodeInfo;

class CXdbGenerator
{
public:
  CXdbGenerator();
  ~CXdbGenerator();

  bool Run();
  CXdbGeneratorConfig& GetConfig();

private:
  CXdbGeneratorConfig iConfig;

  // Length of multibyte character from first byte of Utf8
  const unsigned char iUTF8MultibyteLengthTable[256];

  int getHashIndex(const unsigned char* aKey, int aHashBase, int aHashPrime ) const;
  static bool compareNode(const TNodeInfo& aNodeInfo1, const TNodeInfo& aNodeInfo2);
  void generateBtreeNodeIndex(int aStart, int aEnd, int aFather, int aLevel, std::vector<TNodeInfo>::iterator aIter, int aDir);

  void writeSortedDataToXdb(int aStart, int aEnd, unsigned int aNodeOffset, unsigned int aFatherOffset, int aPrime, std::vector<TNodeInfo>::iterator aIter, FILE *aFileXdb);

};

#endif // XDB_GENERATOR_H
