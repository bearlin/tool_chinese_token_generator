//
// This file contains the CXdbDumper implementation 
//

#include "XdbDumper.h"
#include <iostream>

CXdbDumper::CXdbDumper() :
  word_count(0)
  #ifdef _CONVERT_NORMALIZE_
  , _mblen_table_utf8 {
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
  4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1 }
  #endif //_CONVERT_NORMALIZE_
{
  std::cout << "CXdbDumper" << std::endl;
}

CXdbDumper::~CXdbDumper()
{
  std::cout << "~CXdbDumper" << std::endl;
}

bool CXdbDumper::Run()
{
  std::cout << "Running CXdbDumper" << std::endl;
  std::cout << "iInputPath:" << GetConfig().iInputPath << std::endl;
  std::cout << "iOutputPath:" << GetConfig().iOutputPath<< std::endl;
  std::cout << "iLogPath:" << GetConfig().iLogPath<< std::endl;

  FILE *fd;
  struct xdb_header xdb_header;
  int trav_index;
  unsigned int f_offset;
  unsigned int d_offset;
  unsigned int d_length;
  size_t readSize = 0;

#ifdef _DETAIL_EXPORT_
  file_path = GetConfig().iOutputPath+DATA_DIR;
  file_path += DETAIL_EXP_FILE;
  gLogDetail = fopen(file_path.c_str(), "w");
  if (NULL == gLogDetail)
  {
    printf("gLogDetail err:%s\n", file_path.c_str());
    return false;
  }
  printf("gLogDetail :%s\n", file_path.c_str());
#endif
#ifdef _XDB_GEN_TOOL_EXPORT_
  file_path = GetConfig().iOutputPath+DATA_DIR;
  file_path += SIMPLE_EXP_FILE;
  gLog = fopen(file_path.c_str(), "w");
  if (NULL == gLog)
  {
    printf("gLog err:%s\n", file_path.c_str());
    return false;
  }
  printf("gLog :%s\n", file_path.c_str());
#endif

  file_path = GetConfig().iInputPath+DATA_DIR;
  file_path += SRC_XDB_FILE;
  fd = fopen(file_path.c_str(),"rb");
  if (NULL == fd)
  {
    printf("fd err:%s\n", file_path.c_str());
    return false;
  }
  printf("fd :%s\n", file_path.c_str());

#ifdef _CONVERT_NORMALIZE_
  // Read mapping into tables
  FILE *normalize_fd;
  char szLine[256], *pFind;
  int token_idx,szLine_Len;
  char szSrc[4], szDst[4];

  file_path = GetConfig().iOutputPath+DATA_DIR;
  file_path += NORMAL_EXP_FILE;
  gNormalizeLog = fopen(file_path.c_str(), "w" ); 
  if (NULL == gNormalizeLog)
  {
    printf("gNormalizeLog err:%s\n", file_path.c_str());
    return false;
  }
  printf("gNormalizeLog :%s\n", file_path.c_str());

  file_path = GetConfig().iInputPath+DATA_DIR;
  file_path += NORMAL_MAP_FILE;
  normalize_fd = fopen(file_path.c_str(), "r" );
  if (NULL == normalize_fd)
  {
    printf("normalize_fd err:%s\n", file_path.c_str());
    return false;
  }
  printf("normalize_fd :%s\n", file_path.c_str());

  // For repeat normalized tokens log.
  nor_map.clear();
  nor_repeat_vector.clear();

  file_path = GetConfig().iOutputPath+DATA_DIR;
  file_path += NORMAL_EXP_REPEAT;
  gNormalizeRepeatLog = fopen(file_path.c_str(), "w");
  if (NULL == gNormalizeRepeatLog)
  {
    printf("gNormalizeRepeatLog err:%s\n", file_path.c_str());
    return false;
  }
  printf("gNormalizeRepeatLog :%s\n", file_path.c_str());
  
  while (fgets(szLine, sizeof(szLine), normalize_fd)) 
  {
    szLine_Len = strlen(szLine);
    szLine_Len--;

    while ( ('\r' == szLine[szLine_Len]) || ('\n' == szLine[szLine_Len]) )
    {
      szLine_Len--;
    }

    szLine[szLine_Len+1] = 0;

    token_idx = 0;
    pFind = strtok(szLine, ",");
    while (pFind) 
    {
      if (1 == token_idx) 
      {
        memset(szSrc,0,sizeof(szSrc));
        strcpy(szSrc, pFind);
      } 
      else if (2 == token_idx)
      {
        memset(szDst,0,sizeof(szDst));
        strcpy(szDst, pFind);
      }
      pFind = strtok(NULL, ",");
      token_idx++;
    }
    g_normalize_hash[szSrc] = szDst;
  }
  fclose(normalize_fd);
#endif //_CONVERT_NORMALIZE_

  printf("XDB header size=%ld\n", sizeof(struct xdb_header));
  readSize = fread(&xdb_header, 1, sizeof(struct xdb_header), fd);
  if (readSize != sizeof(struct xdb_header)*1)
  {
    fputs("Reading error 01", stderr);
    exit(EXIT_FAILURE);
  }

  //printf("unsigned int size=%d\n", sizeof(unsigned int) );
  printf("unsigned base=%d\n", xdb_header.base);
  printf("unsigned size=%d\n", xdb_header.fsize);
  printf("unsigned prime=%d\n", xdb_header.prime);

  gPrime = xdb_header.prime;
  gHashBase = xdb_header.base;

  //sprintf(szLog, "DB base=%d size=%d prime=%d\n", xdb_header.base, xdb_header.fsize, xdb_header.prime );
  //fputs(szLog, gLog );

  trav_index = 0;

  do 
  {
    f_offset = trav_index*8+32;
    fseek(fd, f_offset, SEEK_SET);
    readSize = fread(&d_offset, 1, sizeof(unsigned int), fd);
    if (readSize != sizeof(unsigned int)*1)
    {
      fputs("Reading error 02", stderr);
      exit(EXIT_FAILURE);
    }

    readSize = fread(&d_length, 1, sizeof(unsigned int), fd);
    if (readSize != sizeof(unsigned int)*1)
    {
      fputs("Reading error 03", stderr);
      exit(EXIT_FAILURE);
    }

    if( 0 != d_length ) 
    {
      printf("trav_index=%d d_offset=%d d_length=%d\n", trav_index, d_offset, d_length);
      get_record(fd, d_offset, d_length, 0, 0, "");
      word_count = 0;

      //sprintf(szLog, "+++++++++++++++++++Prime_index=%d\n", trav_index );
      //fputs(szLog, gLog );
    }
    trav_index++;
  } while ( trav_index < xdb_header.prime );

#ifdef _CONVERT_NORMALIZE_
  char *pFindPrev;
  char *pFindCurr;
  char szLinePrev[256],szLineCurr[256];
  int repeatCnt;
  int toklen;
  int total_cnt;

  // For repeat normalized tokens log.
  // sort and log nor_repeat_vector to file.
  std::sort(nor_repeat_vector.begin(), nor_repeat_vector.end()); // using default comparison (operator <):

  repeatCnt = 0;
  toklen = 0;
  total_cnt = 0;
  for (std::vector<std::string>::iterator nor_repeat_vector_it = nor_repeat_vector.begin() ; nor_repeat_vector_it != nor_repeat_vector.end(); ++nor_repeat_vector_it)
  {
    // Collect statistic informations.
    //----------------------------------------
    //printf("repeatCnt(%d)\n", repeatCnt);
    if (nor_repeat_vector_it == nor_repeat_vector.begin())
    {
      strcpy(szLinePrev, (*nor_repeat_vector_it).c_str());
      //printf("11AA szLinePrev %s", szLinePrev);
      pFindPrev = strtok(szLinePrev, "<");
      //printf("11BB pFindPrev %s\n", pFindPrev);
      repeatCnt++;
      fprintf(gNormalizeRepeatLog, "------------------------ [X] x (X) \n");
    }
    else
    {
      strcpy(szLineCurr, (*nor_repeat_vector_it).c_str());
      //printf("22AA szLineCurr %s", szLineCurr);
      pFindCurr = strtok(szLineCurr, "<");
      //printf("22BB pFindPrev %s\n", pFindPrev);
      //printf("22BB pFindCurr %s\n", pFindCurr);

      if (strcmp(pFindPrev, pFindCurr) == 0)
      {
        repeatCnt++;
      }
      else
      {
        toklen = strlen(pFindPrev);
        fprintf(gNormalizeRepeatLog, "------------------------ [%d] x (%d) \n", toklen/3, repeatCnt);
        total_cnt++;
        
        strcpy(szLinePrev, szLineCurr);
        pFindPrev = strtok(szLinePrev, "<");
        
        repeatCnt = 1;
      }
    }
  //----------------------------------------
    
  // Just log this item.
    fprintf(gNormalizeRepeatLog, "%s", (*nor_repeat_vector_it).c_str());
  }
  
  if (repeatCnt)
  {
    fprintf(gNormalizeRepeatLog, "------------------------ [%d] x (%d) \n", toklen/3, repeatCnt);
    total_cnt++;
  }
  if (total_cnt)
  {
    fprintf(gNormalizeRepeatLog, "total_cnt:%d\n", total_cnt);
  }
#endif //_CONVERT_NORMALIZE_

  fclose(fd);
  fclose(gLog); 
#ifdef _CONVERT_NORMALIZE_
  fclose(gNormalizeLog); 
  fclose(gNormalizeRepeatLog); 
#endif

  return true;
}

