////////////////////////////////////////////////////////////////////////////////
// This file contains the CXdbDumperConfig implementation
////////////////////////////////////////////////////////////////////////////////

#include "XdbDumperConfig.h"
#include <iostream>

namespace NFtsTokenGenerator
{

CXdbDumperConfig::CXdbDumperConfig()
{
}

CXdbDumperConfig::~CXdbDumperConfig()
{
}

void CXdbDumperConfig::SetInputScwsXdb(const std::string& aInputScwsXdb)
{
  iInputScwsXdb = aInputScwsXdb;
}

void CXdbDumperConfig::SetInputNormalizeMap(const std::string& aInputNormalizeMap)
{
  iInputNormalizeMap = aInputNormalizeMap;
}

const std::string CXdbDumperConfig::GetOutputDumpText() const
{
  return iOutputDumpText;
}

} // namespace NFtsTokenGenerator

