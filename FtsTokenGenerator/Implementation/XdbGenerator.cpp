//
// This file contains the CXdbGenerator implementation 
// [Normalization procedures]
// [Abandon] step 0: use xdb_dump tool to dump the original simple_export file from original raw xdb.
// [Abandon] step 1: Add/Delete/Update tokens in the original simple_export file and save to another updated simple_export file.
// step 1: use xdb_filter tool to filter, optimize and generate tt_tokens_list according to TomTom Addresses/POIs.
// step 2: use xdb_gen tool to generate a xdb(non-fuzzy) from this tt_tokens_list file.
// step 3: use xdb_dump tool to generate the non-normalized/normalized token files from this non-fuzzy xdb(with the latest fts-{tc|sc}-n.tok file), then merge them to a merge_export file.
// step 4: use xdb_gen tool generate a final normalized xdb(fuzzy) from this merge_export file.
// TODO: We can just skip step 3(skip xdb_dump), because we can just generage a normalized tt_tokens_list after step 1(we can get both tt_tokens_list(non-normailzed) and tt_tokens_list(normailzed) together).


#include "XdbGenerator.h"
#include "xdict.h"
#include "xdb.h"
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <string>
#include <algorithm>

CXdbGenerator::CXdbGenerator() :
  iInputTokenListFile(NULL),
  iXdbFile(NULL),
#ifdef ENABLE_LOG
  iLogFile(NULL),
  iLogRepeatPathFile(NULL)
#endif //ENABLE_LOG
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

  if (!OpenFile())
  {
    return false;
  }

  TNodeInfo nodeInfo;
  std::vector<TNodeInfo> nodes[SCWS_XDICT_PRIME];
  int lineCount = 0;
  int primeIndex = 0;
  char buf[512] = {'\0'};

  while (fgets(buf, sizeof(buf)-1, iInputTokenListFile) != NULL)
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
    primeIndex = GetHashIndex((const unsigned char*)str, XDB_HASH_BASE, SCWS_XDICT_PRIME);

    int found = 0;
    for (std::vector<TNodeInfo>::iterator iter = nodes[primeIndex].begin(); ((iter != nodes[primeIndex].end()) && (found == 0)); ++iter)
    {
      if (strcmp(str, (const char*)iter->k_data) == 0)
      {
        found = 1;

      #ifdef ENABLE_LOG
        fprintf(iLogRepeatPathFile, "found existed node[%s\t%f\t%f\t%d\t%s]\n",iter->k_data, iter->tf, iter->idf, iter->flag, iter->attr);
        fprintf(iLogRepeatPathFile, "new node[%s\t%f\t%f\t%d\t%s]\n",str, nodeInfo.tf, nodeInfo.idf, nodeInfo.flag, nodeInfo.attr);
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
        fprintf(iLogRepeatPathFile, "updated node[%s\t%f\t%f\t%d\t%s]\n",iter->k_data, iter->tf, iter->idf, iter->flag, iter->attr);
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
    int totalLength = CTokenGeneratorChineseCommon::iUTF8MultibyteLengthTable[(unsigned char)(str[0])];
    while (1)
    {
      totalLength += CTokenGeneratorChineseCommon::iUTF8MultibyteLengthTable[(unsigned char)(str[totalLength])];
      if (totalLength >= length)
      {
        break;
      }

      char part[512] = {0};
      strncpy(part, str, totalLength);
      primeIndex = GetHashIndex((const unsigned char*)part, XDB_HASH_BASE, SCWS_XDICT_PRIME);

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
      CloseFile();
      std::cout << "PRIME[ " << index << " ] illegal!!!  Count = " << nodes[index].size() << std::endl;
      return false;
    }
  }

  std::cout << "Sorting................................................." << std::endl;

  for (int index = 0; index < SCWS_XDICT_PRIME; index++)
  {
    std::sort(nodes[index].begin(), nodes[index].end(), CompareNode);
  }

  std::cout << "\n\nWriting XDB to " << (GetConfig().GetOutputPath() + GetConfig().GetOutputXdb()) << std::endl;
  TXdb_header xdbHeader;
  memset(&xdbHeader, 0, sizeof(TXdb_header));
  memcpy(&xdbHeader.tag, XDB_TAGNAME, 3);
  xdbHeader.ver = XDB_VERSION;
  xdbHeader.prime = SCWS_XDICT_PRIME;
  xdbHeader.base = XDB_HASH_BASE;
  xdbHeader.fsize = sizeof(TXdb_header) + (xdbHeader.prime * 8);
  xdbHeader.check = (float)XDB_FLOAT_CHECK;

  fseek(iXdbFile, 0L, SEEK_SET);
  fwrite(&xdbHeader, sizeof(TXdb_header), 1, iXdbFile);

  unsigned char* pMem = new unsigned char [xdbHeader.prime * 8];
  memset(pMem, 0, xdbHeader.prime * 8);
  fwrite(pMem, sizeof(unsigned char), xdbHeader.prime * 8, iXdbFile);
  delete [] pMem;

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
      fprintf(iLogFile, "===========================\n");
      int iterIndex = 0;
      for (std::vector<TNodeInfo>::iterator iter = nodes[index].begin(); (iter != nodes[index].end()); ++iter)
      {
        fprintf(iLogFile, "SORT[Prime:%d][%d]%s\n", index, iterIndex++, iter->k_data);
      }
    }
  #endif //ENABLE_LOG

    long filePosistion = ftell(iXdbFile); // save current position
    WriteSortedDataToXdb(0, nodes[index].size()-1, filePosistion, NO_FATHER, index, nodes[index].begin(), iXdbFile);
  }

  // update size
  fseek(iXdbFile, 0L, SEEK_END);
  xdbHeader.fsize = ftell(iXdbFile);
  fseek(iXdbFile, 0L, SEEK_SET);
  fwrite(&xdbHeader, sizeof(TXdb_header), 1, iXdbFile);

  CloseFile();

  return true;
}

