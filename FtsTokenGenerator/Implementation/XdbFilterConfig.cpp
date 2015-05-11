//
// This file contains the CXdbFilterConfig implementation 
//

#include "XdbFilterConfig.h"
#include <iostream>

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
  std::cout << "CXdbFilterConfig" << std::endl;
}

CXdbFilterConfig::~CXdbFilterConfig()
{
  std::cout << "~CXdbFilterConfig" << std::endl;
}

void CXdbFilterConfig::SetInputScwsXdb(std::string aInputScwsXdb)
{
  iInputScwsXdb = aInputScwsXdb;
}

void CXdbFilterConfig::SetInputScwsRule(std::string aInputScwsRule)
{
  iInputScwsRule = aInputScwsRule;
}

void CXdbFilterConfig::SetInputSourceData(std::string aInputSourceData)
{
  iInputSourceData = aInputSourceData;
}

void CXdbFilterConfig::SetInputNormalizeMap(std::string aInputNormalizeMap)
{
  iInputNormalizeMap = aInputNormalizeMap;
}

void CXdbFilterConfig::SetInputSpecialSuffixTable(std::string aInputSpecialSuffixTable)
{
  iInputSpecialSuffixTable = aInputSpecialSuffixTable;
}

void CXdbFilterConfig::SetInputAreaName(std::string aInputAreaName)
{
  iInputAreaName = aInputAreaName;
}

void CXdbFilterConfig::SetInputRemoveToken(std::string aInputRemoveToken)
{
  iInputRemoveToken= aInputRemoveToken;
}

std::string CXdbFilterConfig::GetOutputTokenList()
{
  return iOutputTokenList;
}

std::string CXdbFilterConfig::GetOutputTokenListNormalized()
{
  return iOutputTokenListNormalized;
}

std::string CXdbFilterConfig::GetOutputTokenListFuzzy()
{
  return iOutputTokenListFuzzy;
}

