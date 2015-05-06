//
// This file contains the CXdbFilter implementation 
//

#include "XdbFilter.h"

CXdbFilter::CXdbFilter() :
  g_mblen_table_utf8{
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
  4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1},
  KNormBufUnitSize(0x200),
  KCJKBytes(3),
  token_idx(0),
  line_no(0),
  pline(0),
  idx_tmp(0),
  idx_max(0),
  readSize(0)
{
  std::cout << "CXdbFilter" << std::endl;
}

CXdbFilter::~CXdbFilter()
{
  std::cout << "~CXdbFilter" << std::endl;
}

bool CXdbFilter::Run()
{
  std::cout << "Running CXdbFilter" << std::endl;
  std::cout << "iInputPath:" << iInputPath << std::endl;
  std::cout << "iOutputPath:" << iOutputPath<< std::endl;
  std::cout << "iLogPath:" << iLogPath<< std::endl;

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

void CXdbFilter::SetInputScwsXdb(std::string aInputScwsXdb)
{
  iInputScwsXdb = aInputScwsXdb;
}

void CXdbFilter::SetInputScwsRule(std::string aInputScwsRule)
{
  iInputScwsRule = aInputScwsRule;
}

void CXdbFilter::SetInputSourceData(std::string aInputSourceData)
{
  iInputSourceData = aInputSourceData;
}

void CXdbFilter::SetInputNormalizeMap(std::string aInputNormalizeMap)
{
  iInputNormalizeMap = aInputNormalizeMap;
}

void CXdbFilter::SetInputSpecialSuffixTable(std::string aInputSpecialSuffixTable)
{
  iInputSpecialSuffixTable = aInputSpecialSuffixTable;
}

void CXdbFilter::SetInputAreaName(std::string aInputAreaName)
{
  iInputAreaName = aInputAreaName;
}

void CXdbFilter::SetInputRemoveToken(std::string aInputRemoveToken)
{
  iInputRemoveToken= aInputRemoveToken;
}

std::string CXdbFilter::GetOutputTokenList()
{
  return iOutputTokenList;
}

std::string CXdbFilter::GetOutputTokenListNormalized()
{
  return iOutputTokenListNormalized;
}

std::string CXdbFilter::GetOutputTokenListFuzzy()
{
  return iOutputTokenListFuzzy;
}

bool CXdbFilter::Init()
{
  std::cout << "Init" << std::endl;
  // Stage 0: Initializations.
  //------------------------------------------------------------------------------------------------
  printf("Stage 0: Initializations.\n");
  if (!SuffixTokenMapInit(suffix_token_map))
  {
    printf("ERROR: cann't init the SuffixTokenMap!\n");
    return false;
  }

  // Log suffix_token_map.
  file_path = iLogPath + OUT_PATH_S00_SUFFIX_TOK_MAP;
  ofs.open(file_path.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = suffix_token_map.begin(); it != suffix_token_map.end(); ++it)
  {
    ofs << it->first << "|" << it->second << '\n';
  }
  ofs.close();
  //------------------------------------------------------------------------------------------------
  return true;
}

bool CXdbFilter::CollectTokens()
{
  std::cout << "CollectTokens" << std::endl;
  // Stage 1: Collect token list from "input raw file" or "saved token_map log file" to token_map.
  //------------------------------------------------------------------------------------------------
#ifdef MAP_INIT
  printf("Stage 1: Collect token list from \"input raw file\" to token_map.\n");
  char szTmp[170];
  scws_t s;
  scws_res_t res, cur;
  int ret;
  char text[MAX_LINE_SIZE];

  FILE *fp_s01_raw;
  FILE *fp_s01_not_ch;
  FILE *fp_s01_duplicate;

  char line_text[MAX_LINE_SIZE];//,*first_semicolon;
  int text_size;

  file_path = iLogPath + OUT_PATH_S01_NOT_CH;
  fp_s01_not_ch = fopen(file_path.c_str(), "w");
  if (NULL == fp_s01_not_ch)
  {
    printf("fp_s01_not_ch err:%s\n", file_path.c_str());
    return false;
  }
  printf("fp_s01_not_ch:%s\n", file_path.c_str());

  file_path = iLogPath + OUT_PATH_S01_DUPLICATE;
  fp_s01_duplicate = fopen(file_path.c_str(), "w");
  if (NULL == fp_s01_duplicate)
  {
    printf("fp_s01_duplicate err:%s\n", file_path.c_str());
    return false;
  }
  printf("fp_s01_duplicate:%s\n", file_path.c_str());

  if (!(s = scws_new())) {
    printf("ERROR: cann't init the scws!\n");
    return false;
  }

  scws_set_charset(s, "utf8");

  file_path = iInputPath + iInputScwsXdb;
  ret = scws_set_dict(s, file_path.c_str(), SCWS_XDICT_XDB);

  file_path = iInputPath + iInputScwsRule;
  scws_set_rule(s, file_path.c_str() );

  file_path = iInputPath + iInputSourceData;
  printf("input src:%s\n", file_path.c_str());
  fp_s01_raw = fopen(file_path.c_str(), "r");
  if (NULL == fp_s01_raw)
  {
    printf("fp_s01_raw err:%s\n", file_path.c_str());
    return false;
  }
  printf("fp_s01_raw:%s\n", file_path.c_str());
  
  while( NULL != fgets(line_text, MAX_LINE_SIZE, fp_s01_raw) ) 
  {
    line_no++;
    if (0 == line_no%1000)
      printf("Parsing Line(%d)....\n", line_no );
    
    if( 0 == strlen(line_text) )
      continue;

    text_size = 0;
    memset(text,0, sizeof(text) );

    pline = line_text;
    while( ('\r' != *pline) && ( '\n' != *pline) && ( 0 != *pline) )
      text[text_size++] = *(pline++);

    //printf("%s\n", text);
    scws_send_text(s, text, text_size );
    while (res = cur = scws_get_result(s))
    {
      while (cur != NULL)
      {
        memset(szTmp, 0, sizeof(szTmp));
        memcpy(szTmp, &text[cur->off], cur->len);

        if( isValidChineseToken(cur, szTmp) ) 
        {
          token_item.clear();
          token_item.append(szTmp);

          //std::vector<std::string>::iterator it = find(token_items.begin(), token_items.end(), token_item);
          std::map<std::string,int>::iterator it = token_map.find(token_item);

          //if(it != token_items.end()) 
          if(it != token_map.end()) 
          {
            // found ignore the duplicate items
            fprintf(fp_s01_duplicate, "%s off=%d idf=%4.2f len=%d attr=%s\n", szTmp, cur->off, cur->idf, cur->len, cur->attr);
          }
          else 
          {
            //token_items.push_back(token_item);
            token_map.insert( std::pair<std::string,int>(token_item, ++token_idx) );
          }
        } 
        else 
        {
          fprintf(fp_s01_not_ch, "%s off=%d idf=%4.2f len=%d attr=%s\n", szTmp, cur->off, cur->idf, cur->len, cur->attr);
        }

        cur = cur->next;
        //if( NULL == cur) cur = scws_get_result(s);
      }

      scws_free_result(res);
    }
  }
  scws_free(s);
  printf("Total Parsing Line(%d)....\n", line_no );

  // Log token_map.
  file_path = iLogPath + OUT_PATH_S01_MAP_RAW;
  ofs.open(file_path.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = token_map.begin(); it != token_map.end(); ++it)
  {
    ofs << it->first << "|" << it->second << '\n';
  }
  ofs.close();
  fclose(fp_s01_raw);
  fclose(fp_s01_not_ch);
  fclose(fp_s01_duplicate);
  
#else //MAP_INIT
  printf("Stage 1: Collect token list from \"saved token_map log file\" to token_map.\n");
  
  // Re-load token list from token_map log file.
  file_path = iLogPath + OUT_PATH_S01_MAP_RAW;
  printf("Re-load token list from token_map log file:%s\n", file_path.c_str() );

  ifs.open(file_path.c_str(), std::ifstream::in);
  if (!ifs.is_open())
  {
    // show message:
    std::cout<<"Error opening file: "<<file_path<<std::endl;
    return false;
  }
  token_map.clear();
  while (std::getline(ifs, tmp_line).good())
  {
    pos = tmp_line.find_first_of("|");
    str1 = tmp_line.substr(0, pos);
    str2 = tmp_line.substr(pos + 1, std::string::npos);
    idx_tmp = atoi(str2.c_str());
    //std::cout<<"tmp_line:"<<tmp_line<<" str1:"<<str1<<" str2:"<<str2<<" str2.c_str():"<<str2.c_str()<<std::endl;

    token_map.insert( std::pair<std::string,int>(str1, idx_tmp) );

    if (idx_tmp > idx_max)
    {
      idx_max = idx_tmp;
    }
  }
  token_idx = idx_max;
  ifs.close();

  // Log reloaded token_map.
  file_path = iLogPath + OUT_PATH_S01_MAP_RELOAD;
  printf("Re-load token list:%s\n", file_path.c_str() );
  ofs.open(file_path.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = token_map.begin(); it != token_map.end(); ++it)
  {
    ofs << it->first << "|" << it->second << '\n';
  }
  ofs.close();
  //return true;
#endif //MAP_INIT
  //------------------------------------------------------------------------------------------------
  return true;
}

bool CXdbFilter::AddExtraTokens()
{
  std::cout << "AddExtraTokens" << std::endl;
  // Stage 2: Add "additional tokens" from _ignored_suffix_table_{tc|sc}_utf8.txt to token_map.
  //------------------------------------------------------------------------------------------------
  printf("Stage 2: Add \"additional tokens\" from _ignored_suffix_table_{tc|sc}_utf8.txt to token_map.\n");
#if 1
  for (std::map<std::string,int>::iterator it = suffix_token_map.begin(); it != suffix_token_map.end(); ++it)
  {
    //ofs << it->first << "|" << it->second << '\n';
    if (token_map.end() == token_map.find(it->first))
    {
      // Word not found, add to map.
      token_map.insert( std::pair<std::string,int>(it->first, ++token_idx) );
      printf("Not Found [%s] added as : %s|%d\n", it->first.c_str(), it->first.c_str(),token_idx);
    }
  }

  // Log  token_map with missing tokens in table _ignored_suffix_table_{tc|sc}_utf8.txt.
  file_path = iLogPath + OUT_PATH_S02_MAP_SUFFIX;
  ofs.open(file_path.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = token_map.begin(); it != token_map.end(); ++it)
  {
    ofs << it->first << "|" << it->second << '\n';
  }
  ofs.close();
  //return true;
#endif
  //------------------------------------------------------------------------------------------------

// Stage 2.1: Add "AreaName tokens" from {TC|SC}\input\areas\05_all_area_map02.txt to token_map.
  //------------------------------------------------------------------------------------------------
  printf("Stage 2.1: Add \"AreaName tokens\" from {TC|SC}\\input\\areas\\05_all_area_map02.txt to token_map.\n");

  // Load area name token list from file to all_area_map.
  file_path = iInputPath + iInputAreaName;
  ifs.open(file_path.c_str(), std::ifstream::in);
  if (!ifs.is_open())
  {
    // show message:
    std::cout<<"Error opening file: "<<file_path<<std::endl;
    return false;
  }
  all_area_map.clear();
  while (std::getline(ifs, tmp_line).good())
  {
    pos = tmp_line.find_first_of(",");
    str1 = tmp_line.substr(0, pos);
    str2 = tmp_line.substr(pos + 1, std::string::npos);
    idx_tmp = atoi(str2.c_str());
    //std::cout<<"tmp_line:"<<tmp_line<<" str1:"<<str1<<" str2:"<<str2<<" str2.c_str():"<<str2.c_str()<<std::endl;

    all_area_map.insert( std::pair<std::string,int>(str1, idx_tmp) );
  }
  ifs.close();

  // Log all_area_map.
  file_path = iLogPath + OUT_PATH_S02_LOG_AREA_NAME_FILE;
  ofs.open(file_path.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = all_area_map.begin(); it != all_area_map.end(); ++it)
  {
    ofs << it->first << "," << it->second << '\n';
  }
  ofs.close();

  // Save all_area_map to token_map.
  for (std::map<std::string,int>::iterator it = all_area_map.begin(); it != all_area_map.end(); ++it)
  {
    //ofs << it->first << "|" << it->second << '\n';
    if (token_map.end() == token_map.find(it->first))
    {
      // Word not found, add to map.
      token_map.insert( std::pair<std::string,int>(it->first, ++token_idx) );
      printf("Not Found [%s] added as : %s|%d\n", it->first.c_str(), it->first.c_str(),token_idx);
    }
  }

  // Log  token_map with missing tokens in {TC|SC}\input\areas\05_all_area_map02.txt.
  file_path = iLogPath + OUT_PATH_S02_LOG_MAP_PLUS_AREA_NAME;
  ofs.open(file_path.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = token_map.begin(); it != token_map.end(); ++it)
  {
    ofs << it->first << "|" << it->second << '\n';
  }
  ofs.close();
  //return true;
  //------------------------------------------------------------------------------------------------
  return true;
}

bool CXdbFilter::RemoveUnwantTokens()
{
  std::cout << "RemoveUnwantTokens" << std::endl;
  // Stage 2.2: Remove "unwant tokens" from {TC|SC}\input\_removed_tokens_table.txt from token_map.
    //------------------------------------------------------------------------------------------------
    printf("Stage 2.2: Remove \"unwant tokens\" from {TC|SC}\\input\\_removed_tokens_table.txt from token_map.\n");
  
    std::map<std::string,int> unwant_tokens_map;
    std::map<std::string,int>::iterator unwant_tokens_iter;
    
    // Load unwant token list from file to unwant_tokens_map.
    file_path = iInputPath + iInputRemoveToken;
  
    ifs.open(file_path.c_str(), std::ifstream::in);
    if (!ifs.is_open())
    {
      // show message:
      std::cout<<"Error opening file: "<<file_path<<std::endl;
      return false;
    }
    unwant_tokens_map.clear();
    while (std::getline(ifs, tmp_line).good())
    {
      if (0 == tmp_line.length())
        continue;
      if ((tmp_line[0] == '#') || (tmp_line[0] == ';'))
        continue;
      
      pos = tmp_line.find_first_of(" \n\r\0");
      str1 = tmp_line.substr(0, pos);
      std::cout<<"tmp_line:"<<tmp_line<<" str1:"<<str1<<std::endl;
      //str2 = tmp_line.substr(pos + 1, std::string::npos);
      //idx_tmp = atoi(str2.c_str());
      //std::cout<<"tmp_line:"<<tmp_line<<" str1:"<<str1<<" str2:"<<str2<<" str2.c_str():"<<str2.c_str()<<std::endl;
  
      unwant_tokens_map.insert( std::pair<std::string,int>(str1, 1) );
    }
    ifs.close();
  
    // Log unwant_tokens_map.
    file_path = iLogPath + OUT_PATH_S02_LOG_REMOVE_TOK_FILE;
    ofs.open(file_path.c_str(), std::ofstream::out);
    for (std::map<std::string,int>::iterator it = unwant_tokens_map.begin(); it != unwant_tokens_map.end(); ++it)
    {
      ofs << it->first << "," << it->second << '\n';
    }
    ofs.close();
  
    // Remove unwant_tokens_map from token_map.
    for (std::map<std::string,int>::iterator it = unwant_tokens_map.begin(); it != unwant_tokens_map.end(); ++it)
    {
      if (token_map.end() != token_map.find(it->first))
      {
        // Word found, remove from map.
        token_map.erase(it->first);       // erasing by key
      }
    }
  
    // Log  token_map with missing tokens in {TC|SC}\input\areas\05_unwant_tokens_map02.txt.
    file_path = iLogPath + OUT_PATH_S02_LOG_MAP_AFTER_REMOVE_TOK;
    ofs.open(file_path.c_str(), std::ofstream::out);
    for (std::map<std::string,int>::iterator it = token_map.begin(); it != token_map.end(); ++it)
    {
      ofs << it->first << "|" << it->second << '\n';
    }
    ofs.close();
    //return true;
    //------------------------------------------------------------------------------------------------
  return true;
}

bool CXdbFilter::AddMissingOneWordTokens()
{
  std::cout << "AddMissingOneWordTokens" << std::endl;
  // Stage 3: Add "missing one word tokens" from token_map into token_map_with_one_word.
  //------------------------------------------------------------------------------------------------
  printf("Stage 3: Add \"missing one word tokens\" from token_map into token_map_with_one_word.\n");
  // Copy token_map to token_map_with_one_word.
  for (std::map<std::string,int>::iterator it = token_map.begin(); it != token_map.end(); ++it)
    token_map_with_one_word.insert( std::pair<std::string,int>(it->first, it->second) );

  // Add one word tokens from original tokens.
  char tmpWord[6];
  int wordIdx;
  file_path = iLogPath + OUT_PATH_S03_CH_ONE_WORD;
  ofs.open(file_path.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = token_map.begin(); it != token_map.end(); ++it)
  {
    // break token into words and add them.
    strcpy(tmp_buf, it->first.c_str());
    strtok(tmp_buf,"\n\r");
    wordIdx = 0;
    while (TokenGetOneWord(tmp_buf, tmpWord, sizeof(tmpWord), wordIdx++))
    {
      if (token_map_with_one_word.end() == token_map_with_one_word.find(tmpWord))
      {
        // Word not found, add to map.
        token_map_with_one_word.insert( std::pair<std::string,int>(tmpWord, ++token_idx) );
        //printf("Added One Word:%s|%d\n", tmpWord,token_idx);
        ofs<<tmpWord<<"|"<<token_idx<<std::endl;
      }
    }
  }
  ofs.close();

  // Log token_map_with_one_word.
  file_path = iLogPath + OUT_PATH_S03_MAP_WITH_ONE_WORDS;
  ofs.open(file_path.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = token_map_with_one_word.begin(); it != token_map_with_one_word.end(); ++it)
  {
    ofs << it->first << "|" << it->second << '\n';
  }
  ofs.close();
  //------------------------------------------------------------------------------------------------

  // Stage 4: [JUST FOR LOG] Retreive every map item's info from xdb, then log info to files.
  //------------------------------------------------------------------------------------------------
  printf("Stage 4: [JUST FOR LOG] Retreive every map item's info from xdb, then log info to files.\n");

  FILE *fp_s04_ch_full;
  FILE *fp_s04_ch_part;
  FILE *fp_s04_ch_not_found;
  
  file_path = iLogPath + OUT_PATH_S04_CH_FULL;
  fp_s04_ch_full = fopen(file_path.c_str(), "w");
  if (NULL == fp_s04_ch_full)
  {
    printf("fp_s04_ch_full err:%s\n", file_path.c_str());
    return false;
  }
  printf("fp_s04_ch_full:%s\n", file_path.c_str());

  file_path = iLogPath + OUT_PATH_S04_CH_PART;
  fp_s04_ch_part = fopen(file_path.c_str(), "w");
  if (NULL == fp_s04_ch_part)
  {
    printf("fp_s04_ch_part err:%s\n", file_path.c_str());
    return false;
  }
  printf("fp_s04_ch_part:%s\n", file_path.c_str());

  file_path = iLogPath + OUT_PATH_S04_CH_NOT_FOUND;
  fp_s04_ch_not_found = fopen(file_path.c_str(), "w");
  if (NULL == fp_s04_ch_not_found)
  {
    printf("fp_s04_ch_not_found err:%s\n", file_path.c_str());
    return false;
  }
  printf("fp_s04_ch_not_found:%s\n", file_path.c_str());
  
  // get xdb header
  file_path = iInputPath + iInputScwsXdb;
  fp_xdb = fopen(file_path.c_str(), "rb");
  if( NULL == fp_xdb )
  if (NULL == fp_xdb)
  {
    printf("fp_xdb err:%s\n", file_path.c_str());
    return false;
  }
  printf("fp_xdb:%s\n", file_path.c_str());
  readSize = fread( &header, 1, sizeof(xdb_header), fp_xdb);
  if (readSize != sizeof(xdb_header)*1) {fputs("Reading error 13", stderr); exit(EXIT_FAILURE);}
  
#if 1
  // Start to parse each token in token_map.
  for (std::map<std::string,int>::iterator it = token_map_with_one_word.begin(); it != token_map_with_one_word.end(); ++it)
  {
    // Search this token in xdb.
    strcpy(tmp_buf, it->first.c_str());
    strtok(tmp_buf,"\n\r");
    memset(&node_ia, 0, sizeof(node_ia));
    if( SEARCH_TOKEN_INFO_FOUND == searchTokenInfo( tmp_buf, -1, -1, header.base, header.prime, &node_ia, fp_xdb  ) ) 
    {
      // Save only full tokens.
      if( 2 == node_ia.flag)
      {
        fprintf(fp_s04_ch_part, "PART:%s\t\%4.2f\t%4.2f\t%d\t%s\n", tmp_buf, node_ia.tf, node_ia.idf, node_ia.flag, node_ia.attr  );
      }
      else
      {
        fprintf(fp_s04_ch_full, "%s\t\%4.2f\t%4.2f\t%d\t%s\n", tmp_buf, node_ia.tf, node_ia.idf, node_ia.flag, node_ia.attr  );
      }
    } 
    else 
    {
      fprintf(fp_s04_ch_not_found, "NOT_FOUND:%s\t\%4.2f\t%4.2f\t%d\t%s\n", tmp_buf, node_ia.tf, node_ia.idf, node_ia.flag, node_ia.attr  );
    }
  }
#endif
  fclose(fp_xdb);
  fclose(fp_s04_ch_full);
  fclose(fp_s04_ch_part);
  fclose(fp_s04_ch_not_found);
  //------------------------------------------------------------------------------------------------
  return true;
}

bool CXdbFilter::RemoveSpecialSuffixTokens()
{
  std::cout << "RemoveSpecialSuffixTokens" << std::endl;
  // Stage 5: Start to build token_map_opt by parsing special suffix of each token in token_map_with_one_word.
  //------------------------------------------------------------------------------------------------
  printf("Stage 5: Start to build token_map_opt by parsing special suffix of each token in token_map_with_one_word.\n");
  FILE *fp_out_ch_parse2_suffix_log;
  int MaxSuffixTokenLength = 0;
  int SuffixOff;
  char rem = 0;
  std::string tmpStr;

#ifdef REMOVE_FULL_TOKENS_WITH_SPECIAL_END
  FILE *fp_out_log_removed_special_end_tok;

  file_path = iLogPath + OUT_PATH_S05_REMOVED_SPECIAL_END_TOKENS;
  fp_out_log_removed_special_end_tok = fopen(file_path.c_str(), "w");
  if (NULL == fp_out_log_removed_special_end_tok)
  {
    printf("fp_out_log_removed_special_end_tok err:%s\n", file_path.c_str());
    return false;
  }
  printf("fp_out_log_removed_special_end_tok:%s\n", file_path.c_str());
#endif

  file_path = iLogPath + OUT_PATH_S05_ADDED_REMOVED_SUFFIX;
  fp_out_ch_parse2_suffix_log = fopen(file_path.c_str(), "w");
  if (NULL == fp_out_ch_parse2_suffix_log)
  {
    printf("fp_out_ch_parse2_suffix_log err:%s\n", file_path.c_str());
    return false;
  }
  printf("fp_out_ch_parse2_suffix_log:%s\n", file_path.c_str());
  
  MaxSuffixTokenLength = MaxSuffixTokenLengthGet(suffix_token_map);
  
  file_path = iLogPath + OUT_PATH_S05_FOUND_SUFFIX;
  ofs.open(file_path.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = token_map_with_one_word.begin(); it != token_map_with_one_word.end(); ++it)
  {
    //std::cout<<it->first<<std::endl;
    SuffixOff = 0;

    // We will add this full token to token_map_opt.
    token_map_opt.insert( std::pair<std::string,int>(it->first, it->second) );

    // The full token in <token_map_with_one_word> already added to <token_map_opt>, 
    // now we will tell if this full token is a special-suffix-tokens, and will have some optimzed operations as below: 
    // 1. Remove special suffix.
    // 2. Check if this removed-suffix-token already exist in <token_map_with_one_word>, 
    //     if YES we can just bypass to next token, if NO, user can choose if he want to add this removed-suffix-token to <token_map_opt> or not.
    //
    if ( isTokenEndWithIgnoredSuffix(it->first.c_str(), &SuffixOff, tmp_buf, MaxSuffixTokenLength, suffix_token_map) )
    {
      // Log FoundIgnoredSuffixToken.
      //std::cout<<"FoundIgnoredSuffixToken:["<<it->first<<"] SuffixOff:"<<SuffixOff<<" Suffix:["<<it->first.c_str()+SuffixOff<<"]"<<std::endl;
      ofs<<"FoundIgnoredSuffixToken:["<<it->first<<"] SuffixOff:"<<SuffixOff<<" Suffix:["<<it->first.c_str()+SuffixOff<<"]"<<std::endl;

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

        //remove this token from token_map_opt.
        token_map_opt.erase(it->first);       // erasing by key
        std::cout<<"Remove special["<<it->first<<"]["<<it->second<<"] from token_map_opt!"<<std::endl;
        fprintf(fp_out_log_removed_special_end_tok, "Remove special end token [%s][%d] from token_map_opt!\n", it->first.c_str(), it->second);
      }
    #endif //REMOVE_FULL_TOKENS_WITH_SPECIAL_END
      
      // Remove suffix.
      tmpStr = it->first;
      tmpStr.erase(SuffixOff, std::string::npos);
    #ifdef ENABLE_USER_INTERACTIVE_CONTROL //Ask user.
      std::cout<<"FoundIgnoredSuffixToken:["<<it->first<<"]"<<" AfterRemoveSuffix:["<<tmpStr<<"]"<<std::endl;
    #endif

      // Is the removed-suffix-token exist in token_map_with_one_word?
      if (token_map_with_one_word.end() != token_map_with_one_word.find(tmpStr)) // Removed-suffix-token ALREADY in token_map_with_one_word!
      {
        //Add to token_map_opt.
        token_map_opt.insert( std::pair<std::string,int>(tmpStr, it->second) );
        
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
          token_map_opt.insert( std::pair<std::string,int>(tmpStr, ++token_idx) );
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
    } //isTokenEndWithIgnoredSuffix
  }
  ofs.close();

  // Log optimized token_map_opt.
  file_path = iLogPath + OUT_PATH_S05_MAP_OPTIMIZED;
  ofs.open(file_path.c_str(), std::ofstream::out);
  for (std::map<std::string,int>::iterator it = token_map_opt.begin(); it != token_map_opt.end(); ++it)
  {
    ofs << it->first << "|" << it->second << '\n';
  }
  ofs.close();
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
#if 1
  FILE *fp_out_s06_suffix_full;
  FILE *fp_out_s06_suffix_part;
  FILE *fp_out_s06_suffix_not_found;

   // get xdb header
  file_path = iInputPath + iInputScwsXdb;
  fp_xdb = fopen(file_path.c_str(), "rb");
  if( NULL == fp_xdb )
    return false;
  readSize = fread( &header, 1, sizeof(xdb_header), fp_xdb);
  if (readSize != sizeof(xdb_header)*1) {fputs("Reading error 14", stderr); exit(EXIT_FAILURE);}
  
  file_path = iOutputPath + OUT_PATH_S06_SUFFIX_FULL;
  fp_out_s06_suffix_full = fopen(file_path.c_str(), "w");
  if (NULL == fp_out_s06_suffix_full)
  {
    printf("fp_out_s06_suffix_full err:%s\n", file_path.c_str());
    return false;
  }
  printf("fp_out_s06_suffix_full:%s\n", file_path.c_str());
  iOutputTokenList = file_path;

  file_path = iLogPath + OUT_PATH_S06_SUFFIX_PART;
  fp_out_s06_suffix_part = fopen(file_path.c_str(), "w");
  if (NULL == fp_out_s06_suffix_part)
  {
    printf("fp_out_s06_suffix_part err:%s\n", file_path.c_str());
    return false;
  }
  printf("fp_out_s06_suffix_part:%s\n", file_path.c_str());

  file_path = iLogPath + OUT_PATH_S06_SUFFIX_NOT_FOUND;
  fp_out_s06_suffix_not_found = fopen(file_path.c_str(), "w");
  if (NULL == fp_out_s06_suffix_not_found)
  {
    printf("fp_out_s06_suffix_not_found err:%s\n", file_path.c_str());
    return false;
  }
  printf("fp_out_s06_suffix_not_found:%s\n", file_path.c_str());
  
  // Start to parse each token in token_map_opt.
  for (std::map<std::string,int>::iterator it = token_map_opt.begin(); it != token_map_opt.end(); ++it)
  {
    // Search this token in xdb.
    strcpy(tmp_buf, it->first.c_str());
    strtok(tmp_buf,"\n\r");
    if( SEARCH_TOKEN_INFO_FOUND == searchTokenInfo( tmp_buf, -1, -1, header.base, header.prime, &node_ia, fp_xdb  ) ) 
    {
      if( 2 == node_ia.flag)
      {
        fprintf(fp_out_s06_suffix_part, "PART:%s\t\%4.2f\t%4.2f\t%d\t%s\n", tmp_buf, node_ia.tf, node_ia.idf, node_ia.flag, node_ia.attr  );

        // Force add this token as full token.
        node_ia.tf = 1.0;
        node_ia.idf = 1.0;
        node_ia.flag = 1;
        strcpy(node_ia.attr, "@");

        // If this is a AreaName token: 1. Change attr to "ns". 2. idf+105.
        if (all_area_map.end() != all_area_map.find(it->first))
        {
          // AreaName found, update attr and idf.
          node_ia.idf += 100.0;
          strcpy(node_ia.attr, "ns");

          fprintf(fp_out_s06_suffix_full, "%s\t\%4.2f\t%4.2f\t%d\t%s     111\n", tmp_buf, node_ia.tf, node_ia.idf, node_ia.flag, node_ia.attr  );
        }
        else
        {
          fprintf(fp_out_s06_suffix_full, "%s\t\%4.2f\t%4.2f\t%d\t%s     !!!\n", tmp_buf, node_ia.tf, node_ia.idf, node_ia.flag, node_ia.attr  );
        }
      }
      else
      {
        // If this is a AreaName token: 1. Change attr to "ns". 2. idf+105.
        if (all_area_map.end() != all_area_map.find(it->first))
        {
          // AreaName found, update attr and idf.
          node_ia.idf += 100.0;
          strcpy(node_ia.attr, "ns");

          fprintf(fp_out_s06_suffix_full, "%s\t\%4.2f\t%4.2f\t%d\t%s     222\n", tmp_buf, node_ia.tf, node_ia.idf, node_ia.flag, node_ia.attr  );
        }
        else
        {
          fprintf(fp_out_s06_suffix_full, "%s\t\%4.2f\t%4.2f\t%d\t%s\n", tmp_buf, node_ia.tf, node_ia.idf, node_ia.flag, node_ia.attr  );
        }
      }
    } 
    else 
    {
      fprintf(fp_out_s06_suffix_not_found, "NOT_FOUND:%s\n", tmp_buf);

      // Force add this token as full token.
      node_ia.tf = 1.0;
      node_ia.idf = 1.0;
      node_ia.flag = 1;
      strcpy(node_ia.attr, "@");

      // If this is a AreaName token: 1. Change attr to "ns". 2. idf+105.
      if (all_area_map.end() != all_area_map.find(it->first))
      {
        // AreaName found, update attr and idf.
        node_ia.idf += 100.0;
        strcpy(node_ia.attr, "ns");

        fprintf(fp_out_s06_suffix_full, "%s\t\%4.2f\t%4.2f\t%d\t%s     333\n", tmp_buf, node_ia.tf, node_ia.idf, node_ia.flag, node_ia.attr  );
      }
      else
      {
        fprintf(fp_out_s06_suffix_full, "%s\t\%4.2f\t%4.2f\t%d\t%s     !!!\n", tmp_buf, node_ia.tf, node_ia.idf, node_ia.flag, node_ia.attr  );
      }
    }
  }

  fclose(fp_out_s06_suffix_full);
  fclose(fp_out_s06_suffix_part);
  fclose(fp_out_s06_suffix_not_found);
  fclose(fp_xdb);
#endif
  //------------------------------------------------------------------------------------------------
  return true;
}

bool CXdbFilter::ConvertToNormalizedTokens()
{
  std::cout << "ConvertToNormalizedTokens" << std::endl;
#ifdef _CONVERT_NORMALIZE_
  // Stage 7: Convert results to normalized form.
  //------------------------------------------------------------------------------------------------
#if 1
  printf("Stage 7: Convert results to normalized form.\n");
  FILE *fp_in_s07_optimized_full;
  FILE *fp_out_s07_normalized_full;
  int szLine_Len;
  char* pKey=0;

  int iEnableHPNormalize = 0;
  std::string iNormText;

  file_path = iInputPath + iInputNormalizeMap;
  if (!CHomophoneNormalizer_Init(file_path.c_str()))
  {
    printf("CHomophoneNormalizer_Init err:%s\n", file_path.c_str());
    iEnableHPNormalize = 0;
  }
  else
  {
    printf("CHomophoneNormalizer_Init OK\n");
    iEnableHPNormalize = 1;
  }
  
  file_path = iOutputPath + OUT_PATH_S06_SUFFIX_FULL;
  fp_in_s07_optimized_full = fopen(file_path.c_str(), "r");
  if (NULL == fp_in_s07_optimized_full)
  {
    printf("fp_in_s07_optimized_full err:%s\n", file_path.c_str());
    return false;
  }
  printf("fp_in_s07_optimized_full:%s\n", file_path.c_str());
  file_path = iOutputPath + OUT_PATH_S07_SUFFIX_FULL_NOR;
  fp_out_s07_normalized_full = fopen(file_path.c_str(), "w");
  if (NULL == fp_out_s07_normalized_full)
  {
    printf("fp_out_s07_normalized_full err:%s\n", file_path.c_str());
    return false;
  }
  printf("fp_out_s07_normalized_full:%s\n", file_path.c_str());
  iOutputTokenListNormalized = file_path;

  // Start converting.
  iNormText.resize(32);
  while (fgets(tmp_buf1, sizeof(tmp_buf1), fp_in_s07_optimized_full)) 
  {
    line_no++;
    if (0 == line_no%1000)
      printf("Converting Line(%d)....\n", line_no );

    if ((tmp_buf1[0] == '#') || (tmp_buf1[0] == ';'))
      continue;

    strncpy(tmp_buf, tmp_buf1, sizeof(tmp_buf));
    szLine_Len = strlen(tmp_buf);

    if( 0 == szLine_Len )
      continue;

    pKey = strtok(tmp_buf, "\t ");
    if (pKey) 
    {
      //printf("Converting [%s] of %s", pKey, tmp_buf1 );

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

        //printf("AAA Un-Normali str:%s", tmp_buf1);
        strncpy(tmp_buf1, iNormText.c_str(), strlen(iNormText.c_str()));
        //printf("BBB Normalized str:%s", tmp_buf1);

        //log to file.
        fprintf(fp_out_s07_normalized_full, "%s", tmp_buf1);
      }
    }
    else
    {
      printf("Failed converting:%s", tmp_buf1 );
    }
  }
  printf("Total converting Line(%d)....\n", line_no );

  fclose(fp_in_s07_optimized_full);
  fclose(fp_out_s07_normalized_full);
#endif
  //------------------------------------------------------------------------------------------------

#endif //_CONVERT_NORMALIZE_
  return true;
}

