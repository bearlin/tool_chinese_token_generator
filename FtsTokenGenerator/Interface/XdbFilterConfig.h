//
// This file contains the Chinese token filter config interface
//

#ifndef __XDB_FILTER_CONFIG_H__
#define __XDB_FILTER_CONFIG_H__

#include "TokenGeneratorChineseConfig.h"

class CXdbFilterConfig : public CTokenGeneratorChineseConfig
{
public:
  CXdbFilterConfig();
  ~CXdbFilterConfig();

  void SetInputScwsXdb(std::string aInputScwsXdb);
  void SetInputScwsRule(std::string aInputScwsRule);
  void SetInputSourceData(std::string aInputSourceData);
  void SetInputNormalizeMap(std::string aInputNormalizeMap);
  void SetInputSpecialSuffixTable(std::string aInputSpecialSuffixTable);
  void SetInputAreaName(std::string aInputAreaName);
  void SetInputRemoveToken(std::string aInputRemoveToken);
  std::string GetOutputTokenList();
  std::string GetOutputTokenListNormalized();
  std::string GetOutputTokenListFuzzy();

  std::string iInputScwsXdb;
  std::string iInputScwsRule;
  std::string iInputNormalizeMap;
  std::string iInputSpecialSuffixTable;
  std::string iInputAreaName;
  std::string iInputRemoveToken;
  std::string iInputSourceData;
  std::string iOutputTokenList;
  std::string iOutputTokenListNormalized;
  std::string iOutputTokenListFuzzy;
};

#endif // __XDB_FILTER_CONFIG_H__
