////////////////////////////////////////////////////////////////////////////////
// This file contains the FTS token generator implementation
////////////////////////////////////////////////////////////////////////////////

#include "FtsTokenGenerator.h"
#include "TokenGeneratorChinese.h"
#include "TokenGeneratorThai.h"
#include <iostream>

#include "TTLog.h"
DEFINE_LOGGER(gLogCFtsTokenGenerator, "CFtsTokenGenerator")

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

TLanguageType CFtsTokenGenerator::GetLanguageType() const
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
  else if (EThai == iLanguageType)
  {
    CTokenGeneratorThai thaiTokenGenerator;
    return thaiTokenGenerator.Run();
  }
  else
  {
    LOG_INFO(gLogCFtsTokenGenerator, "Unsupported language type:%d\n", static_cast<int>(iLanguageType));
    return false;
  }
}

} // namespace NFtsTokenGenerator

