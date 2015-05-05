//
// This file contains the Chinese token generator interface
//

#ifndef __TOKEN_GENERATOR_CHINESE_H__
#define __TOKEN_GENERATOR_CHINESE_H__

#include "FtsTokenGenerator.h"
#include <string>

class CTokenGeneratorChinese
{
public:
  CTokenGeneratorChinese();
  ~CTokenGeneratorChinese();

  virtual bool Run();

private:

};

#endif // __TOKEN_GENERATOR_CHINESE_H__
