////////////////////////////////////////////////////////////////////////////////
// This file contains the FTS token generator entry point
////////////////////////////////////////////////////////////////////////////////

#include "FtsTokenGenerator.h"
#include <iostream>
#include <cstdlib>

#include "TTLog.h"
DEFINE_LOGGER(gLogFtsTokenGeneratorMain, "FtsTokenGeneratorMain")

using ::NFtsTokenGenerator::CFtsTokenGenerator;
using ::NFtsTokenGenerator::TLanguageType;
using ::NFtsTokenGenerator::EChinese;
using ::NFtsTokenGenerator::EThai;

int main(int argc, char* argv[])
{
  CFtsTokenGenerator tokenGenerator;

  // Set token generator language type
  tokenGenerator.SetLanguageType(EChinese);
  //tokenGenerator.SetLanguageType(EThai);
  TLanguageType currentLanguage = tokenGenerator.GetLanguageType();
  LOG_INFO(gLogFtsTokenGeneratorMain, "currentLanguage:%d\n", currentLanguage);

  // Run token generator
  if (true == tokenGenerator.Run())
  {
    LOG_INFO(gLogFtsTokenGeneratorMain, "token generating successfully!\n");
    return EXIT_SUCCESS;
  }
  else
  {
    LOG_INFO(gLogFtsTokenGeneratorMain, "token generating failed!\n");
    return EXIT_FAILURE;
  }
}
