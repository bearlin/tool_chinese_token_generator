////////////////////////////////////////////////////////////////////////////////
// This file contains the Chinese token generator config implementation
////////////////////////////////////////////////////////////////////////////////

#include "TokenGeneratorChineseConfig.h"
#include <iostream>

namespace NFtsTokenGenerator
{

CTokenGeneratorChineseConfig::CTokenGeneratorChineseConfig() :
  iInputPath("../config/default/input/"),
  iOutputPath("../config/default/output_ALL/"),
  iLogPath("../config/default/output_ALL/")
{
}

CTokenGeneratorChineseConfig::~CTokenGeneratorChineseConfig()
{
}

void CTokenGeneratorChineseConfig::SetInputPath(const std::string& aInputPath)
{
  iInputPath = aInputPath;
}

void CTokenGeneratorChineseConfig::SetOutputPath(const std::string& aOutputPath)
{
  iOutputPath = aOutputPath;
}

const std::string& CTokenGeneratorChineseConfig::GetOutputPath() const
{
  return iOutputPath;
}

void CTokenGeneratorChineseConfig::SetLogPath(const std::string& aLogPath)
{
  iLogPath = aLogPath;
}

} // namespace NFtsTokenGenerator

