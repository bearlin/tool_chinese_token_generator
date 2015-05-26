//
// This file contains the Chinese token generator interface
//

#ifndef TOKEN_GENERATOR_CHINESE_H
#define TOKEN_GENERATOR_CHINESE_H

#include "FtsTokenGenerator.h"
#include <string>

namespace NFtsTokenGenerator
{

class CTokenGeneratorChinese
{
public:
  CTokenGeneratorChinese();
  ~CTokenGeneratorChinese();

  virtual bool Run();

private:

};

} // namespace NFtsTokenGenerator

#endif // TOKEN_GENERATOR_CHINESE_H
