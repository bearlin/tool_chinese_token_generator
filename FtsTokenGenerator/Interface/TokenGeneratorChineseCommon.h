//
// This file contains the Chinese token generator common interface
//

#ifndef __TOKEN_GENERATOR_CHINESE_COMMON_H__
#define __TOKEN_GENERATOR_CHINESE_COMMON_H__

#include <string>

class CTokenGeneratorChineseCommon
{
public:
  CTokenGeneratorChineseCommon();
  virtual ~CTokenGeneratorChineseCommon();

  static const int KMblenTableUTF8Size = 256;
  // Length of multibyte character from first byte of Utf8
  static const unsigned char iUTF8MultibyteLengthTable[KMblenTableUTF8Size];
};

#endif // __TOKEN_GENERATOR_CHINESE_COMMON_H__
