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
  std::cout << "CXdbGenerator" << std::endl;
}

CXdbGenerator::~CXdbGenerator()
{
  std::cout << "~CXdbGenerator" << std::endl;
}

bool CXdbGenerator::Run()
{
  std::cout << "Running CXdbGenerator" << std::endl;
  std::cout << "iInputPath:" << GetConfig().iInputPath << std::endl;
  std::cout << "iOutputPath:" << GetConfig().iOutputPath<< std::endl;
  std::cout << "iLogPath:" << GetConfig().iLogPath<< std::endl;

  TNodeInfo one_node;
  std::vector<TNodeInfo> nodes[SCWS_XDICT_PRIME];

  // XDB related
  TXdb_header xdb_h;
  unsigned char *pMem;
  long fPos,fTmpPos;
  //fpos_t fPos,fTmpPos;

  std::string filePath;
  filePath =  GetConfig().iInputPath + GetConfig().iInputTokenList;

  FILE *inputTokenListFile = fopen(filePath.c_str(), "r");

  if (inputTokenListFile == NULL)
  {
    printf("[ERROR] Unable to open input token list file :%s to read.\n", filePath.c_str());
    return false;
  }

  printf("Input token list file :%s is opened.\n", filePath.c_str());

#ifdef ENABLE_LOG
  filePath = GetConfig().iLogPath + LOG_PATH;
  FILE *logFile = fopen(filePath.c_str(), "w");
  if (logFile == NULL)
  {
    fclose(inputTokenListFile);
    printf("[ERROR] Unable to create log file :%s\n", filePath.c_str());
    return false;
  }
  printf("Log file :%s is created.\n", filePath.c_str());

  filePath = GetConfig().iLogPath + LOG_REPEAT_PATH;
  FILE *logRepeatPathFile = fopen(filePath.c_str(), "w");

  if (logRepeatPathFile == NULL)
  {
    fclose(inputTokenListFile);
    fclose(logFile);
    printf("[ERROR]Unable to create log repeat path file :%s\n", filePath.c_str());
    return false;
  }
  printf("Log repeat path file :%s is created.\n", filePath.c_str());
#endif

  filePath = GetConfig().GetOutputPath() + GetConfig().GetOutputXdb();
  // xdb usage
  FILE *xdbFile = fopen(filePath.c_str(), "wb");
  if (xdbFile == NULL)
  {
    fclose(inputTokenListFile);

    #ifdef ENABLE_LOG
      fclose(logFile);
      fclose(logRepeatPathFile);
    #endif

    printf("[ERROR]Unable to create xdb file :%s\n", filePath.c_str());
    return false;
  }
  printf("xdb file :%s is create\n", filePath.c_str());


  int lineCount = 0;
  int primeIndex = 0;

  char buf[512];
  while (fgets(buf, sizeof(buf)-1, inputTokenListFile) != NULL)
  {
    printf("Read line %d\n", lineCount++);
    std::string line(buf);

    memset(&one_node,0, sizeof(TNodeInfo));

    // <word>\t<tf>\t<idf>\t<attr>\n
    if ((buf[0] == ';') || (buf[0] == '#'))
    {
      continue;
    }

    const char* delim = " \t\r\n";
    char* str = strtok(buf, delim);
    if (str == NULL) continue;
    int length = strlen(str);
    
    // init the word
    do
    {  
      one_node.tf = one_node.idf = 1.0;
      one_node.flag = SCWS_WORD_FULL;
      one_node.attr[0] = '\0';
      //one_node.attr[0] = '@';
      //one_node.attr[1] = '\0';

      char* ptr = NULL;
      if (!(ptr = strtok(NULL, delim))) break;
      one_node.tf = (float) atof(ptr);

      if (!(ptr = strtok(NULL, delim))) break;
      one_node.idf = (float) atof(ptr);

      // bypass flag
      if (NULL != (ptr = strtok(NULL, delim))) {
        one_node.flag = atoi(ptr);
        //continue;
      }

      if (ptr = strtok(NULL, delim))
      {
        one_node.attr[0] = ptr[0];
        if (ptr[1]) one_node.attr[1] = ptr[1];
      }
    } while (0);

    //Skip partial tokens from the raw input file..
    if (!(one_node.flag & SCWS_WORD_FULL))
    {
      printf("skip_part:%s", line.c_str());
      continue;
    }
    
    // save the word
    primeIndex = getHashIndex((unsigned char*)str, XDB_BASE, SCWS_XDICT_PRIME  );

    int found = 0;
    for (std::vector<TNodeInfo>::iterator iter = nodes[primeIndex].begin(); (iter != nodes[primeIndex].end()) && (found == 0) ; ++iter)
    {
      if (strcmp(str, (const char*)iter->k_data) == 0)
      {
        found = 1;
        #if 1
          #ifdef ENABLE_LOG
            fprintf(logRepeatPathFile, "found existed node[%s\t%f\t%f\t%d\t%s]\n",iter->k_data, iter->tf, iter->idf, iter->flag, iter->attr);
            fprintf(logRepeatPathFile, "new node[%s\t%f\t%f\t%d\t%s]\n",str, one_node.tf, one_node.idf, one_node.flag, one_node.attr);
          #endif
      
          // Update as max(tf, idf).
          if (one_node.tf > iter->tf)
            iter->tf = one_node.tf;
          if (one_node.idf > iter->idf)
            iter->idf = one_node.idf;

          iter->flag |= SCWS_WORD_FULL;

          // Use last attr[].
          strcpy(iter->attr, one_node.attr);
          #ifdef ENABLE_LOG
            fprintf(logRepeatPathFile, "updated node[%s\t%f\t%f\t%d\t%s]\n",iter->k_data, iter->tf, iter->idf, iter->flag, iter->attr);
          #endif
        #else //#if 1
          iter->tf = one_node.tf;
          iter->idf = one_node.idf;
          iter->flag |= SCWS_WORD_FULL;
          strcpy(iter->attr, one_node.attr);
        #endif //#if 1
      }
    }

    if (found ==0)
    {
      one_node.prime_index = primeIndex;
      one_node.k_length = length;
      strcpy((char*)one_node.k_data, str);
      nodes[primeIndex].push_back(one_node);
    }

    // Generate partial tokens for this raw token.
    /* parse the part */    
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
      primeIndex = getHashIndex((unsigned char*)part, XDB_BASE, SCWS_XDICT_PRIME  );

      found = 0;
      for (std::vector<TNodeInfo>::iterator iter = nodes[primeIndex].begin(); (iter != nodes[primeIndex].end()) && (0 == found) ; ++iter)
      {
        if (strcmp(part, (const char*)iter->k_data) == 0)
        {
          found = 1;
          iter->flag |= SCWS_WORD_PART;
        }
      }

      if (found == 0)
      {
        memset(&one_node,0,sizeof(one_node));
        one_node.prime_index = primeIndex;
        one_node.k_length = totalLength;
        one_node.flag = SCWS_WORD_PART;
        //one_node.flag = SCWS_WORD_FULL;
        strcpy((char*)one_node.k_data, part);
        nodes[primeIndex].push_back(one_node);
      }
    }
  }

  printf("Validation checking(Phase1).................................................\n");

  for (primeIndex=0; primeIndex < SCWS_XDICT_PRIME; primeIndex++)
  {
    if( nodes[primeIndex].size() > MAX_NODE_COUNT )
    {
        printf("PRIME[%d] illegal!!!  Count=%ld.\n", primeIndex, nodes[primeIndex].size());
    }
  }

  printf("Validation checking.................................................\n");

  for (primeIndex = 0; primeIndex < SCWS_XDICT_PRIME; primeIndex++)
  {
    if (nodes[primeIndex].size() > MAX_NODE_COUNT)
    {
      fclose(xdbFile);
      fclose(inputTokenListFile);
      printf("PRIME[%d] illegal!!!  Count=%ld.\n", primeIndex, nodes[primeIndex].size());
      return -1;
    }
  }

  printf("Sorting.................................................\n");

  for(primeIndex = 0; primeIndex < SCWS_XDICT_PRIME; primeIndex++ )
  {
    std::sort(nodes[primeIndex].begin(), nodes[primeIndex].end(), compareNode);
  }

  printf("\n\nWriting XDB to %s  ...............................\n", (GetConfig().GetOutputPath()+GetConfig().GetOutputXdb()).c_str());
  memset(&xdb_h, 0 , sizeof(TXdb_header));
  memcpy(&xdb_h.tag, XDB_TAGNAME, 3);
  xdb_h.ver = XDB_VERSION;
  xdb_h.prime = SCWS_XDICT_PRIME;
  xdb_h.base = XDB_BASE;
  xdb_h.fsize = sizeof(TXdb_header)+xdb_h.prime*8;
  xdb_h.check = (float)XDB_FLOAT_CHECK;

  fseek(xdbFile, 0, SEEK_SET);
  fwrite(&xdb_h, sizeof(xdb_h), 1, xdbFile);

  pMem = (unsigned char*)malloc(xdb_h.prime*8);
  memset(pMem, 0 , xdb_h.prime*8);
  fwrite(pMem, sizeof(unsigned char), xdb_h.prime*8, xdbFile);
  free(pMem);

  for (primeIndex = 0; primeIndex < SCWS_XDICT_PRIME; primeIndex++ )
  {
    // if no element in the list, move to next one.
    if (nodes[primeIndex].size() == 0)
    {
      continue;
    }

#ifdef ENABLE_LOG
    int iter_index = 0;
    //if( 0 == nodes[prime_index].size() % 2 ) //print tree only with odd node size.
    {
      fprintf(logFile, "===========================\n");
      for (std::vector<TNodeInfo>::iterator iter = nodes[primeIndex].begin(); (iter != nodes[primeIndex].end()); ++iter)
      {
        fprintf(logFile, "SORT[Prime:%d][%d]%s\n", primeIndex, iter_index++, iter->k_data);
      }
    }
#endif

    fPos=ftell(xdbFile);// save current position
    writeSortedDataToXdb(0, nodes[primeIndex].size()-1, fPos, NO_FATHER, primeIndex, nodes[primeIndex].begin(), xdbFile);
  }

  // update size
  fseek(xdbFile, 0L, SEEK_END);
  xdb_h.fsize = ftell(xdbFile);
  fseek(xdbFile, 0, SEEK_SET);
  fwrite(&xdb_h, sizeof(xdb_h), 1, xdbFile);

