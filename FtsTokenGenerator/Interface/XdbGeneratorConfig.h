//
// This file contains the Chinese token generator config interface
//

#ifndef __XDB_GENERATOR_CONFIG_H__
#define __XDB_GENERATOR_CONFIG_H__

#include "TokenGeneratorChineseConfig.h"

class CXdbGeneratorConfig : public CTokenGeneratorChineseConfig
{
public:
  CXdbGeneratorConfig();
  ~CXdbGeneratorConfig();

  void SetInputTokenList(const std::string& aInputTokenList);
  void SetOutputXdb(const std::string& aOutputXdb);
  const std::string GetOutputXdb() const;

  std::string iInputTokenList;
  std::string iOutputXdb;
};

#endif // __XDB_GENERATOR_CONFIG_H__
