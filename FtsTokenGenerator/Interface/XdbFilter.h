//
// This file contains the Chinese token filter interface
//

#ifndef __XDB_FILTER_H__
#define __XDB_FILTER_H__

#include "TokenGeneratorChineseConfig.h"

class CXdbFilter : public CTokenGeneratorChineseConfig
{
public:
  CXdbFilter();
  ~CXdbFilter();

  bool Run();

private:

  bool CollectTokens();
  bool AddExtraTokens();
  bool RemoveUnwantTokens();
  bool AddMissingOneWordTokens();
  bool RemoveSpecialSuffixTokens();
  bool RetrieveTokenInfo();
};

#endif // __XDB_FILTER_H__