CXdbDumperConfig& CXdbDumper::GetConfig()
{
  return iConfig;
}

int CXdbDumper::_get_index(unsigned char* key, int hash_base, int hash_prime )
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

void CXdbDumper::get_record(FILE *fd, unsigned int off, unsigned int len, int direction, int level, const char* father) {
  unsigned int l_offset, l_len;
  unsigned int r_offset, r_len;
  char k_len;
  unsigned char key_name[256];
  unsigned char attr[4];
  node_content ct;
  size_t readSize = 0;

  fseek(fd, off, SEEK_SET);
  //r_len = XDB_MAXKLEN+17;
  //if(r_len > len) r_len = len;
  //readSize = fread(buff, 1, r_len, fd);  
  //if (readSize != r_len*1) {fputs("Reading error 04", stderr); exit(EXIT_FAILURE);}

  readSize = fread(&l_offset, 1, sizeof(unsigned int), fd);
  if (readSize != sizeof(unsigned int)*1)
  {
    fputs("Reading error 05", stderr);
    exit(EXIT_FAILURE);
  }

  readSize = fread(&l_len, 1, sizeof(unsigned int), fd);
  if (readSize != sizeof(unsigned int)*1)
  {
    fputs("Reading error 06", stderr);
    exit(EXIT_FAILURE);
  }

  readSize = fread(&r_offset, 1, sizeof(unsigned int), fd);
  if (readSize != sizeof(unsigned int)*1)
  {
    fputs("Reading error 07", stderr);
    exit(EXIT_FAILURE);
  }

  readSize = fread(&r_len, 1, sizeof(unsigned int), fd);
  if (readSize != sizeof(unsigned int)*1)
  {
    fputs("Reading error 08", stderr);
    exit(EXIT_FAILURE);
  }

  readSize = fread(&k_len, 1, sizeof(unsigned char), fd);
  if (readSize != sizeof(unsigned char)*1)
  {
    fputs("Reading error 09", stderr);
    exit(EXIT_FAILURE);
  }

  memset(key_name, 0, sizeof(key_name));
  //key_name[0] = '\"';
  //readSize = fread(&key_name[0], sizeof(unsigned char), k_len, fd );
  //if (readSize != sizeof(unsigned char)*k_len) {fputs("Reading error 10", stderr); exit(EXIT_FAILURE);}
  readSize = fread(key_name, sizeof(unsigned char), k_len, fd);
  if (readSize != sizeof(unsigned char)*k_len)
  {
    fputs("Reading error 11", stderr);
    exit(EXIT_FAILURE);
  }
  //key_name[k_len+1] = '\"';
  //key_name[k_len+2] = '\n';
  //fputs((const char*)key_name, gLog);
  readSize = fread(&ct, 1, sizeof(node_content), fd);
  if (readSize != sizeof(node_content)*1)
  {
    fputs("Reading error 12", stderr);
    exit(EXIT_FAILURE);
  }

  if ((l_offset > 0) && (l_len > 0))
  {
    get_record(fd, l_offset, l_len, 1, level+1,(char*)key_name);
  }

  if ((r_offset > 0) && (r_len > 0))
  {
    get_record(fd, r_offset, r_len, 2, level+1,(char*)key_name);
  }

  word_count++;
  /*
  printf("Level[%d] Dir=%c word[%ld] l_offset=%ld l_len=%ld r_offset=%d r_len=%d k_len=%d father=%s tf=%f idf=%f flag=%d attr[0]=%c key=%s\n", 
    level, (0 == direction) ? 'N': (1 == direction) ?'L':'R', word_count, l_offset, l_len, r_offset, r_len, k_len, father, 
    ct.tf, ct.idf, ct.flag, ct.attr[0], key_name );
    */

  //sprintf(szLog, "%s\n", key_name ); 

#ifdef _DETAIL_EXPORT_
  // Detail log
  sprintf(szLog, "Level[%d] Dir=%c word[%ld] l_offset=%ld l_len=%ld r_offset=%d r_len=%d k_len=%d father=%s tf=%f idf=%f flag=%d attr[0]=%c attr[1]=%c attr[2]=%c key=\"%s\" prime_index=%d\n", 
    level, (0 == direction) ? 'N': (1 == direction) ?'L':'R', word_count, l_offset, l_len, r_offset, r_len, k_len, father, 
    ct.tf, ct.idf, ct.flag, 
    (0 == ct.attr[0]) ? 0x01:ct.attr[0],
    (0 == ct.attr[1]) ? 0x01:ct.attr[1],
    (0 == ct.attr[2]) ? 0x01:ct.attr[2],
    key_name,
    _get_index(key_name,gHashBase, gPrime)
    );
    fputs(szLog, gLogDetail);
  // Detail log
#endif

  memset(attr, 0, 4);
#ifdef _XDB_GEN_TOOL_EXPORT_
  // OWN toolsusage
  memcpy(attr, ct.attr, 3);
  sprintf(szLog, "%s\t%f\t%f\t%d\t%s\n", key_name, ct.tf, ct.idf, ct.flag, attr);
  fputs(szLog, gLog);
  // PHP usage
#endif

#ifdef _CONVERT_NORMALIZE_
  unsigned char normal_key_name[256];
  unsigned char cht_utf8[4];
  int key_len;
  int char_utf8_len;
  std::string char_utf8;
  std::map<std::string, std::string> ::const_iterator findIter;

  // convert by normalize hash
  memset(normal_key_name, 0, sizeof(normal_key_name));

  key_len = strlen((char*)key_name);

  for (int z =0; z < key_len; )
  {
    char_utf8_len = _mblen_table_utf8[key_name[z]];

    if (3 != char_utf8_len) 
    {
      memcpy(&normal_key_name[z], &key_name[z], char_utf8_len);
    } 
    else 
    {
      memset(cht_utf8, 0, sizeof(cht_utf8));
      memcpy(cht_utf8, &key_name[z], char_utf8_len);

      char_utf8.clear();
      char_utf8 += (char*)cht_utf8;

      findIter = g_normalize_hash.find(char_utf8);
      if (findIter == g_normalize_hash.end()) 
      {
        printf("NOT Found  [%x][%x][%x]...................\n", cht_utf8[0], cht_utf8[1], cht_utf8[2]);
        memcpy(&normal_key_name[z], &key_name[z], char_utf8_len);
      } 
      else 
      {
        memcpy(&normal_key_name[z], g_normalize_hash[char_utf8].c_str(), char_utf8_len);
      }
    }

    z += char_utf8_len;
  }

  // For repeat normalized tokens log.
  //-----------------------------------------------------------------
  if (key_len > 3)
  {
    std::string ori_str;
    std::string nor_str;
    std::map<std::string,node_content>::iterator nor_map_it;
    std::vector<std::string>::iterator nor_repeat_vector_it;

    ori_str = (char*)key_name;
    nor_str = (char*)normal_key_name;
    nor_map_it = nor_map.find(nor_str);
    if (nor_map.end() != nor_map_it)
    {
      // This is repeat string.
      //printf("Repeat: [%s<=%s\t%f\t%f\t%d\t%s] v.s. ",nor_str.c_str(), first_nor_to_ori_map[nor_str].c_str(), nor_map_it->second.tf, nor_map_it->second.idf, nor_map_it->second.flag, nor_map_it->second.attr);
      //printf("[%s<=%s\t%f\t%f\t%d\t%s]\n",normal_key_name, key_name, ct.tf, ct.idf, ct.flag, ct.attr);

      // get original one info.
      sprintf(szLog, "%s<=%s\t%f\t%f\t%d\t%s\n",nor_str.c_str(), first_nor_to_ori_map[nor_str].c_str(), 
              nor_map_it->second.tf, nor_map_it->second.idf, nor_map_it->second.flag, nor_map_it->second.attr);
      
      // is original one info in vector?
      nor_repeat_vector_it = std::find(nor_repeat_vector.begin(), nor_repeat_vector.end(), szLog);
      if (nor_repeat_vector.end() != nor_repeat_vector_it)
      {
        // original one alread log in vector, log just "repeat one".

        // repeat one
        sprintf(szLog, "%s<=%s\t%f\t%f\t%d\t%s\n",normal_key_name, key_name, ct.tf, ct.idf, ct.flag, ct.attr);
        nor_repeat_vector.push_back(szLog);
      }
      else
      {
        // original one not in vector yet, log "original one" and "repeat one".

        // original one
        nor_repeat_vector.push_back(szLog);

        // repeat one
        sprintf(szLog, "%s<=%s\t%f\t%f\t%d\t%s\n",normal_key_name, key_name, ct.tf, ct.idf, ct.flag, ct.attr);
        nor_repeat_vector.push_back(szLog);
      }
    }
    else
    {
      // This is first time we get this key, so save infos to nor_map and first_nor_to_ori_map.
      nor_map.insert(std::pair<std::string,node_content>(nor_str,ct));
      first_nor_to_ori_map.insert(std::pair<std::string,std::string>(nor_str,ori_str));
    }
  }
  //-----------------------------------------------------------------
  
  memcpy(attr, ct.attr, 3);
  sprintf(szLog, "%s\t%f\t%f\t%d\t%s\n", normal_key_name, ct.tf, ct.idf, ct.flag, attr);
  fputs(szLog, gNormalizeLog);
#endif //_CONVERT_NORMALIZE_
}

