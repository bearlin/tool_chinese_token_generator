//
// This file contains the CXdbGeneratorConfig implementation 
//

#include "XdbGeneratorConfig.h"
#include <iostream>

CXdbGeneratorConfig::CXdbGeneratorConfig() :
  iInputTokenList("")
  , iOutputXdb("")
{
  std::cout << "CXdbGeneratorConfig" << std::endl;
}

CXdbGeneratorConfig::~CXdbGeneratorConfig()
{
  std::cout << "~CXdbGeneratorConfig" << std::endl;
}

void CXdbGeneratorConfig::SetInputTokenList(const std::string& aInputTokenList)
{
  iInputTokenList = aInputTokenList;
}

void CXdbGeneratorConfig::SetOutputXdb(const std::string& aOutputXdb)
{
  iOutputXdb = aOutputXdb;
}

const std::string CXdbGeneratorConfig::GetOutputXdb() const
{
  return iOutputXdb;
}

