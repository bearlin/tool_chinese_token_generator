//
// This file contains the CXdbDumper implementation 
//

#include "XdbDumper.h"
#include <iostream>

static const int KKeyNameSize = 256;
static const int KAttributeSize = 4;

#ifdef DETAIL_EXPORT_FILE
static const int KNoDirection = 0;
static const int KLeftDirection = 1;
static const int KNodeAttributeDefaultValue = 0;
#endif

#ifdef CONVERT_NORMALIZE
static const int KSzLineSize = 256;
static const int KSzSourceSize = 4;
static const int KSzdestinationSize = 4;
static const int KSzLinePrevSize = 256;
static const int KSzLineCurrSize = 256;
static const int KNormalizeKeyNameSize = 256;
static const int KChtUTF8Size = 4;
static const int KSourceTokenIndex = 1;
static const int KDestinationTokenIndex = 2;
#endif

CXdbDumper::CXdbDumper() :
  iLog(NULL)
  ,iLogDetail(NULL)
#ifdef CONVERT_NORMALIZE
  ,iNormalizeLog(NULL)
  ,iNormalizeRepeatLog(NULL)
#endif
  ,iWordCount(0)
  ,iPrime(0)
  ,iHashBase(0)
{
}

CXdbDumper::~CXdbDumper()
{
  if (iLogDetail != NULL)
  {
    fclose(iLogDetail);
  }
  if (iLog != NULL)
  {
    fclose(iLog);
  }
  #ifdef CONVERT_NORMALIZE
  if (iNormalizeLog != NULL)
  {
    fclose(iNormalizeLog);
  }
  if (iNormalizeRepeatLog != NULL)
  {
    fclose(iNormalizeRepeatLog);
  }
  #endif
}

