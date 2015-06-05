////////////////////////////////////////////////////////////////////////////////
// This file contains the Chinese token generator interface
////////////////////////////////////////////////////////////////////////////////

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
  //! \brief Empty virtual destructor avoids warning:
  //!        Class has virtual functions and accessible non-virtual destructor [-Wnon-virtual-dtor]
  virtual ~CTokenGeneratorChinese();

  virtual bool Run();

private:

};

} // namespace NFtsTokenGenerator

#endif // TOKEN_GENERATOR_CHINESE_H
