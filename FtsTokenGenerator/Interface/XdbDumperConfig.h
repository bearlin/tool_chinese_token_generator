//
// This file contains the Chinese token dumper config interface
//

#ifndef XDB_DUMPER_CONFIG_H
#define XDB_DUMPER_CONFIG_H

#include "TokenGeneratorChineseConfig.h"

namespace NFtsTokenGenerator
{

class CXdbDumperConfig : public CTokenGeneratorChineseConfig
{
public:
  CXdbDumperConfig();
  ~CXdbDumperConfig();

  void SetInputScwsXdb(const std::string& aInputScwsXdb);
  void SetInputNormalizeMap(const std::string& aInputNormalizeMap);
  const std::string GetOutputDumpText() const;

  std::string iInputScwsXdb;
  std::string iInputNormalizeMap;
  std::string iOutputDumpText;

};

} // namespace NFtsTokenGenerator

#endif // XDB_DUMPER_CONFIG_H
