//
// This file contains the Implementation of FTS token generator interface
//

#ifndef __FTS_TOKEN_GENERATOR_H__
#define __FTS_TOKEN_GENERATOR_H__

#include "IFtsTokenGenerator.h"
#include <string>

class CFtsTokenGenerator : public IFtsTokenGenerator
{
public:
   CFtsTokenGenerator();
  ~CFtsTokenGenerator();

  void SetLanguageType(TLanguageType aType);
  TLanguageType GetLanguageType();
  bool Run();

private:
  TLanguageType iLanguageType;
};

#endif // __FTS_TOKEN_GENERATOR_H__