bool CXdbDumper::Run()
{
  std::cout << "Running CXdbDumper" << std::endl;
  std::cout << "iInputPath:" << GetConfig().iInputPath << std::endl;
  std::cout << "iOutputPath:" << GetConfig().iOutputPath << std::endl;
  std::cout << "iLogPath:" << GetConfig().iLogPath << std::endl;

  FILE* fd;
  TXdbHeader xdbHeader;
  int travelIndex;
  unsigned int forwardOffset;
  unsigned int destinationOffset;
  unsigned int destinationLength;
  size_t readSize = 0;

#ifdef DETAIL_EXPORT_FILE
  iFilePath = GetConfig().iLogPath + DETAIL_EXPORT_FILENAME;
  iLogDetail = fopen(iFilePath.c_str(), "w");
  if (iLogDetail == NULL)
  {
    printf("iLogDetail err:%s\n", iFilePath.c_str());
    return false;
  }
  printf("iLogDetail :%s\n", iFilePath.c_str());
#endif
#ifdef XDB_GEN_TOOL_EXPORT_FILE
  GetConfig().iOutputDumpText = SIMPLE_EXPORT_FILENAME;
  iFilePath = GetConfig().iOutputPath + GetConfig().iOutputDumpText;
  iLog = fopen(iFilePath.c_str(), "w");
  if (iLog == NULL)
  {
    printf("iLog err:%s\n", iFilePath.c_str());
#ifdef DETAIL_EXPORT_FILE
    if (iLogDetail != NULL)
    {
      fclose(iLogDetail);
      iLogDetail = NULL;
    }
#endif
    return false;
  }
  printf("iLog :%s\n", iFilePath.c_str());
#endif

  iFilePath = GetConfig().iInputPath + GetConfig().iInputScwsXdb;
  fd = fopen(iFilePath.c_str(), "rb");
  if (fd == NULL)
  {
    printf("fd err:%s\n", iFilePath.c_str());
#ifdef DETAIL_EXPORT_FILE
    if (iLogDetail != NULL)
    {
      fclose(iLogDetail);
      iLogDetail = NULL;
    }
#endif
#ifdef XDB_GEN_TOOL_EXPORT_FILE
    if (iLog != NULL)
    {
      fclose(iLog);
      iLog = NULL;
    }
#endif
    return false;
  }
  printf("fd :%s\n", iFilePath.c_str());

#ifdef CONVERT_NORMALIZE
  // Read mapping into tables
  FILE* normalizeFd;
  char* pFind;
  char szLine[KSzLineSize];
  int tokenIndex, szLineLength;
  char szSource[KSzSourceSize], szDestination[KSzdestinationSize];

  GetConfig().iOutputDumpText = NORMAL_EXPORT_FILENAME;
  iFilePath = GetConfig().iOutputPath + GetConfig().iOutputDumpText;
  iNormalizeLog = fopen(iFilePath.c_str(), "w"); 
  if (iNormalizeLog == NULL)
  {
    printf("iNormalizeLog err:%s\n", iFilePath.c_str());
    if (fd != NULL)
    {
      fclose(fd);
      fd = NULL;
    }
#ifdef DETAIL_EXPORT_FILE
    if (iLogDetail != NULL)
    {
      fclose(iLogDetail);
      iLogDetail = NULL;
    }
#endif
#ifdef XDB_GEN_TOOL_EXPORT_FILE
    if (iLog != NULL)
    {
      fclose(iLog);
      iLog = NULL;
    }
#endif
    return false;
  }
  printf("iNormalizeLog :%s\n", iFilePath.c_str());

  iFilePath = GetConfig().iInputPath + GetConfig().iInputNormalizeMap;
  normalizeFd = fopen(iFilePath.c_str(), "r");
  if (normalizeFd == NULL)
  {
    printf("normalizeFd err:%s\n", iFilePath.c_str());
    fclose(iNormalizeLog);
    iNormalizeLog = NULL;
    if (fd != NULL)
    {
      fclose(fd);
      fd = NULL;
    }
#ifdef DETAIL_EXPORT_FILE
    if (iLogDetail != NULL)
    {
      fclose(iLogDetail);
      iLogDetail = NULL;
    }
#endif
#ifdef XDB_GEN_TOOL_EXPORT_FILE
    if (iLog != NULL)
    {
      fclose(iLog);
      iLog = NULL;
    }
#endif
    return false;
  }
  printf("normalizeFd :%s\n", iFilePath.c_str());

  // For repeat normalized tokens log.
  iNormalizeMap.clear();
  iNormalizeRepeatVector.clear();

  iFilePath = GetConfig().iLogPath + NORMAL_EXPORT_REPEAT_FILENAME;
  iNormalizeRepeatLog = fopen(iFilePath.c_str(), "w");
  if (iNormalizeRepeatLog == NULL)
  {
    printf("iNormalizeRepeatLog err:%s\n", iFilePath.c_str());
    fclose(iNormalizeLog);
    iNormalizeLog = NULL;
    if (fd != NULL)
    {
      fclose(fd);
      fd = NULL;
    }
    if (normalizeFd != NULL)
    {
      fclose(normalizeFd);
      normalizeFd = NULL;
    }
#ifdef DETAIL_EXPORT_FILE
    if (iLogDetail != NULL)
    {
      fclose(iLogDetail);
      iLogDetail = NULL;
    }
#endif
#ifdef XDB_GEN_TOOL_EXPORT_FILE 
    if (iLog != NULL)
    {
      fclose(iLog);
      iLog = NULL;
    }
#endif
    return false;
  }
  printf("iNormalizeRepeatLog :%s\n", iFilePath.c_str());
  
  while (fgets(szLine, sizeof(szLine), normalizeFd) != NULL) 
  {
    szLineLength = strlen(szLine);
    szLineLength--;

    while (('\r' == szLine[szLineLength]) || ('\n' == szLine[szLineLength]))
    {
      szLineLength--;
    }

    szLine[szLineLength + 1] = 0;

    tokenIndex = 0;
    pFind = strtok(szLine, ",");
    while (pFind != NULL) 
    {
      if (KSourceTokenIndex == tokenIndex) 
      {
        memset(szSource, 0, KSzSourceSize);
        strcpy(szSource, pFind);
      } 
      else if (KDestinationTokenIndex == tokenIndex)
      {
        memset(szDestination, 0, KSzdestinationSize);
        strcpy(szDestination, pFind);
      }
      pFind = strtok(NULL, ",");
      tokenIndex++;
    }
    iNormalizeHash[szSource] = szDestination;
  }
  if (normalizeFd != NULL)
  {
    fclose(normalizeFd);
    normalizeFd = NULL;
  }
#endif //CONVERT_NORMALIZE

  printf("XDB header size=%ld\n", sizeof(TXdbHeader));
  readSize = fread(&xdbHeader, 1, sizeof(TXdbHeader), fd);
  if (readSize != sizeof(TXdbHeader))
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
    if (readSize != sizeof(unsigned int))
    {
      fputs("Reading error 02", stderr);
      exit(EXIT_FAILURE);
    }

    readSize = fread(&destinationLength, 1, sizeof(unsigned int), fd);
    if (readSize != sizeof(unsigned int))
    {
      fputs("Reading error 03", stderr);
      exit(EXIT_FAILURE);
    }

    if (destinationLength != 0) 
    {
      printf("travelIndex=%d destinationOffset=%d destinationLength=%d\n", travelIndex, destinationOffset, destinationLength);
      GetRecord(fd, destinationOffset, destinationLength, 0, 0, "");
      iWordCount = 0;

      //sprintf(szLog, "+++++++++++++++++++Prime_index=%d\n", trav_index );
      //fputs(szLog, iLog );
    }
    travelIndex++;
  } while (travelIndex < xdbHeader.prime);

