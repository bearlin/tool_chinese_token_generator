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

void CXdbGeneratorConfig::SetInputTokenList(std::string aInputTokenList)
{
  iInputTokenList = aInputTokenList;
}

void CXdbGeneratorConfig::SetOutputXdb(std::string aOutputXdb)
{
  iOutputXdb = aOutputXdb;
}

std::string CXdbGeneratorConfig::GetOutputXdb()
{
  return iOutputXdb;
}

