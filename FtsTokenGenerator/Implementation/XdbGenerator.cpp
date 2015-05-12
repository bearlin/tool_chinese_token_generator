//
// This file contains the CXdbGenerator implementation 
//

#include "XdbGenerator.h"
#include <iostream>

CXdbGenerator::CXdbGenerator() :
  iUTF8MultibyteLengthTable {
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
{
}

CXdbGenerator::~CXdbGenerator()
{
}

bool CXdbGenerator::Run()
{
  std::cout << "Running CXdbGenerator" << std::endl;
  std::cout << "iInputPath:" << GetConfig().iInputPath << std::endl;
  std::cout << "iOutputPath:" << GetConfig().iOutputPath<< std::endl;
  std::cout << "iLogPath:" << GetConfig().iLogPath<< std::endl;

  std::string tempFilePath = GetConfig().iInputPath + GetConfig().iInputTokenList;
  FILE* inputTokenListFile = fopen(tempFilePath.c_str(), "r");

  if (inputTokenListFile == NULL)
  {
    std::cout << "[ERROR] Unable to open input token list file:" << tempFilePath << " to read." << std::endl;
    return false;
  }

  tempFilePath = GetConfig().GetOutputPath() + GetConfig().GetOutputXdb();
  // xdb usage
  FILE* xdbFile = fopen(tempFilePath.c_str(), "wb");
  if (xdbFile == NULL)
  {
    fclose(inputTokenListFile);
    std::cout << "[ERROR] Unable to create xdb file :" << tempFilePath << std::endl;
    return false;
  }

#ifdef ENABLE_LOG
  tempFilePath = GetConfig().iLogPath + LOG_PATH;
  FILE* logFile = fopen(tempFilePath.c_str(), "w");
  if (logFile == NULL)
  {
    fclose(inputTokenListFile);
    fclose(xdbFile);
    std::cout << "[ERROR] Unable to create log file :" << tempFilePath << std::endl;
    return false;
  }

  tempFilePath = GetConfig().iLogPath + LOG_REPEAT_PATH;
  FILE* logRepeatPathFile = fopen(tempFilePath.c_str(), "w");

  if (logRepeatPathFile == NULL)
  {
    fclose(inputTokenListFile);
    fclose(xdbFile);
    fclose(logFile);
    std::cout << "[ERROR] Unable to create log repeat path file :" << tempFilePath << std::endl;
    return false;
  }
#endif //ENABLE_LOG

  TNodeInfo nodeInfo;
  std::vector<TNodeInfo> nodes[SCWS_XDICT_PRIME];
  int lineCount = 0;
  int primeIndex = 0;
  char buf[512] = {'\0'};

  while (fgets(buf, sizeof(buf)-1, inputTokenListFile) != NULL)
  {
    lineCount++;
    std::string line(buf);

    memset(&nodeInfo, 0, sizeof(TNodeInfo));

    // <word>\t<tf>\t<idf>\t<attr>\n
    if ((buf[0] == ';') || (buf[0] == '#'))
    {
      continue;
    }

    const char* delim = " \t\r\n";
    char* str = strtok(buf, delim);
    if (str == NULL)
    {
      continue;
    }
    
    // init the word
    do
    {
      nodeInfo.tf = nodeInfo.idf = 1.0;
      nodeInfo.flag = SCWS_WORD_FULL;
      nodeInfo.attr[0] = '\0';
      //nodeInfo.attr[0] = '@';
      //nodeInfo.attr[1] = '\0';

      char* ptr = NULL;
      if ((ptr = strtok(NULL, delim)) == NULL)
      {
        break;
      }
      nodeInfo.tf = (float) atof(ptr);

      if ((ptr = strtok(NULL, delim)) == NULL)
      {
        break;
      }
      nodeInfo.idf = (float) atof(ptr);

      // bypass flag
      if ((ptr = strtok(NULL, delim)) != NULL)
      {
        nodeInfo.flag = (unsigned char) atoi(ptr);
      }

      if ((ptr = strtok(NULL, delim)) != NULL)
      {
        nodeInfo.attr[0] = ptr[0];
        if (ptr[1])
        {
          nodeInfo.attr[1] = ptr[1];
        }
      }
    } while (0);

    //Skip partial tokens from the raw input file..
    if (!(nodeInfo.flag & SCWS_WORD_FULL))
    {
      std::cout << "skip_part: " << line;
      continue;
    }
    
    // save the word
    primeIndex = getHashIndex((const unsigned char*)str, XDB_BASE, SCWS_XDICT_PRIME);

    int found = 0;
    for (std::vector<TNodeInfo>::iterator iter = nodes[primeIndex].begin(); ((iter != nodes[primeIndex].end()) && (found == 0)); ++iter)
    {
      if (strcmp(str, (const char*)iter->k_data) == 0)
      {
        found = 1;

      #ifdef ENABLE_LOG
        fprintf(logRepeatPathFile, "found existed node[%s\t%f\t%f\t%d\t%s]\n",iter->k_data, iter->tf, iter->idf, iter->flag, iter->attr);
        fprintf(logRepeatPathFile, "new node[%s\t%f\t%f\t%d\t%s]\n",str, nodeInfo.tf, nodeInfo.idf, nodeInfo.flag, nodeInfo.attr);
      #endif //ENABLE_LOG
      
        // Update as max(tf, idf).
        if (nodeInfo.tf > iter->tf)
        {
          iter->tf = nodeInfo.tf;
        }

        if (nodeInfo.idf > iter->idf)
        {
          iter->idf = nodeInfo.idf;
        }

        iter->flag |= SCWS_WORD_FULL;

        // Use last attr[].
        strcpy(iter->attr, nodeInfo.attr);

      #ifdef ENABLE_LOG
        fprintf(logRepeatPathFile, "updated node[%s\t%f\t%f\t%d\t%s]\n",iter->k_data, iter->tf, iter->idf, iter->flag, iter->attr);
      #endif //ENABLE_LOG
      }
    }

    int length = strlen(str);
    if (found == 0)
    {
      nodeInfo.prime_index = primeIndex;
      nodeInfo.k_length = length;
      strcpy((char*)nodeInfo.k_data, str);
      nodes[primeIndex].push_back(nodeInfo);
    }

    // Generate partial tokens for this raw token.
    // parse the part
    int totalLength = iUTF8MultibyteLengthTable[(unsigned char)(str[0])];
    while (1)
    {
      totalLength += iUTF8MultibyteLengthTable[(unsigned char)(str[totalLength])];
      if (totalLength >= length)
      {
        break;
      }

      char part[512] = {0};
      strncpy(part, str, totalLength);
      primeIndex = getHashIndex((const unsigned char*)part, XDB_BASE, SCWS_XDICT_PRIME);

      found = 0;
      for (std::vector<TNodeInfo>::iterator iter = nodes[primeIndex].begin(); ((iter != nodes[primeIndex].end()) && (0 == found)); ++iter)
      {
        if (strcmp(part, (const char*)iter->k_data) == 0)
        {
          found = 1;
          iter->flag |= SCWS_WORD_PART;
        }
      }

      if (found == 0)
      {
        memset(&nodeInfo, 0, sizeof(nodeInfo));
        nodeInfo.prime_index = primeIndex;
        nodeInfo.k_length = totalLength;
        nodeInfo.flag = SCWS_WORD_PART;
        strcpy((char*)nodeInfo.k_data, part);
        nodes[primeIndex].push_back(nodeInfo);
      }
    }
  }
  std::cout << "Total read line: " << lineCount << std::endl;
  std::cout <<"Validation checking................................................." << std::endl;

  for (int index = 0; index < SCWS_XDICT_PRIME; index++)
  {
    if (nodes[index].size() > MAX_NODE_COUNT)
    {
      fclose(xdbFile);
      fclose(inputTokenListFile);

    #ifdef ENABLE_LOG
      fclose(logFile);
      fclose(logRepeatPathFile);
    #endif //ENABLE_LOG

      std::cout << "PRIME[ " << index << " ] illegal!!!  Count = " << nodes[index].size() << std::endl;
      return false;
    }
  }

  std::cout << "Sorting................................................." << std::endl;

  for (int index = 0; index < SCWS_XDICT_PRIME; index++)
  {
    std::sort(nodes[index].begin(), nodes[index].end(), compareNode);
  }

  std::cout << "\n\nWriting XDB to " << (GetConfig().GetOutputPath() + GetConfig().GetOutputXdb()) << std::endl;
  TXdb_header xdbHeader;
  memset(&xdbHeader, 0, sizeof(TXdb_header));
  memcpy(&xdbHeader.tag, XDB_TAGNAME, 3);
  xdbHeader.ver = XDB_VERSION;
  xdbHeader.prime = SCWS_XDICT_PRIME;
  xdbHeader.base = XDB_BASE;
  xdbHeader.fsize = sizeof(TXdb_header) + (xdbHeader.prime * 8);
  xdbHeader.check = (float)XDB_FLOAT_CHECK;

  fseek(xdbFile, 0L, SEEK_SET);
  fwrite(&xdbHeader, sizeof(TXdb_header), 1, xdbFile);

  unsigned char* pMem = (unsigned char*)malloc(xdbHeader.prime * 8);
  memset(pMem, 0, xdbHeader.prime * 8);
  fwrite(pMem, sizeof(unsigned char), xdbHeader.prime * 8, xdbFile);
  free(pMem);

  for (int index = 0; index < SCWS_XDICT_PRIME; index++)
  {
    // if no element in the list, move to next one.
    if (nodes[index].size() == 0)
    {
      continue;
    }

  #ifdef ENABLE_LOG
    //if( 0 == nodes[prime_index].size() % 2 ) //print tree only with odd node size.
    {
      fprintf(logFile, "===========================\n");
      int iterIndex = 0;
      for (std::vector<TNodeInfo>::iterator iter = nodes[index].begin(); (iter != nodes[index].end()); ++iter)
      {
        fprintf(logFile, "SORT[Prime:%d][%d]%s\n", index, iterIndex++, iter->k_data);
      }
    }
  #endif //ENABLE_LOG

    long filePosistion = ftell(xdbFile);// save current position
    writeSortedDataToXdb(0, nodes[index].size()-1, filePosistion, NO_FATHER, index, nodes[index].begin(), xdbFile);
  }

  // update size
  fseek(xdbFile, 0L, SEEK_END);
  xdbHeader.fsize = ftell(xdbFile);
  fseek(xdbFile, 0L, SEEK_SET);
  fwrite(&xdbHeader, sizeof(TXdb_header), 1, xdbFile);

#ifdef ENABLE_LOG
  fclose(logFile);
  fclose(logRepeatPathFile);
#endif //ENABLE_LOG

  fclose(xdbFile);
  fclose(inputTokenListFile);

  return true;
}

CXdbGeneratorConfig& CXdbGenerator::GetConfig()
{
  return iConfig;
}

int CXdbGenerator::getHashIndex(const unsigned char* aKey, int aHashBase, int aHashPrime ) const
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

bool CXdbGenerator::compareNode(const TNodeInfo& aNodeInfo1, const TNodeInfo& aNodeInfo2)
{
   int ret = strcmp((const char*)aNodeInfo1.k_data, (const char*)aNodeInfo2.k_data);
   return (ret < 0);
}

void CXdbGenerator::generateBtreeNodeIndex(int aStart, int aEnd, int aFather, int aLevel, std::vector<TNodeInfo>::iterator aIter, int aDir)
{
  int diff = aEnd - aStart;
  int avg = diff/2;
  int mid = aStart + avg;

  // Leaf node
  if (aStart == aEnd)
  {
    aIter[mid-1].level = aLevel;
    aIter[mid-1].father = aFather;

    // update father node
    if (aFather > 0)
    {
      if (DIR_LEFT == aDir)
      {
        aIter[aFather-1].l_offset = aIter[aStart-1].offset;
        aIter[aFather-1].l_length = aIter[aStart-1].length;
      }
      else
      {
        aIter[aFather-1].r_offset = aIter[aStart-1].offset;
        aIter[aFather-1].r_length = aIter[aStart-1].length;
      }
    }
    return;
  }

  if (diff == 1)  // even
  {
    if (NO_FATHER != aFather)
    {
      // update father node
      if (DIR_LEFT == aDir)
      {
        aIter[aFather-1].l_offset = aIter[aStart-1].offset;
        aIter[aFather-1].l_length = aIter[aStart-1].length;
      }
      else
      {
        aIter[aFather-1].r_offset = aIter[aStart-1].offset;
        aIter[aFather-1].r_length = aIter[aStart-1].length;
      }
    }

    aIter[aStart-1].level = aLevel;
    aIter[aStart-1].father = aFather;
    
    return generateBtreeNodeIndex(aEnd, aEnd, aStart, aLevel + 1, aIter, DIR_RIGHT);
  }
  else
  {
    aIter[mid-1].level = aLevel;
    aIter[mid-1].father = aFather;

    if (NO_FATHER != aFather)
    {
      if (DIR_LEFT == aDir)
      {
        aIter[aFather-1].l_offset = aIter[mid-1].offset;
        aIter[aFather-1].l_length = aIter[mid-1].length;
      }
      else
      {
        aIter[aFather-1].r_offset = aIter[mid-1].offset;
        aIter[aFather-1].r_length = aIter[mid-1].length;
      }
    }
  }

  // travel left
  generateBtreeNodeIndex(aStart, mid - 1, mid, aLevel + 1, aIter, DIR_LEFT);

  // travel left+1
  generateBtreeNodeIndex(mid + 1, aEnd, mid, aLevel + 1, aIter, DIR_RIGHT);
}

void CXdbGenerator::writeSortedDataToXdb(int aStart, int aEnd, unsigned int aNodeOffset, unsigned int aFatherOffset, int aPrime, std::vector<TNodeInfo>::iterator aIter, FILE *aFileXdb)
{
  int count = aEnd - aStart + 1;
  int mid = aStart + (count+1)/2 - 1;

  aIter[mid].offset = aNodeOffset;
  aIter[mid].length = 17 + aIter[mid].k_length + 12;

  if (NO_FATHER == aFatherOffset)
  {
    long fileOffset = 32 + 8 * aPrime;
    fseek(aFileXdb, fileOffset, SEEK_SET);// move to prime info
    fwrite(&aIter[mid].offset, sizeof(unsigned int), 1, aFileXdb);
    fwrite(&aIter[mid].length, sizeof(unsigned int), 1, aFileXdb);
  }

  fseek(aFileXdb, aNodeOffset, SEEK_SET);// move to node start offset
  fwrite(&aIter[mid].l_offset, sizeof(unsigned int), 1, aFileXdb);
  fwrite(&aIter[mid].l_length, sizeof(unsigned int), 1, aFileXdb);
  fwrite(&aIter[mid].r_offset, sizeof(unsigned int), 1, aFileXdb);
  fwrite(&aIter[mid].r_length, sizeof(unsigned int), 1, aFileXdb);
  fwrite(&aIter[mid].k_length, sizeof(unsigned char), 1, aFileXdb);
  fwrite(&aIter[mid].k_data, sizeof(unsigned char), aIter[mid].k_length, aFileXdb);
  fwrite(&aIter[mid].tf, sizeof(float), 1, aFileXdb);
  fwrite(&aIter[mid].idf, sizeof(float), 1, aFileXdb);
  fwrite(&aIter[mid].flag, sizeof(unsigned char), 1, aFileXdb);
  fwrite(&aIter[mid].attr, sizeof(char), 3, aFileXdb);

  // travel from left tree, if necessary
  long filePosistion = ftell(aFileXdb);// save current position

  int newMid;
  unsigned int tmpItemSize;
  if (aStart <= (mid-1))
  {
    int newEnd = mid - 1;
    count = newEnd - aStart + 1;
    newMid = aStart + (count+1)/2 - 1;

    fseek(aFileXdb, aNodeOffset, SEEK_SET);// move to node start offset
    fwrite(&filePosistion, sizeof(unsigned int), 1, aFileXdb);

    tmpItemSize = 17 + aIter[newMid].k_length + 12;
    fwrite(&tmpItemSize, sizeof(unsigned int), 1, aFileXdb);
    fseek(aFileXdb, filePosistion, SEEK_SET);

    // write left node info.
    writeSortedDataToXdb(aStart, newEnd, filePosistion, aNodeOffset, aPrime, aIter, aFileXdb);
  }

  // Right node first
  filePosistion = ftell(aFileXdb);// save current position

  if (aEnd >= (mid+1))
  {
    int newStart = mid + 1;
    count = aEnd - newStart + 1;
    newMid = newStart + (count + 1)/2 - 1;

    fseek(aFileXdb, aNodeOffset + 8, SEEK_SET);// move to node start offset
    fwrite(&filePosistion, sizeof(unsigned int), 1, aFileXdb);

    tmpItemSize = 17 + aIter[newMid].k_length + 12;
    fwrite(&tmpItemSize, sizeof(unsigned int), 1, aFileXdb);
    fseek(aFileXdb, filePosistion, SEEK_SET);

    writeSortedDataToXdb(newStart, aEnd, filePosistion, aNodeOffset, aPrime, aIter, aFileXdb);
  }
}