bool CXdbFilter::MergeToFuzzyTokens()
{
  std::cout << "MergeToFuzzyTokens" << std::endl;
#ifdef _CONVERT_NORMALIZE_
  // Stage 8: Merge TokenList and TokenListNormalized as TokenListFuzzy
  //------------------------------------------------------------------------------------------------
  FILE *fp_fuzzy=NULL;
  FILE *fp_tokenList=NULL;
  FILE *fp_tokenListNorm=NULL;
  std::string file_path;

  file_path = iOutputPath + OUT_PATH_S08_SUFFIX_FULL_FUZZY;
  fp_fuzzy = fopen(file_path.c_str(), "w");
  if (NULL == fp_fuzzy)
  {
    printf("fp_fuzzy err:%s\n", file_path.c_str());
    return false;
  }
  printf("fp_fuzzy:%s\n", file_path.c_str());
  iOutputTokenListFuzzy = file_path;

  file_path = GetOutputTokenList();
  fp_tokenList = fopen(file_path.c_str(), "r");
  if(NULL == fp_tokenList)
  {
    printf("Error open fp_tokenList: %s\n", file_path.c_str());
    return false;
  }
  printf("fp_tokenList:%s\n", file_path.c_str());

  file_path = GetOutputTokenListNormalized();
  fp_tokenListNorm = fopen(file_path.c_str(), "r");
  if(NULL == fp_tokenListNorm)
  {
    printf("Error open fp_tokenListNorm: %s\n", file_path.c_str());
    return false;
  }
  printf("fp_tokenListNorm:%s\n", file_path.c_str());

  while( NULL != fgets(tmp_buf1, sizeof(tmp_buf1), fp_tokenList) ) 
  {
    fputs(tmp_buf1, fp_fuzzy);
  }
  fclose(fp_tokenList);

  while( NULL != fgets(tmp_buf1, sizeof(tmp_buf1), fp_tokenListNorm) ) 
  {
    fputs(tmp_buf1, fp_fuzzy);
  }
  fclose(fp_tokenListNorm);
  fclose(fp_fuzzy);
  //------------------------------------------------------------------------------------------------
#endif //_CONVERT_NORMALIZE_
  return true;
}
////////////////////// Search Node /////////////////////////////////////
int CXdbFilter::_get_hash_index(unsigned char* key, int hash_base, int hash_prime )
{
  int l = strlen((char*)key);
  int h = hash_base;
  while (l--)
  {
    h += (int)(h << 5);
    h ^= key[l];
    h &= 0x7fffffff;
  }

  return (h % hash_prime);
}