CXdbGeneratorConfig& CXdbGenerator::GetConfig()
{
  return iConfig;
}

int CXdbGenerator::GetHashIndex(const unsigned char* aKey, int aHashBase, int aHashPrime) const
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

bool CXdbGenerator::CompareNode(const TNodeInfo& aNodeInfo1, const TNodeInfo& aNodeInfo2)
{
   int ret = strcmp((const char*)aNodeInfo1.k_data, (const char*)aNodeInfo2.k_data);
   return (ret < 0);
}

bool CXdbGenerator::OpenFile()
{
  std::string tempFilePath = GetConfig().iInputPath + GetConfig().iInputTokenList;
  iInputTokenListFile = fopen(tempFilePath.c_str(), "r");

  if (iInputTokenListFile == NULL)
  {
    std::cout << "[ERROR] Unable to open input token list file:" << tempFilePath << " to read." << std::endl;
    return false;
  }

  tempFilePath = GetConfig().GetOutputPath() + GetConfig().GetOutputXdb();
  // xdb usage
  iXdbFile = fopen(tempFilePath.c_str(), "wb");
  if (iXdbFile == NULL)
  {
    CloseFile();
    std::cout << "[ERROR] Unable to create xdb file :" << tempFilePath << std::endl;
    return false;
  }

#ifdef ENABLE_LOG
  tempFilePath = GetConfig().iLogPath + LOG_FILE_NAME;
  iLogFile = fopen(tempFilePath.c_str(), "w");
  if (iLogFile == NULL)
  {
    CloseFile();
    std::cout << "[ERROR] Unable to create log file :" << tempFilePath << std::endl;
    return false;
  }

  tempFilePath = GetConfig().iLogPath + LOG_REPEAT_FILE_NAME;
  iLogRepeatPathFile = fopen(tempFilePath.c_str(), "w");

  if (iLogRepeatPathFile == NULL)
  {
    CloseFile();
    std::cout << "[ERROR] Unable to create log repeat path file :" << tempFilePath << std::endl;
    return false;
  }
#endif //ENABLE_LOG
  return true;
}

