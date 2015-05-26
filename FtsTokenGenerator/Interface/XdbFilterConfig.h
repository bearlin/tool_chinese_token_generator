//
// This file contains the Chinese token filter config interface
//

#ifndef XDB_FILTER_CONFIG_H
#define XDB_FILTER_CONFIG_H

#include "TokenGeneratorChineseConfig.h"

namespace NFtsTokenGenerator
{

class CXdbFilterConfig : public CTokenGeneratorChineseConfig
{
public:
  CXdbFilterConfig();
  ~CXdbFilterConfig();

  void SetInputScwsXdb(const std::string& aInputScwsXdb);
  void SetInputScwsRule(const std::string& aInputScwsRule);
  void SetInputSourceData(const std::string& aInputSourceData);
  void SetInputNormalizeMap(const std::string& aInputNormalizeMap);
  void SetInputSpecialSuffixTable(const std::string& aInputSpecialSuffixTable);
  void SetInputAreaName(const std::string& aInputAreaName);
  void SetInputRemoveToken(const std::string& aInputRemoveToken);
  const std::string GetOutputTokenList() const;
  const std::string GetOutputTokenListNormalized() const;
  const std::string GetOutputTokenListFuzzy() const;

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

} // namespace NFtsTokenGenerator

#endif // XDB_FILTER_CONFIG_H