int CXdbFilter::searchTokenInfo( char *token_content, long node_offset, long node_length, int hash_base, int hash_prime, node_info_attr *attr, FILE* fp_xdb  ) {
  int hash_index;
  //fpos_t f_offset;
  long f_offset;
  prime_node pr_node;
  node_info_header node_ih;
  node_info_attr node_ia;
  char szBuff[300];
  int cmp_result;
  size_t readSize = 0;

  unsigned int l_offset, l_len;
  unsigned int r_offset, r_len;
  char k_len;

  if ( NULL == fp_xdb ) 
  {
    printf("searchTokenInfo fp_xdb err\n");
    return SEARCH_TOKEN_INFO_NOT_FOUND;
  }

  if( (node_offset < 0 ) && (node_length < 0) ) {
    hash_index = _get_hash_index((unsigned char*)token_content, hash_base, hash_prime );

    int t = sizeof(xdb_header);
    int g = sizeof(prime_node);

    f_offset = sizeof(xdb_header)+hash_index*sizeof(prime_node);
    //fsetpos(fp_xdb, &f_offset);
    fseek(fp_xdb, f_offset, SEEK_SET);
    readSize = fread(&pr_node, 1, sizeof(prime_node), fp_xdb);
    if (readSize != sizeof(prime_node)*1) {fputs("Reading error 01", stderr); exit(EXIT_FAILURE);}

    node_offset = pr_node.offset;
    node_length = pr_node.length;
  }

  // read node
  f_offset = node_offset;
  //fsetpos(fp_xdb, &f_offset);
  fseek(fp_xdb, f_offset, SEEK_SET);

  readSize = fread(&node_ih.l_offset, 1, sizeof(unsigned int), fp_xdb );
  if (readSize != sizeof(unsigned int)*1) {fputs("Reading error 02", stderr); exit(EXIT_FAILURE);}
  readSize = fread(&node_ih.l_length, 1, sizeof(unsigned int), fp_xdb );
  if (readSize != sizeof(unsigned int)*1) {fputs("Reading error 03", stderr); exit(EXIT_FAILURE);}
  readSize = fread(&node_ih.r_offset, 1, sizeof(unsigned int), fp_xdb );
  if (readSize != sizeof(unsigned int)*1) {fputs("Reading error 04", stderr); exit(EXIT_FAILURE);}
  readSize = fread(&node_ih.r_length, 1, sizeof(unsigned int), fp_xdb );
  if (readSize != sizeof(unsigned int)*1) {fputs("Reading error 05", stderr); exit(EXIT_FAILURE);}
  readSize = fread(&node_ih.k_length, 1, sizeof(unsigned char), fp_xdb );
  if (readSize != sizeof(unsigned char)*1) {fputs("Reading error 06", stderr); exit(EXIT_FAILURE);}

  memset(szBuff, 0, sizeof(szBuff) );
  readSize = fread(szBuff, 1, node_ih.k_length, fp_xdb);
  if (readSize != node_ih.k_length*1) {fputs("Reading error 07", stderr); exit(EXIT_FAILURE);}

  cmp_result = strcmp(szBuff, token_content);

  if( 0 == cmp_result )
  {
    readSize = fread( &(attr->tf), 1, sizeof(float), fp_xdb );
    if (readSize != sizeof(float)*1) {fputs("Reading error 08", stderr); exit(EXIT_FAILURE);}
    readSize = fread( &(attr->idf), 1, sizeof(float), fp_xdb );
    if (readSize != sizeof(float)*1) {fputs("Reading error 09", stderr); exit(EXIT_FAILURE);}
    readSize = fread( &(attr->flag), 1, sizeof(unsigned char), fp_xdb );
    if (readSize != sizeof(unsigned char)*1) {fputs("Reading error 10", stderr); exit(EXIT_FAILURE);}
    readSize = fread( &(attr->attr), sizeof(unsigned char), 3, fp_xdb );
    if (readSize != sizeof(unsigned char)*3) {fputs("Reading error 11", stderr); exit(EXIT_FAILURE);}
    //readSize = fread(attr, 1, sizeof(node_info_attr), fp_xdb);
    //if (readSize != sizeof(node_info_attr)*1) {fputs("Reading error 12", stderr); exit(EXIT_FAILURE);}
    return SEARCH_TOKEN_INFO_FOUND;
  } else if( cmp_result > 0 ) {
    if( (0 == node_ih.l_offset) && (0 == node_ih.l_length) )
      return SEARCH_TOKEN_INFO_NOT_FOUND; // no node

    return searchTokenInfo( token_content, node_ih.l_offset, node_ih.l_length, hash_base, hash_prime, attr, fp_xdb );
  } else {
    //if( cmp_result < 0 )

    if( (0 == node_ih.r_offset) && (0 == node_ih.r_length) )
      return SEARCH_TOKEN_INFO_NOT_FOUND; // no node

    return searchTokenInfo( token_content, node_ih.r_offset, node_ih.r_length, hash_base, hash_prime, attr, fp_xdb );
  }

  return SEARCH_TOKEN_INFO_NOT_FOUND;
}
////////////////////// Search Node /////////////////////////////////////

