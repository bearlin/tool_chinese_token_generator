// xdb_gen.cpp : Defines the entry point for the console application.
//

#include <cstdio>
#include <cstring>
#include <cstdlib>     /* atoi */
#include <algorithm>
#include <vector>
#include <string>

//#define _GET_PRIME_
#define _GEN_XDB_

//[Normalization procedures]
//[Abandon] step 0: use xdb_dump tool to dump the original simple_export file from original raw xdb.
//[Abandon] step 1: Add/Delete/Update tokens in the original simple_export file and save to another updated simple_export file.
//step 1: use xdb_filter tool to filter, optimize and generate tt_tokens_list according to TomTom Addresses/POIs.
//step 2: use xdb_gen tool to generate a xdb(non-fuzzy) from this tt_tokens_list file.
//step 3: use xdb_dump tool to generate the non-normalized/normalized token files from this non-fuzzy xdb(with the latest fts-{tc|sc}-n.tok file), then merge them to a merge_export file.
//step 4: use xdb_gen tool generate a final normalized xdb(fuzzy) from this merge_export file.
// TODO: We can just skip step 3(skip xdb_dump), because we can just generage a normalized tt_tokens_list after step 1(we can get both tt_tokens_list(non-normailzed) and tt_tokens_list(normailzed) together).

#if 1 // traditional Chinese
#if 1 //step 2: use xdb_gen tool to generate a updated xdb from this updated simple_export file.
#define SRC_PATH "../TC/Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/s06_log_tokens_full.txt" 
#define LOG_PATH "../TC/Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/log(s06_log_tokens_full.txt).txt" 
#define XDB_PATH "../TC/Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/xdb(s06_log_tokens_full.txt).xdb" 
#define LOG_REPEAT_PATH "../TC/Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/log_repeat(s06_log_tokens_full.txt).txt" 
#else //step 4: use xdb_gen tool generate a final normalized xdb from this merge_export file.
#define SRC_PATH "../TC/Normalization_20131017_v01_xdb_filter_optimized/2_merge_gen/merge_export(SILK31).txt"
#define LOG_PATH "../TC/Normalization_20131017_v01_xdb_filter_optimized/2_merge_gen/log(merge_export(SILK31).txt).txt" 
#define XDB_PATH "../TC/Normalization_20131017_v01_xdb_filter_optimized/2_merge_gen/xdb(merge_export(SILK31).txt).xdb" 
#define LOG_REPEAT_PATH "../TC/Normalization_20131017_v01_xdb_filter_optimized/2_merge_gen/log_repeat(merge_export(SILK31).txt).txt" 
#endif
#else // simplify Chinese
#if 0 //step 2: use xdb_gen tool to generate a updated xdb from this updated simple_export file.
#define SRC_PATH "../SC/Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/s06_log_tokens_full.txt" 
#define LOG_PATH "../SC/Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/log(s06_log_tokens_full.txt).txt" 
#define XDB_PATH "../SC/Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/xdb(s06_log_tokens_full.txt).xdb" 
#define LOG_REPEAT_PATH "../SC/Normalization_20131017_v01_xdb_filter_optimized/1_simple_gen/log_repeat(s06_log_tokens_full.txt).txt" 
#else //step 4: use xdb_gen tool generate a final normalized xdb from this merge_export file.
#define SRC_PATH "../SC/Normalization_20131017_v01_xdb_filter_optimized/2_merge_gen/merge_export(SILK31).txt" 
#define LOG_PATH "../SC/Normalization_20131017_v01_xdb_filter_optimized/2_merge_gen/log(merge_export(SILK31).txt).txt" 
#define XDB_PATH "../SC/Normalization_20131017_v01_xdb_filter_optimized/2_merge_gen/xdb(merge_export(SILK31).txt).xdb" 
#define LOG_REPEAT_PATH "../SC/Normalization_20131017_v01_xdb_filter_optimized/2_merge_gen/log_repeat(merge_export(SILK31).txt).txt" 
#endif
#endif


#define ENABLE_LOG

