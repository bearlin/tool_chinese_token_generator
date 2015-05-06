//
// This file contains the CXdbGenerator implementation 
//

#include "XdbGenerator.h"
#include <iostream>

CXdbGenerator::CXdbGenerator() :
  _mblen_table_utf8 {
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
  std::cout << "iInputPath:" << iInputPath << std::endl;
  std::cout << "iOutputPath:" << iOutputPath<< std::endl;
  std::cout << "iLogPath:" << iLogPath<< std::endl;

  FILE *fp;
  node_info one_node;
  std::vector<node_info> nodes[SCWS_XDICT_PRIME];
  char buf[512];
  char part[512];
  char *ptr,*str;
  const char *delim = " \t\r\n";
  int c;
  int found;
  int line_count = 0;
  int prime_index = 0;
  int total_len;
  int total_terms=0;

  std::string LineOri;
#ifdef ENABLE_LOG
  // log usage
  FILE *fp_log;
  FILE *fp_log_repeat;
  char szDbg[1024];
#endif

  // xdb usage
  FILE *fp_xdb;

  // XDB related
  struct xdb_header xdb_h;
  unsigned char *pMem;
  long fPos,fTmpPos;
  //fpos_t fPos,fTmpPos;

  file_path = iInputPath + iInputTokenList;
  fp = fopen(file_path.c_str(), "r");
  if( NULL == fp )
  {
    printf("fp err:%s\n", file_path.c_str());
    return false;
  }
  printf("fp :%s\n", file_path.c_str());

#ifdef ENABLE_LOG
  file_path = iLogPath + LOG_PATH;
  fp_log = fopen(file_path.c_str(), "w");
  if(NULL == fp_log)
  {
    fclose(fp);
    printf("fp_log err:%s\n", file_path.c_str());
    return false;
  }
  printf("fp_log :%s\n", file_path.c_str());

  file_path = iLogPath + LOG_REPEAT_PATH;
  fp_log_repeat= fopen(file_path.c_str(), "w");
  if(NULL == fp_log_repeat)
  {
    fclose(fp);
    printf("fp_log_repeat err:%s\n", file_path.c_str());
    return false;
  }
  printf("fp_log_repeat :%s\n", file_path.c_str());
#endif

  file_path = GetOutputPath() + GetOutputXdb();
  fp_xdb = fopen(file_path.c_str(), "wb");
  if(NULL == fp_xdb)
  {
    fclose(fp);
#ifdef ENABLE_LOG
    fclose(fp_log);
#endif
    printf("fp_xdb err:%s\n", file_path.c_str());
    return false;
  }
  printf("fp_xdb :%s\n", file_path.c_str());

  while (fgets(buf, sizeof(buf)-1, fp) != NULL)
  {
    printf("Read line %d\n", line_count++);
    LineOri = buf;
    memset(&one_node,0, sizeof(node_info) );

    // <word>\t<tf>\t<idf>\t<attr>\n
    if (buf[0] == ';' || buf[0] == '#')
      continue;

    str = strtok(buf, delim);
    if (str == NULL) continue;
    c = strlen(str);
    
    // init the word
    do
    {  
      one_node.tf = one_node.idf = 1.0;
      one_node.flag = SCWS_WORD_FULL;
      one_node.attr[0] = '\0';
      //one_node.attr[0] = '@';
      //one_node.attr[1] = '\0';


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
      printf("skip_part:%s", LineOri.c_str());
      continue;
    }
    
    // save the word
    prime_index = _get_hash_index((unsigned char*)str, XDB_BASE, SCWS_XDICT_PRIME  );

    found = 0;
    for (std::vector<node_info>::iterator iter = nodes[prime_index].begin(); (iter != nodes[prime_index].end()) && (0 == found) ; ++iter) 
    {
      if( 0 == strcmp(str, (const char*)iter->k_data) ) 
      {
        found = 1;
      #if 1
        #ifdef ENABLE_LOG
        fprintf(fp_log_repeat, "found existed node[%s\t%f\t%f\t%d\t%s]\n",iter->k_data, iter->tf, iter->idf, iter->flag, iter->attr);
        fprintf(fp_log_repeat, "new node[%s\t%f\t%f\t%d\t%s]\n",str, one_node.tf, one_node.idf, one_node.flag, one_node.attr);
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
        fprintf(fp_log_repeat, "updated node[%s\t%f\t%f\t%d\t%s]\n",iter->k_data, iter->tf, iter->idf, iter->flag, iter->attr);
        #endif
      #else //#if 1
        iter->tf = one_node.tf;
        iter->idf = one_node.idf;
        iter->flag |= SCWS_WORD_FULL;
        strcpy(iter->attr, one_node.attr);
      #endif //#if 1
      }
    }

    if( 0 == found ) 
    {
      one_node.prime_index = prime_index;
      one_node.k_length = c;
      strcpy((char*)one_node.k_data, str);
      nodes[prime_index].push_back(one_node);
    }

#if 1
    // Generate partial tokens for this raw token.
    /* parse the part */    
    total_len = _mblen_table_utf8[(unsigned char)(str[0])];
    while (1)
    {
      total_len += _mblen_table_utf8[(unsigned char)(str[total_len])];
      if (total_len >= c)
        break;

      memset(part,0,sizeof(part));
      strncpy(part, str, total_len);
      prime_index = _get_hash_index((unsigned char*)part, XDB_BASE, SCWS_XDICT_PRIME  );

      found = 0;
      for (std::vector<node_info>::iterator iter = nodes[prime_index].begin(); (iter != nodes[prime_index].end()) && (0 == found) ; ++iter) {
        if( 0 == strcmp(part, (const char*)iter->k_data) ) {
          found = 1;
          iter->flag |= SCWS_WORD_PART;
        }
      }

      if( 0 == found ) {
        memset(&one_node,0,sizeof(one_node));
        one_node.prime_index = prime_index;
        one_node.k_length = total_len;
        one_node.flag = SCWS_WORD_PART;
        //one_node.flag = SCWS_WORD_FULL;
        strcpy((char*)one_node.k_data, part);
        nodes[prime_index].push_back(one_node);
      }
    }
#endif
  }

  printf("Validation checking(Phase1).................................................\n");

  for(prime_index=0; prime_index < SCWS_XDICT_PRIME; prime_index++ ) {
    if( nodes[prime_index].size() > MAX_NODE_COUNT ) {
        printf("PRIME[%d] illegal!!!  Count=%ld.\n", prime_index, nodes[prime_index].size() );
    }
  }

  printf("Validation checking.................................................\n");

  for(prime_index=0; prime_index < SCWS_XDICT_PRIME; prime_index++ ) {
    if( nodes[prime_index].size() > MAX_NODE_COUNT ) {
      fclose(fp_xdb);
      fclose(fp);
        printf("PRIME[%d] illegal!!!  Count=%ld.\n", prime_index, nodes[prime_index].size() );
      return -1;
    }
  }


  printf("Sorting.................................................\n");

  for(prime_index=0; prime_index < SCWS_XDICT_PRIME; prime_index++ ) {
    std::sort(nodes[prime_index].begin(), nodes[prime_index].end(), compare_node);
  }

  printf("\n\nWriting XDB to %s  ...............................\n", (GetOutputPath()+GetOutputXdb()).c_str());
  memset(&xdb_h, 0 , sizeof(struct xdb_header));
  memcpy(&xdb_h.tag, XDB_TAGNAME, 3);
  xdb_h.ver = XDB_VERSION;
  xdb_h.prime = SCWS_XDICT_PRIME;
  xdb_h.base = XDB_BASE;
  xdb_h.fsize = sizeof(struct xdb_header)+xdb_h.prime*8;
  xdb_h.check = (float)XDB_FLOAT_CHECK;

  fseek(fp_xdb, 0, SEEK_SET);
  fwrite( &xdb_h, sizeof(xdb_h), 1, fp_xdb );

  pMem = (unsigned char*)malloc(xdb_h.prime*8);
  memset(pMem, 0 , xdb_h.prime*8);
  fwrite( pMem, sizeof(unsigned char), xdb_h.prime*8, fp_xdb );
  free(pMem);

  for(prime_index=0; prime_index < SCWS_XDICT_PRIME; prime_index++ ) {
    // if no element in the list, move to next one.
    if( 0 == nodes[prime_index].size() )
      continue;

#ifdef ENABLE_LOG
    int iter_index = 0;
    //if( 0 == nodes[prime_index].size() % 2 ) //print tree only with odd node size.
    {
      fprintf(fp_log, "===========================\n");
      for (std::vector<node_info>::iterator iter = nodes[prime_index].begin(); (iter != nodes[prime_index].end()); ++iter)
        fprintf(fp_log, "SORT[Prime:%d][%d]%s\n", prime_index, iter_index++, iter->k_data);
    }
#endif

    //fgetpos(fp_xdb , &fPos);  // save current position
    fPos=ftell(fp_xdb);// save current position
    write_sort_data_to_xdb(0, nodes[prime_index].size()-1, fPos, NO_FATHER, prime_index, nodes[prime_index].begin(), fp_xdb );
  }

  // update size
  fseek(fp_xdb, 0L, SEEK_END);
  xdb_h.fsize = ftell(fp_xdb);
  fseek(fp_xdb, 0, SEEK_SET);
  fwrite( &xdb_h, sizeof(xdb_h), 1, fp_xdb );

#ifdef ENABLE_LOG
  sprintf(szDbg, "Total_Amount=%d \n", total_terms );
  fputs(szDbg,fp_log);
  fclose(fp_log);
  fclose(fp_log_repeat);
#endif

  fclose(fp_xdb);
  fclose(fp);

  return true;
}

