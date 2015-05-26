//
// This file contains the demo of FTS token generator interface
//

#include "FtsTokenGenerator.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[])
{
  CFtsTokenGenerator tokenGenerator;

  // Set token generator language type
  tokenGenerator.SetLanguageType(EChinese);
  //tokenGenerator.SetLanguageType(ETai);
  TLanguageType currentLanguage = tokenGenerator.GetLanguageType();
  std::cout << "currentLanguage:" << currentLanguage << std::endl;

  // Run token generator
  if (true == tokenGenerator.Run())
  {
    std::cout << "token generating successfully!" << std::endl;
    return EXIT_SUCCESS;
  }
  else
  {
    std::cout << "token generating failed!" << std::endl;
    return EXIT_FAILURE;
  }
}