/* constant var define */
#define  SCWS_WORD_FULL    0x01  // 多字: 整词
#define  SCWS_WORD_PART    0x02  // 多字: 前词段
#define  SCWS_WORD_USED    0x04  // 多字: 已使用
#define  SCWS_WORD_RULE    0x08  // 多字: 自动识别的
#define  SCWS_WORD_LONG    0x10  // 多字: 短词组成的长词

#define  SCWS_WORD_MALLOCED  0x80  // xdict_query 结果必须调用 free

#define  SCWS_ZFLAG_PUT    0x02  // 单字: 已使用
#define  SCWS_ZFLAG_N2    0x04  // 单字: 双字名词头
#define  SCWS_ZFLAG_NR2    0x08  // 单字: 词头且为双字人名
#define  SCWS_ZFLAG_WHEAD  0x10  // 单字: 词头
#define  SCWS_ZFLAG_WPART  0x20  // 单字: 词尾或词中
#define  SCWS_ZFLAG_ENGLISH  0x40  // 单字: 夹在中间的英文
#define SCWS_ZFLAG_SYMBOL   0x80    // 单字: 符号系列
#ifndef _GET_PRIME_ 
#define  SCWS_XDICT_PRIME  0x3ffd  // 词典结构树数：16381
#else
#define  SCWS_XDICT_PRIME  30000  // 词典结构树数：30000
#define CHK_MIN_PRIME 18000//16381
#define CHK_MAX_PRIME 30000
#endif

//#define MAX_NODE_COUNT    63
#define MAX_NODE_COUNT    127

typedef struct _node_info_
{
  int prime_index;
  int level;
  int father;

  unsigned int offset;
  unsigned int length;
  unsigned int l_offset;
  unsigned int l_length;
  unsigned int r_offset;
  unsigned int r_length;
  unsigned char k_length;
  unsigned char k_data[300];

  float tf;
  float idf;
  unsigned char flag;
  char attr[3];
} node_info;

/* utf-8: 0xc0, 0xe0, 0xf0, 0xf8, 0xfc */
static unsigned char _mblen_table_utf8[] = 
{
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
  4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1
};

int _get_hash_index(unsigned char* key, int hash_base, int hash_prime )
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

bool compare_node(const node_info s1,const node_info s2)
{
   int ret = strcmp( (const char*)s1.k_data, (const char*)s2.k_data );
   return ( ret < 0);
} 

/* constant var define */
#define  XDB_FLOAT_CHECK    (3.14)
#define  XDB_TAGNAME      "XDB"
#define  XDB_MAXKLEN      0xf0
#define  XDB_VERSION      34      /* version: 3bit+5bit */
#define XDB_BASE 0xf422f
//#define XDB_BASE 0xf4232

/* header struct */
struct xdb_header
{
  char tag[3];
  unsigned char ver;
  int base;
  int prime;
  unsigned int fsize;
  float check;
  char unused[12];
};

#define NO_FATHER      -1
#define DIR_LEFT  0
#define DIR_RIGHT 1

void gen_btree_node_index(int start, int end, int father, int level, std::vector<node_info>::iterator iter, int dir )
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