#ifdef CONVERT_NORMALIZE
  char* pFindPrev;
  char* pFindCurr;
  char szLinePrev[KSzLinePrevSize], szLineCurr[KSzLineCurrSize];
  int repeatCnt;
  int toklen;
  int totalCnt;

  // For repeat normalized tokens log.
  // sort and log iNormalizeRepeatVector to file.
  std::sort(iNormalizeRepeatVector.begin(), iNormalizeRepeatVector.end()); // using default comparison (operator <):

  repeatCnt = 0;
  toklen = 0;
  totalCnt = 0;
  for (std::vector<std::string>::iterator normalizeRepeatVectorIterator = iNormalizeRepeatVector.begin(); normalizeRepeatVectorIterator != iNormalizeRepeatVector.end(); ++normalizeRepeatVectorIterator)
  {
    // Collect statistic informations.
    //----------------------------------------
    //printf("repeatCnt(%d)\n", repeatCnt);
    if (normalizeRepeatVectorIterator == iNormalizeRepeatVector.begin())
    {
      strcpy(szLinePrev, (*normalizeRepeatVectorIterator).c_str());
      //printf("11AA szLinePrev %s", szLinePrev);
      pFindPrev = strtok(szLinePrev, "<");
      //printf("11BB pFindPrev %s\n", pFindPrev);
      repeatCnt++;
      fprintf(iNormalizeRepeatLog, "------------------------ [X] x (X) \n");
    }
    else
    {
      strcpy(szLineCurr, (*normalizeRepeatVectorIterator).c_str());
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
    fprintf(iNormalizeRepeatLog, "%s", (*normalizeRepeatVectorIterator).c_str());
  }
  
  if (repeatCnt)
  {
    fprintf(iNormalizeRepeatLog, "------------------------ [%d] x (%d) \n", toklen / 3, repeatCnt);
    totalCnt++;
  }
  if (totalCnt)
  {
    fprintf(iNormalizeRepeatLog, "total_cnt:%d\n", totalCnt);
  }
#endif //CONVERT_NORMALIZE
  if (fd != NULL)
  {
    fclose(fd);
  }
  if (iLog != NULL)
  {
    fclose(iLog);
    iLog = NULL;
  }
#ifdef CONVERT_NORMALIZE
  if (iNormalizeLog != NULL)
  {
    fclose(iNormalizeLog);
    iNormalizeLog = NULL;
  }
  if (iNormalizeRepeatLog != NULL)
  {
    fclose(iNormalizeRepeatLog);
    iNormalizeRepeatLog = NULL;
  }
#endif

  return true;
}

CXdbDumperConfig& CXdbDumper::GetConfig()
{
  return iConfig;
}

