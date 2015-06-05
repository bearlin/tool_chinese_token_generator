////////////////////////////////////////////////////////////////////////////////
// This file contains the Chinese token generator config interface
////////////////////////////////////////////////////////////////////////////////

#ifndef TOKEN_GENERATOR_CHINESE_CONFIG_H
#define TOKEN_GENERATOR_CHINESE_CONFIG_H

#include <string>

namespace NFtsTokenGenerator
{

class CTokenGeneratorChineseConfig
{
public:
  CTokenGeneratorChineseConfig();
  virtual ~CTokenGeneratorChineseConfig();

  virtual void SetInputPath(const std::string& aInputPath);
  virtual void SetOutputPath(const std::string& aOutputPath);
  virtual const std::string& GetOutputPath() const;
  virtual void SetLogPath(const std::string& aLogPath);

  std::string iInputPath;
  std::string iOutputPath;
  std::string iLogPath;
};

} // namespace NFtsTokenGenerator

#endif // TOKEN_GENERATOR_CHINESE_CONFIG_H