#ifdef _GET_PRIME_
int _tmain(int argc, _TCHAR* argv[])
{
  FILE *fp;
  node_info one_node;
  std::vector<node_info> nodes[SCWS_XDICT_PRIME];
  char buf[512];
  char part[512];
  char *ptr,*str;
  char *delim = " \t\r\n";
  int c;
  int found;
  int line_count = 0;
  int prime_index = 0;
  int total_len;
  int total_terms=0;

#ifdef ENABLE_LOG
  // log usage
  FILE *fp_log;
  char szDbg[1024];
#endif

  // xdb usage
  FILE *fp_xdb;

  // XDB related
  struct xdb_header xdb_h;
  unsigned char *pMem;
  //fpos_t fPos,fTmpPos;
  long fPos,fTmpPos;

  fp = fopen(SRC_PATH, "r");

  if( NULL == fp )
    return -1;

#ifdef ENABLE_LOG
  fp_log = fopen(LOG_PATH, "w");

  if( NULL == fp_log ) {
    fclose(fp);
    return -1;
  }
#endif

  fp_xdb = fopen(XDB_PATH, "wb");

  if( NULL == fp_xdb ) {
    fclose(fp);
#ifdef ENABLE_LOG
    fclose(fp_log);
#endif
    return -1;
  }

  for( int test_prime= CHK_MIN_PRIME; test_prime < CHK_MAX_PRIME; test_prime++ ) {
  printf("Start checking[Prime:%d].................................................\n", test_prime);

  while (fgets(buf, sizeof(buf)-1, fp) != NULL)
  {
    //printf("Read line %d\n", line_count++);

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
      one_node.attr[0] = '@';
      one_node.attr[1] = '\0';

      if (!(ptr = strtok(NULL, delim))) break;
      one_node.tf = (float) atof(ptr);

      if (!(ptr = strtok(NULL, delim))) break;
      one_node.idf = (float) atof(ptr);

      if (ptr = strtok(NULL, delim))
      {
        one_node.attr[0] = ptr[0];
        if (ptr[1]) one_node.attr[1] = ptr[1];
      }
    } while (0);

    // save the word
    prime_index = _get_hash_index((unsigned char*)str, XDB_BASE, test_prime  );

    found = 0;
    for (std::vector<node_info>::iterator iter = nodes[prime_index].begin(); (iter != nodes[prime_index].end()) && (0 == found) ; ++iter) {
      if( 0 == strcmp(str, (const char*)iter->k_data) ) {
        found = 1;
        iter->tf = one_node.tf;
        iter->idf = one_node.idf;
        iter->flag |= SCWS_WORD_FULL;
        strcpy(iter->attr, one_node.attr);
      }
    }

    if( 0 == found ) {
      one_node.prime_index = prime_index;
      one_node.k_length = c;
      strcpy((char*)one_node.k_data, str);
      nodes[prime_index].push_back(one_node);
    }

    /* parse the part */    
    total_len = _mblen_table_utf8[(unsigned char)(str[0])];
    while (1)
    {
      total_len += _mblen_table_utf8[(unsigned char)(str[total_len])];
      if (total_len >= c)
        break;

      memset(part,0,sizeof(part));
      strncpy(part, str, total_len);
      prime_index = _get_hash_index((unsigned char*)part, XDB_BASE, test_prime  );

      found = 0;
      for (std::vector<node_info>::iterator iter = nodes[prime_index].begin(); (iter != nodes[prime_index].end()) && (0 == found) ; ++iter) {
        if( 0 == strcmp(part, (const char*)iter->k_data) ) {
          found = 1;
          iter->flag |= SCWS_WORD_PART;
        }
      }

      if( 0 == found ) {
        one_node.prime_index = prime_index;
        one_node.k_length = total_len;
        one_node.flag = SCWS_WORD_PART;
        strcpy((char*)one_node.k_data, part);
        nodes[prime_index].push_back(one_node);
      }
    }
  }

  printf("Validation checking[Prime:%d].................................................\n", test_prime);
  found = 1;
  for(prime_index=0; prime_index < test_prime; prime_index++ ) {
    if( nodes[prime_index].size() > MAX_NODE_COUNT ) {
      //fclose(fp_xdb);
      //fclose(fp);
        //printf("PRIME[%d] illegal!!!  Count=%d.\n", prime_index, nodes[prime_index].size() );
      //return -1;
      found = 0;
      goto NEXT;
    }
  }

NEXT:
  if( 0 == found ) {
    // try to get data again
    for(int z=0; z < SCWS_XDICT_PRIME; z++ )
      nodes[z].clear();

    fseek( fp, 0 , SEEK_SET);
  }
  }


