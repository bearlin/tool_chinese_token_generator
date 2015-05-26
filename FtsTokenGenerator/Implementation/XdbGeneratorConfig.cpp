//
// This file contains the CXdbGeneratorConfig implementation 
//

#include "XdbGeneratorConfig.h"
#include <iostream>

namespace NFtsTokenGenerator
{

CXdbGeneratorConfig::CXdbGeneratorConfig() :
  iInputTokenList("")
  , iOutputXdb("")
{
}

CXdbGeneratorConfig::~CXdbGeneratorConfig()
{
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

} // namespace NFtsTokenGenerator