void CXdbGenerator::SetInputTokenList(std::string aInputTokenList)
{
  iInputTokenList = aInputTokenList;
}

void CXdbGenerator::SetOutputXdb(std::string aOutputXdb)
{
  iOutputXdb = aOutputXdb;
}

std::string CXdbGenerator::GetOutputXdb()
{
  return iOutputXdb;
}

int CXdbGenerator::_get_hash_index(unsigned char* key, int hash_base, int hash_prime )
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

bool CXdbGenerator::compare_node(const node_info s1,const node_info s2)
{
   int ret = strcmp( (const char*)s1.k_data, (const char*)s2.k_data );
   return ( ret < 0);
}

void CXdbGenerator::gen_btree_node_index(int start, int end, int father, int level, std::vector<node_info>::iterator iter, int dir )
{
  int diff = end-start;
  int avg = diff/2;
  int mid_index = start+avg;

  // Leaf node
  if( start == end ) {
    iter[mid_index-1].level = level;
    iter[mid_index-1].father = father;

    // update father node
    if( father > 0 ) {
      if( DIR_LEFT == dir ) {
        iter[father-1].l_offset = iter[start-1].offset;
        iter[father-1].l_length = iter[start-1].length;
      } else {
        iter[father-1].r_offset = iter[start-1].offset;
        iter[father-1].r_length = iter[start-1].length;
      }
    }
    return;
  }

  if( 1 == diff )  // even
  {
    if( NO_FATHER != father ) {
      // update father node
      if( DIR_LEFT == dir ) {
        iter[father-1].l_offset = iter[start-1].offset;
        iter[father-1].l_length = iter[start-1].length;
      } else {
        iter[father-1].r_offset = iter[start-1].offset;
        iter[father-1].r_length = iter[start-1].length;
      }
    }

    iter[start-1].level = level;
    iter[start-1].father = father;
    
    return gen_btree_node_index( end, end, start, level+1,iter, DIR_RIGHT );
  } else {
     // printf("Level[%d] Index=%d father=%d Dir=%c\n", level, mid_index, father, (DIR_LEFT == dir) ? 'L':'R' );
    // Node
    iter[mid_index-1].level = level;
    iter[mid_index-1].father = father;

    if( NO_FATHER != father ) {
      if( DIR_LEFT == dir ) {
        iter[father-1].l_offset = iter[mid_index-1].offset;
        iter[father-1].l_length = iter[mid_index-1].length;
      } else {
        iter[father-1].r_offset = iter[mid_index-1].offset;
        iter[father-1].r_length = iter[mid_index-1].length;
      }
    }
  }

  // travel left
  gen_btree_node_index( start, mid_index-1, mid_index, level+1,iter, DIR_LEFT );

  // travel left+1
  gen_btree_node_index( mid_index+1, end, mid_index, level+1,iter, DIR_RIGHT );
}

