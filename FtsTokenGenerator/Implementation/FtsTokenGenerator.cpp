//
// This file contains the Implementation of FTS token generator interface
//

#include "FtsTokenGenerator.h"
#include "TokenGeneratorChinese.h"
#include "TokenGeneratorTai.h"
#include <iostream>

namespace NFtsTokenGenerator
{

CFtsTokenGenerator::CFtsTokenGenerator() :
  iLanguageType(EChinese)
{
}

CFtsTokenGenerator::~CFtsTokenGenerator()
{
}

void CFtsTokenGenerator::SetLanguageType(const TLanguageType aType)
{
  iLanguageType = aType;
}

const TLanguageType CFtsTokenGenerator::GetLanguageType() const
{
  return iLanguageType;
}
bool CFtsTokenGenerator::Run()
{
  if (EChinese == iLanguageType)
  {
    CTokenGeneratorChinese chineseTokenGenerator;
    return chineseTokenGenerator.Run();
  }
  else if (ETai == iLanguageType)
  {
    CTokenGeneratorTai taiTokenGenerator;
    return taiTokenGenerator.Run();
  }
  else
  {
    std::cout << "Unsupported language type:" << iLanguageType << std::endl;
    return false;
  }
}

} // namespace NFtsTokenGenerator

