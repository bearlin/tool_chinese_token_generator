//
// This file contains the Chinese token generator interface
//

#ifndef __XDB_GENERATOR_H__
#define __XDB_GENERATOR_H__

#include "TokenGeneratorChineseConfig.h"

class CXdbGenerator : public CTokenGeneratorChineseConfig
{
public:
  CXdbGenerator();
  ~CXdbGenerator();

  bool Run();

private:

};

#endif // __XDB_GENERATOR_H__