void CXdbGenerator::write_sort_data_to_xdb(int start, int end, unsigned int node_offset, unsigned int father_offset, int prime, std::vector<node_info>::iterator iter, FILE *fp_xdb )
{
  int count;
  int mid; 
  unsigned int tmpItemSize;
  //fpos_t fPos,fTmpPos;
  long fPos,fTmpPos;
  int newEnd, newStart, newMid;

  count = (end-start)+1;

  /*if( (2 == count) && (NO_FATHER == father_offset) )
    mid = start+1;
  else*/
    mid = start+(count+1)/2-1;

  iter[mid].offset = node_offset;
  iter[mid].length = 17+iter[mid].k_length+12;

  if( NO_FATHER == father_offset ) {
    fTmpPos = 32+8*prime;
    //fsetpos(fp_xdb , &fTmpPos );  // move to prime info
    fseek(fp_xdb, fTmpPos, SEEK_SET);// move to prime info
    fwrite( &iter[mid].offset, sizeof(unsigned int), 1, fp_xdb);
    fwrite( &iter[mid].length, sizeof(unsigned int), 1, fp_xdb);    
  }

  fTmpPos = node_offset;
  //fsetpos(fp_xdb , &fTmpPos );  // move to node start offset
  fseek(fp_xdb, fTmpPos, SEEK_SET);// move to node start offset
  fwrite( &iter[mid].l_offset, sizeof(unsigned int), 1, fp_xdb);
  fwrite( &iter[mid].l_length, sizeof(unsigned int), 1, fp_xdb);
  fwrite( &iter[mid].r_offset, sizeof(unsigned int), 1, fp_xdb);
  fwrite( &iter[mid].r_length, sizeof(unsigned int), 1, fp_xdb);
  fwrite( &iter[mid].k_length, sizeof(unsigned char), 1, fp_xdb);
  fwrite( &iter[mid].k_data, sizeof(unsigned char), iter[mid].k_length, fp_xdb);
  fwrite( &iter[mid].tf, sizeof(float), 1, fp_xdb);
  fwrite( &iter[mid].idf, sizeof(float), 1, fp_xdb);
  fwrite( &iter[mid].flag, sizeof(unsigned char), 1, fp_xdb);
  fwrite( &iter[mid].attr, sizeof(char), 3, fp_xdb);

  // travel from left tree, if necessary
  //fgetpos(fp_xdb , &fPos);  // save current position
  fPos=ftell(fp_xdb);// save current position

  if( start <= (mid-1) ) {
    newEnd = (mid-1);
    count = (newEnd-start)+1;
    newMid = start+(count+1)/2-1;

    fTmpPos = node_offset;
    //fsetpos(fp_xdb , &fTmpPos );  // move to node start offset
    fseek(fp_xdb, fTmpPos, SEEK_SET);// move to node start offset
    fwrite( &fPos, sizeof(unsigned int), 1, fp_xdb);
    tmpItemSize = 17+iter[newMid].k_length+12;
    fwrite( &tmpItemSize, sizeof(unsigned int), 1, fp_xdb);
    //fsetpos(fp_xdb , &fPos );
    fseek(fp_xdb, fPos, SEEK_SET);

    // write left node info.
    write_sort_data_to_xdb(start, newEnd, fPos, node_offset, prime, iter, fp_xdb );
  }

  // Right node first
  //fgetpos(fp_xdb , &fPos);  // save current position
  fPos=ftell(fp_xdb);// save current position

  if( end >= (mid+1) ) {
    newStart = (mid+1);
    count = (end-newStart)+1;
    newMid = newStart+(count+1)/2-1;

    fTmpPos = node_offset+8;
    //fsetpos(fp_xdb , &fTmpPos );  // move to node start offset
    fseek(fp_xdb, fTmpPos, SEEK_SET);// move to node start offset
    fwrite( &fPos, sizeof(unsigned int), 1, fp_xdb);
    tmpItemSize = 17+iter[newMid].k_length+12;
    fwrite( &tmpItemSize, sizeof(unsigned int), 1, fp_xdb);
    //fsetpos(fp_xdb , &fPos );
    fseek(fp_xdb, fPos, SEEK_SET);

    write_sort_data_to_xdb(newStart, end, fPos, node_offset, prime, iter, fp_xdb );
  }
}
















