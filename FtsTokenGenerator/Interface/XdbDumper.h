//
// This file contains the Chinese token dumper interface
//

#ifndef XDB_DUMPER_H
#define XDB_DUMPER_H

#include "XdbDumperConfig.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>     /* atoi */

#include <string>
#include <vector>
#include <map>
#include <algorithm>    // std::find std::sort

namespace NFtsTokenGenerator
{

static const int KSzLogSize = 1024;

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

} // namespace NFtsTokenGenerator

#endif // XDB_DUMPER_H
