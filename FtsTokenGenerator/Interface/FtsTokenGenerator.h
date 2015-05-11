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
  virtual ~CFtsTokenGenerator();

  void SetLanguageType(const TLanguageType aType);
  const TLanguageType GetLanguageType() const;
  virtual bool Run();

private:
  TLanguageType iLanguageType;
};

#endif // __FTS_TOKEN_GENERATOR_H__
