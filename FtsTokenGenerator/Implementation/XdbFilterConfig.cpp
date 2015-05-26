//
// This file contains the CXdbFilterConfig implementation 
//

#include "XdbFilterConfig.h"
#include <iostream>

namespace NFtsTokenGenerator
{

CXdbFilterConfig::CXdbFilterConfig() :
  iInputScwsXdb("")
  , iInputScwsRule("")
  , iInputNormalizeMap("")
  , iInputSpecialSuffixTable("")
  , iInputAreaName("")
  , iInputRemoveToken("")
  , iInputSourceData("")
  , iOutputTokenList("")
  , iOutputTokenListNormalized("")
  , iOutputTokenListFuzzy("")
{
}

CXdbFilterConfig::~CXdbFilterConfig()
{
}

void CXdbFilterConfig::SetInputScwsXdb(const std::string& aInputScwsXdb)
{
  iInputScwsXdb = aInputScwsXdb;
}

void CXdbFilterConfig::SetInputScwsRule(const std::string& aInputScwsRule)
{
  iInputScwsRule = aInputScwsRule;
}

void CXdbFilterConfig::SetInputSourceData(const std::string& aInputSourceData)
{
  iInputSourceData = aInputSourceData;
}

void CXdbFilterConfig::SetInputNormalizeMap(const std::string& aInputNormalizeMap)
{
  iInputNormalizeMap = aInputNormalizeMap;
}

void CXdbFilterConfig::SetInputSpecialSuffixTable(const std::string& aInputSpecialSuffixTable)
{
  iInputSpecialSuffixTable = aInputSpecialSuffixTable;
}

void CXdbFilterConfig::SetInputAreaName(const std::string& aInputAreaName)
{
  iInputAreaName = aInputAreaName;
}

void CXdbFilterConfig::SetInputRemoveToken(const std::string& aInputRemoveToken)
{
  iInputRemoveToken= aInputRemoveToken;
}

const std::string CXdbFilterConfig::GetOutputTokenList() const
{
  return iOutputTokenList;
}

const std::string CXdbFilterConfig::GetOutputTokenListNormalized() const
{
  return iOutputTokenListNormalized;
}

const std::string CXdbFilterConfig::GetOutputTokenListFuzzy() const
{
  return iOutputTokenListFuzzy;
}

} // namespace NFtsTokenGenerator

