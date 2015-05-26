//
// This file contains the interface of FTS token generator
//

#ifndef IFTS_TOKEN_GENERATOR_H
#define IFTS_TOKEN_GENERATOR_H

#include <string>

namespace NFtsTokenGenerator
{

enum TLanguageType
{
  EChinese  = 0,
  ETai      = 1,
  EMax      = ETai
};

class IFtsTokenGenerator
{
public:
  virtual void SetLanguageType(const TLanguageType aType) = 0;
  virtual const TLanguageType GetLanguageType() const = 0;
  virtual bool Run() = 0;
};

} // namespace NFtsTokenGenerator

#endif // IFTS_TOKEN_GENERATOR_H
