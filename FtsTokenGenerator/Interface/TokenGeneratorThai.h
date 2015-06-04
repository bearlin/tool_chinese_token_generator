//
// This file contains the Thai token generator interface
//

#ifndef TOKEN_GENERATOR_THAI_H
#define TOKEN_GENERATOR_THAI_H

#include "FtsTokenGenerator.h"
#include <string>

namespace NFtsTokenGenerator
{

class CTokenGeneratorThai
{
public:
  CTokenGeneratorThai();
  // Empty virtual destructor avoids warning:
  // Class has virtual functions and accessible non-virtual destructor [-Wnon-virtual-dtor]
  virtual ~CTokenGeneratorThai();

  virtual bool Run();

private:

};

} // namespace NFtsTokenGenerator

#endif // TOKEN_GENERATOR_THAI_H
