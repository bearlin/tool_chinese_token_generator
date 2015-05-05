//
// This file contains the CXdbDumper implementation 
//

#include "XdbDumper.h"
#include <iostream>

CXdbDumper::CXdbDumper()
{
  std::cout << "CXdbDumper" << std::endl;
}

CXdbDumper::~CXdbDumper()
{
  std::cout << "~CXdbDumper" << std::endl;
}

bool CXdbDumper::Run()
{
  std::cout << "Running CXdbDumper" << std::endl;

  return true;
}

