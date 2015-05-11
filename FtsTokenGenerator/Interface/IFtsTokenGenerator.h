//
// This file contains the interface of FTS token generator
//

#ifndef __IFTS_TOKEN_GENERATOR_H__
#define __IFTS_TOKEN_GENERATOR_H__

#include <string>

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

#endif // __IFTS_TOKEN_GENERATOR_H__