int CXdbFilter::getUTF8CharKind( unsigned char x ) 
{
  if( x <= 0x7f )
    return UTF8_CHAR_KIND_1;  // ASCII

  if( x >= 0xC0 && x <= 0xDF )
    return UTF8_CHAR_KIND_2;  // Latin characters

  if( x >= 0xE0 && x <= 0xEF )
    return UTF8_CHAR_KIND_3;  // CJKS

  if( x >= 0xF0 && x <= 0xF7 )
    return UTF8_CHAR_KIND_4;  // CJKS

  return UTF8_CHAR_KIND_NONE;
}

int CXdbFilter::convUTF8ToUTF16(unsigned char* utf8_code_data, unsigned int* data, int data_length ) 
{
  unsigned char* pUTF8_CODE;
  unsigned char  UTF8_CODE;
  unsigned char  UTF8_CODE_NEXT[2];
  unsigned char  hiByte, loByte;
  int result_length;

  if( NULL == utf8_code_data )
    return false;

  if( NULL == data )
    return false;

  if( 0 == data_length )
    return false;

  pUTF8_CODE = utf8_code_data;
  result_length  = 0;

  while( 0x00 != (UTF8_CODE = *pUTF8_CODE) ) {
    if( (UTF8_CODE > 0x00) && (UTF8_CODE <= 0x7F)   ) { 
      // 000000 - 00007F
      data[result_length++] = (unsigned int)UTF8_CODE;
    } else if( (UTF8_CODE >= 0xC0) && (UTF8_CODE <= 0xDF) ) { 
      // 110yyyyy(C0-DF) 10zzzzzz(80-BF)  UTF-8
      // 00000yyy yyzzzzzz(UTF-16)
      UTF8_CODE_NEXT[0] = *(pUTF8_CODE+1);
      if( (UTF8_CODE_NEXT[0] >= 0x80) && (UTF8_CODE_NEXT[0] <= 0xBF) ) {
        hiByte = (UTF8_CODE & 0x1C) >> 2;
        loByte = ((UTF8_CODE & 0x03) << 6) |(UTF8_CODE_NEXT[0] & 0x3f);
        data[result_length++] = ((unsigned int)hiByte << 8) | ((unsigned int)loByte);
        pUTF8_CODE++;
      }
      else {
        data[result_length++] = (unsigned int)UTF8_CODE;
      }
    } else if( (UTF8_CODE >= 0xE0) && (UTF8_CODE <= 0xEF) ) { 
      // 1110xxxx(E0-EF)  10yyyyyy(80-BF) 10zzzzzz(80-BF) UTF-8
      // xxxxyyyy yyzzzzzz(UTF-16)
      UTF8_CODE_NEXT[0] = *(pUTF8_CODE+1);
      UTF8_CODE_NEXT[1] = *(pUTF8_CODE+2);

      if( ((UTF8_CODE_NEXT[0] >= 0x80) && (UTF8_CODE_NEXT[0] <= 0xBF)) && ((UTF8_CODE_NEXT[1] >= 0x80) && (UTF8_CODE_NEXT[1] <= 0xBF)) ) {
        hiByte = ((UTF8_CODE & 0x0f) << 4) | ((UTF8_CODE_NEXT[0] & 0x3C) >> 2);
        loByte = ((UTF8_CODE_NEXT[0] & 0x03) << 6) | (UTF8_CODE_NEXT[1] & 0x3F);
        data[result_length++] = ((unsigned int)hiByte << 8) | ((unsigned int)loByte);
        pUTF8_CODE++;
        pUTF8_CODE++;
      }
      else {
        data[result_length++] = (unsigned int)UTF8_CODE;
      }
    } else {
      data[result_length++] = (unsigned int)UTF8_CODE;
    }

    pUTF8_CODE++;
  }

    return result_length;
}

