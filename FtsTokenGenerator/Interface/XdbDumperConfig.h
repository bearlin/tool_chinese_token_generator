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

  void SetInputScwsXdb(std::string aInputScwsXdb);
  void SetInputNormalizeMap(std::string aInputNormalizeMap);
  std::string GetOutputDumpText();

  std::string iInputScwsXdb;
  std::string iInputNormalizeMap;
  std::string iOutputDumpText;

};

#endif // __XDB_DUMPER_CONFIG_H__
