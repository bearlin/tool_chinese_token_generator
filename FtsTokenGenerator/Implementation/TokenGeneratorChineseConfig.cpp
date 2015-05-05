//
// This file contains the Chinese token generator config implementation 
//

#include "TokenGeneratorChineseConfig.h"
#include <iostream>

CTokenGeneratorChineseConfig::CTokenGeneratorChineseConfig() :
  iInputPath("../config/default/input/"),
  iOutputPath("../config/default/output_ALL/"),
  iLogPath("../config/default/output_ALL/")
{
  //std::cout << "CTokenGeneratorChineseConfig" << std::endl;
}

CTokenGeneratorChineseConfig::~CTokenGeneratorChineseConfig()
{
  //std::cout << "~CTokenGeneratorChineseConfig" << std::endl;
}

void CTokenGeneratorChineseConfig::SetInputPath(std::string aInputPath)
{
  iInputPath = aInputPath;
}

void CTokenGeneratorChineseConfig::SetOutputPath(std::string aOutputPath)
{
  iOutputPath = aOutputPath;
}

void CTokenGeneratorChineseConfig::SetLogPath(std::string aLogPath)
{
  iLogPath = aLogPath;
}