int CXdbFilter::isAllChineseToken( char* token_content, int token_len ) {
  int idx = 0;
  int kind;
  int retLength;
  unsigned int utf16_data[10];

  if( 3 == token_len ) {
    retLength = convUTF8ToUTF16((unsigned char*)token_content, utf16_data, 10 );

    if( 1 == retLength ) {
      // check for symbol
      if( (utf16_data[0] <= 0x3400) || (utf16_data[0] >= 0xFAD9) )
        return false;
    }
  }

  while( idx < token_len ) {
    kind = getUTF8CharKind( (unsigned char)token_content[idx] ) ;

    if( UTF8_CHAR_KIND_3 != kind )
      return false;

    idx += kind;
  }

  return true;
}

int CXdbFilter::isValidChineseToken( scws_res_t cur, char* token_content ) {

  if( cur->len < 3 )
    return false;

  if( 0 == strcasecmp(cur->attr, "en") )
    return false;

  if( (0 == strcasecmp(cur->attr, "un")) || (0 == strcasecmp(cur->attr, "nz"))  ) {
    if( 0 == isAllChineseToken( token_content, cur->len ) )
      return false;
  }

  return true;
}

int CXdbFilter::TokenTotalChineseWordCountGet(const char* str)
{
  int token_len = 0;
  int idx = 0;
  int kind = 0;
  int TotalChineseWordCount = 0;

  TotalChineseWordCount = 0;
  token_len = strlen(str);

  if (0 == token_len)
  {
  	TotalChineseWordCount = 0;
    return TotalChineseWordCount;
  }
  
  while (idx < token_len) 
  {
    kind = getUTF8CharKind( (unsigned char)str[idx] ) ;

    if ( UTF8_CHAR_KIND_3 != kind )
      return TotalChineseWordCount;

    TotalChineseWordCount++;
    idx += kind;
  }
  //printf("TokenTotalChineseWordCountGet[%s]=%d\n", str, TotalChineseWordCount);
  return TotalChineseWordCount;
}

