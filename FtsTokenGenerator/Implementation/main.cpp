//
// This file contains the demo of FTS token generator interface
//

#include "FtsTokenGenerator.h"
#include <iostream>

main()
{
  std::cout << "start main()" << std::endl;
  CFtsTokenGenerator tokenGenerator;
  tokenGenerator.SetLanguageType(EChinese);
  tokenGenerator.GetLanguageType();
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
