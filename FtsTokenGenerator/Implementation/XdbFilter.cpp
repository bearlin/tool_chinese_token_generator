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
  std::cout << "iInputPath:" << iInputPath << std::endl;
  std::cout << "iOutputPath:" << iOutputPath<< std::endl;
  std::cout << "iLogPath:" << iLogPath<< std::endl;

  if (!CollectTokens())
  {
    return false;
  }
  if (!AddExtraTokens())
  {
    return false;
  }
  if (!RemoveUnwantTokens())
  {
    return false;
  }

  if (!AddMissingOneWordTokens())
  {
    return false;
  }

  if (!RemoveSpecialSuffixTokens())
  {
    return false;
  }

  if (!RetrieveTokenInfo())
  {
    return false;
  }

  return true;
}

bool CXdbFilter::CollectTokens()
{
  std::cout << "CollectTokens" << std::endl;

  return true;
}

bool CXdbFilter::AddExtraTokens()
{
  std::cout << "AddExtraTokens" << std::endl;

  return true;
}

bool CXdbFilter::RemoveUnwantTokens()
{
  std::cout << "RemoveUnwantTokens" << std::endl;

  return true;
}

bool CXdbFilter::AddMissingOneWordTokens()
{
  std::cout << "AddMissingOneWordTokens" << std::endl;

  return true;
}

bool CXdbFilter::RemoveSpecialSuffixTokens()
{
  std::cout << "RemoveSpecialSuffixTokens" << std::endl;

  return true;
}

bool CXdbFilter::RetrieveTokenInfo()
{
  std::cout << "RetrieveTokenInfo" << std::endl;

  return true;
}

