//
// This file contains the Chinese token generator interface
//
#ifndef XDB_GENERATOR_H
#define XDB_GENERATOR_H

#include "XdbGeneratorConfig.h"
#include "TokenGeneratorChineseCommon.h"

#include <vector>
#include <cstdio>

namespace NFtsTokenGenerator
{

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
} TXdb_header;

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
  FILE* iInputTokenListFile;
  FILE* iXdbFile;
#ifdef ENABLE_LOG
  FILE* iLogFile;
  FILE* iLogRepeatPathFile;
#endif //ENABLE_LOG
  int GetHashIndex(const unsigned char* aKey, int aHashBase, int aHashPrime) const;
  void GenerateBtreeNodeIndex(int aStart, int aEnd, int aFather, int aLevel, std::vector<TNodeInfo>::iterator aIter, int aDir);
  void WriteSortedDataToXdb(int aStart, int aEnd, unsigned int aNodeOffset, unsigned int aFatherOffset, int aPrime, std::vector<TNodeInfo>::iterator aIter, FILE *aFileXdb);
  bool OpenFile();
  void CloseFile();
  static bool CompareNode(const TNodeInfo& aNodeInfo1, const TNodeInfo& aNodeInfo2);
};

} // namespace NFtsTokenGenerator

#endif // XDB_GENERATOR_H