int CXdbDumper::GetIndex(const unsigned char* aKey, int aHashBase, int aHashPrime)
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
  unsigned char keyName[KKeyNameSize];
  unsigned char attribute[KAttributeSize];
  TNodeContent content;
  size_t readSize = 0;

  fseek(aFd, aOffset, SEEK_SET);
  //r_len = XDB_MAXKLEN+17;
  //if(r_len > len) r_len = len;
  //readSize = fread(buff, 1, r_len, fd);
  //if (readSize != r_len*1) {fputs("Reading error 04", stderr); exit(EXIT_FAILURE);}

  readSize = fread(&leftOffset, 1, sizeof(unsigned int), aFd);
  if (readSize != sizeof(unsigned int))
  {
    fputs("Reading error 05", stderr);
    exit(EXIT_FAILURE);
  }

  readSize = fread(&leftLength, 1, sizeof(unsigned int), aFd);
  if (readSize != sizeof(unsigned int))
  {
    fputs("Reading error 06", stderr);
    exit(EXIT_FAILURE);
  }

  readSize = fread(&rightOffset, 1, sizeof(unsigned int), aFd);
  if (readSize != sizeof(unsigned int))
  {
    fputs("Reading error 07", stderr);
    exit(EXIT_FAILURE);
  }

  readSize = fread(&rightLength, 1, sizeof(unsigned int), aFd);
  if (readSize != sizeof(unsigned int))
  {
    fputs("Reading error 08", stderr);
    exit(EXIT_FAILURE);
  }

  readSize = fread(&keyLength, 1, sizeof(unsigned char), aFd);
  if (readSize != sizeof(unsigned char))
  {
    fputs("Reading error 09", stderr);
    exit(EXIT_FAILURE);
  }

  memset(keyName, 0, KKeyNameSize);
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
  readSize = fread(&content, 1, sizeof(TNodeContent), aFd);
  if (readSize != sizeof(TNodeContent))
  {
    fputs("Reading error 12", stderr);
    exit(EXIT_FAILURE);
  }

  if ((leftOffset > 0) && (leftLength > 0))
  {
    GetRecord(aFd, leftOffset, leftLength, 1, aLevel + 1, (char*)keyName);
  }

  if ((rightOffset > 0) && (rightLength > 0))
  {
    GetRecord(aFd, rightOffset, rightLength, 2, aLevel + 1, (char*)keyName);
  }

  iWordCount++;
  /*
  printf("Level[%d] Dir=%c word[%ld] l_offset=%ld l_len=%ld r_offset=%d r_len=%d k_len=%d father=%s tf=%f idf=%f flag=%d attr[0]=%c key=%s\n", 
    level, (0 == direction) ? 'N': (1 == direction) ?'L':'R', iWordCount, l_offset, l_len, r_offset, r_len, k_len, father, 
    ct.tf, ct.idf, ct.flag, ct.attr[0], key_name );
    */

  //sprintf(szLog, "%s\n", key_name ); 

#ifdef DETAIL_EXPORT_FILE
  // Detail log
  sprintf(iSzLog, "Level[%d] Dir=%c word[%ld] l_offset=%d l_len=%d r_offset=%d r_len=%d k_len=%d father=%s tf=%f idf=%f flag=%d attr[0]=%c attr[1]=%c attr[2]=%c key=\"%s\" prime_index=%d\n", 
    aLevel, (KNoDirection == aDirection) ? 'N': (KLeftDirection == aDirection) ?'L':'R', iWordCount, leftOffset, leftLength, rightOffset, rightLength, keyLength, aFather, 
    content.tf, content.idf, content.flag, 
    (KNodeAttributeDefaultValue == content.attr[0]) ? 0x01:content.attr[0],
    (KNodeAttributeDefaultValue == content.attr[1]) ? 0x01:content.attr[1],
    (KNodeAttributeDefaultValue == content.attr[2]) ? 0x01:content.attr[2],
    keyName,
    GetIndex(keyName,iHashBase, iPrime)
    );
    fputs(iSzLog, iLogDetail);
  // Detail log
#endif

  memset(attribute, 0, KAttributeSize);
#ifdef XDB_GEN_TOOL_EXPORT_FILE
  // OWN toolsusage
  memcpy(attribute, content.attr, 3);
  sprintf(iSzLog, "%s\t%f\t%f\t%d\t%s\n", keyName, content.tf, content.idf, content.flag, attribute);
  fputs(iSzLog, iLog);
  // PHP usage
#endif

