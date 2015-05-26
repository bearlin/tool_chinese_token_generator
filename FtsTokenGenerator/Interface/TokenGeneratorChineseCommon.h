//
// This file contains the Chinese token generator common interface
//

#ifndef TOKEN_GENERATOR_CHINESE_COMMON_H
#define TOKEN_GENERATOR_CHINESE_COMMON_H

#include <string>

namespace NFtsTokenGenerator
{

class CTokenGeneratorChineseCommon
{
public:
  CTokenGeneratorChineseCommon();
  virtual ~CTokenGeneratorChineseCommon();

  static const int KMblenTableUTF8Size = 256;
  // Length of multibyte character from first byte of Utf8
  static const unsigned char iUTF8MultibyteLengthTable[KMblenTableUTF8Size];
};

} // namespace NFtsTokenGenerator

#endif // TOKEN_GENERATOR_CHINESE_COMMON_H
