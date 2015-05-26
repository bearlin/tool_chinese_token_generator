//
// This file contains the Tai token generator interface
//

#ifndef TOKEN_GENERATOR_TAI_H
#define TOKEN_GENERATOR_TAI_H

#include "FtsTokenGenerator.h"
#include <string>

namespace NFtsTokenGenerator
{

class CTokenGeneratorTai
{
public:
  CTokenGeneratorTai();
  ~CTokenGeneratorTai();

  virtual bool Run();

private:

};

} // namespace NFtsTokenGenerator

#endif // TOKEN_GENERATOR_TAI_H
