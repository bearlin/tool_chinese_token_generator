//
// This file contains the Chinese token generator config interface
//

#ifndef __TOKEN_GENERATOR_CHINESE_CONFIG_H__
#define __TOKEN_GENERATOR_CHINESE_CONFIG_H__

#include <string>

class CTokenGeneratorChineseConfig
{
public:
  CTokenGeneratorChineseConfig();
  virtual ~CTokenGeneratorChineseConfig();

  virtual void SetInputPath(std::string aInputPath);
  virtual void SetOutputPath(std::string aOutputPath);
  virtual std::string GetOutputPath();
  virtual void SetLogPath(std::string aLogPath);

protected:
  std::string iInputPath;
  std::string iOutputPath;
  std::string iLogPath;

};

#endif // __TOKEN_GENERATOR_CHINESE_CONFIG_H__