int CXdbFilter::NthChineseWordByteOffsetGet(const char* str, int NthChineseWord)
{
  int token_len = 0;
  int idx = 0;
  int kind = 0;
  int TotalChineseWordCount = 0;

  TotalChineseWordCount = TokenTotalChineseWordCountGet(str);

  if (NthChineseWord > TotalChineseWordCount)
    NthChineseWord = TotalChineseWordCount;
  
  token_len = strlen(str);

  if (0 == token_len)
  {
  	idx = 0;
    return idx;
  }
  
  while (idx < token_len) 
  {
    kind = getUTF8CharKind( (unsigned char)str[idx] ) ;

    if ( UTF8_CHAR_KIND_3 != kind )
      return (idx - UTF8_CHAR_KIND_3);

    if ((idx / UTF8_CHAR_KIND_3) == NthChineseWord)
    {
      //printf("NthChineseWordByteOffsetGet: str:[%s] Nth:[%d] offset:[%d]\n", str, NthChineseWord, idx);
      return idx;
    }
    
    idx += kind;
  }

  return idx;
}

////////////////////// SuffixTokenMap /////////////////////////////////////
int CXdbFilter::SuffixTokenMapInit(std::map<std::string,int> &suffix_token_map)
{
  FILE *fp_table;
  std::string file_path;
  std::string suffix_tok;
  int line_no = 0;
  int tok_no = 0;
  
  file_path = iInputPath + iInputSpecialSuffixTable;
  fp_table = fopen(file_path.c_str(), "r");
  if(NULL == fp_table)
  {
    printf("Error fopen suffix table file: %s\n", file_path.c_str());
    return false;
  }

  suffix_token_map.clear();
  while( NULL != fgets(tmp_buf1, sizeof(tmp_buf1), fp_table) ) 
  {
    line_no++;
    
    if (1 == line_no) // for skip BOM
    {
      //printf("1tmp_buf1[0]:0x%X ",tmp_buf1[0]);
      //printf("1tmp_buf1[1]:0x%X ",tmp_buf1[1]);
      //printf("1tmp_buf1[2]:0x%X\n",tmp_buf1[2]);
      if (((unsigned char)0xEF==(unsigned char)tmp_buf1[0]) && 
            ((unsigned char)0xBB==(unsigned char)tmp_buf1[1]) && 
            ((unsigned char)0xBF==(unsigned char)tmp_buf1[2]))
      {
        for (int i = 0; i < (strlen(tmp_buf1)-2); i++)
          tmp_buf1[i] = tmp_buf1[i+3];
      }
    }

    strtok(tmp_buf1,"\n\r");
    if (3 > strlen(tmp_buf1))
      continue;
    if ((tmp_buf1[0] == '#') || (tmp_buf1[0] == ';'))
      continue;

    //printf("SuffixTable line_no:%d = %s", line_no, tmp_buf1);
    //strtok(tmp_buf1,"\n\r");
    suffix_tok = tmp_buf1;
    if (suffix_token_map.end() == suffix_token_map.find(suffix_tok))
    {
      // Word not found, add to map.
      suffix_token_map.insert( std::pair<std::string,int>(suffix_tok, ++tok_no) );
      //printf("add suffix_tok:%s|%d\n", suffix_tok.c_str(),tok_no);
    }
  }
  fclose(fp_table);

  return true;
}

