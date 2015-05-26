//
// This file contains the Chinese token generator config interface
//

#ifndef XDB_GENERATOR_CONFIG_H
#define XDB_GENERATOR_CONFIG_H

#include "TokenGeneratorChineseConfig.h"

namespace NFtsTokenGenerator
{

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

} // namespace NFtsTokenGenerator

#endif // XDB_GENERATOR_CONFIG_H
