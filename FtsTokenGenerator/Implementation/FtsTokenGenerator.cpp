//
// This file contains the Implementation of FTS token generator interface
//

#include "FtsTokenGenerator.h"
#include <iostream>

CFtsTokenGenerator::CFtsTokenGenerator() :
  iLanguageType(EChinese)
{
  std::cout << "CFtsTokenGenerator: default iLanguageType=" << iLanguageType << std::endl;
}

CFtsTokenGenerator::~CFtsTokenGenerator()
{
  std::cout << "~CFtsTokenGenerator" << std::endl;
}

void CFtsTokenGenerator::SetLanguageType(TLanguageType aType)
{
  std::cout << "SetLanguageType:" << aType << std::endl;
  iLanguageType = aType;
}

TLanguageType CFtsTokenGenerator::GetLanguageType()
{
  std::cout << "GetLanguageType:" << iLanguageType << std::endl;
  return iLanguageType;
}
bool CFtsTokenGenerator::Run()
{
  std::cout << "Run with language type=" << iLanguageType << std::endl;
}

main()
{
  std::cout << "start main()" << std::endl;
  CFtsTokenGenerator tokenGenerator;
  tokenGenerator.SetLanguageType(EChinese);
  tokenGenerator.GetLanguageType();
  tokenGenerator.Run();
  std::cout << "end main()" << std::endl;
}
