////////////////////////////////////////////////////////////////////////////////
// This file contains the FTS token generator interface
////////////////////////////////////////////////////////////////////////////////

#ifndef FTS_TOKEN_GENERATOR_H
#define FTS_TOKEN_GENERATOR_H

#include "IFtsTokenGenerator.h"
#include <string>

namespace NFtsTokenGenerator
{

class CFtsTokenGenerator : public IFtsTokenGenerator
{
public:
  CFtsTokenGenerator();
  virtual ~CFtsTokenGenerator();

  void SetLanguageType(const TLanguageType aType);
  TLanguageType GetLanguageType() const;
  virtual bool Run();

private:
  TLanguageType iLanguageType;
};

} // namespace NFtsTokenGenerator

#endif // FTS_TOKEN_GENERATOR_H