#ifdef ENABLE_LOG
  fclose(logFile);
  fclose(logRepeatPathFile);
#endif

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
   return ( ret < 0);
}

void CXdbGenerator::generateBtreeNodeIndex(int aStart, int aEnd, int aFather, int aLevel, std::vector<TNodeInfo>::iterator aIter, int aDir)
{
  int diff = aEnd - aStart;
  int avg = diff/2;
  int mid_index = aStart + avg;

  // Leaf node
  if( aStart == aEnd ) {
    aIter[mid_index-1].level = aLevel;
    aIter[mid_index-1].father = aFather;

    // update father node
    if( aFather > 0 ) {
      if( DIR_LEFT == aDir ) {
        aIter[aFather-1].l_offset = aIter[aStart-1].offset;
        aIter[aFather-1].l_length = aIter[aStart-1].length;
      } else {
        aIter[aFather-1].r_offset = aIter[aStart-1].offset;
        aIter[aFather-1].r_length = aIter[aStart-1].length;
      }
    }
    return;
  }

  if( 1 == diff )  // even
  {
    if( NO_FATHER != aFather ) {
      // update father node
      if( DIR_LEFT == aDir ) {
        aIter[aFather-1].l_offset = aIter[aStart-1].offset;
        aIter[aFather-1].l_length = aIter[aStart-1].length;
      } else {
        aIter[aFather-1].r_offset = aIter[aStart-1].offset;
        aIter[aFather-1].r_length = aIter[aStart-1].length;
      }
    }

    aIter[aStart-1].level = aLevel;
    aIter[aStart-1].father = aFather;
    
    return generateBtreeNodeIndex( aEnd, aEnd, aStart, aLevel+1, aIter, DIR_RIGHT );
  } else {
     // printf("Level[%d] Index=%d father=%d Dir=%c\n", level, mid_index, father, (DIR_LEFT == dir) ? 'L':'R' );
    // Node
    aIter[mid_index-1].level = aLevel;
    aIter[mid_index-1].father = aFather;

    if( NO_FATHER != aFather ) {
      if( DIR_LEFT == aDir ) {
        aIter[aFather-1].l_offset = aIter[mid_index-1].offset;
        aIter[aFather-1].l_length = aIter[mid_index-1].length;
      } else {
        aIter[aFather-1].r_offset = aIter[mid_index-1].offset;
        aIter[aFather-1].r_length = aIter[mid_index-1].length;
      }
    }
  }

  // travel left
  generateBtreeNodeIndex( aStart, mid_index-1, mid_index, aLevel+1, aIter, DIR_LEFT );

  // travel left+1
  generateBtreeNodeIndex( mid_index+1, aEnd, mid_index, aLevel+1, aIter, DIR_RIGHT );
}