#if 0
  printf("Sorting.................................................\n");

  for(prime_index=0; prime_index < SCWS_XDICT_PRIME; prime_index++ ) {
    std::sort(nodes[prime_index].begin(), nodes[prime_index].end(),compare_node);
  }

  printf("\n\nWriting XDB to %s  ...............................\n",XDB_PATH );
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
    //fgetpos(fp_xdb , &fPos);  // save current position
    fPos=ftell(fp_xdb);// save current position

    // reset node information
    for (std::vector<node_info>::iterator iter = nodes[prime_index].begin(); iter != nodes[prime_index].end(); ++iter) {
      //fgetpos(fp_xdb , &fTmpPos );
      fTmpPos=ftell(fp_xdb);
      iter->offset = fTmpPos;
      iter->length = 17+iter->k_length+12;

      fwrite( &iter->l_offset, sizeof(unsigned int), 1, fp_xdb);
      fwrite( &iter->l_length, sizeof(unsigned int), 1, fp_xdb);
      fwrite( &iter->r_offset, sizeof(unsigned int), 1, fp_xdb);
      fwrite( &iter->r_length, sizeof(unsigned int), 1, fp_xdb);
      fwrite( &iter->k_length, sizeof(unsigned char), 1, fp_xdb);
      fwrite( iter->k_data, sizeof(unsigned char), iter->k_length, fp_xdb);
      fwrite( &iter->tf, sizeof(float), 1, fp_xdb);
      fwrite( &iter->idf, sizeof(float), 1, fp_xdb);
      fwrite( &iter->flag, sizeof(unsigned char), 1, fp_xdb);
      fwrite( &iter->attr, sizeof(char), 3, fp_xdb);
    }

    // build tree
    gen_btree_node_index(1, nodes[prime_index].size() , NO_FATHER, 1 , nodes[prime_index].begin(), DIR_LEFT);

    // write data again
    fseek(fp_xdb, fPos, SEEK_SET);
    for (std::vector<node_info>::iterator iter = nodes[prime_index].begin(); iter != nodes[prime_index].end(); ++iter) {
      if( 1 == iter->level ) {
        //fgetpos(fp_xdb , &fTmpPos );
        fTmpPos=ftell(fp_xdb);
        fseek(fp_xdb, 32+iter->prime_index*8, SEEK_SET);
        fwrite( &iter->offset, sizeof(unsigned int), 1, fp_xdb);
        fwrite( &iter->length, sizeof(unsigned int), 1, fp_xdb);
        fseek(fp_xdb, fTmpPos, SEEK_SET);
      }

      fwrite( &iter->l_offset, sizeof(unsigned int), 1, fp_xdb);
      fwrite( &iter->l_length, sizeof(unsigned int), 1, fp_xdb);
      fwrite( &iter->r_offset, sizeof(unsigned int), 1, fp_xdb);
      fwrite( &iter->r_length, sizeof(unsigned int), 1, fp_xdb);
      fwrite( &iter->k_length, sizeof(unsigned char), 1, fp_xdb);
      fwrite( iter->k_data, sizeof(unsigned char), iter->k_length, fp_xdb);
      fwrite( &iter->tf, sizeof(float), 1, fp_xdb);
      fwrite( &iter->idf, sizeof(float), 1, fp_xdb);
      fwrite( &iter->flag, sizeof(unsigned char), 1, fp_xdb);
      fwrite( &iter->attr, sizeof(char), 3, fp_xdb);
    }
  }

  // update size
  fseek(fp_xdb, 0L, SEEK_END);
  xdb_h.fsize = ftell(fp_xdb);
  fseek(fp_xdb, 0, SEEK_SET);
  fwrite( &xdb_h, sizeof(xdb_h), 1, fp_xdb );
#endif

#ifdef ENABLE_LOG
  sprintf(szDbg, "Total_Amount=%d \n", total_terms );
  fputs(szDbg,fp_log);
  fclose(fp_log);
#endif

  fclose(fp_xdb);
  fclose(fp);
  return 0;
}
#endif

#ifdef _GEN_XDB_
void write_sort_data_to_xdb(int start, int end, unsigned int node_offset, unsigned int father_offset, int prime, std::vector<node_info>::iterator iter, FILE *fp_xdb )
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