void CXdbGenerator::CloseFile()
{
  if (iInputTokenListFile != NULL)
  {
    fclose(iInputTokenListFile);
    iInputTokenListFile = NULL;
  }

  if (iXdbFile != NULL)
  {
    fclose(iXdbFile);
    iXdbFile = NULL;
  }

#ifdef ENABLE_LOG
  if (iLogFile != NULL)
  {
    fclose(iLogFile);
    iLogFile = NULL;
  }
  if (iLogRepeatPathFile != NULL)
  {
    fclose(iLogRepeatPathFile);
    iLogRepeatPathFile = NULL;
  }
#endif //ENABLE_LOG

}

void CXdbGenerator::GenerateBtreeNodeIndex(int aStart, int aEnd, int aFather, int aLevel, std::vector<TNodeInfo>::iterator aIter, int aDir)
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
    
    return GenerateBtreeNodeIndex(aEnd, aEnd, aStart, aLevel + 1, aIter, DIR_RIGHT);
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
  GenerateBtreeNodeIndex(aStart, mid - 1, mid, aLevel + 1, aIter, DIR_LEFT);

  // travel left+1
  GenerateBtreeNodeIndex(mid + 1, aEnd, mid, aLevel + 1, aIter, DIR_RIGHT);
}

void CXdbGenerator::WriteSortedDataToXdb(int aStart, int aEnd, unsigned int aNodeOffset, unsigned int aFatherOffset, int aPrime, std::vector<TNodeInfo>::iterator aIter, FILE *aFileXdb)
{
  int count = aEnd - aStart + 1;
  int mid = aStart + (count+1)/2 - 1;

  aIter[mid].offset = aNodeOffset;
  aIter[mid].length = 17 + aIter[mid].k_length + 12;

  if (NO_FATHER == aFatherOffset)
  {
    long fileOffset = 32 + 8 * aPrime;
    fseek(aFileXdb, fileOffset, SEEK_SET); // move to prime info
    fwrite(&aIter[mid].offset, sizeof(unsigned int), 1, aFileXdb);
    fwrite(&aIter[mid].length, sizeof(unsigned int), 1, aFileXdb);
  }

  fseek(aFileXdb, aNodeOffset, SEEK_SET); // move to node start offset
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
  long filePosistion = ftell(aFileXdb); // save current position

  int newMid;
  unsigned int tmpItemSize;
  if (aStart <= (mid-1))
  {
    int newEnd = mid - 1;
    count = newEnd - aStart + 1;
    newMid = aStart + (count+1)/2 - 1;

    fseek(aFileXdb, aNodeOffset, SEEK_SET); // move to node start offset
    fwrite(&filePosistion, sizeof(unsigned int), 1, aFileXdb);

    tmpItemSize = 17 + aIter[newMid].k_length + 12;
    fwrite(&tmpItemSize, sizeof(unsigned int), 1, aFileXdb);
    fseek(aFileXdb, filePosistion, SEEK_SET);

    // write left node info.
    WriteSortedDataToXdb(aStart, newEnd, filePosistion, aNodeOffset, aPrime, aIter, aFileXdb);
  }

  // Right node first
  filePosistion = ftell(aFileXdb); // save current position

  if (aEnd >= (mid+1))
  {
    int newStart = mid + 1;
    count = aEnd - newStart + 1;
    newMid = newStart + (count + 1)/2 - 1;

    fseek(aFileXdb, aNodeOffset + 8, SEEK_SET); // move to node start offset
    fwrite(&filePosistion, sizeof(unsigned int), 1, aFileXdb);

    tmpItemSize = 17 + aIter[newMid].k_length + 12;
    fwrite(&tmpItemSize, sizeof(unsigned int), 1, aFileXdb);
    fseek(aFileXdb, filePosistion, SEEK_SET);

    WriteSortedDataToXdb(newStart, aEnd, filePosistion, aNodeOffset, aPrime, aIter, aFileXdb);
  }
}
