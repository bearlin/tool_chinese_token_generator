//
// This file contains the CXdbFilter implementation 
//

#include "XdbFilter.h"

const unsigned char CXdbFilter::iUTF8MultibyteLengthTable[256] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1 };

CXdbFilter::CXdbFilter() :
  KNormBufUnitSize(0x200),
  KCJKBytes(3),
  iTokenIndex(0)
{
  GetConfig().iOutputTokenList = OUT_PATH_S06_SUFFIX_FULL;
  GetConfig().iOutputTokenListNormalized = OUT_PATH_S07_SUFFIX_FULL_NOR;
  GetConfig().iOutputTokenListFuzzy = OUT_PATH_S08_SUFFIX_FULL_FUZZY;
}

CXdbFilter::~CXdbFilter()
{
}

bool CXdbFilter::Run()
{
  std::cout << "Running CXdbFilter" << std::endl;
  std::cout << "iInputPath:" << GetConfig().iInputPath << std::endl;
  std::cout << "iOutputPath:" << GetConfig().iOutputPath<< std::endl;
  std::cout << "iLogPath:" << GetConfig().iLogPath<< std::endl;

  if (!Init())
  {
    return false;
  }
  if (!CollectTokens())
  {
    return false;
  }
  if (!AddExtraTokens())
  {
    return false;
  }
  if (!RemoveUnwantTokens())
  {
    return false;
  }

  if (!AddMissingOneWordTokens())
  {
    return false;
  }

  if (!RemoveSpecialSuffixTokens())
  {
    return false;
  }

  if (!RetrieveTokenInfo())
  {
    return false;
  }

  if (!ConvertToNormalizedTokens())
  {
    return false;
  }

  if (!MergeToFuzzyTokens())
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
  std::cout << "Init" << std::endl;

  // Stage 0: Initializations.
  //------------------------------------------------------------------------------------------------
  printf("Stage 0: Initializations.\n");
  if (!InitSuffixTokenMap(iSuffixTokenMap))
  {
    printf("ERROR: cann't init the SuffixTokenMap!\n");
    return false;
  }

  // Log iSuffixTokenMap.
  iFilePath = GetConfig().iLogPath + OUT_PATH_S00_SUFFIX_TOK_MAP;
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
  std::cout << "CollectTokens" << std::endl;

  // Stage 1: Collect token list from "input raw file" or "saved iTokenMap log file" to iTokenMap.
  //------------------------------------------------------------------------------------------------
#ifdef MAP_INIT
  printf("Stage 1: Collect token list from \"input raw file\" to iTokenMap.\n");
  char tmpToken[170];
  scws_t scws;
  scws_res_t scwsRes, scwsCur;
  int status;

  char lineTextToScws[MAX_LINE_SIZE];
  char lineText[MAX_LINE_SIZE];
  int textSize;
  int lineNumber = 0;

  FILE* fp_s01_raw;
  FILE* fp_s01_not_ch;
  FILE* fp_s01_duplicate;

  iFilePath = GetConfig().iLogPath + OUT_PATH_S01_NOT_CH;
  fp_s01_not_ch = fopen(iFilePath.c_str(), "w");
  if (NULL == fp_s01_not_ch)
  {
    printf("fp_s01_not_ch err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fp_s01_not_ch:%s\n", iFilePath.c_str());

  iFilePath = GetConfig().iLogPath + OUT_PATH_S01_DUPLICATE;
  fp_s01_duplicate = fopen(iFilePath.c_str(), "w");
  if (NULL == fp_s01_duplicate)
  {
    printf("fp_s01_duplicate err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fp_s01_duplicate:%s\n", iFilePath.c_str());

  if (!(scws = scws_new()))
  {
    printf("ERROR: cann't init the scws!\n");
    return false;
  }

  scws_set_charset(scws, "utf8");

  iFilePath = GetConfig().iInputPath + GetConfig().iInputScwsXdb;
  status = scws_set_dict(scws, iFilePath.c_str(), SCWS_XDICT_XDB);
  if (0 != status)
  {
    printf("Err: Failed to set the dict file:%s\n", iFilePath.c_str());
    return false;
  }

  iFilePath = GetConfig().iInputPath + GetConfig().iInputScwsRule;
  scws_set_rule(scws, iFilePath.c_str() );

  iFilePath = GetConfig().iInputPath + GetConfig().iInputSourceData;
  printf("input src:%s\n", iFilePath.c_str());
  fp_s01_raw = fopen(iFilePath.c_str(), "r");
  if (NULL == fp_s01_raw)
  {
    printf("fp_s01_raw err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fp_s01_raw:%s\n", iFilePath.c_str());

  while (NULL != fgets(lineText, MAX_LINE_SIZE, fp_s01_raw))
  {
    lineNumber++;
    if (0 == (lineNumber % 100000))
    {
      printf("Parsing Line(%d)....\n", lineNumber);
    }

    if (0 == strlen(lineText))
    {
      continue;
    }

    textSize = 0;
    memset(lineTextToScws, 0, sizeof(lineTextToScws) );

    char* ptrLine = lineText;
    while (('\r' != *ptrLine) && ('\n' != *ptrLine) && (0 != *ptrLine))
    {
      lineTextToScws[textSize++] = *(ptrLine++);
    }

    //printf("%s\n", lineTextToScws);
    scws_send_text(scws, lineTextToScws, textSize);
    while (scwsRes = scwsCur = scws_get_result(scws))
    {
      while (scwsCur != NULL)
      {
        memset(tmpToken, 0, sizeof(tmpToken));
        memcpy(tmpToken, &lineTextToScws[scwsCur->off], scwsCur->len);

        if (IsValidChineseToken(scwsCur, tmpToken))
        {
          iTokenItem.clear();
          iTokenItem.append(tmpToken);

          std::map<std::string,int>::iterator it = iTokenMap.find(iTokenItem);

          if(it != iTokenMap.end())
          {
            // found ignore the duplicate items
            fprintf(fp_s01_duplicate, "%s off=%d idf=%4.2f len=%d attr=%s\n", tmpToken, scwsCur->off, scwsCur->idf, scwsCur->len, scwsCur->attr);
          }
          else
          {
            iTokenMap.insert(std::pair<std::string,int>(iTokenItem, ++iTokenIndex));
          }
        }
        else
        {
          fprintf(fp_s01_not_ch, "%s off=%d idf=%4.2f len=%d attr=%s\n", tmpToken, scwsCur->off, scwsCur->idf, scwsCur->len, scwsCur->attr);
        }

        scwsCur = scwsCur->next;
      }

      scws_free_result(scwsRes);
    }
  }
  scws_free(scws);
  printf("Total Parsing Line(%d)....\n", lineNumber);

  // Log iTokenMap
  iFilePath = GetConfig().iLogPath + OUT_PATH_S01_MAP_RAW;
  iOutputFile.open(iFilePath.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = iTokenMap.begin(); it != iTokenMap.end(); ++it)
  {
    iOutputFile << it->first << "|" << it->second << '\n';
  }
  iOutputFile.close();
  fclose(fp_s01_raw);
  fclose(fp_s01_not_ch);
  fclose(fp_s01_duplicate);

#else //MAP_INIT

  printf("Stage 1: Collect token list from \"saved iTokenMap log file\" to iTokenMap.\n");

  std::string str1;
  std::string str2;
  std::size_t pos = 0;
  int idx_tmp = 0;
  int idx_max = 0;

  // Re-load token list from iTokenMap log file.
  iFilePath = GetConfig().iLogPath + OUT_PATH_S01_MAP_RAW;
  iInputFile.open(iFilePath.c_str(), std::ifstream::in);
  if (!iInputFile.is_open())
  {
    // show message:
    std::cout<<"Error opening file: "<<iFilePath<<std::endl;
    return false;
  }
  printf("Re-load token list from iTokenMap log file:%s\n", iFilePath.c_str() );

  iTokenMap.clear();
  while (std::getline(iInputFile, iTmpLine).good())
  {
    pos = iTmpLine.find_first_of("|");
    str1 = iTmpLine.substr(0, pos);
    str2 = iTmpLine.substr(pos + 1, std::string::npos);
    idx_tmp = atoi(str2.c_str());
    //std::cout<<"iTmpLine:"<<iTmpLine<<" str1:"<<str1<<" str2:"<<str2<<" str2.c_str():"<<str2.c_str()<<std::endl;

    iTokenMap.insert( std::pair<std::string,int>(str1, idx_tmp) );

    if (idx_tmp > idx_max)
    {
      idx_max = idx_tmp;
    }
  }

  iTokenIndex = idx_max;
  iInputFile.close();

  // Log reloaded iTokenMap.
  iFilePath = GetConfig().iLogPath + OUT_PATH_S01_MAP_RELOAD;
  printf("Re-load token list:%s\n", iFilePath.c_str() );
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
  std::cout << "AddExtraTokens" << std::endl;

  // Stage 2: Add "additional tokens" from _ignored_suffix_table_{tc|sc}_utf8.txt to iTokenMap.
  //------------------------------------------------------------------------------------------------
  printf("Stage 2: Add \"additional tokens\" from _ignored_suffix_table_{tc|sc}_utf8.txt to iTokenMap.\n");

  std::string str1;
  std::string str2;
  std::size_t pos = 0;
  int idx_tmp = 0;

  for (std::map<std::string,int>::iterator it = iSuffixTokenMap.begin(); it != iSuffixTokenMap.end(); ++it)
  {
    //iOutputFile << it->first << "|" << it->second << '\n';
    if (iTokenMap.end() == iTokenMap.find(it->first))
    {
      // Word not found, add to map.
      iTokenMap.insert(std::pair<std::string,int>(it->first, ++iTokenIndex));
      printf("Not Found [%s] added as : %s|%d\n", it->first.c_str(), it->first.c_str(), iTokenIndex);
    }
  }

  // Log  iTokenMap with missing tokens in table _ignored_suffix_table_{tc|sc}_utf8.txt.
  iFilePath = GetConfig().iLogPath + OUT_PATH_S02_MAP_SUFFIX;
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
    std::cout<<"Error opening file: "<<iFilePath<<std::endl;
    return false;
  }

  iAllAreaMap.clear();
  while (std::getline(iInputFile, iTmpLine).good())
  {
    pos = iTmpLine.find_first_of(",");
    str1 = iTmpLine.substr(0, pos);
    str2 = iTmpLine.substr(pos + 1, std::string::npos);
    idx_tmp = atoi(str2.c_str());
    //std::cout<<"iTmpLine:"<<iTmpLine<<" str1:"<<str1<<" str2:"<<str2<<" str2.c_str():"<<str2.c_str()<<std::endl;

    iAllAreaMap.insert( std::pair<std::string,int>(str1, idx_tmp) );
  }

  iInputFile.close();

  // Log iAllAreaMap.
  iFilePath = GetConfig().iLogPath + OUT_PATH_S02_LOG_AREA_NAME_FILE;
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
  iFilePath = GetConfig().iLogPath + OUT_PATH_S02_LOG_MAP_PLUS_AREA_NAME;
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
  std::cout << "RemoveUnwantTokens" << std::endl;

  // Stage 2.2: Remove "unwant tokens" from {TC|SC}\input\_removed_tokens_table.txt from iTokenMap.
  //------------------------------------------------------------------------------------------------
  printf("Stage 2.2: Remove \"unwant tokens\" from {TC|SC}\\input\\_removed_tokens_table.txt from iTokenMap.\n");

  std::map<std::string,int> unwantTokensMap;
  std::string str1;
  std::size_t pos = 0;

  // Load unwant token list from file to unwantTokensMap.
  iFilePath = GetConfig().iInputPath + GetConfig().iInputRemoveToken;
  iInputFile.open(iFilePath.c_str(), std::ifstream::in);
  if (!iInputFile.is_open())
  {
    // show message:
    std::cout<<"Error opening file: "<<iFilePath<<std::endl;
    return false;
  }

  unwantTokensMap.clear();
  while (std::getline(iInputFile, iTmpLine).good())
  {
    if (0 == iTmpLine.length())
      continue;
    if ((iTmpLine[0] == '#') || (iTmpLine[0] == ';'))
      continue;

    pos = iTmpLine.find_first_of(" \n\r\0");
    str1 = iTmpLine.substr(0, pos);
    std::cout<<"iTmpLine:"<<iTmpLine<<" str1:"<<str1<<std::endl;

    unwantTokensMap.insert( std::pair<std::string,int>(str1, 1) );
  }

  iInputFile.close();

  // Log unwantTokensMap.
  iFilePath = GetConfig().iLogPath + OUT_PATH_S02_LOG_REMOVE_TOK_FILE;
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
      iTokenMap.erase(it->first);       // erasing by key
    }
  }

  // Log  iTokenMap with missing tokens in {TC|SC}\input\areas\05_unwant_tokens_map02.txt.
  iFilePath = GetConfig().iLogPath + OUT_PATH_S02_LOG_MAP_AFTER_REMOVE_TOK;
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
  std::cout << "AddMissingOneWordTokens" << std::endl;

  // Stage 3: Add "missing one word tokens" from iTokenMap into iTokenMap_with_one_word.
  //------------------------------------------------------------------------------------------------
  printf("Stage 3: Add \"missing one word tokens\" from iTokenMap into iTokenMap_with_one_word.\n");

  // Copy iTokenMap to iTokenMap_with_one_word.
  for (std::map<std::string,int>::iterator it = iTokenMap.begin(); it != iTokenMap.end(); ++it)
  {
    iTokenMapWithOneWord.insert( std::pair<std::string,int>(it->first, it->second) );
  }

  // Add one word tokens from original tokens.
  char tmpWord[6];
  int wordIdx;
  iFilePath = GetConfig().iLogPath + OUT_PATH_S03_CH_ONE_WORD;
  iOutputFile.open(iFilePath.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = iTokenMap.begin(); it != iTokenMap.end(); ++it)
  {
    // break token into words and add them.
    strcpy(iTmpBuffer, it->first.c_str());
    strtok(iTmpBuffer,"\n\r");
    wordIdx = 0;
    while (GetOneWordInToken(iTmpBuffer, tmpWord, sizeof(tmpWord), wordIdx++))
    {
      if (iTokenMapWithOneWord.end() == iTokenMapWithOneWord.find(tmpWord))
      {
        // Word not found, add to map.
        iTokenMapWithOneWord.insert(std::pair<std::string,int>(tmpWord, ++iTokenIndex));
        //printf("Added One Word: %s|%d\n", tmpWord, iTokenIndex);
        iOutputFile<<tmpWord<<"|"<<iTokenIndex<<std::endl;
      }
    }
  }
  iOutputFile.close();

  // Log iTokenMapWithOneWord.
  iFilePath = GetConfig().iLogPath + OUT_PATH_S03_MAP_WITH_ONE_WORDS;
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

  FILE *fp_s04_ch_full;
  FILE *fp_s04_ch_part;
  FILE *fp_s04_ch_not_found;
  size_t readSize = 0;
  
  iFilePath = GetConfig().iLogPath + OUT_PATH_S04_CH_FULL;
  fp_s04_ch_full = fopen(iFilePath.c_str(), "w");
  if (NULL == fp_s04_ch_full)
  {
    printf("fp_s04_ch_full err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fp_s04_ch_full:%s\n", iFilePath.c_str());

  iFilePath = GetConfig().iLogPath + OUT_PATH_S04_CH_PART;
  fp_s04_ch_part = fopen(iFilePath.c_str(), "w");
  if (NULL == fp_s04_ch_part)
  {
    printf("fp_s04_ch_part err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fp_s04_ch_part:%s\n", iFilePath.c_str());

  iFilePath = GetConfig().iLogPath + OUT_PATH_S04_CH_NOT_FOUND;
  fp_s04_ch_not_found = fopen(iFilePath.c_str(), "w");
  if (NULL == fp_s04_ch_not_found)
  {
    printf("fp_s04_ch_not_found err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fp_s04_ch_not_found:%s\n", iFilePath.c_str());

  // get xdb header
  iFilePath = GetConfig().iInputPath + GetConfig().iInputScwsXdb;
  iFileXdb = fopen(iFilePath.c_str(), "rb");
  if( NULL == iFileXdb )
  if (NULL == iFileXdb)
  {
    printf("iFileXdb err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("iFileXdb:%s\n", iFilePath.c_str());

  readSize = fread(&iXdbHeader, 1, sizeof(xdb_header), iFileXdb);
  if (readSize != sizeof(xdb_header)*1)
  {
    fputs("Reading error 13", stderr); exit(EXIT_FAILURE);
  }

#if 1
  // Start to parse each token in iTokenMap.
  for (std::map<std::string,int>::iterator it = iTokenMapWithOneWord.begin(); it != iTokenMapWithOneWord.end(); ++it)
  {
    // Search this token in xdb.
    strcpy(iTmpBuffer, it->first.c_str());
    strtok(iTmpBuffer,"\n\r");
    memset(&iNodeInfoAttr, 0, sizeof(iNodeInfoAttr));

    if( SEARCH_TOKEN_INFO_FOUND == SearchTokenInfo(iTmpBuffer, -1, -1, iXdbHeader.base, iXdbHeader.prime, &iNodeInfoAttr, iFileXdb))
    {
      // Save only full tokens.
      if( 2 == iNodeInfoAttr.flag)
      {
        fprintf(fp_s04_ch_part, "PART:%s\t\%4.2f\t%4.2f\t%d\t%s\n", iTmpBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr);
      }
      else
      {
        fprintf(fp_s04_ch_full, "%s\t\%4.2f\t%4.2f\t%d\t%s\n", iTmpBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr  );
      }
    } 
    else
    {
      fprintf(fp_s04_ch_not_found, "NOT_FOUND:%s\t\%4.2f\t%4.2f\t%d\t%s\n", iTmpBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr  );
    }
  }
#endif

  fclose(iFileXdb);
  fclose(fp_s04_ch_full);
  fclose(fp_s04_ch_part);
  fclose(fp_s04_ch_not_found);
  //------------------------------------------------------------------------------------------------

  return true;
}

bool CXdbFilter::RemoveSpecialSuffixTokens()
{
  std::cout << "RemoveSpecialSuffixTokens" << std::endl;

  // Stage 5: Start to build iTokenMapOpt by parsing special suffix of each token in iTokenMapWithOneWord.
  //------------------------------------------------------------------------------------------------
  printf("Stage 5: Start to build iTokenMapOpt by parsing special suffix of each token in iTokenMapWithOneWord.\n");

  FILE *fp_out_ch_parse2_suffix_log;
  int maxSuffixTokenLength = 0;
  int SuffixOff;
  char rem = 0;
  std::string tmpStr;

#ifdef REMOVE_FULL_TOKENS_WITH_SPECIAL_END
  FILE *fp_out_log_removed_special_end_tok;

  iFilePath = GetConfig().iLogPath + OUT_PATH_S05_REMOVED_SPECIAL_END_TOKENS;
  fp_out_log_removed_special_end_tok = fopen(iFilePath.c_str(), "w");
  if (NULL == fp_out_log_removed_special_end_tok)
  {
    printf("fp_out_log_removed_special_end_tok err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fp_out_log_removed_special_end_tok:%s\n", iFilePath.c_str());
#endif

  iFilePath = GetConfig().iLogPath + OUT_PATH_S05_ADDED_REMOVED_SUFFIX;
  fp_out_ch_parse2_suffix_log = fopen(iFilePath.c_str(), "w");
  if (NULL == fp_out_ch_parse2_suffix_log)
  {
    printf("fp_out_ch_parse2_suffix_log err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fp_out_ch_parse2_suffix_log:%s\n", iFilePath.c_str());

  maxSuffixTokenLength= GetMaxSuffixTokenLength(iSuffixTokenMap);

  iFilePath = GetConfig().iLogPath + OUT_PATH_S05_FOUND_SUFFIX;
  iOutputFile.open(iFilePath.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = iTokenMapWithOneWord.begin(); it != iTokenMapWithOneWord.end(); ++it)
  {
    //std::cout<<it->first<<std::endl;
    SuffixOff = 0;

    // We will add this full token to iTokenMapOpt.
    iTokenMapOpt.insert( std::pair<std::string,int>(it->first, it->second) );

    // The full token in <iTokenMapWithOneWord> already added to <iTokenMapOpt>, 
    // now we will tell if this full token is a special-suffix-tokens, and will have some optimzed operations as below: 
    // 1. Remove special suffix.
    // 2. Check if this removed-suffix-token already exist in <iTokenMapWithOneWord>, 
    //     if YES we can just bypass to next token, if NO, user can choose if he want to add this removed-suffix-token to <iTokenMapOpt> or not.
    //
    if (IsTokenEndWithIgnoredSuffix(it->first.c_str(), &SuffixOff, iTmpBuffer, maxSuffixTokenLength, iSuffixTokenMap))
    {
      // Log FoundIgnoredSuffixToken.
      //std::cout<<"FoundIgnoredSuffixToken:["<<it->first<<"] SuffixOff:"<<SuffixOff<<" Suffix:["<<it->first.c_str()+SuffixOff<<"]"<<std::endl;
      iOutputFile<<"FoundIgnoredSuffixToken:["<<it->first<<"] SuffixOff:"<<SuffixOff<<" Suffix:["<<it->first.c_str()+SuffixOff<<"]"<<std::endl;

    #ifdef REMOVE_FULL_TOKENS_WITH_SPECIAL_END
      // Just test if this token end with special word "路".
      tmpStr = it->first;
      std::string lastword = tmpStr.substr(tmpStr.length()-3);
      //std::cout<<"lastword:["<<lastword<<"]"<<std::endl;
      //"路"'s UTF-8 code = E8 B7 AF
      std::string specialword;
      specialword.resize(3);
      specialword[0] = 0xE8;
      specialword[1] = 0xB7;
      specialword[2] = 0xAF;
      //std::cout<<"specialword:["<<specialword<<"]"<<std::endl;
      if (lastword == specialword)
      {
        //std::cout<<"["<<tmpStr<<"] end with special word ["<<specialword<<"]"<<std::endl;

        //remove this token from iTokenMapOpt.
        iTokenMapOpt.erase(it->first);       // erasing by key
        std::cout<<"Remove special["<<it->first<<"]["<<it->second<<"] from iTokenMapOpt!"<<std::endl;
        fprintf(fp_out_log_removed_special_end_tok, "Remove special end token [%s][%d] from token_map_opt!\n", it->first.c_str(), it->second);
      }
    #endif //REMOVE_FULL_TOKENS_WITH_SPECIAL_END

      // Remove suffix.
      tmpStr = it->first;
      tmpStr.erase(SuffixOff, std::string::npos);
    #ifdef ENABLE_USER_INTERACTIVE_CONTROL //Ask user.
      std::cout<<"FoundIgnoredSuffixToken:["<<it->first<<"]"<<" AfterRemoveSuffix:["<<tmpStr<<"]"<<std::endl;
    #endif

      // Is the removed-suffix-token exist in iTokenMapWithOneWord?
      if (iTokenMapWithOneWord.end() != iTokenMapWithOneWord.find(tmpStr)) // Removed-suffix-token ALREADY in iTokenMapWithOneWord!
      {
        //Add to iTokenMapOpt.
        iTokenMapOpt.insert( std::pair<std::string,int>(tmpStr, it->second) );

        //std::cout<<"["<<tmpStr<<"] alread in map"<<std::endl;
        fprintf(fp_out_ch_parse2_suffix_log, "Removed-suffix-token ALREADY in old map:[%s] and [%s] ADDED to new map!\n", tmpStr.c_str(), it->first.c_str());
      #ifdef ENABLE_USER_INTERACTIVE_CONTROL //Notify user.
        std::cout<<"Removed-suffix-token ALREADY in old map:["<<tmpStr<<"] and ["<<it->first<<"] ADDED to new map!"<<std::endl;
      #endif
      }
      else // Removed-suffix-token NOT in map.
      {
        //std::cout<<"["<<tmpStr<<"] NOT in map"<<std::endl;
      #ifdef ENABLE_USER_INTERACTIVE_CONTROL //Ask user.
        std::cout<<"Removed-suffix-token NOT in old map:["<<tmpStr<<"] "<<std::endl;

        std::cout<<"Enter 'y' to add both ["<<tmpStr<<"] and ["<<it->first<<"], otherwise will just add ["<<it->first<<"]. (y/n):";
        std::cin>>rem;
      #else //Force add.
        rem = 'y';
      #endif

        //fprintf(fp_out_ch_parse2_suffix_log, "Removed-suffix-token NOT in old map:[%s] ", tmpStr.c_str());
        if ('y' == rem)
        {
          iTokenMapOpt.insert(std::pair<std::string,int>(tmpStr, ++iTokenIndex));
          fprintf(fp_out_ch_parse2_suffix_log, "Removed-suffix-token NOT in old map:[%s] and [%s] ADDED to new map!\n", tmpStr.c_str(), it->first.c_str());
        #ifdef ENABLE_USER_INTERACTIVE_CONTROL //Ask user.
          std::cout<<"SUCCESSFULLY ADDED ["<<tmpStr<<"] and ["<<it->first<<"] to new map!"<<std::endl;
        #endif
        }
        else
        {
          // skip this token.
          fprintf(fp_out_ch_parse2_suffix_log, "Removed-suffix-token NOT in old map: SKIP ADDING [%s], RESERVED [%s] to new map\n", tmpStr.c_str(), it->first.c_str());
        #ifdef ENABLE_USER_INTERACTIVE_CONTROL //Ask user.
          std::cout<<"Removed-suffix-token NOT in old map: SKIP ADDING ["<<tmpStr<<"], RESERVED ["<<it->first<<"] to new map"<<std::endl;
        #endif
          continue;
        }
      }
    } //IsTokenEndWithIgnoredSuffix
  }
  iOutputFile.close();

  // Log optimized iTokenMapOpt.
  iFilePath = GetConfig().iLogPath + OUT_PATH_S05_MAP_OPTIMIZED;
  iOutputFile.open(iFilePath.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = iTokenMapOpt.begin(); it != iTokenMapOpt.end(); ++it)
  {
    iOutputFile << it->first << "|" << it->second << '\n';
  }
  iOutputFile.close();
  fclose(fp_out_ch_parse2_suffix_log);
#ifdef REMOVE_FULL_TOKENS_WITH_SPECIAL_END
  fclose(fp_out_log_removed_special_end_tok);
#endif
  //------------------------------------------------------------------------------------------------

  return true;
}

bool CXdbFilter::RetrieveTokenInfo()
{
  std::cout << "RetrieveTokenInfo" << std::endl;

  // Stage 6: Retreive every optimized map item's info from xdb.
  //------------------------------------------------------------------------------------------------
  printf("Stage 6: Retreive every optimized map item's info from xdb.\n");

  FILE *fp_out_s06_suffix_full;
  FILE *fp_out_s06_suffix_part;
  FILE *fp_out_s06_suffix_not_found;
  size_t readSize = 0;

   // get xdb header
  iFilePath = GetConfig().iInputPath + GetConfig().iInputScwsXdb;
  iFileXdb = fopen(iFilePath.c_str(), "rb");
  if( NULL == iFileXdb )
  {
    return false;
  }

  readSize = fread(&iXdbHeader, 1, sizeof(xdb_header), iFileXdb);
  if (readSize != sizeof(xdb_header)*1)
  {
    fputs("Reading error 14", stderr); exit(EXIT_FAILURE);
  }

  iFilePath = GetConfig().iOutputPath + OUT_PATH_S06_SUFFIX_FULL;
  fp_out_s06_suffix_full = fopen(iFilePath.c_str(), "w");
  if (NULL == fp_out_s06_suffix_full)
  {
    printf("fp_out_s06_suffix_full err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fp_out_s06_suffix_full:%s\n", iFilePath.c_str());

  iFilePath = GetConfig().iLogPath + OUT_PATH_S06_SUFFIX_PART;
  fp_out_s06_suffix_part = fopen(iFilePath.c_str(), "w");
  if (NULL == fp_out_s06_suffix_part)
  {
    printf("fp_out_s06_suffix_part err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fp_out_s06_suffix_part:%s\n", iFilePath.c_str());

  iFilePath = GetConfig().iLogPath + OUT_PATH_S06_SUFFIX_NOT_FOUND;
  fp_out_s06_suffix_not_found = fopen(iFilePath.c_str(), "w");
  if (NULL == fp_out_s06_suffix_not_found)
  {
    printf("fp_out_s06_suffix_not_found err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fp_out_s06_suffix_not_found:%s\n", iFilePath.c_str());

  // Start to parse each token in iTokenMapOpt.
  for (std::map<std::string,int>::iterator it = iTokenMapOpt.begin(); it != iTokenMapOpt.end(); ++it)
  {
    // Search this token in xdb.
    strcpy(iTmpBuffer, it->first.c_str());
    strtok(iTmpBuffer,"\n\r");
    if (SEARCH_TOKEN_INFO_FOUND == SearchTokenInfo(iTmpBuffer, -1, -1, iXdbHeader.base, iXdbHeader.prime, &iNodeInfoAttr, iFileXdb))
    {
      if (2 == iNodeInfoAttr.flag)
      {
        fprintf(fp_out_s06_suffix_part, "PART:%s\t\%4.2f\t%4.2f\t%d\t%s\n", iTmpBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr  );

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

          fprintf(fp_out_s06_suffix_full, "%s\t\%4.2f\t%4.2f\t%d\t%s     111\n", iTmpBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr  );
        }
        else
        {
          fprintf(fp_out_s06_suffix_full, "%s\t\%4.2f\t%4.2f\t%d\t%s     !!!\n", iTmpBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr  );
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

          fprintf(fp_out_s06_suffix_full, "%s\t\%4.2f\t%4.2f\t%d\t%s     222\n", iTmpBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr  );
        }
        else
        {
          fprintf(fp_out_s06_suffix_full, "%s\t\%4.2f\t%4.2f\t%d\t%s\n", iTmpBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr  );
        }
      }
    }
    else
    {
      fprintf(fp_out_s06_suffix_not_found, "NOT_FOUND:%s\n", iTmpBuffer);

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

        fprintf(fp_out_s06_suffix_full, "%s\t\%4.2f\t%4.2f\t%d\t%s     333\n", iTmpBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr  );
      }
      else
      {
        fprintf(fp_out_s06_suffix_full, "%s\t\%4.2f\t%4.2f\t%d\t%s     !!!\n", iTmpBuffer, iNodeInfoAttr.tf, iNodeInfoAttr.idf, iNodeInfoAttr.flag, iNodeInfoAttr.attr  );
      }
    }
  }

  fclose(fp_out_s06_suffix_full);
  fclose(fp_out_s06_suffix_part);
  fclose(fp_out_s06_suffix_not_found);
  fclose(iFileXdb);
  //------------------------------------------------------------------------------------------------

  return true;
}

bool CXdbFilter::ConvertToNormalizedTokens()
{
  std::cout << "ConvertToNormalizedTokens" << std::endl;

  // Stage 7: Convert results to normalized form.
  //------------------------------------------------------------------------------------------------
  printf("Stage 7: Convert results to normalized form.\n");

  FILE *fp_in_s07_optimized_full;
  FILE *fp_out_s07_normalized_full;
  int szLine_Len;
  char* pKey=0;

  int iEnableHPNormalize = 0;
  std::string iNormText;
  int lineNumber = 0;

  iFilePath = GetConfig().iInputPath + GetConfig().iInputNormalizeMap;
  if (!CHomophoneNormalizer_Init(iFilePath.c_str()))
  {
    printf("CHomophoneNormalizer_Init err:%s\n", iFilePath.c_str());
    iEnableHPNormalize = 0;
  }
  else
  {
    printf("CHomophoneNormalizer_Init OK\n");
    iEnableHPNormalize = 1;
  }

  iFilePath = GetConfig().iOutputPath + OUT_PATH_S06_SUFFIX_FULL;
  fp_in_s07_optimized_full = fopen(iFilePath.c_str(), "r");
  if (NULL == fp_in_s07_optimized_full)
  {
    printf("fp_in_s07_optimized_full err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fp_in_s07_optimized_full:%s\n", iFilePath.c_str());

  iFilePath = GetConfig().iOutputPath + OUT_PATH_S07_SUFFIX_FULL_NOR;
  fp_out_s07_normalized_full = fopen(iFilePath.c_str(), "w");
  if (NULL == fp_out_s07_normalized_full)
  {
    printf("fp_out_s07_normalized_full err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fp_out_s07_normalized_full:%s\n", iFilePath.c_str());

  // Start converting.
  iNormText.resize(32);
  while (fgets(iBuffer, sizeof(iBuffer), fp_in_s07_optimized_full))
  {
    lineNumber++;
    if (0 == lineNumber % 1000)
    {
      printf("Converting Line(%d)....\n", lineNumber );
    }

    if ((iBuffer[0] == '#') || (iBuffer[0] == ';'))
    {
      continue;
    }

    strncpy(iTmpBuffer, iBuffer, sizeof(iTmpBuffer));
    szLine_Len = strlen(iTmpBuffer);

    if( 0 == szLine_Len )
    {
      continue;
    }

    pKey = strtok(iTmpBuffer, "\t ");
    if (pKey) 
    {
      //printf("Converting [%s] of %s", pKey, iBuffer );

      // Normalizer
      //iNormText.resize(32);
      if(iEnableHPNormalize)
      {
        size_t res_len = CHomophoneNormalizer_Normalize(pKey, &iNormText[0], iNormText.capacity());

        if(res_len != 0) // Buffer size is too small
        {
          //printf("@@@ Buffer size is too small, pKey length=%d,  iNormText.size()=%d,  iNormText.capacity()=%d\n", res_len, iNormText.size(), iNormText.capacity());
          CFtsTokenizerExtChinese_ReserveStringCapacity(iNormText, res_len, KNormBufUnitSize);

          if((res_len = CHomophoneNormalizer_Normalize(pKey, &iNormText[0], iNormText.capacity())))
          {
            printf("Normalize error=%ld\n", res_len);
            return false;
          }
        }
        //printf("iNormText.c_str():%s\n", iNormText.c_str());
        //printf("strlen(iNormText.c_str()):%d\n", strlen(iNormText.c_str()));

        //printf("AAA Un-Normali aString:%s", iBuffer);
        strncpy(iBuffer, iNormText.c_str(), strlen(iNormText.c_str()));
        //printf("BBB Normalized aString:%s", iBuffer);

        //log to file.
        fprintf(fp_out_s07_normalized_full, "%s", iBuffer);
      }
    }
    else
    {
      printf("Failed converting:%s", iBuffer );
    }
  }
  printf("Total converting Line(%d)....\n", lineNumber );

  fclose(fp_in_s07_optimized_full);
  fclose(fp_out_s07_normalized_full);
  //------------------------------------------------------------------------------------------------

  return true;
}

bool CXdbFilter::MergeToFuzzyTokens()
{
  std::cout << "MergeToFuzzyTokens" << std::endl;

  // Stage 8: Merge TokenList and TokenListNormalized as TokenListFuzzy
  //------------------------------------------------------------------------------------------------
  FILE *fp_fuzzy=NULL;
  FILE *fp_tokenList=NULL;
  FILE *fp_tokenListNorm=NULL;

  iFilePath = GetConfig().iOutputPath + OUT_PATH_S08_SUFFIX_FULL_FUZZY;
  fp_fuzzy = fopen(iFilePath.c_str(), "w");
  if (NULL == fp_fuzzy)
  {
    printf("fp_fuzzy err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fp_fuzzy:%s\n", iFilePath.c_str());

  iFilePath = GetConfig().GetOutputPath() + GetConfig().GetOutputTokenList();
  fp_tokenList = fopen(iFilePath.c_str(), "r");
  if(NULL == fp_tokenList)
  {
    printf("Error open fp_tokenList: %s\n", iFilePath.c_str());
    return false;
  }
  printf("fp_tokenList:%s\n", iFilePath.c_str());

  iFilePath = GetConfig().GetOutputPath() + GetConfig().GetOutputTokenListNormalized();
  fp_tokenListNorm = fopen(iFilePath.c_str(), "r");
  if(NULL == fp_tokenListNorm)
  {
    printf("Error open fp_tokenListNorm: %s\n", iFilePath.c_str());
    return false;
  }
  printf("fp_tokenListNorm:%s\n", iFilePath.c_str());

  while ( NULL != fgets(iBuffer, sizeof(iBuffer), fp_tokenList))
  {
    fputs(iBuffer, fp_fuzzy);
  }
  fclose(fp_tokenList);

  while (NULL != fgets(iBuffer, sizeof(iBuffer), fp_tokenListNorm))
  {
    fputs(iBuffer, fp_fuzzy);
  }

  fclose(fp_tokenListNorm);
  fclose(fp_fuzzy);
  //------------------------------------------------------------------------------------------------

  return true;
}
////////////////////// Search Node /////////////////////////////////////
int CXdbFilter::GetHashIndex( const unsigned char*  aKey,
                              int                   aHashBase,
                              int                   aHashPrime) const
{
  int l = strlen((char*)aKey);
  int h = aHashBase;

  while (l--)
  {
    h += (int)(h << 5);
    h ^= aKey[l];
    h &= 0x7fffffff;
  }

  return (h % aHashPrime);
}

int CXdbFilter::SearchTokenInfo(const char*     aTokenContent,
                                long            aNodeOffset,
                                long            aNodeLength,
                                int             aHashBase,
                                int             aHashPrime,
                                TNodeInfoAttr*  aAttribute,
                                FILE*           aFileXdb)
{
  int hash_index;
  //fpos_t f_offset;
  long f_offset;
  TPrimeNode pr_node;
  TNodeInfoHeader nodeInfoHeader;
  char szBuff[300];
  int cmp_result;
  size_t readSize = 0;

  unsigned int l_offset, l_len;
  unsigned int r_offset, r_len;
  char k_len;

  if (NULL == aFileXdb)
  {
    printf("SearchTokenInfoaFileXdb err\n");
    return SEARCH_TOKEN_INFO_NOT_FOUND;
  }

  if ((aNodeOffset < 0) && (aNodeLength < 0))
  {
    hash_index = GetHashIndex((unsigned char*)aTokenContent, aHashBase, aHashPrime);

    int t = sizeof(xdb_header);
    int g = sizeof(TPrimeNode);

    f_offset = sizeof(xdb_header)+hash_index*sizeof(TPrimeNode);
    //fsetpos(aFileXdb, &f_offset);
    fseek(aFileXdb, f_offset, SEEK_SET);
    readSize = fread(&pr_node, 1, sizeof(TPrimeNode), aFileXdb);
    if (readSize != sizeof(TPrimeNode) * 1)
    {
      fputs("Reading error 01", stderr); exit(EXIT_FAILURE);
    }

    aNodeOffset = pr_node.offset;
    aNodeLength = pr_node.length;
  }

  // read node
  f_offset = aNodeOffset;
  fseek(aFileXdb, f_offset, SEEK_SET);

  readSize = fread(&nodeInfoHeader.l_offset, 1, sizeof(unsigned int), aFileXdb);
  if (readSize != sizeof(unsigned int) * 1)
  {
    fputs("Reading error 02", stderr); exit(EXIT_FAILURE);
  }
  readSize = fread(&nodeInfoHeader.l_length, 1, sizeof(unsigned int), aFileXdb);
  if (readSize != sizeof(unsigned int) * 1)
  {
    fputs("Reading error 03", stderr); exit(EXIT_FAILURE);
  }
  readSize = fread(&nodeInfoHeader.r_offset, 1, sizeof(unsigned int), aFileXdb);
  if (readSize != sizeof(unsigned int) * 1)
  {
    fputs("Reading error 04", stderr); exit(EXIT_FAILURE);
  }
  readSize = fread(&nodeInfoHeader.r_length, 1, sizeof(unsigned int), aFileXdb);
  if (readSize != sizeof(unsigned int) * 1)
  {
    fputs("Reading error 05", stderr); exit(EXIT_FAILURE);
  }
  readSize = fread(&nodeInfoHeader.k_length, 1, sizeof(unsigned char), aFileXdb);
  if (readSize != sizeof(unsigned char) * 1)
  {
    fputs("Reading error 06", stderr); exit(EXIT_FAILURE);
  }

  memset(szBuff, 0, sizeof(szBuff) );
  readSize = fread(szBuff, 1, nodeInfoHeader.k_length, aFileXdb);
  if (readSize != nodeInfoHeader.k_length * 1)
  {
    fputs("Reading error 07", stderr); exit(EXIT_FAILURE);
  }

  cmp_result = strcmp(szBuff, aTokenContent);

  if (0 == cmp_result)
  {
    readSize = fread( &(aAttribute->tf), 1, sizeof(float), aFileXdb);
    if (readSize != sizeof(float) * 1)
    {
      fputs("Reading error 08", stderr); exit(EXIT_FAILURE);
    }
    readSize = fread( &(aAttribute->idf), 1, sizeof(float), aFileXdb);
    if (readSize != sizeof(float) * 1)
    {
      fputs("Reading error 09", stderr); exit(EXIT_FAILURE);
    }
    readSize = fread( &(aAttribute->flag), 1, sizeof(unsigned char), aFileXdb);
    if (readSize != sizeof(unsigned char) * 1)
    {
      fputs("Reading error 10", stderr); exit(EXIT_FAILURE);
    }
    readSize = fread( &(aAttribute->attr), sizeof(unsigned char), 3, aFileXdb);
    if (readSize != sizeof(unsigned char) * 3)
    {
      fputs("Reading error 11", stderr); exit(EXIT_FAILURE);
    }
    //readSize = fread(aAttribute, 1, sizeof(TNodeInfoAttr), aFileXdb);
    //if (readSize != sizeof(TNodeInfoAttr) * 1)
    //{
    //  fputs("Reading error 12", stderr); exit(EXIT_FAILURE);
    //}
    return SEARCH_TOKEN_INFO_FOUND;
  }
  else if (cmp_result > 0)
  {
    if ((0 == nodeInfoHeader.l_offset) && (0 == nodeInfoHeader.l_length))
    {
      return SEARCH_TOKEN_INFO_NOT_FOUND; // no node
    }

    return SearchTokenInfo(aTokenContent, nodeInfoHeader.l_offset, nodeInfoHeader.l_length, aHashBase, aHashPrime, aAttribute, aFileXdb);
  }
  else
  {
    if( (0 == nodeInfoHeader.r_offset) && (0 == nodeInfoHeader.r_length) )
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

int CXdbFilter::ConvUTF8ToUTF16(unsigned char* aUtf8CodeData,
                                unsigned int* aData,
                                int aDataLength)
{
  unsigned char* pUTF8_CODE;
  unsigned char  UTF8_CODE;
  unsigned char  UTF8_CODE_NEXT[2];
  unsigned char  hiByte, loByte;
  int result_length;

  if (NULL == aUtf8CodeData)
  {
    return false;
  }

  if (NULL == aData)
  {
    return false;
  }

  if (0 == aDataLength)
  {
    return false;
  }

  pUTF8_CODE = aUtf8CodeData;
  result_length = 0;

  while (0x00 != (UTF8_CODE = *pUTF8_CODE))
  {
    if ((UTF8_CODE > 0x00) && (UTF8_CODE <= 0x7F))
    {
      // 000000 - 00007F
      aData[result_length++] = (unsigned int)UTF8_CODE;
    }
    else if ((UTF8_CODE >= 0xC0) && (UTF8_CODE <= 0xDF))
    {
      // 110yyyyy(C0-DF) 10zzzzzz(80-BF)  UTF-8
      // 00000yyy yyzzzzzz(UTF-16)
      UTF8_CODE_NEXT[0] = *(pUTF8_CODE+1);
      if ((UTF8_CODE_NEXT[0] >= 0x80) && (UTF8_CODE_NEXT[0] <= 0xBF))
      {
        hiByte = (UTF8_CODE & 0x1C) >> 2;
        loByte = ((UTF8_CODE & 0x03) << 6) |(UTF8_CODE_NEXT[0] & 0x3f);
        aData[result_length++] = ((unsigned int)hiByte << 8) | ((unsigned int)loByte);
        pUTF8_CODE++;
      }
      else
      {
        aData[result_length++] = (unsigned int)UTF8_CODE;
      }
    }
    else if ((UTF8_CODE >= 0xE0) && (UTF8_CODE <= 0xEF))
    {
      // 1110xxxx(E0-EF)  10yyyyyy(80-BF) 10zzzzzz(80-BF) UTF-8
      // xxxxyyyy yyzzzzzz(UTF-16)
      UTF8_CODE_NEXT[0] = *(pUTF8_CODE+1);
      UTF8_CODE_NEXT[1] = *(pUTF8_CODE+2);

      if (((UTF8_CODE_NEXT[0] >= 0x80) && (UTF8_CODE_NEXT[0] <= 0xBF)) && ((UTF8_CODE_NEXT[1] >= 0x80) && (UTF8_CODE_NEXT[1] <= 0xBF)))
      {
        hiByte = ((UTF8_CODE & 0x0f) << 4) | ((UTF8_CODE_NEXT[0] & 0x3C) >> 2);
        loByte = ((UTF8_CODE_NEXT[0] & 0x03) << 6) | (UTF8_CODE_NEXT[1] & 0x3F);
        aData[result_length++] = ((unsigned int)hiByte << 8) | ((unsigned int)loByte);
        pUTF8_CODE++;
        pUTF8_CODE++;
      }
      else
      {
        aData[result_length++] = (unsigned int)UTF8_CODE;
      }
    }
    else
    {
      aData[result_length++] = (unsigned int)UTF8_CODE;
    }

    pUTF8_CODE++;
  }

  return result_length;
}

int CXdbFilter::IsAllChineseToken(const char* aTokenContent,
                                  int aTokenLen)
{
  int idx = 0;
  int kind;
  int retLength;
  unsigned int utf16_data[10];

  if (3 == aTokenLen)
  {
    retLength = ConvUTF8ToUTF16((unsigned char*)aTokenContent, utf16_data, 10);

    if (1 == retLength)
    {
      // check for symbol
      if ((utf16_data[0] <= 0x3400) || (utf16_data[0] >= 0xFAD9))
      {
        return false;
      }
    }
  }

  while (idx < aTokenLen)
  {
    kind = GetUTF8CharKind((unsigned char)aTokenContent[idx]);

    if (UTF8_CHAR_KIND_3 != kind)
    {
      return false;
    }

    idx += kind;
  }

  return true;
}

int CXdbFilter::IsValidChineseToken(scws_res_t  aScwsCur,
                                    const char* aTokenContent)
{
  if (aScwsCur->len < 3)
  {
    return false;
  }

  if (0 == strcasecmp(aScwsCur->attr, "en"))
  {
    return false;
  }

  if ((0 == strcasecmp(aScwsCur->attr, "un")) || (0 == strcasecmp(aScwsCur->attr, "nz")))
  {
    if (0 == IsAllChineseToken(aTokenContent, aScwsCur->len))
    {
      return false;
    }
  }

  return true;
}

int CXdbFilter::GetTokenTotalChineseWordCount(const char* aString)
{
  int aTokenLen = 0;
  int idx = 0;
  int kind = 0;
  int TotalChineseWordCount = 0;

  TotalChineseWordCount = 0;
  aTokenLen = strlen(aString);

  if (0 == aTokenLen)
  {
    TotalChineseWordCount = 0;
    return TotalChineseWordCount;
  }

  while (idx < aTokenLen)
  {
    kind = GetUTF8CharKind((unsigned char)aString[idx]);

    if (UTF8_CHAR_KIND_3 != kind)
    {
      return TotalChineseWordCount;
    }

    TotalChineseWordCount++;
    idx += kind;
  }

  //printf("GetTokenTotalChineseWordCount[%s]=%d\n", aString, TotalChineseWordCount);
  return TotalChineseWordCount;
}

int CXdbFilter::GetNthChineseWordByteOffset(const char* aString,
                                            int aNthChineseWord)
{
  int aTokenLen = 0;
  int idx = 0;
  int kind = 0;
  int TotalChineseWordCount = 0;

  TotalChineseWordCount = GetTokenTotalChineseWordCount(aString);

  if (aNthChineseWord> TotalChineseWordCount)
  {
    aNthChineseWord= TotalChineseWordCount;
  }

  aTokenLen = strlen(aString);

  if (0 == aTokenLen)
  {
    idx = 0;
    return idx;
  }
  
  while (idx < aTokenLen)
  {
    kind = GetUTF8CharKind((unsigned char)aString[idx]);

    if (UTF8_CHAR_KIND_3 != kind)
    {
      return (idx - UTF8_CHAR_KIND_3);
    }

    if ((idx / UTF8_CHAR_KIND_3) == aNthChineseWord)
    {
      //printf("GetNthChineseWordByteOffset: aString:[%s] Nth:[%d] offset:[%d]\n", aString, aNthChineseWord, idx);
      return idx;
    }
    idx += kind;
  }

  return idx;
}

////////////////////// SuffixTokenMap /////////////////////////////////////
int CXdbFilter::InitSuffixTokenMap(std::map<std::string,int>& aSuffixTokenMap)
{
  FILE *fp_table;
  std::string suffix_tok;
  int lineNumber = 0;
  int tok_no = 0;

  iFilePath = GetConfig().iInputPath + GetConfig().iInputSpecialSuffixTable;
  fp_table = fopen(iFilePath.c_str(), "r");
  if (NULL == fp_table)
  {
    printf("Error fopen suffix table file: %s\n", iFilePath.c_str());
    return false;
  }

  aSuffixTokenMap.clear();
  while (NULL != fgets(iBuffer, sizeof(iBuffer), fp_table))
  {
    lineNumber++;

    if (1 == lineNumber) // for skip BOM
    {
      //printf("1iBuffer[0]:0x%X ",iBuffer[0]);
      //printf("1iBuffer[1]:0x%X ",iBuffer[1]);
      //printf("1iBuffer[2]:0x%X\n",iBuffer[2]);
      if (((unsigned char)0xEF==(unsigned char)iBuffer[0]) &&
            ((unsigned char)0xBB==(unsigned char)iBuffer[1]) &&
            ((unsigned char)0xBF==(unsigned char)iBuffer[2]))
      {
        for (int i = 0; i < (strlen(iBuffer)-2); i++)
          iBuffer[i] = iBuffer[i+3];
      }
    }

    strtok(iBuffer,"\n\r");
    if (3 > strlen(iBuffer))
    {
      continue;
    }
    if ((iBuffer[0] == '#') || (iBuffer[0] == ';'))
    {
      continue;
    }

    //printf("SuffixTable lineNumber:%d = %s", lineNumber, iBuffer);
    //strtok(iBuffer,"\n\r");
    suffix_tok = iBuffer;
    if (aSuffixTokenMap.end() == aSuffixTokenMap.find(suffix_tok))
    {
      // Word not found, add to map.
      aSuffixTokenMap.insert( std::pair<std::string,int>(suffix_tok, ++tok_no) );
      //printf("add suffix_tok:%s|%d\n", suffix_tok.c_str(),tok_no);
    }
  }
  fclose(fp_table);

  return true;
}

int CXdbFilter::GetMaxSuffixTokenLength(std::map<std::string,int>& aSuffixTokenMap)
{
  int MaxLenInBytes = 0;
  int aTokenLen = 0;

  for (std::map<std::string,int>::iterator it = aSuffixTokenMap.begin(); it != aSuffixTokenMap.end(); ++it)
  {
    aTokenLen = it->first.length();
    if (aTokenLen > MaxLenInBytes)
    {
      MaxLenInBytes = aTokenLen;
    }
  }

  return MaxLenInBytes / UTF8_CHAR_KIND_3;
}
////////////////////// SuffixTokenMap /////////////////////////////////////

int CXdbFilter::IsTokenEndWithIgnoredSuffix(const char* aString,
                                            int* aSuffixOffset,
                                            char* aTmpBuffer,
                                            int aMaxSuffixTokenLength,
                                            std::map<std::string,int>& aSuffixTokenMap)
{
  int aTokenLen = 0;
  int TokenTotalChineseWordCount = 0;
  int SuffixWordCnt = 0;
  int NthChineseWordByteOffset;
  char* pStr;

  std::string SuffixStr;

  strcpy(aTmpBuffer, aString);
  strtok(aTmpBuffer,"\n\r");
  aTokenLen = strlen(aTmpBuffer);

  if (UTF8_CHAR_KIND_3 > aTokenLen)
  {
      return false;
  }

  TokenTotalChineseWordCount = GetTokenTotalChineseWordCount(aTmpBuffer);
  //printf("aString[%s] TokenTotalChineseWordCount[%d] aMaxSuffixTokenLength[%d]\n", aString, TokenTotalChineseWordCount, aMaxSuffixTokenLength);

  if (aMaxSuffixTokenLength >= TokenTotalChineseWordCount)
  {
    aMaxSuffixTokenLength = TokenTotalChineseWordCount - 1;
  }
  if (0 == aMaxSuffixTokenLength)
  {
    return false;
  }

  for (SuffixWordCnt = aMaxSuffixTokenLength; SuffixWordCnt > 0; SuffixWordCnt--)
  {
    NthChineseWordByteOffset = GetNthChineseWordByteOffset(aTmpBuffer, (TokenTotalChineseWordCount - SuffixWordCnt));
    pStr = aTmpBuffer+ NthChineseWordByteOffset;
    //printf("SuffixWordCnt[%d] NthChineseWordByteOffset[%d] pStr[%s]\n", SuffixWordCnt, NthChineseWordByteOffset, pStr);

    // Compare pStr with all words(with length SuffixWordCnt) in _ignored_suffix_table_{tc|sc}_utf8.txt.
    SuffixStr = pStr;
    if (aSuffixTokenMap.end() != aSuffixTokenMap.find(SuffixStr))
    {
      // Found suffix!
      *aSuffixOffset = NthChineseWordByteOffset;
      //printf("aString[%s] Suffix[%s] *aSuffixOffset[%d]\n", aString, SuffixStr.c_str(), *aSuffixOffset);
      return true;
    }
  }

  return false;
}

int CXdbFilter::GetOneWordInToken(const char* aString,
                                  char* aTmpBuffer,
                                  size_t aBufferLength,
                                  int aWordIdx)
{
  int aTokenLen = 0;
  int idx = 0;
  int kind = 0;
  int found = 0;

  //printf("GetOneWordInTokenaString:[%s] aWordIdx:%d\n", aString, aWordIdx);
  memset(aTmpBuffer, 0, aBufferLength);
  aTokenLen = strlen(aString);

  if (3 > aTokenLen)
  {
    return false;
  }

  while (idx < aTokenLen)
  {
    kind = GetUTF8CharKind((unsigned char)aString[idx]);

    if (UTF8_CHAR_KIND_3 != kind)
    {
      return false;
    }

    if ((idx / UTF8_CHAR_KIND_3) == aWordIdx)
    {
      found = 1;
      memcpy(aTmpBuffer, &aString[idx], UTF8_CHAR_KIND_3);
      aTmpBuffer[UTF8_CHAR_KIND_3] = '\0';   /* null character manually added */
      //printf("GetOneWordInToken:%s\n", aTmpBuffer);
      break;
    }

    idx += kind;
  }

  return found;
}

int CXdbFilter::CHomophoneNormalizer_Init(const char* aFile)
{
  FILE* fd;
  char  line[256];

  fd = fopen(aFile, "r");
  if (NULL == fd)
  {
    printf("Error fopen homophone map file: %s\n", aFile);
    return false;
  }

  int ct = 0;
  while (fgets(line, sizeof(line), fd))
  {
    strtok(line, "\r\n");

    const char *pinyin, *src, *dst;

    pinyin = strtok(line, ",");
    src    = pinyin ? strtok(NULL, ",") : NULL;
    dst    = src ?    strtok(NULL, ",") : NULL;

    if (pinyin && src && dst)
    {
      //iNormalizerMap.Set((CHomophoneMap::TUnit) src, (CHomophoneMap::TUnit) dst);
      if (iNormalizerMap.end() == iNormalizerMap.find(src))
      {
        iNormalizerMap.insert( std::pair<std::string,std::string>(src,dst) );
        ++ct;
      }
    }
    //LOG_DEBUG(gFtsChHPNormalizer, "iHomophoneMap[%s]=%s\n", src, dst);
  }

  fclose(fd);
  printf("Read Homophone mapping into map DONE, total=%d.\n", ct);

  return true;
}

size_t CXdbFilter::CHomophoneNormalizer_Normalize(const char* aSource, char* aOutput, size_t aLength)
{
  size_t i;
  size_t retLength = 0;
  const unsigned char* aInput = (const unsigned char*)aSource;

  //printf("CHomophoneNormalizer_Normalize aSource:%s aLength:%d\n", aSource, aLength);
  if ((i = strlen(aSource)) + 1 > aLength)
  {
    retLength = i + 1; // aOutput length equals to aSource
    return retLength;
  }

  for (i = 0; aInput[i];)
  {
    size_t lenMB = iUTF8MultibyteLengthTable[aInput[i]];

    if (KCJKBytes != lenMB) /* not CJK */
    {
      memcpy(aOutput + i, aInput + i, lenMB);
    }
    else  /* CJK */
    {
      //if(iNormalizerMap.IsContained(aInput+i)) 
      std::string CJKStr((const char*)(aInput+i), lenMB);
      if (iNormalizerMap.end() != iNormalizerMap.find(CJKStr))
      {
        memcpy(aOutput + i, iNormalizerMap[CJKStr].data(), lenMB);
      }
      else
      {
        memcpy(aOutput + i, aInput + i, lenMB);
      }
    }

    i += lenMB;
  }

  aOutput[i] = 0;
  //printf("HPNormalized result:%s\n", aOutput);
  retLength = 0;
  return retLength;
}

void CXdbFilter::CFtsTokenizerExtChinese_ReserveStringCapacity(std::string& aString, size_t aSize, size_t aUnitSize)
{
  const size_t allocSize = (aSize + aUnitSize - 1) & ~(aUnitSize - 1); // pack with aUnitSize
  aString.resize(allocSize);
}

