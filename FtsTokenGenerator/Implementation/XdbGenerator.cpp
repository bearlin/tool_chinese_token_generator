//
// This file contains the CXdbGenerator implementation 
//

#include "XdbGenerator.h"
#include <iostream>

CXdbGenerator::CXdbGenerator()
{
  std::cout << "CXdbGenerator" << std::endl;
}

CXdbGenerator::~CXdbGenerator()
{
  std::cout << "~CXdbGenerator" << std::endl;
}

bool CXdbGenerator::Run()
{
  std::cout << "Running CXdbGenerator" << std::endl;
  std::cout << "iInputPath:" << iInputPath << std::endl;
  std::cout << "iOutputPath:" << iOutputPath<< std::endl;
  std::cout << "iLogPath:" << iLogPath<< std::endl;

  return true;
}

