//
// This file contains the demo of FTS token generator interface
//

#include "FtsTokenGenerator.h"
#include <iostream>

int main(int argc, char* argv[])
{
  std::cout << "start main()" << std::endl;

  CFtsTokenGenerator tokenGenerator;

  // Set token generator language type
  tokenGenerator.SetLanguageType(EChinese);
  //tokenGenerator.SetLanguageType(ETai);
  tokenGenerator.GetLanguageType();

  // Run token generator
  if (true == tokenGenerator.Run())
  {
    std::cout << "token generating successfully!" << std::endl;
  }
  else
  {
    std::cout << "token generating failed!" << std::endl;
  }

  std::cout << "end main()" << std::endl;
}
