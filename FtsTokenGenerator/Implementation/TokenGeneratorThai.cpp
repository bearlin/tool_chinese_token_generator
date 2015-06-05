////////////////////////////////////////////////////////////////////////////////
// This file contains the Thai token generator implementation
////////////////////////////////////////////////////////////////////////////////

#include "TokenGeneratorThai.h"
#include <iostream>

#include "TTLog.h"
DEFINE_LOGGER(gLogCTokenGeneratorThai, "CTokenGeneratorThai")

namespace NFtsTokenGenerator
{

CTokenGeneratorThai::CTokenGeneratorThai()
{
}

CTokenGeneratorThai::~CTokenGeneratorThai()
{
}

bool CTokenGeneratorThai::Run()
{
  // Run Thai tokenizer engine...
  // TODO.
  LOG_INFO(gLogCTokenGeneratorThai, "Running CTokenGeneratorThai... (For Thai language, we still don't know if it need a token generating process or not)\n");

  return true;
}

} // namespace NFtsTokenGenerator