int main(int argc, char* argv[])
{
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

  fp = fopen(SRC_PATH, "r");

  if( NULL == fp )
    return -1;

#ifdef ENABLE_LOG
  fp_log = fopen(LOG_PATH, "w");

  if( NULL == fp_log ) {
    fclose(fp);
    return -1;
  }

  
  fp_log_repeat= fopen(LOG_REPEAT_PATH, "w");

  if( NULL == fp_log_repeat ) {
    fclose(fp);
    return -1;
  }
#endif

  fp_xdb = fopen(XDB_PATH, "wb");

  if( NULL == fp_xdb ) {
    fclose(fp);
#ifdef ENABLE_LOG
    fclose(fp_log);
#endif
    return -1;
  }

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
    std::sort(nodes[prime_index].begin(), nodes[prime_index].end(),compare_node);
  }

  printf("\n\nWriting XDB to %s  ...............................\n",XDB_PATH );
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


#if 0 
  for(prime_index=0; prime_index < SCWS_XDICT_PRIME; prime_index++ ) {
    // if no element in the list, move to next one.
    if( 0 == nodes[prime_index].size() )
      continue;

    //fgetpos(fp_xdb , &fPos);  // save current position
    fPos=ftell(fp_xdb);// save current position

    // reset node information
    for (std::vector<node_info>::iterator iter = nodes[prime_index].begin(); iter != nodes[prime_index].end(); ++iter) {
      //fgetpos(fp_xdb , &fTmpPos );
      fTmpPos=ftell(fp_xdb);
      iter->offset = fTmpPos;
      iter->length = 17+iter->k_length+12;

      fwrite( &iter->l_offset, sizeof(unsigned int), 1, fp_xdb);
      fwrite( &iter->l_length, sizeof(unsigned int), 1, fp_xdb);
      fwrite( &iter->r_offset, sizeof(unsigned int), 1, fp_xdb);
      fwrite( &iter->r_length, sizeof(unsigned int), 1, fp_xdb);
      fwrite( &iter->k_length, sizeof(unsigned char), 1, fp_xdb);
      fwrite( iter->k_data, sizeof(unsigned char), iter->k_length, fp_xdb);
      fwrite( &iter->tf, sizeof(float), 1, fp_xdb);
      fwrite( &iter->idf, sizeof(float), 1, fp_xdb);
      fwrite( &iter->flag, sizeof(unsigned char), 1, fp_xdb);
      fwrite( &iter->attr, sizeof(char), 3, fp_xdb);
    }

    // build tree
    gen_btree_node_index(1, nodes[prime_index].size() , NO_FATHER, 1 , nodes[prime_index].begin(), DIR_LEFT);

    // write data again
    fseek(fp_xdb, fPos, SEEK_SET);
    for (std::vector<node_info>::iterator iter = nodes[prime_index].begin(); iter != nodes[prime_index].end(); ++iter) {
      if( 1 == iter->level ) {
        //fgetpos(fp_xdb , &fTmpPos );
        fTmpPos=ftell(fp_xdb);
        fseek(fp_xdb, 32+iter->prime_index*8, SEEK_SET);
        fwrite( &iter->offset, sizeof(unsigned int), 1, fp_xdb);
        fwrite( &iter->length, sizeof(unsigned int), 1, fp_xdb);
        fseek(fp_xdb, fTmpPos, SEEK_SET);
      }

      fwrite( &iter->l_offset, sizeof(unsigned int), 1, fp_xdb);
      fwrite( &iter->l_length, sizeof(unsigned int), 1, fp_xdb);
      fwrite( &iter->r_offset, sizeof(unsigned int), 1, fp_xdb);
      fwrite( &iter->r_length, sizeof(unsigned int), 1, fp_xdb);
      fwrite( &iter->k_length, sizeof(unsigned char), 1, fp_xdb);
      fwrite( iter->k_data, sizeof(unsigned char), iter->k_length, fp_xdb);
      fwrite( &iter->tf, sizeof(float), 1, fp_xdb);
      fwrite( &iter->idf, sizeof(float), 1, fp_xdb);
      fwrite( &iter->flag, sizeof(unsigned char), 1, fp_xdb);
      fwrite( &iter->attr, sizeof(char), 3, fp_xdb);
    }
  }
#endif

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
  return 0;
}
#endif