void CXdbGenerator::writeSortedDataToXdb(int aStart, int aEnd, unsigned int aNodeOffset, unsigned int aFatherOffset, int aPrime, std::vector<TNodeInfo>::iterator aIter, FILE *aFileXdb)
{
  unsigned int tmpItemSize;
  //fpos_t fPos,fTmpPos;
  long fPos;
  long fTmpPos;
  int newEnd;
  int newStart;
  int newMid;
  int count = aEnd - aStart + 1;
  int mid = aStart + (count+1)/2-1;

  aIter[mid].offset = aNodeOffset;
  aIter[mid].length = 17+aIter[mid].k_length+12;

  if (NO_FATHER == aFatherOffset)
  {
    fTmpPos = 32 + 8 * aPrime;
    //fsetpos(fp_xdb , &fTmpPos );  // move to prime info
    fseek(aFileXdb, fTmpPos, SEEK_SET);// move to prime info
    fwrite( &aIter[mid].offset, sizeof(unsigned int), 1, aFileXdb);
    fwrite( &aIter[mid].length, sizeof(unsigned int), 1, aFileXdb);
  }

  fTmpPos = aNodeOffset;
  //fsetpos(aFileXdb , &fTmpPos );  // move to node start offset
  fseek(aFileXdb, fTmpPos, SEEK_SET);// move to node start offset
  fwrite( &aIter[mid].l_offset, sizeof(unsigned int), 1, aFileXdb);
  fwrite( &aIter[mid].l_length, sizeof(unsigned int), 1, aFileXdb);
  fwrite( &aIter[mid].r_offset, sizeof(unsigned int), 1, aFileXdb);
  fwrite( &aIter[mid].r_length, sizeof(unsigned int), 1, aFileXdb);
  fwrite( &aIter[mid].k_length, sizeof(unsigned char), 1, aFileXdb);
  fwrite( &aIter[mid].k_data, sizeof(unsigned char), aIter[mid].k_length, aFileXdb);
  fwrite( &aIter[mid].tf, sizeof(float), 1, aFileXdb);
  fwrite( &aIter[mid].idf, sizeof(float), 1, aFileXdb);
  fwrite( &aIter[mid].flag, sizeof(unsigned char), 1, aFileXdb);
  fwrite( &aIter[mid].attr, sizeof(char), 3, aFileXdb);

  // travel from left tree, if necessary
  //fgetpos(fp_xdb , &fPos);  // save current position
  fPos=ftell(aFileXdb);// save current position

  if (aStart <= (mid-1))
  {
    newEnd = mid - 1;
    count = newEnd - aStart + 1;
    newMid = aStart + (count+1)/2 - 1;

    fTmpPos = aNodeOffset;
    //fsetpos(fp_xdb , &fTmpPos );  // move to node start offset
    fseek(aFileXdb, fTmpPos, SEEK_SET);// move to node start offset
    fwrite( &fPos, sizeof(unsigned int), 1, aFileXdb);
    tmpItemSize = 17 + aIter[newMid].k_length + 12;
    fwrite( &tmpItemSize, sizeof(unsigned int), 1, aFileXdb);
    //fsetpos(fp_xdb , &fPos );
    fseek(aFileXdb, fPos, SEEK_SET);

    // write left node info.
    writeSortedDataToXdb(aStart, newEnd, fPos, aNodeOffset, aPrime, aIter, aFileXdb);
  }

  // Right node first
  //fgetpos(aFileXdb , &fPos);  // save current position
  fPos=ftell(aFileXdb);// save current position

  if (aEnd >= (mid+1))
  {
    newStart = mid + 1;
    count = aEnd - newStart + 1;
    newMid = newStart + (count + 1)/2 - 1;

    fTmpPos = aNodeOffset + 8;
    //fsetpos(aFileXdb , &fTmpPos );  // move to node start offset
    fseek(aFileXdb, fTmpPos, SEEK_SET);// move to node start offset
    fwrite( &fPos, sizeof(unsigned int), 1, aFileXdb);
    tmpItemSize = 17 + aIter[newMid].k_length + 12;
    fwrite( &tmpItemSize, sizeof(unsigned int), 1, aFileXdb);
    //fsetpos(aFileXdb , &fPos );
    fseek(aFileXdb, fPos, SEEK_SET);

    writeSortedDataToXdb(newStart, aEnd, fPos, aNodeOffset, aPrime, aIter, aFileXdb);
  }
}
















