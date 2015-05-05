//
// This file contains the CXdbFilter implementation 
//

#include "XdbFilter.h"
#include <iostream>

CXdbFilter::CXdbFilter()
{
  std::cout << "CXdbFilter" << std::endl;
}

CXdbFilter::~CXdbFilter()
{
  std::cout << "~CXdbFilter" << std::endl;
}

bool CXdbFilter::Run()
{
  std::cout << "Running CXdbFilter" << std::endl;

  return true;
}

