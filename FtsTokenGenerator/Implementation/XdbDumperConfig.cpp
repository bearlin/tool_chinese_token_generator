//
// This file contains the CXdbDumperConfig implementation 
//

#include "XdbDumperConfig.h"
#include <iostream>

CXdbDumperConfig::CXdbDumperConfig()
{
  std::cout << "CXdbDumperConfig" << std::endl;
}

CXdbDumperConfig::~CXdbDumperConfig()
{
  std::cout << "~CXdbDumperConfig" << std::endl;
}

void CXdbDumperConfig::SetInputScwsXdb(std::string aInputScwsXdb)
{
  iInputScwsXdb = aInputScwsXdb;
}

void CXdbDumperConfig::SetInputNormalizeMap(std::string aInputNormalizeMap)
{
  iInputNormalizeMap = aInputNormalizeMap;
}

std::string CXdbDumperConfig::GetOutputDumpText()
{
  return iOutputDumpText;
}

