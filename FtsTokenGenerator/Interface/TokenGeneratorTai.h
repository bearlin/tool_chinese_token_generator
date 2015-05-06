//
// This file contains the Tai token generator interface
//

#ifndef __TOKEN_GENERATOR_TAI_H__
#define __TOKEN_GENERATOR_TAI_H__

#include "FtsTokenGenerator.h"
#include <string>

class CTokenGeneratorTai
{
public:
  CTokenGeneratorTai();
  ~CTokenGeneratorTai();

  virtual bool Run();

private:

};

#endif // __TOKEN_GENERATOR_TAI_H__