int CXdbFilter::MaxSuffixTokenLengthGet(std::map<std::string,int> &suffix_token_map)
{
  int MaxLenInBytes = 0;
  int token_len = 0;
  
  for (std::map<std::string,int>::iterator it = suffix_token_map.begin(); it != suffix_token_map.end(); ++it)
  {
    token_len = it->first.length();//strlen((const char*)it->first.c_str());
    if (token_len > MaxLenInBytes)
    {
      MaxLenInBytes = token_len;
    }
  }
  
  return MaxLenInBytes / UTF8_CHAR_KIND_3;
}
////////////////////// SuffixTokenMap /////////////////////////////////////

int CXdbFilter::isTokenEndWithIgnoredSuffix(const char* str, int* pSuffixOff, char* tmp_buf, int MaxSuffixTokenLength, std::map<std::string,int> &suffix_token_map)
{
  int token_len = 0;
  int TokenTotalChineseWordCount = 0;
  int SuffixWordCnt = 0;
  int NthChineseWordByteOffset;
  char* pStr;

  std::string SuffixStr;

  strcpy(tmp_buf, str);
  strtok(tmp_buf,"\n\r");
  token_len = strlen(tmp_buf);
  
  if (UTF8_CHAR_KIND_3 > token_len)
      return false;

  TokenTotalChineseWordCount = TokenTotalChineseWordCountGet(tmp_buf);
  //printf("str[%s] TokenTotalChineseWordCount[%d] MaxSuffixTokenLength[%d]\n", str, TokenTotalChineseWordCount, MaxSuffixTokenLength);

  if (MaxSuffixTokenLength >= TokenTotalChineseWordCount)
    MaxSuffixTokenLength = TokenTotalChineseWordCount - 1;
  if (0 == MaxSuffixTokenLength)
      return false;
  
  for (SuffixWordCnt = MaxSuffixTokenLength; SuffixWordCnt > 0; SuffixWordCnt--)
  {
    NthChineseWordByteOffset = NthChineseWordByteOffsetGet(tmp_buf, (TokenTotalChineseWordCount - SuffixWordCnt));
    pStr = tmp_buf + NthChineseWordByteOffset;
    //printf("SuffixWordCnt[%d] NthChineseWordByteOffset[%d] pStr[%s]\n", SuffixWordCnt, NthChineseWordByteOffset, pStr);

    // Compare pStr with all words(with length SuffixWordCnt) in _ignored_suffix_table_{tc|sc}_utf8.txt.
    SuffixStr = pStr;
    if (suffix_token_map.end() != suffix_token_map.find(SuffixStr))
    {
      // Found suffix!
      *pSuffixOff = NthChineseWordByteOffset;
      //printf("str[%s] Suffix[%s] *pSuffixOff[%d]\n", str, SuffixStr.c_str(), *pSuffixOff);
      return true;
    }
  }
  
  return false;
}

