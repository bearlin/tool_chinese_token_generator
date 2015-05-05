//
// This file contains the Chinese token dumper interface
//

#ifndef __XDB_DUMPER_H__
#define __XDB_DUMPER_H__

#include "TokenGeneratorChineseConfig.h"

class CXdbDumper : public CTokenGeneratorChineseConfig
{
public:
  CXdbDumper();
  ~CXdbDumper();

  bool Run();

private:

};

#endif // __XDB_DUMPER_H__
