//
// This file contains the Chinese token dumper config interface
//

#ifndef __XDB_DUMPER_CONFIG_H__
#define __XDB_DUMPER_CONFIG_H__

#include "TokenGeneratorChineseConfig.h"

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

#endif // __XDB_DUMPER_CONFIG_H__