#ifdef CONVERT_NORMALIZE
  unsigned char normalizeKeyName[KNormalizeKeyNameSize];
  unsigned char chtUTF8[KChtUTF8Size];
  int keyLength;
  int charUTF8Length;
  std::string charUTF8;
  std::map<std::string, std::string>::const_iterator findIterator;

  // convert by normalize hash
  memset(normalizeKeyName, 0, KNormalizeKeyNameSize);

  keyLength = strlen((char*)keyName);

  for (int index = 0; index < keyLength; )
  {
    charUTF8Length = CTokenGeneratorChineseConfig::iUTF8MultibyteLengthTable[keyName[index]];

    if (charUTF8Length != 3) 
    {
      memcpy(&normalizeKeyName[index], &keyName[index], charUTF8Length);
    } 
    else 
    {
      memset(chtUTF8, 0, KChtUTF8Size);
      memcpy(chtUTF8, &keyName[index], charUTF8Length);

      charUTF8.clear();
      charUTF8 += (char*)chtUTF8;

      findIterator = iNormalizeHash.find(charUTF8);
      if (findIterator == iNormalizeHash.end()) 
      {
        printf("NOT Found  [%x][%x][%x]...................\n", chtUTF8[0], chtUTF8[1], chtUTF8[2]);
        memcpy(&normalizeKeyName[index], &keyName[index], charUTF8Length);
      } 
      else 
      {
        memcpy(&normalizeKeyName[index], iNormalizeHash[charUTF8].c_str(), charUTF8Length);
      }
    }

    index += charUTF8Length;
  }

  // For repeat normalized tokens log.
  //-----------------------------------------------------------------
  if (keyLength > 3)
  {
    std::string originalString;
    std::string normalizeString;
    std::map<std::string, TNodeContent>::iterator normalizeMapIterator;
    std::vector<std::string>::iterator normalizeRepeatVectorIterator;

    originalString = (char*)keyName;
    normalizeString = (char*)normalizeKeyName;
    normalizeMapIterator = iNormalizeMap.find(normalizeString);
    if (iNormalizeMap.end() != normalizeMapIterator)
    {
      // This is repeat string.
      //printf("Repeat: [%s<=%s\t%f\t%f\t%d\t%s] v.s. ",nor_str.c_str(), iFirstNormalizeToOriginalMap[nor_str].c_str(), nor_map_it->second.tf, nor_map_it->second.idf, nor_map_it->second.flag, nor_map_it->second.attr);
      //printf("[%s<=%s\t%f\t%f\t%d\t%s]\n",normal_key_name, key_name, ct.tf, ct.idf, ct.flag, ct.attr);

      // get original one info.
      sprintf(iSzLog, "%s<=%s\t%f\t%f\t%d\t%s\n", normalizeString.c_str(), iFirstNormalizeToOriginalMap[normalizeString].c_str(), 
              normalizeMapIterator->second.tf, normalizeMapIterator->second.idf, normalizeMapIterator->second.flag, normalizeMapIterator->second.attr);
      
      // is original one info in vector?
      normalizeRepeatVectorIterator = std::find(iNormalizeRepeatVector.begin(), iNormalizeRepeatVector.end(), iSzLog);
      if (iNormalizeRepeatVector.end() != normalizeRepeatVectorIterator)
      {
        // original one alread log in vector, log just "repeat one".

        // repeat one
        sprintf(iSzLog, "%s<=%s\t%f\t%f\t%d\t%s\n", normalizeKeyName, keyName, content.tf, content.idf, content.flag, content.attr);
        iNormalizeRepeatVector.push_back(iSzLog);
      }
      else
      {
        // original one not in vector yet, log "original one" and "repeat one".

        // original one
        iNormalizeRepeatVector.push_back(iSzLog);

        // repeat one
        sprintf(iSzLog, "%s<=%s\t%f\t%f\t%d\t%s\n", normalizeKeyName, keyName, content.tf, content.idf, content.flag, content.attr);
        iNormalizeRepeatVector.push_back(iSzLog);
      }
    }
    else
    {
      // This is first time we get this key, so save infos to iNormalizeMap and iFirstNormalizeToOriginalMap.
      iNormalizeMap.insert(std::pair<std::string, TNodeContent>(normalizeString, content));
      iFirstNormalizeToOriginalMap.insert(std::pair<std::string,std::string>(normalizeString, originalString));
    }
  }
  //-----------------------------------------------------------------
  
  memcpy(attribute, content.attr, 3);
  sprintf(iSzLog, "%s\t%f\t%f\t%d\t%s\n", normalizeKeyName, content.tf, content.idf, content.flag, attribute);
  fputs(iSzLog, iNormalizeLog);
#endif //CONVERT_NORMALIZE
}

