//
// This file contains the CXdbDumper implementation 
//

#include "XdbDumper.h"
#include <iostream>

CXdbDumper::CXdbDumper() :
  iWordCount(0)
  ,iPrime(0)
  ,iHashBase(0)
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
  struct xdb_header xdbHeader;
  int travelIndex;
  unsigned int forwardOffset;
  unsigned int destinationOffset;
  unsigned int destinationLength;
  size_t readSize = 0;

#ifdef _DETAIL_EXPORT_
  iFilePath = GetConfig().iLogPath + DETAIL_EXP_FILE;
  iLogDetail = fopen(iFilePath.c_str(), "w");
  if (NULL == iLogDetail)
  {
    printf("iLogDetail err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("iLogDetail :%s\n", iFilePath.c_str());
#endif
#ifdef _XDB_GEN_TOOL_EXPORT_
  GetConfig().iOutputDumpText = SIMPLE_EXP_FILE;
  iFilePath = GetConfig().iOutputPath + GetConfig().iOutputDumpText;
  iLog = fopen(iFilePath.c_str(), "w");
  if (NULL == iLog)
  {
    printf("iLog err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("iLog :%s\n", iFilePath.c_str());
#endif

  iFilePath = GetConfig().iInputPath + GetConfig().iInputScwsXdb;
  fd = fopen(iFilePath.c_str(),"rb");
  if (NULL == fd)
  {
    printf("fd err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("fd :%s\n", iFilePath.c_str());

#ifdef _CONVERT_NORMALIZE_
  // Read mapping into tables
  FILE *normalizeFd;
  char szLine[256], *pFind;
  int tokenIndex,szLineLength;
  char szSrc[4], szDst[4];

  GetConfig().iOutputDumpText = NORMAL_EXP_FILE;
  iFilePath = GetConfig().iOutputPath + GetConfig().iOutputDumpText;
  iNormalizeLog = fopen(iFilePath.c_str(), "w" ); 
  if (NULL == iNormalizeLog)
  {
    printf("iNormalizeLog err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("iNormalizeLog :%s\n", iFilePath.c_str());

  iFilePath = GetConfig().iInputPath + GetConfig().iInputNormalizeMap;
  normalizeFd = fopen(iFilePath.c_str(), "r" );
  if (NULL == normalizeFd)
  {
    printf("normalizeFd err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("normalizeFd :%s\n", iFilePath.c_str());

  // For repeat normalized tokens log.
  iNormalizeMap.clear();
  iNormalizeRepeatVector.clear();

  iFilePath = GetConfig().iLogPath + NORMAL_EXP_REPEAT;
  iNormalizeRepeatLog = fopen(iFilePath.c_str(), "w");
  if (NULL == iNormalizeRepeatLog)
  {
    printf("iNormalizeRepeatLog err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("iNormalizeRepeatLog :%s\n", iFilePath.c_str());
  
  while (fgets(szLine, sizeof(szLine), normalizeFd)) 
  {
    szLineLength = strlen(szLine);
    szLineLength--;

    while ( ('\r' == szLine[szLineLength]) || ('\n' == szLine[szLineLength]) )
    {
      szLineLength--;
    }

    szLine[szLineLength + 1] = 0;

    tokenIndex = 0;
    pFind = strtok(szLine, ",");
    while (pFind) 
    {
      if (1 == tokenIndex) 
      {
        memset(szSrc,0,sizeof(szSrc));
        strcpy(szSrc, pFind);
      } 
      else if (2 == tokenIndex)
      {
        memset(szDst,0,sizeof(szDst));
        strcpy(szDst, pFind);
      }
      pFind = strtok(NULL, ",");
      tokenIndex++;
    }
    iNormalizeHash[szSrc] = szDst;
  }
  fclose(normalizeFd);
#endif //_CONVERT_NORMALIZE_

  printf("XDB header size=%ld\n", sizeof(struct xdb_header));
  readSize = fread(&xdbHeader, 1, sizeof(struct xdb_header), fd);
  if (readSize != sizeof(struct xdb_header)*1)
  {
    fputs("Reading error 01", stderr);
    exit(EXIT_FAILURE);
  }

  //printf("unsigned int size=%d\n", sizeof(unsigned int) );
  printf("unsigned base=%d\n", xdbHeader.base);
  printf("unsigned size=%d\n", xdbHeader.fsize);
  printf("unsigned prime=%d\n", xdbHeader.prime);

  iPrime = xdbHeader.prime;
  iHashBase = xdbHeader.base;

  //sprintf(szLog, "DB base=%d size=%d prime=%d\n", xdb_header.base, xdb_header.fsize, xdb_header.prime );
  //fputs(szLog, iLog );

  travelIndex = 0;

  do 
  {
    forwardOffset = travelIndex * 8 + 32;
    fseek(fd, forwardOffset, SEEK_SET);
    readSize = fread(&destinationOffset, 1, sizeof(unsigned int), fd);
    if (readSize != sizeof(unsigned int)*1)
    {
      fputs("Reading error 02", stderr);
      exit(EXIT_FAILURE);
    }

    readSize = fread(&destinationLength, 1, sizeof(unsigned int), fd);
    if (readSize != sizeof(unsigned int)*1)
    {
      fputs("Reading error 03", stderr);
      exit(EXIT_FAILURE);
    }

    if( 0 != destinationLength ) 
    {
      printf("travelIndex=%d destinationOffset=%d destinationLength=%d\n", travelIndex, destinationOffset, destinationLength);
      GetRecord(fd, destinationOffset, destinationLength, 0, 0, "");
      iWordCount = 0;

      //sprintf(szLog, "+++++++++++++++++++Prime_index=%d\n", trav_index );
      //fputs(szLog, iLog );
    }
    travelIndex++;
  } while (travelIndex < xdbHeader.prime);

#ifdef _CONVERT_NORMALIZE_
  char *pFindPrev;
  char *pFindCurr;
  char szLinePrev[256],szLineCurr[256];
  int repeatCnt;
  int toklen;
  int totalCnt;

  // For repeat normalized tokens log.
  // sort and log iNormalizeRepeatVector to file.
  std::sort(iNormalizeRepeatVector.begin(), iNormalizeRepeatVector.end()); // using default comparison (operator <):

  repeatCnt = 0;
  toklen = 0;
  totalCnt = 0;
  for (std::vector<std::string>::iterator nor_repeat_vector_it = iNormalizeRepeatVector.begin() ; nor_repeat_vector_it != iNormalizeRepeatVector.end(); ++nor_repeat_vector_it)
  {
    // Collect statistic informations.
    //----------------------------------------
    //printf("repeatCnt(%d)\n", repeatCnt);
    if (nor_repeat_vector_it == iNormalizeRepeatVector.begin())
    {
      strcpy(szLinePrev, (*nor_repeat_vector_it).c_str());
      //printf("11AA szLinePrev %s", szLinePrev);
      pFindPrev = strtok(szLinePrev, "<");
      //printf("11BB pFindPrev %s\n", pFindPrev);
      repeatCnt++;
      fprintf(iNormalizeRepeatLog, "------------------------ [X] x (X) \n");
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
        fprintf(iNormalizeRepeatLog, "------------------------ [%d] x (%d) \n", toklen / 3, repeatCnt);
        totalCnt++;
        
        strcpy(szLinePrev, szLineCurr);
        pFindPrev = strtok(szLinePrev, "<");
        
        repeatCnt = 1;
      }
    }
  //----------------------------------------
    
  // Just log this item.
    fprintf(iNormalizeRepeatLog, "%s", (*nor_repeat_vector_it).c_str());
  }
  
  if (repeatCnt)
  {
    fprintf(iNormalizeRepeatLog, "------------------------ [%d] x (%d) \n", toklen/3, repeatCnt);
    totalCnt++;
  }
  if (totalCnt)
  {
    fprintf(iNormalizeRepeatLog, "total_cnt:%d\n", totalCnt);
  }
#endif //_CONVERT_NORMALIZE_

  fclose(fd);
  fclose(iLog); 
#ifdef _CONVERT_NORMALIZE_
  fclose(iNormalizeLog); 
  fclose(iNormalizeRepeatLog); 
#endif

  return true;
}

CXdbDumperConfig& CXdbDumper::GetConfig()
{
  return iConfig;
}

int CXdbDumper::GetIndex(unsigned char* aKey, int aHashBase, int aHashPrime)
{
  int length = strlen((char*)aKey);
  int hashNumber = aHashBase;
  while (length--)
  {
    hashNumber += (int)(hashNumber << 5);
    hashNumber ^= aKey[length];
    hashNumber &= 0x7fffffff;
  }

  return (hashNumber % aHashPrime);
}

void CXdbDumper::GetRecord(FILE *aFd, unsigned int aOffset, unsigned int aLength, int aDirection, int aLevel, const char* aFather) {
  unsigned int leftOffset, leftLength;
  unsigned int rightOffset, rightLength;
  char keyLength;
  unsigned char keyName[256];
  unsigned char attribute[4];
  node_content content;
  size_t readSize = 0;

  fseek(aFd, aOffset, SEEK_SET);
  //r_len = XDB_MAXKLEN+17;
  //if(r_len > len) r_len = len;
  //readSize = fread(buff, 1, r_len, fd);  
  //if (readSize != r_len*1) {fputs("Reading error 04", stderr); exit(EXIT_FAILURE);}

  readSize = fread(&leftOffset, 1, sizeof(unsigned int), aFd);
  if (readSize != sizeof(unsigned int)*1)
  {
    fputs("Reading error 05", stderr);
    exit(EXIT_FAILURE);
  }

  readSize = fread(&leftLength, 1, sizeof(unsigned int), aFd);
  if (readSize != sizeof(unsigned int)*1)
  {
    fputs("Reading error 06", stderr);
    exit(EXIT_FAILURE);
  }

  readSize = fread(&rightOffset, 1, sizeof(unsigned int), aFd);
  if (readSize != sizeof(unsigned int)*1)
  {
    fputs("Reading error 07", stderr);
    exit(EXIT_FAILURE);
  }

  readSize = fread(&rightLength, 1, sizeof(unsigned int), aFd);
  if (readSize != sizeof(unsigned int)*1)
  {
    fputs("Reading error 08", stderr);
    exit(EXIT_FAILURE);
  }

  readSize = fread(&keyLength, 1, sizeof(unsigned char), aFd);
  if (readSize != sizeof(unsigned char)*1)
  {
    fputs("Reading error 09", stderr);
    exit(EXIT_FAILURE);
  }

  memset(keyName, 0, sizeof(keyName));
  //key_name[0] = '\"';
  //readSize = fread(&key_name[0], sizeof(unsigned char), k_len, fd );
  //if (readSize != sizeof(unsigned char)*k_len) {fputs("Reading error 10", stderr); exit(EXIT_FAILURE);}
  readSize = fread(keyName, sizeof(unsigned char), keyLength, aFd);
  if (readSize != sizeof(unsigned char)*keyLength)
  {
    fputs("Reading error 11", stderr);
    exit(EXIT_FAILURE);
  }
  //key_name[k_len+1] = '\"';
  //key_name[k_len+2] = '\n';
  //fputs((const char*)key_name, iLog);
  readSize = fread(&content, 1, sizeof(node_content), aFd);
  if (readSize != sizeof(node_content)*1)
  {
    fputs("Reading error 12", stderr);
    exit(EXIT_FAILURE);
  }

  if ((leftOffset > 0) && (leftLength > 0))
  {
    GetRecord(aFd, leftOffset, leftLength, 1, aLevel + 1,(char*)keyName);
  }

  if ((rightOffset > 0) && (rightLength > 0))
  {
    GetRecord(aFd, rightOffset, rightLength, 2, aLevel + 1,(char*)keyName);
  }

  iWordCount++;
  /*
  printf("Level[%d] Dir=%c word[%ld] l_offset=%ld l_len=%ld r_offset=%d r_len=%d k_len=%d father=%s tf=%f idf=%f flag=%d attr[0]=%c key=%s\n", 
    level, (0 == direction) ? 'N': (1 == direction) ?'L':'R', iWordCount, l_offset, l_len, r_offset, r_len, k_len, father, 
    ct.tf, ct.idf, ct.flag, ct.attr[0], key_name );
    */

  //sprintf(szLog, "%s\n", key_name ); 

#ifdef _DETAIL_EXPORT_
  // Detail log
  sprintf(iSzLog, "Level[%d] Dir=%c word[%ld] l_offset=%ld l_len=%ld r_offset=%d r_len=%d k_len=%d father=%s tf=%f idf=%f flag=%d attr[0]=%c attr[1]=%c attr[2]=%c key=\"%s\" prime_index=%d\n", 
    aLevel, (0 == aDirection) ? 'N': (1 == aDirection) ?'L':'R', iWordCount, leftOffset, leftLength, rightOffset, rightLength, keyLength, aFather, 
    content.tf, content.idf, content.flag, 
    (0 == content.attr[0]) ? 0x01:content.attr[0],
    (0 == content.attr[1]) ? 0x01:content.attr[1],
    (0 == content.attr[2]) ? 0x01:content.attr[2],
    keyName,
    GetIndex(keyName,iHashBase, iPrime)
    );
    fputs(iSzLog, iLogDetail);
  // Detail log
#endif

  memset(attribute, 0, 4);
#ifdef _XDB_GEN_TOOL_EXPORT_
  // OWN toolsusage
  memcpy(attribute, content.attr, 3);
  sprintf(iSzLog, "%s\t%f\t%f\t%d\t%s\n", keyName, content.tf, content.idf, content.flag, attribute);
  fputs(iSzLog, iLog);
  // PHP usage
#endif

#ifdef _CONVERT_NORMALIZE_
  unsigned char normalKeyName[256];
  unsigned char chtUTF8[4];
  int keyLength;
  int charUTF8Length;
  std::string charUTF8;
  std::map<std::string, std::string> ::const_iterator findIterator;

  // convert by normalize hash
  memset(normalKeyName, 0, sizeof(normalKeyName));

  keyLength = strlen((char*)keyName);

  for (int z =0; z < keyLength; )
  {
    charUTF8Length = _mblen_table_utf8[keyName[z]];

    if (3 != charUTF8Length) 
    {
      memcpy(&normalKeyName[z], &keyName[z], charUTF8Length);
    } 
    else 
    {
      memset(chtUTF8, 0, sizeof(chtUTF8));
      memcpy(chtUTF8, &keyName[z], charUTF8Length);

      charUTF8.clear();
      charUTF8 += (char*)chtUTF8;

      findIterator = iNormalizeHash.find(charUTF8);
      if (findIterator == iNormalizeHash.end()) 
      {
        printf("NOT Found  [%x][%x][%x]...................\n", chtUTF8[0], chtUTF8[1], chtUTF8[2]);
        memcpy(&normalKeyName[z], &keyName[z], charUTF8Length);
      } 
      else 
      {
        memcpy(&normalKeyName[z], iNormalizeHash[charUTF8].c_str(), charUTF8Length);
      }
    }

    z += charUTF8Length;
  }

  // For repeat normalized tokens log.
  //-----------------------------------------------------------------
  if (keyLength > 3)
  {
    std::string ori_str;
    std::string nor_str;
    std::map<std::string,node_content>::iterator nor_map_it;
    std::vector<std::string>::iterator nor_repeat_vector_it;

    ori_str = (char*)keyName;
    nor_str = (char*)normalKeyName;
    nor_map_it = iNormalizeMap.find(nor_str);
    if (iNormalizeMap.end() != nor_map_it)
    {
      // This is repeat string.
      //printf("Repeat: [%s<=%s\t%f\t%f\t%d\t%s] v.s. ",nor_str.c_str(), iFirstNormalizeToOriginalMap[nor_str].c_str(), nor_map_it->second.tf, nor_map_it->second.idf, nor_map_it->second.flag, nor_map_it->second.attr);
      //printf("[%s<=%s\t%f\t%f\t%d\t%s]\n",normal_key_name, key_name, ct.tf, ct.idf, ct.flag, ct.attr);

      // get original one info.
      sprintf(iSzLog, "%s<=%s\t%f\t%f\t%d\t%s\n",nor_str.c_str(), iFirstNormalizeToOriginalMap[nor_str].c_str(), 
              nor_map_it->second.tf, nor_map_it->second.idf, nor_map_it->second.flag, nor_map_it->second.attr);
      
      // is original one info in vector?
      nor_repeat_vector_it = std::find(iNormalizeRepeatVector.begin(), iNormalizeRepeatVector.end(), iSzLog);
      if (iNormalizeRepeatVector.end() != nor_repeat_vector_it)
      {
        // original one alread log in vector, log just "repeat one".

        // repeat one
        sprintf(iSzLog, "%s<=%s\t%f\t%f\t%d\t%s\n",normalKeyName, keyName, content.tf, content.idf, content.flag, content.attr);
        iNormalizeRepeatVector.push_back(iSzLog);
      }
      else
      {
        // original one not in vector yet, log "original one" and "repeat one".

        // original one
        iNormalizeRepeatVector.push_back(iSzLog);

        // repeat one
        sprintf(iSzLog, "%s<=%s\t%f\t%f\t%d\t%s\n",normalKeyName, keyName, content.tf, content.idf, content.flag, content.attr);
        iNormalizeRepeatVector.push_back(iSzLog);
      }
    }
    else
    {
      // This is first time we get this key, so save infos to iNormalizeMap and iFirstNormalizeToOriginalMap.
      iNormalizeMap.insert(std::pair<std::string,node_content>(nor_str, content));
      iFirstNormalizeToOriginalMap.insert(std::pair<std::string,std::string>(nor_str,ori_str));
    }
  }
  //-----------------------------------------------------------------
  
  memcpy(attribute, content.attr, 3);
  sprintf(iSzLog, "%s\t%f\t%f\t%d\t%s\n", normalKeyName, content.tf, content.idf, content.flag, attribute);
  fputs(iSzLog, iNormalizeLog);
#endif //_CONVERT_NORMALIZE_
}