int CXdbFilter::TokenGetOneWord(const char* str, char* tmp_buf, size_t aBufferLength, int wordIdx)
{
  int token_len = 0;
  int idx = 0;
  int kind = 0;
  int found = 0;
  
  //printf("TokenGetOneWord str:[%s] wordIdx:%d\n", str, wordIdx);
  memset(tmp_buf , 0, aBufferLength);
  
  token_len = strlen(str);

  if (3 > token_len)
    return false;
  
  while (idx < token_len) 
  {
    kind = getUTF8CharKind( (unsigned char)str[idx] ) ;

    if( UTF8_CHAR_KIND_3 != kind )
      return false;

    if ((idx / UTF8_CHAR_KIND_3) == wordIdx)
    {
      found = 1;
      memcpy( tmp_buf, &str[idx], UTF8_CHAR_KIND_3 );
      tmp_buf[UTF8_CHAR_KIND_3] = '\0';   /* null character manually added */
      //printf("TokenGetOneWord %s\n", tmp_buf);
      break;
    }

    idx += kind;
  }
    
  return found;
}

#ifdef _CONVERT_NORMALIZE_
int CXdbFilter::CHomophoneNormalizer_Init(const char* aFile)
{
  FILE* fd;
  char  line[256];
  
  fd = fopen(aFile, "r");
  if(NULL == fd)
  {
    printf("Error fopen homophone map file: %s\n", aFile);
    return false;
  }

  int ct = 0;
  while(fgets(line, sizeof(line), fd)) 
  {
    strtok(line, "\r\n");
  
    const char *pinyin, *src, *dst;

    pinyin = strtok(line, ",");
    src    = pinyin ? strtok(NULL, ",") : NULL;
    dst    = src ?    strtok(NULL, ",") : NULL;

    if(pinyin && src && dst)
    {
      //iMap.Set((CHomophoneMap::TUnit) src, (CHomophoneMap::TUnit) dst);
      if (iMap.end() == iMap.find(src))
      {
        iMap.insert( std::pair<std::string,std::string>(src,dst) );
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
  const unsigned char* aInput = (const unsigned char*) aSource;

  //printf("CHomophoneNormalizer_Normalize aSource:%s aLength:%d\n", aSource, aLength);
  
  if((i = strlen(aSource)) + 1 > aLength)
  {
    retLength = i + 1; // aOutput length equals to aSource
    return retLength; 
  }

  for(i = 0; aInput[i];)
  {
    size_t lenMB = g_mblen_table_utf8[aInput[i]];

    if(KCJKBytes != lenMB) /* not CJK */
    {
      memcpy(aOutput + i, aInput + i, lenMB);
    }
    else  /* CJK */
    {
      //if(iMap.IsContained(aInput+i)) 
      std::string CJKStr((const char*)(aInput+i), lenMB);
      if (iMap.end() != iMap.find(CJKStr))
      {
        memcpy(aOutput + i, iMap[CJKStr].data(), lenMB);
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
#endif //_CONVERT_NORMALIZE_






