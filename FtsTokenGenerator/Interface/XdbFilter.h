//
// This file contains the Chinese token filter interface
//

#ifndef XDB_FILTER_H
#define XDB_FILTER_H

#include "XdbFilterConfig.h"
#include "TokenGeneratorChineseCommon.h"

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

namespace NFtsTokenGenerator
{

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
  #define SEARCH_TOKEN_INFO_FOUND (0)
  #define SEARCH_TOKEN_INFO_NOT_FOUND (-1)

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

  int GetHashIndex(const unsigned char*  aKey, int aHashBase, int aHashPrime) const;

  int SearchTokenInfo(const char*     aTokenContent,
                      long            aNodeOffset,
                      long            aNodeLength,
                      int             aHashBase,
                      int             aHashPrime,
                      TNodeInfoAttr*  aAttribute,
                      FILE*           aFileXdb);
  ////////////////////// Search Node /////////////////////////////////////

  ////////////////////// UTF8 Handling /////////////////////////////////////
  #define UTF8_CHAR_KIND_NONE (0)
  #define UTF8_CHAR_KIND_1 (1)  // ASCII(0x00~0x7F, 1 BYTE in UTF8)
  #define UTF8_CHAR_KIND_2 (2)  // Latin Characters(First byte range:0xC0-0xDF, 2 BYTE in UTF8)
  #define UTF8_CHAR_KIND_3 (3)  // CJKS  Characters(First byte range:0xE0-0xEF, 3 BYTE in UTF8)
  #define UTF8_CHAR_KIND_4 (4)  // UTF-32 Characters(First byte range:0xF0-0xF7, 4 BYTE in UTF8)

  int GetUTF8CharKind(unsigned char aChar);
  int ConvUTF8ToUTF16(unsigned char* aUtf8CodeData, unsigned int* aData, int aDataLength);
  int IsAllChineseToken(const char* aTokenContent, unsigned int aTokenLength);
  int IsValidChineseToken(scws_res_t aScwsCur, const char* aTokenContent);
  int GetTokenTotalChineseWordCount(const char* aString);
  int GetNthChineseWordByteOffset(const char* aString, int aNthChineseWord);
  ////////////////////// UTF8 Handling /////////////////////////////////////

  ////////////////////// SuffixTokenMap /////////////////////////////////////
  bool IsBufferStartWithBOM(char* aBuffer);
  int InitSuffixTokenMap(std::map<std::string,int>& aSuffixTokenMap);
  int GetMaxSuffixTokenLength(std::map<std::string,int>& aSuffixTokenMap);
  ////////////////////// SuffixTokenMap /////////////////////////////////////

  int IsTokenEndWithIgnoredSuffix(const char*                aString,
                                  int*                       aSuffixOffset,
                                  char*                      aTempBuffer,
                                  int                        aMaxSuffixTokenLength,
                                  std::map<std::string,int>& aSuffixTokenMap);

  int GetOneWordInToken(const char* aString,
                        char*       aTempBuffer,
                        size_t      aBufferLength,
                        int         aWordIndex);

  const unsigned int KNormBufUnitSize;
  const unsigned int KCJKBytes;
  const unsigned int ASCIIBytes;
  std::map<std::string,std::string> iNormalizerMap;

  int CHomophoneNormalizer_Init(const char* aFile);
  size_t CHomophoneNormalizer_Normalize(const char* aSourceString, char* aOutputString, size_t aLength);
  void CFtsTokenizerExtChinese_ReserveStringCapacity(std::string& aString, size_t aSize, size_t aUnitSize);

  std::string iFilePath;
  FILE* iFileXdb;
  xdb_header iXdbHeader;
  TNodeInfoAttr iNodeInfoAttr;

  char iTempBuffer[MAX_LINE_SIZE];

  std::string iTokenItem;

  int iTokenIndex;
  std::map<std::string,int> iTokenMap, iTokenMapWithOneWord, iTokenMapOpt;
  std::map<std::string,int> iSuffixTokenMap;
  std::map<std::string,int> iAllAreaMap;

  std::ofstream iOutputFile;
  std::ifstream iInputFile;
  std::string iTempLine;

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

} // namespace NFtsTokenGenerator

#endif // XDB_FILTER_H_
