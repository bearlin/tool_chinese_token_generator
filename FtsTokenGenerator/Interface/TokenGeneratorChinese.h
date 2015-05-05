//
// This file contains the Chinese token generator interface
//

#ifndef __TOKEN_GENERATOR_CHINESE_H__
#define __TOKEN_GENERATOR_CHINESE_H__

#include "FtsTokenGenerator.h"
#include <string>

class CTokenGeneratorChinese : public CFtsTokenGenerator
{
public:
  CTokenGeneratorChinese();
  ~CTokenGeneratorChinese();

  virtual bool Run();

  void SetInputFilePath(std::string aInputPath);
  void SetOutputFilePath(std::string aOutputPath);

private:
  std::string iInputPath;
  std::string iOutputPath;

};

#endif // __TOKEN_GENERATOR_CHINESE_H__
