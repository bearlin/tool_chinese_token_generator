//
// This file contains the CXdbFilter implementation
//

#include "XdbFilter.h"

namespace NFtsTokenGenerator
{

#if defined(WIN32)
  #define strcasecmp _stricmp
#endif

CXdbFilter::CXdbFilter() :
  KNormBufUnitSize(0x200),
  KCJKBytes(3),
  ASCIIBytes(1),
  iTokenIndex(0)
{
  GetConfig().iOutputTokenList = OUT_FILE_S06_SUFFIX_FULL;
  GetConfig().iOutputTokenListNormalized = OUT_FILE_S07_SUFFIX_FULL_NOR;
  GetConfig().iOutputTokenListFuzzy = OUT_FILE_S08_SUFFIX_FULL_FUZZY;
}

CXdbFilter::~CXdbFilter()
{
}

bool CXdbFilter::Run()
{
  std::cout << "Running CXdbFilter" << std::endl;
  std::cout << "iInputPath:" << GetConfig().iInputPath << std::endl;
  std::cout << "iOutputPath:" << GetConfig().iOutputPath << std::endl;
  std::cout << "iLogPath:" << GetConfig().iLogPath << std::endl;

  if (!Init() || 
      !CollectTokens() || 
      !AddExtraTokens() || 
      !RemoveUnwantTokens() || 
      !AddMissingOneWordTokens() || 
      !RemoveSpecialSuffixTokens() || 
      !RetrieveTokenInfo() || 
      !ConvertToNormalizedTokens() || 
      !MergeToFuzzyTokens())
  {
    return false;
  }

  return true;
}

CXdbFilterConfig& CXdbFilter::GetConfig()
{
  return iConfig;
}

bool CXdbFilter::Init()
{
  // Stage 0: Initializations.
  //------------------------------------------------------------------------------------------------
  printf("Stage 0: Initializations.\n");
  if (!InitSuffixTokenMap(iSuffixTokenMap))
  {
    printf("ERROR: cann't init the SuffixTokenMap!\n");
    return false;
  }

  // Log iSuffixTokenMap.
  iFilePath = GetConfig().iLogPath + OUT_FILE_S00_SUFFIX_TOK_MAP;
  iOutputFile.open(iFilePath.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = iSuffixTokenMap.begin(); it != iSuffixTokenMap.end(); ++it)
  {
    iOutputFile << it->first << "|" << it->second << '\n';
  }
  iOutputFile.close();
  //------------------------------------------------------------------------------------------------

  return true;
}

bool CXdbFilter::CollectTokens()
{
  // Stage 1: Collect token list from "input raw file" or "saved iTokenMap log file" to iTokenMap.
  //------------------------------------------------------------------------------------------------
#ifdef MAP_INIT
  printf("Stage 1: Collect token list from \"input raw file\" to iTokenMap.\n");
  char tempToken[MAX_LINE_SIZE];
  scws_t scwsHandle;
  scws_res_t scwsResults, scwsCurrentResult;
  int status;

  char lineTextToScws[MAX_LINE_SIZE];
  char lineText[MAX_LINE_SIZE];
  int textSize;
  int lineNumber = 0;

  FILE* fileStage01Raw;
  FILE* fileStage01NotChinese;
  FILE* fileStage01Duplicated;

  iFilePath = GetConfig().iLogPath + OUT_FILE_S01_NOT_CH;
  fileStage01NotChinese = fopen(iFilePath.c_str(), "w");
  if (fileStage01NotChinese == NULL)
  {
    printf("fileStage01NotChinese err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fileStage01NotChinese:%s\n", iFilePath.c_str());

  iFilePath = GetConfig().iLogPath + OUT_FILE_S01_DUPLICATE;
  fileStage01Duplicated = fopen(iFilePath.c_str(), "w");
  if (fileStage01Duplicated == NULL)
  {
    printf("fileStage01Duplicated err:%s\n", iFilePath.c_str());
    fclose(fileStage01NotChinese);
    return false;
  }
  printf("fileStage01Duplicated:%s\n", iFilePath.c_str());

  scwsHandle = scws_new();
  if (scwsHandle == false)
  {
    printf("ERROR: cann't init the scwsHandle!\n");
    fclose(fileStage01NotChinese);
    fclose(fileStage01Duplicated);
    return false;
  }

  scws_set_charset(scwsHandle, "utf8");

  iFilePath = GetConfig().iInputPath + GetConfig().iInputScwsXdb;
  status = scws_set_dict(scwsHandle, iFilePath.c_str(), SCWS_XDICT_XDB);
  if (status != 0)
  {
    printf("Err: Failed to set the dict file:%s\n", iFilePath.c_str());
    fclose(fileStage01NotChinese);
    fclose(fileStage01Duplicated);
    return false;
  }

  iFilePath = GetConfig().iInputPath + GetConfig().iInputScwsRule;
  scws_set_rule(scwsHandle, iFilePath.c_str());

  iFilePath = GetConfig().iInputPath + GetConfig().iInputSourceData;
  printf("input src:%s\n", iFilePath.c_str());
  fileStage01Raw = fopen(iFilePath.c_str(), "r");
  if (fileStage01Raw == NULL)
  {
    printf("fileStage01Raw err:%s\n", iFilePath.c_str());
    fclose(fileStage01NotChinese);
    fclose(fileStage01Duplicated);
    return false;
  }
  printf("fileStage01Raw:%s\n", iFilePath.c_str());

  while (fgets(lineText, MAX_LINE_SIZE, fileStage01Raw) != NULL)
  {
    lineNumber++;
    if ((lineNumber % 100000) == 0)
    {
      printf("Parsing Line(%d)....\n", lineNumber);
    }

    if (strlen(lineText) == 0)
    {
      continue;
    }

    textSize = 0;
    memset(lineTextToScws, 0, sizeof(lineTextToScws));

    char* ptrLine = lineText;
    while (('\r' != *ptrLine) && ('\n' != *ptrLine) && (0 != *ptrLine))
    {
      lineTextToScws[textSize++] = *(ptrLine++);
    }

    //printf("%s\n", lineTextToScws);
    scws_send_text(scwsHandle, lineTextToScws, textSize);
    while ((scwsResults = (scwsCurrentResult = scws_get_result(scwsHandle))))
    {
      while (scwsCurrentResult != NULL)
      {
        memset(tempToken, 0, sizeof(tempToken));
        memcpy(tempToken, &lineTextToScws[scwsCurrentResult->off], scwsCurrentResult->len);

        if (IsValidChineseToken(scwsCurrentResult, tempToken))
        {
          iTokenItem.clear();
          iTokenItem.append(tempToken);

          std::map<std::string,int>::iterator it = iTokenMap.find(iTokenItem);

          if (it != iTokenMap.end())
          {
            // found ignore the duplicate items
            fprintf(fileStage01Duplicated, "%s off=%d idf=%4.2f len=%d attr=%s\n", tempToken, scwsCurrentResult->off, scwsCurrentResult->idf, scwsCurrentResult->len, scwsCurrentResult->attr);
          }
          else
          {
            iTokenMap.insert(std::pair<std::string,int>(iTokenItem, ++iTokenIndex));
          }
        }
        else
        {
          fprintf(fileStage01NotChinese, "%s off=%d idf=%4.2f len=%d attr=%s\n", tempToken, scwsCurrentResult->off, scwsCurrentResult->idf, scwsCurrentResult->len, scwsCurrentResult->attr);
        }

        scwsCurrentResult = scwsCurrentResult->next;
      }

      scws_free_result(scwsResults);
    }
  }
  scws_free(scwsHandle);
  printf("Total Parsing Line(%d)....\n", lineNumber);

  // Log iTokenMap
  iFilePath = GetConfig().iLogPath + OUT_FILE_S01_MAP_RAW;
  iOutputFile.open(iFilePath.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = iTokenMap.begin(); it != iTokenMap.end(); ++it)
  {
    iOutputFile << it->first << "|" << it->second << '\n';
  }
  iOutputFile.close();
  fclose(fileStage01Raw);
  fclose(fileStage01NotChinese);
  fclose(fileStage01Duplicated);

#else //MAP_INIT

  printf("Stage 1: Collect token list from \"saved iTokenMap log file\" to iTokenMap.\n");

  std::string string1;
  std::string string2;
  std::size_t position = 0;
  int tempIndex = 0;
  int maxIndex = 0;

  // Re-load token list from iTokenMap log file.
  iFilePath = GetConfig().iLogPath + OUT_FILE_S01_MAP_RAW;
  iInputFile.open(iFilePath.c_str(), std::ifstream::in);
  if (!iInputFile.is_open())
  {
    // show message:
    std::cout << "Error opening file: " << iFilePath << std::endl;
    return false;
  }
  printf("Re-load token list from iTokenMap log file:%s\n", iFilePath.c_str());

  iTokenMap.clear();
  while (std::getline(iInputFile, iTempLine).good())
  {
    position = iTempLine.find_first_of("|");
    string1 = iTempLine.substr(0, position);
    string2 = iTempLine.substr(position + 1, std::string::npos);
    tempIndex = atoi(string2.c_str());
    //std::cout << "iTempLine:" << iTempLine << " string1:" << string1 << " string2:" << string2 << " string2.c_str():" << string2.c_str() << std::endl;

    iTokenMap.insert(std::pair<std::string,int>(string1, tempIndex));

    if (tempIndex > maxIndex)
    {
      maxIndex = tempIndex;
    }
  }

  iTokenIndex = maxIndex;
  iInputFile.close();

  // Log reloaded iTokenMap.
  iFilePath = GetConfig().iLogPath + OUT_FILE_S01_MAP_RELOAD;
  printf("Re-load token list:%s\n", iFilePath.c_str());
  iOutputFile.open(iFilePath.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = iTokenMap.begin(); it != iTokenMap.end(); ++it)
  {
    iOutputFile << it->first << "|" << it->second << '\n';
  }
  iOutputFile.close();
  //return true;
#endif //MAP_INIT
  //------------------------------------------------------------------------------------------------

  return true;
}

bool CXdbFilter::AddExtraTokens()
{
  // Stage 2: Add "additional tokens" from _ignored_suffix_table_{tc|sc}_utf8.txt to iTokenMap.
  //------------------------------------------------------------------------------------------------
  printf("Stage 2: Add \"additional tokens\" from _ignored_suffix_table_{tc|sc}_utf8.txt to iTokenMap.\n");

  std::string string1;
  std::string string2;
  std::size_t position = 0;
  int tempIndex = 0;

  for (std::map<std::string,int>::iterator it = iSuffixTokenMap.begin(); it != iSuffixTokenMap.end(); ++it)
  {
    if (iTokenMap.end() == iTokenMap.find(it->first))
    {
      // Word not found, add to map.
      iTokenMap.insert(std::pair<std::string,int>(it->first, ++iTokenIndex));
      printf("Not Found [%s] added as : %s|%d\n", it->first.c_str(), it->first.c_str(), iTokenIndex);
    }
  }

  // Log  iTokenMap with missing tokens in table _ignored_suffix_table_{tc|sc}_utf8.txt.
  iFilePath = GetConfig().iLogPath + OUT_FILE_S02_MAP_SUFFIX;
  iOutputFile.open(iFilePath.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = iTokenMap.begin(); it != iTokenMap.end(); ++it)
  {
    iOutputFile << it->first << "|" << it->second << '\n';
  }
  iOutputFile.close();
  //------------------------------------------------------------------------------------------------

// Stage 2.1: Add "AreaName tokens" from {TC|SC}\input\areas\05_all_area_map02.txt to iTokenMap.
  //------------------------------------------------------------------------------------------------
  printf("Stage 2.1: Add \"AreaName tokens\" from {TC|SC}\\input\\areas\\05_all_area_map02.txt to iTokenMap.\n");

  // Load area name token list from file to iAllAreaMap.
  iFilePath = GetConfig().iInputPath + GetConfig().iInputAreaName;
  iInputFile.open(iFilePath.c_str(), std::ifstream::in);
  if (!iInputFile.is_open())
  {
    // show message:
    std::cout << "Error opening file: " << iFilePath << std::endl;
    return false;
  }

  iAllAreaMap.clear();
  while (std::getline(iInputFile, iTempLine).good())
  {
    position = iTempLine.find_first_of(",");
    string1 = iTempLine.substr(0, position);
    string2 = iTempLine.substr(position + 1, std::string::npos);
    tempIndex = atoi(string2.c_str());

    iAllAreaMap.insert(std::pair<std::string,int>(string1, tempIndex));
  }

  iInputFile.close();

  // Log iAllAreaMap.
  iFilePath = GetConfig().iLogPath + OUT_FILE_S02_LOG_AREA_NAME_FILE;
  iOutputFile.open(iFilePath.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = iAllAreaMap.begin(); it != iAllAreaMap.end(); ++it)
  {
    iOutputFile << it->first << "," << it->second << '\n';
  }
  iOutputFile.close();

  // Save iAllAreaMap to iTokenMap.
  for (std::map<std::string,int>::iterator it = iAllAreaMap.begin(); it != iAllAreaMap.end(); ++it)
  {
    //iOutputFile << it->first << "|" << it->second << '\n';
    if (iTokenMap.end() == iTokenMap.find(it->first))
    {
      // Word not found, add to map.
      iTokenMap.insert(std::pair<std::string,int>(it->first, ++iTokenIndex));
      printf("Not Found [%s] added as : %s|%d\n", it->first.c_str(), it->first.c_str(), iTokenIndex);
    }
  }

  // Log  iTokenMap with missing tokens in {TC|SC}\input\areas\05_all_area_map02.txt.
  iFilePath = GetConfig().iLogPath + OUT_FILE_S02_LOG_MAP_PLUS_AREA_NAME;
  iOutputFile.open(iFilePath.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = iTokenMap.begin(); it != iTokenMap.end(); ++it)
  {
    iOutputFile << it->first << "|" << it->second << '\n';
  }
  iOutputFile.close();
  //------------------------------------------------------------------------------------------------

  return true;
}

bool CXdbFilter::RemoveUnwantTokens()
{
  // Stage 2.2: Remove "unwant tokens" from {TC|SC}\input\_removed_tokens_table.txt from iTokenMap.
  //------------------------------------------------------------------------------------------------
  printf("Stage 2.2: Remove \"unwant tokens\" from {TC|SC}\\input\\_removed_tokens_table.txt from iTokenMap.\n");

  std::map<std::string,int> unwantTokensMap;
  std::string string1;
  std::size_t position = 0;

  // Load unwant token list from file to unwantTokensMap.
  iFilePath = GetConfig().iInputPath + GetConfig().iInputRemoveToken;
  iInputFile.open(iFilePath.c_str(), std::ifstream::in);
  if (!iInputFile.is_open())
  {
    // show message:
    std::cout << "Error opening file: " << iFilePath << std::endl;
    return false;
  }

  unwantTokensMap.clear();
  while (std::getline(iInputFile, iTempLine).good())
  {
    if (iTempLine.length() == 0)
    {
      continue;
    }

    if ((iTempLine[0] == '#') || (iTempLine[0] == ';'))
    {
      continue;
    }

    position = iTempLine.find_first_of(" \n\r\0");
    string1 = iTempLine.substr(0, position);
    std::cout << "iTempLine:" << iTempLine << " string1:" << string1 << std::endl;

    unwantTokensMap.insert(std::pair<std::string,int>(string1, 1));
  }

  iInputFile.close();

  // Log unwantTokensMap.
  iFilePath = GetConfig().iLogPath + OUT_FILE_S02_LOG_REMOVE_TOK_FILE;
  iOutputFile.open(iFilePath.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = unwantTokensMap.begin(); it != unwantTokensMap.end(); ++it)
  {
    iOutputFile << it->first << "," << it->second << '\n';
  }
  iOutputFile.close();

  // Remove unwantTokensMap from iTokenMap.
  for (std::map<std::string,int>::iterator it = unwantTokensMap.begin(); it != unwantTokensMap.end(); ++it)
  {
    if (iTokenMap.end() != iTokenMap.find(it->first))
    {
      // Word found, remove from map.
      iTokenMap.erase(it->first); // erasing by key
    }
  }

  // Log iTokenMap with missing tokens in {TC|SC}\input\areas\05_unwant_tokens_map02.txt.
  iFilePath = GetConfig().iLogPath + OUT_FILE_S02_LOG_MAP_AFTER_REMOVE_TOK;
  iOutputFile.open(iFilePath.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = iTokenMap.begin(); it != iTokenMap.end(); ++it)
  {
    iOutputFile << it->first << "|" << it->second << '\n';
  }
  iOutputFile.close();
  //------------------------------------------------------------------------------------------------

  return true;
}

bool CXdbFilter::AddMissingOneWordTokens()
{
  // Stage 3: Add "missing one word tokens" from iTokenMap into iTokenMap_with_one_word.
  //------------------------------------------------------------------------------------------------
  printf("Stage 3: Add \"missing one word tokens\" from iTokenMap into iTokenMap_with_one_word.\n");

  // Copy iTokenMap to iTokenMap_with_one_word.
  for (std::map<std::string,int>::iterator it = iTokenMap.begin(); it != iTokenMap.end(); ++it)
  {
    iTokenMapWithOneWord.insert(std::pair<std::string,int>(it->first, it->second));
  }

  // Add one word tokens from original tokens.
  char tempWord[MAX_UTF8_CHARACTER_LENGTH];
  int wordIndex;
  iFilePath = GetConfig().iLogPath + OUT_FILE_S03_CH_ONE_WORD;
  iOutputFile.open(iFilePath.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = iTokenMap.begin(); it != iTokenMap.end(); ++it)
  {
    // break token into words and add them.
    strcpy(iTempBuffer, it->first.c_str());
    strtok(iTempBuffer,"\n\r");
    wordIndex = 0;
    while (GetOneWordInToken(iTempBuffer, tempWord, sizeof(tempWord), wordIndex++))
    {
      if (iTokenMapWithOneWord.end() == iTokenMapWithOneWord.find(tempWord))
      {
        // Word not found, add to map.
        iTokenMapWithOneWord.insert(std::pair<std::string,int>(tempWord, ++iTokenIndex));
        //printf("Added One Word: %s|%d\n", tempWord, iTokenIndex);
        iOutputFile << tempWord << "|" << iTokenIndex << std::endl;
      }
    }
  }
  iOutputFile.close();

  // Log iTokenMapWithOneWord.
  iFilePath = GetConfig().iLogPath + OUT_FILE_S03_MAP_WITH_ONE_WORDS;
  iOutputFile.open(iFilePath.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = iTokenMapWithOneWord.begin(); it != iTokenMapWithOneWord.end(); ++it)
  {
    iOutputFile << it->first << "|" << it->second << '\n';
  }
  iOutputFile.close();
  //------------------------------------------------------------------------------------------------

  // Stage 4: [JUST FOR LOG] Retreive every map item's info from xdb, then log info to files.
  //------------------------------------------------------------------------------------------------
  printf("Stage 4: [JUST FOR LOG] Retreive every map item's info from xdb, then log info to files.\n");

  FILE* fileStage04FullChineseToken;
  FILE* fileStage04PartChineseToken;
  FILE* fileStage04ChineseTokenNotFound;

  size_t readSize = 0;
  
  iFilePath = GetConfig().iLogPath + OUT_FILE_S04_CH_FULL;
  fileStage04FullChineseToken = fopen(iFilePath.c_str(), "w");
  if (fileStage04FullChineseToken == NULL)
  {
    printf("fileStage04FullChineseToken err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fileStage04FullChineseToken:%s\n", iFilePath.c_str());

  iFilePath = GetConfig().iLogPath + OUT_FILE_S04_CH_PART;
  fileStage04PartChineseToken = fopen(iFilePath.c_str(), "w");
  if (fileStage04PartChineseToken == NULL)
  {
    printf("fileStage04PartChineseToken err:%s\n", iFilePath.c_str());
    fclose(fileStage04FullChineseToken);
    return false;
  }
  printf("fileStage04PartChineseToken:%s\n", iFilePath.c_str());

  iFilePath = GetConfig().iLogPath + OUT_FILE_S04_CH_NOT_FOUND;
  fileStage04ChineseTokenNotFound = fopen(iFilePath.c_str(), "w");
  if (fileStage04ChineseTokenNotFound == NULL)
  {
    printf("fileStage04ChineseTokenNotFound err:%s\n", iFilePath.c_str());
    fclose(fileStage04FullChineseToken);
    fclose(fileStage04PartChineseToken);
    return false;
  }
  printf("fileStage04ChineseTokenNotFound:%s\n", iFilePath.c_str());

  // get xdb header
  iFilePath = GetConfig().iInputPath + GetConfig().iInputScwsXdb;
  iFileXdb = fopen(iFilePath.c_str(), "rb");
  if (iFileXdb == NULL)
  {
    printf("iFileXdb err:%s\n", iFilePath.c_str());
    fclose(fileStage04FullChineseToken);
    fclose(fileStage04PartChineseToken);
    fclose(fileStage04ChineseTokenNotFound);
    return false;
  }
  printf("iFileXdb:%s\n", iFilePath.c_str());

  readSize = fread(&iXdbHeader, 1, sizeof(xdb_header), iFileXdb);
  if (readSize != sizeof(xdb_header))
  {
    fputs("Reading error 13", stderr); exit(EXIT_FAILURE);
  }

  // Start to parse each token in iTokenMap.
  for (std::map<std::string,int>::iterator it = iTokenMapWithOneWord.begin(); it != iTokenMapWithOneWord.end(); ++it)
  {
    // Search this token in xdb.
    strcpy(iTempBuffer, it->first.c_str());
    strtok(iTempBuffer,"\n\r");
    memset(&iNodeInfoAttr, 0, sizeof(iNodeInfoAttr));

    if (SEARCH_TOKEN_INFO_FOUND == SearchTokenInfo(iTempBuffer, -1, -1, iXdbHeader.base, iXdbHeader.prime, &iNodeInfoAttr, iFileXdb))
    {
      // Save only full tokens.
      if (2 == iNodeInfoAttr.flag)
      {
        fprintf(fileStage04PartChineseToken, "PART:%s\t%4.2f\t%4.2f\t%d\t%s\n", iTempBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr);
      }
      else
      {
        fprintf(fileStage04FullChineseToken, "%s\t%4.2f\t%4.2f\t%d\t%s\n", iTempBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr);
      }
    } 
    else
    {
      fprintf(fileStage04ChineseTokenNotFound, "NOT_FOUND:%s\t%4.2f\t%4.2f\t%d\t%s\n", iTempBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr);
    }
  }

  fclose(iFileXdb);
  fclose(fileStage04FullChineseToken);
  fclose(fileStage04PartChineseToken);
  fclose(fileStage04ChineseTokenNotFound);
  //------------------------------------------------------------------------------------------------

  return true;
}

bool CXdbFilter::RemoveSpecialSuffixTokens()
{
  // Stage 5: Start to build iTokenMapOpt by parsing special suffix of each token in iTokenMapWithOneWord.
  //------------------------------------------------------------------------------------------------
  printf("Stage 5: Start to build iTokenMapOpt by parsing special suffix of each token in iTokenMapWithOneWord.\n");

  FILE* fileStage05LogAddedAndRemovedSuffix;
  int maxSuffixTokenLength = 0;
  int suffixOffset;
  char removeSuffixToken = 0;
  std::string tempString;

#ifdef REMOVE_FULL_TOKENS_WITH_SPECIAL_END
  FILE* fileStage05LogRemovedSpecialEndingTokens;

  iFilePath = GetConfig().iLogPath + OUT_FILE_S05_REMOVED_SPECIAL_END_TOKENS;
  fileStage05LogRemovedSpecialEndingTokens = fopen(iFilePath.c_str(), "w");
  if (fileStage05LogRemovedSpecialEndingTokens == NULL)
  {
    printf("fileStage05LogRemovedSpecialEndingTokens err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fileStage05LogRemovedSpecialEndingTokens:%s\n", iFilePath.c_str());
#endif

  iFilePath = GetConfig().iLogPath + OUT_FILE_S05_ADDED_REMOVED_SUFFIX;
  fileStage05LogAddedAndRemovedSuffix = fopen(iFilePath.c_str(), "w");
  if (fileStage05LogAddedAndRemovedSuffix == NULL)
  {
    printf("fileStage05LogAddedAndRemovedSuffix err:%s\n", iFilePath.c_str());
  #ifdef REMOVE_FULL_TOKENS_WITH_SPECIAL_END
    fclose(fileStage05LogRemovedSpecialEndingTokens);
  #endif
    return false;
  }
  printf("fileStage05LogAddedAndRemovedSuffix:%s\n", iFilePath.c_str());

  maxSuffixTokenLength = GetMaxSuffixTokenLength(iSuffixTokenMap);

  iFilePath = GetConfig().iLogPath + OUT_FILE_S05_FOUND_SUFFIX;
  iOutputFile.open(iFilePath.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = iTokenMapWithOneWord.begin(); it != iTokenMapWithOneWord.end(); ++it)
  {
    //std::cout << it->first << std::endl;
    suffixOffset = 0;

    // We will add this full token to iTokenMapOpt.
    iTokenMapOpt.insert(std::pair<std::string,int>(it->first, it->second));

    // The full token in <iTokenMapWithOneWord> already added to <iTokenMapOpt>, 
    // now we will tell if this full token is a special-suffix-tokens, and will have some optimzed operations as below: 
    // 1. Remove special suffix.
    // 2. Check if this removed-suffix-token already exist in <iTokenMapWithOneWord>, 
    //     if YES we can just bypass to next token, if NO, user can choose if he want to add this removed-suffix-token to <iTokenMapOpt> or not.
    //
    if (IsTokenEndWithIgnoredSuffix(it->first.c_str(), &suffixOffset, iTempBuffer, maxSuffixTokenLength, iSuffixTokenMap))
    {
      // Log FoundIgnoredSuffixToken.
      iOutputFile << "FoundIgnoredSuffixToken:[" << it->first << "] SuffixOff:" << suffixOffset << " Suffix:[" << it->first.c_str() + suffixOffset << "]" << std::endl;

    #ifdef REMOVE_FULL_TOKENS_WITH_SPECIAL_END
      // Just test if this token end with special word "road" in Chinese
      tempString = it->first;
      std::string lastWord = tempString.substr(tempString.length() - KCJKBytes);
      //std::cout << "lastWord:[" << lastWord << "]" << std::endl;
      //UTF-8 code of "road" in Chinese = E8 B7 AF
      std::string specialWord;
      specialWord.resize(KCJKBytes);
      specialWord[0] = 0xE8;
      specialWord[1] = 0xB7;
      specialWord[2] = 0xAF;
      //std::cout << "specialWord:[" << specialWord << "]" << std::endl;
      if (lastWord == specialWord)
      {
        std::cout << "[" << tempString << "] end with special word [" << specialWord << "]" << std::endl;

        //remove this token from iTokenMapOpt.
        iTokenMapOpt.erase(it->first); // erasing by key
        std::cout << "Remove special[" << it->first << "][" << it->second << "] from iTokenMapOpt!" << std::endl;
        fprintf(fileStage05LogRemovedSpecialEndingTokens, "Remove special end token [%s][%d] from token_map_opt!\n", it->first.c_str(), it->second);
      }
    #endif //REMOVE_FULL_TOKENS_WITH_SPECIAL_END

      // Remove suffix.
      tempString = it->first;
      tempString.erase(suffixOffset, std::string::npos);
    #ifdef ENABLE_USER_INTERACTIVE_CONTROL //Ask user.
      std::cout << "FoundIgnoredSuffixToken:[" << it->first << "]" << " AfterRemoveSuffix:[" << tempString << "]" << std::endl;
    #endif

      // Is the removed-suffix-token exist in iTokenMapWithOneWord?
      if (iTokenMapWithOneWord.end() != iTokenMapWithOneWord.find(tempString)) // Removed-suffix-token ALREADY in iTokenMapWithOneWord!
      {
        //Add to iTokenMapOpt.
        iTokenMapOpt.insert(std::pair<std::string,int>(tempString, it->second));

        //std::cout << "[" << tempString << "] alread in map" << std::endl;
        fprintf(fileStage05LogAddedAndRemovedSuffix, "Removed-suffix-token ALREADY in old map:[%s] and [%s] ADDED to new map!\n", tempString.c_str(), it->first.c_str());
      #ifdef ENABLE_USER_INTERACTIVE_CONTROL //Notify user.
        std::cout << "Removed-suffix-token ALREADY in old map:[" << tempString << "] and [" << it->first << "] ADDED to new map!" << std::endl;
      #endif
      }
      else // Removed-suffix-token NOT in map.
      {
        //std::cout << "[" << tempString << "] NOT in map" << std::endl;
      #ifdef ENABLE_USER_INTERACTIVE_CONTROL //Ask user.
        std::cout << "Removed-suffix-token NOT in old map:[" << tempString << "] " << std::endl;
        std::cout << "Enter 'y' to add both [" << tempString << "] and [" << it->first << "], otherwise will just add [" << it->first << "]. (y/n):";
        std::cin >> removeSuffixToken;
      #else //Force add.
        removeSuffixToken = 'y';
      #endif

        //fprintf(fileStage05LogAddedAndRemovedSuffix, "Removed-suffix-token NOT in old map:[%s] ", tempString.c_str());
        if (removeSuffixToken == 'y')
        {
          iTokenMapOpt.insert(std::pair<std::string,int>(tempString, ++iTokenIndex));
          fprintf(fileStage05LogAddedAndRemovedSuffix, "Removed-suffix-token NOT in old map:[%s] and [%s] ADDED to new map!\n", tempString.c_str(), it->first.c_str());
        #ifdef ENABLE_USER_INTERACTIVE_CONTROL //Ask user.
          std::cout << "SUCCESSFULLY ADDED [" << tempString << "] and [" << it->first << "] to new map!" << std::endl;
        #endif
        }
        else
        {
          // skip this token.
          fprintf(fileStage05LogAddedAndRemovedSuffix, "Removed-suffix-token NOT in old map: SKIP ADDING [%s], RESERVED [%s] to new map\n", tempString.c_str(), it->first.c_str());
        #ifdef ENABLE_USER_INTERACTIVE_CONTROL //Ask user.
          std::cout << "Removed-suffix-token NOT in old map: SKIP ADDING [" << tempString << "], RESERVED [" << it->first << "] to new map" << std::endl;
        #endif
          continue;
        }
      }
    } //IsTokenEndWithIgnoredSuffix
  }
  iOutputFile.close();

  // Log optimized iTokenMapOpt.
  iFilePath = GetConfig().iLogPath + OUT_FILE_S05_MAP_OPTIMIZED;
  iOutputFile.open(iFilePath.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = iTokenMapOpt.begin(); it != iTokenMapOpt.end(); ++it)
  {
    iOutputFile << it->first << "|" << it->second << '\n';
  }
  iOutputFile.close();
  fclose(fileStage05LogAddedAndRemovedSuffix);
#ifdef REMOVE_FULL_TOKENS_WITH_SPECIAL_END
  fclose(fileStage05LogRemovedSpecialEndingTokens);
#endif
  //------------------------------------------------------------------------------------------------

  return true;
}

bool CXdbFilter::RetrieveTokenInfo()
{
  // Stage 6: Retreive every optimized map item's info from xdb.
  //------------------------------------------------------------------------------------------------
  printf("Stage 6: Retreive every optimized map item's info from xdb.\n");

  FILE* fileStage06TokenInfoFull;
  FILE* fileStage06TokenInfoPartial;
  FILE* fileStage06TokenInfoNotFound;
  size_t readSize = 0;

   // get xdb header
  iFilePath = GetConfig().iInputPath + GetConfig().iInputScwsXdb;
  iFileXdb = fopen(iFilePath.c_str(), "rb");
  if (iFileXdb == NULL)
  {
    return false;
  }

  readSize = fread(&iXdbHeader, 1, sizeof(xdb_header), iFileXdb);
  if (readSize != sizeof(xdb_header))
  {
    fputs("Reading error 14", stderr); exit(EXIT_FAILURE);
  }

  iFilePath = GetConfig().iOutputPath + OUT_FILE_S06_SUFFIX_FULL;
  fileStage06TokenInfoFull = fopen(iFilePath.c_str(), "w");
  if (fileStage06TokenInfoFull == NULL)
  {
    printf("fileStage06TokenInfoFull err:%s\n", iFilePath.c_str());
    fclose(iFileXdb);
    return false;
  }
  printf("fileStage06TokenInfoFull:%s\n", iFilePath.c_str());

  iFilePath = GetConfig().iLogPath + OUT_FILE_S06_SUFFIX_PART;
  fileStage06TokenInfoPartial = fopen(iFilePath.c_str(), "w");
  if (fileStage06TokenInfoPartial == NULL)
  {
    printf("fileStage06TokenInfoPartial err:%s\n", iFilePath.c_str());
    fclose(iFileXdb);
    fclose(fileStage06TokenInfoFull);
    return false;
  }
  printf("fileStage06TokenInfoPartial:%s\n", iFilePath.c_str());

  iFilePath = GetConfig().iLogPath + OUT_FILE_S06_SUFFIX_NOT_FOUND;
  fileStage06TokenInfoNotFound = fopen(iFilePath.c_str(), "w");
  if (fileStage06TokenInfoNotFound == NULL)
  {
    printf("fileStage06TokenInfoNotFound err:%s\n", iFilePath.c_str());
    fclose(iFileXdb);
    fclose(fileStage06TokenInfoFull);
    fclose(fileStage06TokenInfoPartial);
    return false;
  }
  printf("fileStage06TokenInfoNotFound:%s\n", iFilePath.c_str());

  // Start to parse each token in iTokenMapOpt.
  for (std::map<std::string,int>::iterator it = iTokenMapOpt.begin(); it != iTokenMapOpt.end(); ++it)
  {
    // Search this token in xdb.
    strcpy(iTempBuffer, it->first.c_str());
    strtok(iTempBuffer, "\n\r");
    if (SEARCH_TOKEN_INFO_FOUND == SearchTokenInfo(iTempBuffer, -1, -1, iXdbHeader.base, iXdbHeader.prime, &iNodeInfoAttr, iFileXdb))
    {
      if (iNodeInfoAttr.flag == 2)
      {
        fprintf(fileStage06TokenInfoPartial, "PART:%s\t%4.2f\t%4.2f\t%d\t%s\n", iTempBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr);

        // Force add this token as full token.
        iNodeInfoAttr.tf = 1.0;
        iNodeInfoAttr.idf = 1.0;
        iNodeInfoAttr.flag = 1;
        strcpy(iNodeInfoAttr.attr, "@");

        // If this is a AreaName token: 1. Change attr to "ns". 2. idf+105.
        if (iAllAreaMap.end() != iAllAreaMap.find(it->first))
        {
          // AreaName found, update attr and idf.
          iNodeInfoAttr.idf += 100.0;
          strcpy(iNodeInfoAttr.attr, "ns");

          fprintf(fileStage06TokenInfoFull, "%s\t%4.2f\t%4.2f\t%d\t%s     111\n", iTempBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr);
        }
        else
        {
          fprintf(fileStage06TokenInfoFull, "%s\t%4.2f\t%4.2f\t%d\t%s     !!!\n", iTempBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr);
        }
      }
      else
      {
        // If this is a AreaName token: 1. Change attr to "ns". 2. idf+105.
        if (iAllAreaMap.end() != iAllAreaMap.find(it->first))
        {
          // AreaName found, update attr and idf.
          iNodeInfoAttr.idf += 100.0;
          strcpy(iNodeInfoAttr.attr, "ns");

          fprintf(fileStage06TokenInfoFull, "%s\t%4.2f\t%4.2f\t%d\t%s     222\n", iTempBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr);
        }
        else
        {
          fprintf(fileStage06TokenInfoFull, "%s\t%4.2f\t%4.2f\t%d\t%s\n", iTempBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr);
        }
      }
    }
    else
    {
      fprintf(fileStage06TokenInfoNotFound, "NOT_FOUND:%s\n", iTempBuffer);

      // Force add this token as full token.
      iNodeInfoAttr.tf = 1.0;
      iNodeInfoAttr.idf = 1.0;
      iNodeInfoAttr.flag = 1;
      strcpy(iNodeInfoAttr.attr, "@");

      // If this is a AreaName token: 1. Change attr to "ns". 2. idf+105.
      if (iAllAreaMap.end() != iAllAreaMap.find(it->first))
      {
        // AreaName found, update attr and idf.
        iNodeInfoAttr.idf += 100.0;
        strcpy(iNodeInfoAttr.attr, "ns");

        fprintf(fileStage06TokenInfoFull, "%s\t%4.2f\t%4.2f\t%d\t%s     333\n", iTempBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr);
      }
      else
      {
        fprintf(fileStage06TokenInfoFull, "%s\t%4.2f\t%4.2f\t%d\t%s     !!!\n", iTempBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr);
      }
    }
  }

  fclose(fileStage06TokenInfoFull);
  fclose(fileStage06TokenInfoPartial);
  fclose(fileStage06TokenInfoNotFound);
  fclose(iFileXdb);
  //------------------------------------------------------------------------------------------------

  return true;
}

bool CXdbFilter::ConvertToNormalizedTokens()
{
  // Stage 7: Convert results to normalized form.
  //------------------------------------------------------------------------------------------------
  printf("Stage 7: Convert results to normalized form.\n");

  FILE* fileStage07TokenInfoFull;
  FILE* fileStage07TokenInfoFullNormalized;
  int lineLength;
  char* tokenBeginningPointer = 0;

  int enableHPNormalize = 0;
  std::string normalizedText;
  int lineNumber = 0;

  iFilePath = GetConfig().iInputPath + GetConfig().iInputNormalizeMap;
  if (!CHomophoneNormalizer_Init(iFilePath.c_str()))
  {
    printf("CHomophoneNormalizer_Init err:%s\n", iFilePath.c_str());
    enableHPNormalize = 0;
  }
  else
  {
    printf("CHomophoneNormalizer_Init OK\n");
    enableHPNormalize = 1;
  }

  iFilePath = GetConfig().iOutputPath + OUT_FILE_S06_SUFFIX_FULL;
  fileStage07TokenInfoFull = fopen(iFilePath.c_str(), "r");
  if (fileStage07TokenInfoFull == NULL)
  {
    printf("fileStage07TokenInfoFull err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fileStage07TokenInfoFull:%s\n", iFilePath.c_str());

  iFilePath = GetConfig().iOutputPath + OUT_FILE_S07_SUFFIX_FULL_NOR;
  fileStage07TokenInfoFullNormalized = fopen(iFilePath.c_str(), "w");
  if (fileStage07TokenInfoFullNormalized == NULL)
  {
    printf("fileStage07TokenInfoFullNormalized err:%s\n", iFilePath.c_str());
    fclose(fileStage07TokenInfoFull);
    return false;
  }
  printf("fileStage07TokenInfoFullNormalized:%s\n", iFilePath.c_str());

  // Start converting.
  normalizedText.resize(MAX_TOKEN_SIZE);
  while (fgets(iBuffer, sizeof(iBuffer), fileStage07TokenInfoFull) != NULL)
  {
    lineNumber++;
    if (lineNumber % 1000 == 0)
    {
      printf("Converting Line(%d)....\n", lineNumber);
    }

    if ((iBuffer[0] == '#') || (iBuffer[0] == ';'))
    {
      continue;
    }

    strncpy(iTempBuffer, iBuffer, sizeof(iTempBuffer));
    lineLength = strlen(iTempBuffer);

    if (lineLength == 0)
    {
      continue;
    }

    tokenBeginningPointer = strtok(iTempBuffer, "\t ");
    if (tokenBeginningPointer != NULL)
    {
      //printf("Converting [%s] of %s", tokenBeginningPointer, iBuffer);

      // Normalizer
      //normalizedText.resize(MAX_TOKEN_SIZE);
      if (enableHPNormalize)
      {
        size_t res_len = CHomophoneNormalizer_Normalize(tokenBeginningPointer, &normalizedText[0], normalizedText.capacity());

        if (res_len != 0) // Buffer size is too small
        {
          //printf("@@@ Buffer size is too small, tokenBeginningPointer length=%d,  normalizedText.size()=%d,  normalizedText.capacity()=%d\n", res_len, normalizedText.size(), normalizedText.capacity());
          CFtsTokenizerExtChinese_ReserveStringCapacity(normalizedText, res_len, KNormBufUnitSize);

          if ((res_len = CHomophoneNormalizer_Normalize(tokenBeginningPointer, &normalizedText[0], normalizedText.capacity())))
          {
            printf("Normalize error=%ld\n", res_len);
            return false;
          }
        }
        //printf("normalizedText.c_str():%s\n", normalizedText.c_str());
        //printf("strlen(normalizedText.c_str()):%d\n", strlen(normalizedText.c_str()));

        //printf("AAA Un-Normali aString:%s", iBuffer);
        strncpy(iBuffer, normalizedText.c_str(), strlen(normalizedText.c_str()));
        //printf("BBB Normalized aString:%s", iBuffer);

        //log to file.
        fprintf(fileStage07TokenInfoFullNormalized, "%s", iBuffer);
      }
    }
    else
    {
      printf("Failed converting:%s", iBuffer);
    }
  }
  printf("Total converting Line(%d)....\n", lineNumber);

  fclose(fileStage07TokenInfoFull);
  fclose(fileStage07TokenInfoFullNormalized);
  //------------------------------------------------------------------------------------------------

  return true;
}

bool CXdbFilter::MergeToFuzzyTokens()
{
  // Stage 8: Merge TokenList and TokenListNormalized as TokenListFuzzy
  //------------------------------------------------------------------------------------------------
  FILE* fileStage08TokenListFuzzy = NULL;
  FILE* fileStage08TokenList = NULL;
  FILE* fileStage08TokenListNormalized = NULL;

  iFilePath = GetConfig().iOutputPath + OUT_FILE_S08_SUFFIX_FULL_FUZZY;
  fileStage08TokenListFuzzy = fopen(iFilePath.c_str(), "w");
  if (fileStage08TokenListFuzzy == NULL)
  {
    printf("fileStage08TokenListFuzzy err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fileStage08TokenListFuzzy:%s\n", iFilePath.c_str());

  iFilePath = GetConfig().GetOutputPath() + GetConfig().GetOutputTokenList();
  fileStage08TokenList = fopen(iFilePath.c_str(), "r");
  if (fileStage08TokenList == NULL)
  {
    printf("Error open fileStage08TokenList: %s\n", iFilePath.c_str());
    fclose(fileStage08TokenListFuzzy);
    return false;
  }
  printf("fileStage08TokenList:%s\n", iFilePath.c_str());

  iFilePath = GetConfig().GetOutputPath() + GetConfig().GetOutputTokenListNormalized();
  fileStage08TokenListNormalized = fopen(iFilePath.c_str(), "r");
  if (fileStage08TokenListNormalized == NULL)
  {
    printf("Error open fileStage08TokenListNormalized: %s\n", iFilePath.c_str());
    fclose(fileStage08TokenListFuzzy);
    fclose(fileStage08TokenList);
    return false;
  }
  printf("fileStage08TokenListNormalized:%s\n", iFilePath.c_str());

  while (fgets(iBuffer, sizeof(iBuffer), fileStage08TokenList) != NULL)
  {
    fputs(iBuffer, fileStage08TokenListFuzzy);
  }
  fclose(fileStage08TokenList);

  while (fgets(iBuffer, sizeof(iBuffer), fileStage08TokenListNormalized) != NULL)
  {
    fputs(iBuffer, fileStage08TokenListFuzzy);
  }

  fclose(fileStage08TokenListNormalized);
  fclose(fileStage08TokenListFuzzy);
  //------------------------------------------------------------------------------------------------

  return true;
}
////////////////////// Search Node /////////////////////////////////////
int CXdbFilter::GetHashIndex(const unsigned char* aKey, int aHashBase, int aHashPrime) const
{
  int length = strlen((char*)aKey);

  while (length--)
  {
    aHashBase += (int)(aHashBase << 5);
    aHashBase ^= aKey[length];
    aHashBase &= 0x7fffffff;
  }

  return (aHashBase % aHashPrime);
}

int CXdbFilter::SearchTokenInfo(const char*    aTokenContent,
                                long           aNodeOffset,
                                long           aNodeLength,
                                int            aHashBase,
                                int            aHashPrime,
                                TNodeInfoAttr* aAttribute,
                                FILE*          aFileXdb)
{
  int hashIndex;
  long fileOffset;
  TPrimeNode primeNode;
  TNodeInfoHeader nodeInfoHeader;
  char tokenBuffer[MAX_TOKEN_SIZE];
  int comparedResult;
  size_t readSize = 0;

  if (aFileXdb == NULL)
  {
    printf("SearchTokenInfoaFileXdb err\n");
    return SEARCH_TOKEN_INFO_NOT_FOUND;
  }

  if ((aNodeOffset < 0) && (aNodeLength < 0))
  {
    hashIndex = GetHashIndex((unsigned char*)aTokenContent, aHashBase, aHashPrime);

    fileOffset = sizeof(xdb_header) + hashIndex * sizeof(TPrimeNode);
    //fsetpos(aFileXdb, &fileOffset);
    fseek(aFileXdb, fileOffset, SEEK_SET);
    readSize = fread(&primeNode, 1, sizeof(TPrimeNode), aFileXdb);
    if (readSize != sizeof(TPrimeNode))
    {
      fputs("Reading error 01", stderr); exit(EXIT_FAILURE);
    }

    aNodeOffset = primeNode.offset;
    aNodeLength = primeNode.length;
  }

  // read node
  fileOffset = aNodeOffset;
  fseek(aFileXdb, fileOffset, SEEK_SET);

  readSize = fread(&nodeInfoHeader.l_offset, 1, sizeof(unsigned int), aFileXdb);
  if (readSize != sizeof(unsigned int))
  {
    fputs("Reading error 02", stderr); exit(EXIT_FAILURE);
  }
  readSize = fread(&nodeInfoHeader.l_length, 1, sizeof(unsigned int), aFileXdb);
  if (readSize != sizeof(unsigned int))
  {
    fputs("Reading error 03", stderr); exit(EXIT_FAILURE);
  }
  readSize = fread(&nodeInfoHeader.r_offset, 1, sizeof(unsigned int), aFileXdb);
  if (readSize != sizeof(unsigned int))
  {
    fputs("Reading error 04", stderr); exit(EXIT_FAILURE);
  }
  readSize = fread(&nodeInfoHeader.r_length, 1, sizeof(unsigned int), aFileXdb);
  if (readSize != sizeof(unsigned int))
  {
    fputs("Reading error 05", stderr); exit(EXIT_FAILURE);
  }
  readSize = fread(&nodeInfoHeader.k_length, 1, sizeof(unsigned char), aFileXdb);
  if (readSize != sizeof(unsigned char))
  {
    fputs("Reading error 06", stderr); exit(EXIT_FAILURE);
  }

  memset(tokenBuffer, 0, sizeof(tokenBuffer));
  readSize = fread(tokenBuffer, 1, nodeInfoHeader.k_length, aFileXdb);
  if (readSize != nodeInfoHeader.k_length)
  {
    fputs("Reading error 07", stderr); exit(EXIT_FAILURE);
  }

  comparedResult = strcmp(tokenBuffer, aTokenContent);

  if (comparedResult == 0)
  {
    readSize = fread(&(aAttribute->tf), 1, sizeof(float), aFileXdb);
    if (readSize != sizeof(float))
    {
      fputs("Reading error 08", stderr); exit(EXIT_FAILURE);
    }
    readSize = fread(&(aAttribute->idf), 1, sizeof(float), aFileXdb);
    if (readSize != sizeof(float))
    {
      fputs("Reading error 09", stderr); exit(EXIT_FAILURE);
    }
    readSize = fread(&(aAttribute->flag), 1, sizeof(unsigned char), aFileXdb);
    if (readSize != sizeof(unsigned char))
    {
      fputs("Reading error 10", stderr); exit(EXIT_FAILURE);
    }
    readSize = fread(&(aAttribute->attr), sizeof(unsigned char), 3, aFileXdb);
    if (readSize != sizeof(unsigned char) * 3)
    {
      fputs("Reading error 11", stderr); exit(EXIT_FAILURE);
    }
    //readSize = fread(aAttribute, 1, sizeof(TNodeInfoAttr), aFileXdb);
    //if (readSize != sizeof(TNodeInfoAttr))
    //{
    //  fputs("Reading error 12", stderr); exit(EXIT_FAILURE);
    //}
    return SEARCH_TOKEN_INFO_FOUND;
  }
  else if (comparedResult > 0)
  {
    if ((nodeInfoHeader.l_offset == 0) && (nodeInfoHeader.l_length == 0))
    {
      return SEARCH_TOKEN_INFO_NOT_FOUND; // no node
    }

    return SearchTokenInfo(aTokenContent, nodeInfoHeader.l_offset, nodeInfoHeader.l_length, aHashBase, aHashPrime, aAttribute, aFileXdb);
  }
  else
  {
    if ((nodeInfoHeader.r_offset == 0) && (nodeInfoHeader.r_length == 0))
    {
      return SEARCH_TOKEN_INFO_NOT_FOUND; // no node
    }

    return SearchTokenInfo(aTokenContent, nodeInfoHeader.r_offset, nodeInfoHeader.r_length, aHashBase, aHashPrime, aAttribute, aFileXdb);
  }

  return SEARCH_TOKEN_INFO_NOT_FOUND;
}
////////////////////// Search Node /////////////////////////////////////

int CXdbFilter::GetUTF8CharKind(unsigned char aChar)
{
  if (aChar <= 0x7f)
  {
    return UTF8_CHAR_KIND_1;  // ASCII
  }

  if (aChar >= 0xC0 && aChar <= 0xDF)
  {
    return UTF8_CHAR_KIND_2;  // Latin characters
  }

  if (aChar >= 0xE0 && aChar <= 0xEF)
  {
    return UTF8_CHAR_KIND_3;  // CJKS
  }

  if (aChar >= 0xF0 && aChar <= 0xF7)
  {
    return UTF8_CHAR_KIND_4;  // CJKS
  }

  return UTF8_CHAR_KIND_NONE;
}

int CXdbFilter::ConvUTF8ToUTF16(unsigned char* aUtf8CodeData, unsigned int* aData, int aDataLength)
{
  unsigned char* utf8CodePointer = NULL;
  unsigned char utf8Code;
  unsigned char utf8CodeNext[2];
  unsigned char highByte, lowByte;
  int resultLength;

  if (aUtf8CodeData == NULL)
  {
    return false;
  }

  if (aData == NULL)
  {
    return false;
  }

  if (aDataLength == 0)
  {
    return false;
  }

  utf8CodePointer = aUtf8CodeData;
  resultLength = 0;

  while ((utf8Code = *utf8CodePointer) != 0x00)
  {
    if ((utf8Code > 0x00) && (utf8Code <= 0x7F))
    {
      // 000000 - 00007F
      aData[resultLength++] = (unsigned int)utf8Code;
    }
    else if ((utf8Code >= 0xC0) && (utf8Code <= 0xDF))
    {
      // 110yyyyy(C0-DF) 10zzzzzz(80-BF)  UTF-8
      // 00000yyy yyzzzzzz(UTF-16)
      utf8CodeNext[0] = *(utf8CodePointer + 1);
      if ((utf8CodeNext[0] >= 0x80) && (utf8CodeNext[0] <= 0xBF))
      {
        highByte = (utf8Code & 0x1C) >> 2;
        lowByte = ((utf8Code & 0x03) << 6) |(utf8CodeNext[0] & 0x3f);
        aData[resultLength++] = ((unsigned int)highByte << 8) | ((unsigned int)lowByte);
        utf8CodePointer++;
      }
      else
      {
        aData[resultLength++] = (unsigned int)utf8Code;
      }
    }
    else if ((utf8Code >= 0xE0) && (utf8Code <= 0xEF))
    {
      // 1110xxxx(E0-EF)  10yyyyyy(80-BF) 10zzzzzz(80-BF) UTF-8
      // xxxxyyyy yyzzzzzz(UTF-16)
      utf8CodeNext[0] = *(utf8CodePointer + 1);
      utf8CodeNext[1] = *(utf8CodePointer + 2);

      if (((utf8CodeNext[0] >= 0x80) && (utf8CodeNext[0] <= 0xBF)) && ((utf8CodeNext[1] >= 0x80) && (utf8CodeNext[1] <= 0xBF)))
      {
        highByte = ((utf8Code & 0x0f) << 4) | ((utf8CodeNext[0] & 0x3C) >> 2);
        lowByte = ((utf8CodeNext[0] & 0x03) << 6) | (utf8CodeNext[1] & 0x3F);
        aData[resultLength++] = ((unsigned int)highByte << 8) | ((unsigned int)lowByte);
        utf8CodePointer++;
        utf8CodePointer++;
      }
      else
      {
        aData[resultLength++] = (unsigned int)utf8Code;
      }
    }
    else
    {
      aData[resultLength++] = (unsigned int)utf8Code;
    }

    utf8CodePointer++;
  }

  return resultLength;
}

int CXdbFilter::IsAllChineseToken(const char* aTokenContent, unsigned int aTokenLength)
{
  unsigned int index = 0;
  unsigned int utf8CharKind;
  unsigned int returnLength;
  unsigned int utf16Data[MAX_UTF16_TOKEN_LENGTH];

  if (aTokenLength == KCJKBytes)
  {
    returnLength = ConvUTF8ToUTF16((unsigned char*)aTokenContent, utf16Data, 10);

    if (returnLength == ASCIIBytes)
    {
      // check for symbol
      if ((utf16Data[0] <= 0x3400) || (utf16Data[0] >= 0xFAD9))
      {
        return false;
      }
    }
  }

  while (index < aTokenLength)
  {
    utf8CharKind = GetUTF8CharKind((unsigned char)aTokenContent[index]);

    if (UTF8_CHAR_KIND_3 != utf8CharKind)
    {
      return false;
    }

    index += utf8CharKind;
  }

  return true;
}

int CXdbFilter::IsValidChineseToken(scws_res_t aScwsCur, const char* aTokenContent)
{
  if (aScwsCur->len < KCJKBytes)
  {
    return false;
  }

  if (strcasecmp(aScwsCur->attr, "en") == 0)
  {
    return false;
  }

  if ((strcasecmp(aScwsCur->attr, "un") == 0) || (strcasecmp(aScwsCur->attr, "nz") == 0))
  {
    if (IsAllChineseToken(aTokenContent, aScwsCur->len) == 0)
    {
      return false;
    }
  }

  return true;
}

int CXdbFilter::GetTokenTotalChineseWordCount(const char* aString)
{
  int tokenLength = 0;
  int index = 0;
  int utf8CharKind = 0;
  int totalChineseWordCount = 0;

  totalChineseWordCount = 0;
  tokenLength = strlen(aString);

  if (tokenLength == 0)
  {
    totalChineseWordCount = 0;
    return totalChineseWordCount;
  }

  while (index < tokenLength)
  {
    utf8CharKind = GetUTF8CharKind((unsigned char)aString[index]);

    if (UTF8_CHAR_KIND_3 != utf8CharKind)
    {
      return totalChineseWordCount;
    }

    totalChineseWordCount++;
    index += utf8CharKind;
  }

  //printf("GetTokenTotalChineseWordCount[%s]=%d\n", aString, totalChineseWordCount);
  return totalChineseWordCount;
}

int CXdbFilter::GetNthChineseWordByteOffset(const char* aString, int aNthChineseWord)
{
  int tokenLength = 0;
  int index = 0;
  int utf8CharKind = 0;
  int totalChineseWordCount = 0;

  totalChineseWordCount = GetTokenTotalChineseWordCount(aString);

  if (aNthChineseWord > totalChineseWordCount)
  {
    aNthChineseWord = totalChineseWordCount;
  }

  tokenLength = strlen(aString);

  if (tokenLength == 0)
  {
    index = 0;
    return index;
  }
  
  while (index < tokenLength)
  {
    utf8CharKind = GetUTF8CharKind((unsigned char)aString[index]);

    if (UTF8_CHAR_KIND_3 != utf8CharKind)
    {
      return (index - UTF8_CHAR_KIND_3);
    }

    if ((index / UTF8_CHAR_KIND_3) == aNthChineseWord)
    {
      //printf("GetNthChineseWordByteOffset: aString:[%s] Nth:[%d] offset:[%d]\n", aString, aNthChineseWord, index);
      return index;
    }
    index += utf8CharKind;
  }

  return index;
}

////////////////////// SuffixTokenMap /////////////////////////////////////
bool CXdbFilter::IsBufferStartWithBOM(char* aBuffer)
{

  if (((unsigned char)0xEF == (unsigned char)aBuffer[0]) &&
      ((unsigned char)0xBB == (unsigned char)aBuffer[1]) &&
      ((unsigned char)0xBF == (unsigned char)aBuffer[2]))
  {
    return true;
  }

  return false;
}

int CXdbFilter::InitSuffixTokenMap(std::map<std::string,int>& aSuffixTokenMap)
{
  FILE* fileStage00SpecialSuffixTable;
  std::string suffixToken;
  int lineNumber = 0;
  int tokenNumber = 0;

  iFilePath = GetConfig().iInputPath + GetConfig().iInputSpecialSuffixTable;
  fileStage00SpecialSuffixTable = fopen(iFilePath.c_str(), "r");
  if (fileStage00SpecialSuffixTable == NULL)
  {
    printf("Error fopen suffix table file: %s\n", iFilePath.c_str());
    return false;
  }

  aSuffixTokenMap.clear();
  while (fgets(iBuffer, sizeof(iBuffer), fileStage00SpecialSuffixTable) != NULL)
  {
    lineNumber++;

    if (lineNumber == 1) // for skip BOM
    {
      //printf("1iBuffer[0]:0x%X ",iBuffer[0]);
      //printf("1iBuffer[1]:0x%X ",iBuffer[1]);
      //printf("1iBuffer[2]:0x%X\n",iBuffer[2]);
      if (IsBufferStartWithBOM(iBuffer))
      {
        for (int i = 0; i < (strlen(iBuffer)-2); i++)
        {
          iBuffer[i] = iBuffer[i+3];
        }
      }
    }

    strtok(iBuffer,"\n\r");
    if (strlen(iBuffer) < KCJKBytes)
    {
      continue;
    }

    if ((iBuffer[0] == '#') || (iBuffer[0] == ';'))
    {
      continue;
    }

    //printf("SuffixTable lineNumber:%d = %s", lineNumber, iBuffer);
    //strtok(iBuffer,"\n\r");
    suffixToken = iBuffer;
    if (aSuffixTokenMap.end() == aSuffixTokenMap.find(suffixToken))
    {
      // Word not found, add to map.
      aSuffixTokenMap.insert(std::pair<std::string,int>(suffixToken, ++tokenNumber));
      //printf("add suffixToken:%s|%d\n", suffixToken.c_str(),tokenNumber);
    }
  }
  fclose(fileStage00SpecialSuffixTable);

  return true;
}

int CXdbFilter::GetMaxSuffixTokenLength(std::map<std::string,int>& aSuffixTokenMap)
{
  int maxLengthInBytes = 0;
  int tokenLength = 0;

  for (std::map<std::string,int>::iterator it = aSuffixTokenMap.begin(); it != aSuffixTokenMap.end(); ++it)
  {
    tokenLength = it->first.length();
    if (tokenLength > maxLengthInBytes)
    {
      maxLengthInBytes = tokenLength;
    }
  }

  return maxLengthInBytes / UTF8_CHAR_KIND_3;
}
////////////////////// SuffixTokenMap /////////////////////////////////////

int CXdbFilter::IsTokenEndWithIgnoredSuffix(const char*                aString,
                                            int*                       aSuffixOffset,
                                            char*                      aTempBuffer,
                                            int                        aMaxSuffixTokenLength,
                                            std::map<std::string,int>& aSuffixTokenMap)
{
  int tokenLength = 0;
  int tokenTotalChineseWordCount = 0;
  int suffixWordCount = 0;
  int chineseNthWordByteOffset;
  char* stringPointer;
  std::string suffixString;

  strcpy(aTempBuffer, aString);
  strtok(aTempBuffer,"\n\r");
  tokenLength = strlen(aTempBuffer);

  if (UTF8_CHAR_KIND_3 > tokenLength)
  {
      return false;
  }

  tokenTotalChineseWordCount = GetTokenTotalChineseWordCount(aTempBuffer);
  //printf("aString[%s] tokenTotalChineseWordCount[%d] aMaxSuffixTokenLength[%d]\n", aString, tokenTotalChineseWordCount, aMaxSuffixTokenLength);

  if (aMaxSuffixTokenLength >= tokenTotalChineseWordCount)
  {
    aMaxSuffixTokenLength = tokenTotalChineseWordCount - 1;
  }
  if (aMaxSuffixTokenLength == 0)
  {
    return false;
  }

  for (suffixWordCount = aMaxSuffixTokenLength; suffixWordCount > 0; suffixWordCount--)
  {
    chineseNthWordByteOffset = GetNthChineseWordByteOffset(aTempBuffer, (tokenTotalChineseWordCount - suffixWordCount));
    stringPointer = aTempBuffer + chineseNthWordByteOffset;
    //printf("suffixWordCount[%d] chineseNthWordByteOffset[%d] stringPointer[%s]\n", suffixWordCount, chineseNthWordByteOffset, stringPointer);

    // Compare stringPointer with all words(with length suffixWordCount) in _ignored_suffix_table_{tc|sc}_utf8.txt.
    suffixString = stringPointer;
    if (aSuffixTokenMap.end() != aSuffixTokenMap.find(suffixString))
    {
      // Found suffix!
      *aSuffixOffset = chineseNthWordByteOffset;
      //printf("aString[%s] Suffix[%s] *aSuffixOffset[%d]\n", aString, suffixString.c_str(), *aSuffixOffset);
      return true;
    }
  }

  return false;
}

int CXdbFilter::GetOneWordInToken(const char* aString,
                                  char*       aTempBuffer,
                                  size_t      aBufferLength,
                                  int         aWordindex)
{
  unsigned int tokenLength = 0;
  unsigned int index = 0;
  unsigned int utf8CharKind = 0;
  int found = 0;

  //printf("GetOneWordInTokenaString:[%s] aWordindex:%d\n", aString, aWordindex);
  memset(aTempBuffer, 0, aBufferLength);
  tokenLength = strlen(aString);

  if (tokenLength < KCJKBytes)
  {
    return false;
  }

  while (index < tokenLength)
  {
    utf8CharKind = GetUTF8CharKind((unsigned char)aString[index]);

    if (UTF8_CHAR_KIND_3 != utf8CharKind)
    {
      return false;
    }

    if ((index / UTF8_CHAR_KIND_3) == aWordindex)
    {
      found = 1;
      memcpy(aTempBuffer, &aString[index], UTF8_CHAR_KIND_3);
      aTempBuffer[UTF8_CHAR_KIND_3] = '\0';   /* null character manually added */
      //printf("GetOneWordInToken:%s\n", aTempBuffer);
      break;
    }

    index += utf8CharKind;
  }

  return found;
}

int CXdbFilter::CHomophoneNormalizer_Init(const char* aFile)
{
  FILE* fileStage07NormalizationMap;
  char tempString[MAX_LINE_SIZE];

  fileStage07NormalizationMap = fopen(aFile, "r");
  if (fileStage07NormalizationMap == NULL)
  {
    printf("Error fopen homophone map file: %s\n", aFile);
    return false;
  }

  int insertCount = 0;
  while (fgets(tempString, sizeof(tempString), fileStage07NormalizationMap) != NULL)
  {
    strtok(tempString, "\r\n");

    const char* pinyinCharacter;
    const char* sourceCharacter;
    const char* mappedCharacter;

    pinyinCharacter = strtok(tempString, ",");
    sourceCharacter = pinyinCharacter ? strtok(NULL, ",") : NULL;
    mappedCharacter = sourceCharacter ? strtok(NULL, ",") : NULL;

    if (pinyinCharacter && sourceCharacter && mappedCharacter)
    {
      //iNormalizerMap.Set((CHomophoneMap::TUnit) sourceCharacter, (CHomophoneMap::TUnit) mappedCharacter);
      if (iNormalizerMap.end() == iNormalizerMap.find(sourceCharacter))
      {
        iNormalizerMap.insert(std::pair<std::string,std::string>(sourceCharacter, mappedCharacter));
        ++insertCount;
      }
    }
    //LOG_DEBUG(gFtsChHPNormalizer, "iHomophoneMap[%s]=%s\n", sourceCharacter, mappedCharacter);
  }

  fclose(fileStage07NormalizationMap);
  printf("Read Homophone mapping into map DONE, total=%d.\n", insertCount);

  return true;
}

size_t CXdbFilter::CHomophoneNormalizer_Normalize(const char* aSourceString, char* aOutputString, size_t aLength)
{
  size_t stringLength = 0;
  size_t utf8FirstByteOffset = 0;
  size_t returnLength = 0;
  const unsigned char* inputString = (const unsigned char*)aSourceString;

  //printf("CHomophoneNormalizer_Normalize aSourceString:%s aLength:%d\n", aSourceString, aLength);
  stringLength = strlen(aSourceString);
  if ((stringLength + 1) > aLength)
  {
    returnLength = stringLength + 1; // aOutputString length equals to aSourceString
    return returnLength;
  }

  for (utf8FirstByteOffset = 0; inputString[utf8FirstByteOffset];)
  {
    size_t characterLengthInBytes = CTokenGeneratorChineseCommon::iUTF8MultibyteLengthTable[inputString[utf8FirstByteOffset]];

    if (KCJKBytes != characterLengthInBytes) /* not CJK */
    {
      memcpy(aOutputString + utf8FirstByteOffset, inputString + utf8FirstByteOffset, characterLengthInBytes);
    }
    else  /* CJK */
    {
      //if (iNormalizerMap.IsContained(inputString + utf8FirstByteOffset))
      std::string CJKStr((const char*)(inputString + utf8FirstByteOffset), characterLengthInBytes);
      if (iNormalizerMap.end() != iNormalizerMap.find(CJKStr))
      {
        memcpy(aOutputString + utf8FirstByteOffset, iNormalizerMap[CJKStr].data(), characterLengthInBytes);
      }
      else
      {
        memcpy(aOutputString + utf8FirstByteOffset, inputString + utf8FirstByteOffset, characterLengthInBytes);
      }
    }

    utf8FirstByteOffset += characterLengthInBytes;
  }

  aOutputString[utf8FirstByteOffset] = 0;
  //printf("HPNormalized result:%s\n", aOutputString);
  returnLength = 0;
  return returnLength;
}

void CXdbFilter::CFtsTokenizerExtChinese_ReserveStringCapacity(std::string& aString, size_t aSize, size_t aUnitSize)
{
  const size_t allocateSize = (aSize + aUnitSize - 1) & ~(aUnitSize - 1); // pack with aUnitSize
  aString.resize(allocateSize);
}

} // namespace NFtsTokenGenerator

