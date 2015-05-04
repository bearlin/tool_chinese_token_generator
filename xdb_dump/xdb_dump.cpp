// xdb_dump.cpp : Defines the entry point for the console application.
//

#include <cstdio>
#include <cstring>
#include <cstdlib>     /* atoi */

#include <string>
#include <vector>
#include <map>
#include <algorithm>    // std::find std::sort

// Select one of below macro:
// _XDB_GEN_TOOL_EXPORT_ will use dict_cht.utf8.xdb as input and output all nodes to simple_export.txt.
// _DETAIL_EXPORT_ will use dict_cht.utf8.xdb as input too, and output detail nodes information to detail_export.txt.
#define _XDB_GEN_TOOL_EXPORT_
//#define _DETAIL_EXPORT_

//_CONVERT_NORMALIZE_ + _XDB_GEN_TOOL_EXPORT_ will output all nodes to normal_export.txt, and its key will be normalized with char_pinyin_normalize_utf8.txt.
//#define _CONVERT_NORMALIZE_

//[Normalization procedures]
//[Abandon] step 0: use xdb_dump tool to dump the original simple_export file from original raw xdb.
//[Abandon] step 1: Add/Delete/Update tokens in the original simple_export file and save to another updated simple_export file.
//step 1: use xdb_filter tool to filter, optimize and generate tt_tokens_list according to TomTom Addresses/POIs.
//step 2: use xdb_gen tool to generate a xdb(non-fuzzy) from this tt_tokens_list file.
//step 3: use xdb_dump tool to generate the non-normalized/normalized token files from this non-fuzzy xdb(with the latest fts-{tc|sc}-n.tok file), then merge them to a merge_export file.
//step 4: use xdb_gen tool generate a final normalized xdb(fuzzy) from this merge_export file.
// TODO: We can just skip step 3(skip xdb_dump), because we can just generage a normalized tt_tokens_list after step 1(we can get both tt_tokens_list(non-normailzed) and tt_tokens_list(normailzed) together).

// TC
#if 1
  #define SRC_XDB_FILE "../TC/Normalization_20131017_v01_xdb_filter_optimized/xdb(s06_log_tokens_full.txt).xdb"
  #ifdef _DETAIL_EXPORT_
    #define DETAIL_EXP_FILE "../TC/Normalization_20131017_v01_xdb_filter_optimized/detail_export.txt"  // detail export file
  #endif
  #ifdef _XDB_GEN_TOOL_EXPORT_
    #define SIMPLE_EXP_FILE "../TC/Normalization_20131017_v01_xdb_filter_optimized/simple_export.txt"  // simple export file
  #endif
  #ifdef _CONVERT_NORMALIZE_
    //#define NORMAL_MAP_FILE "../char_pinyin_normalize_utf8.txt"  // normalize mapping file
    #define NORMAL_MAP_FILE "../TC/Normalization_20131017_v01_xdb_filter_optimized/fts-tc-n.tok"  // normalize mapping file
    #define NORMAL_EXP_FILE "../TC/Normalization_20131017_v01_xdb_filter_optimized/normal_export.txt"  // normalize export file
    #define NORMAL_EXP_REPEAT "../TC/Normalization_20131017_v01_xdb_filter_optimized/normal_export_repeat.txt"  // normalize export repeat file
  #endif
#else
//SC
  #define SRC_XDB_FILE "../SC/Normalization_20131017_v01_xdb_filter_optimized/xdb(s06_log_tokens_full.txt).xdb"
  #ifdef _DETAIL_EXPORT_
    #define DETAIL_EXP_FILE "../SC/Normalization_20131017_v01_xdb_filter_optimized/detail_export.txt"  // detail export file
  #endif
  #ifdef _XDB_GEN_TOOL_EXPORT_
    #define SIMPLE_EXP_FILE "../SC/Normalization_20131017_v01_xdb_filter_optimized/simple_export.txt"  // simple export file
  #endif
  #ifdef _CONVERT_NORMALIZE_
    //#define NORMAL_MAP_FILE "../char_pinyin_normalize_utf8.txt"  // normalize mapping file
    #define NORMAL_MAP_FILE "../SC/Normalization_20131017_v01_xdb_filter_optimized/fts-sc-n.tok"  // normalize mapping file
    #define NORMAL_EXP_FILE "../SC/Normalization_20131017_v01_xdb_filter_optimized/normal_export.txt"  // normalize export file
    #define NORMAL_EXP_REPEAT "../SC/Normalization_20131017_v01_xdb_filter_optimized/normal_export_repeat.txt"  // normalize export repeat file
  #endif
#endif

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

typedef struct _node_content
{
  float tf;
  float idf;
  unsigned char flag;
  char attr[3];
} node_content;

FILE *gLog;
FILE *gLogDetail;

#ifdef _CONVERT_NORMALIZE_
std::map<std::string, std::string>  g_normalize_hash;
FILE *gNormalizeLog;

// For repeat normalized tokens log.
std::map<std::string,node_content> nor_map; //this map will save normalized string and it's node_content informations.
std::map<std::string,std::string> first_nor_to_ori_map; //we will remember the first original_string -> normalized_string pair.
std::vector<std::string> nor_repeat_vector;
FILE *gNormalizeRepeatLog;

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
#endif //_CONVERT_NORMALIZE_

unsigned long word_count = 0;
char szLog[1024];
//unsigned int XDB_MAXKLEN = 0xf0;
int gPrime,gHashBase;

int _get_index(unsigned char* key, int hash_base, int hash_prime )
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

void get_record(FILE *fd, unsigned int off, unsigned int len, int direction, int level, const char* father ) {
  unsigned int l_offset, l_len;
  unsigned int r_offset, r_len;
  char k_len;
  unsigned char key_name[256];
  unsigned char attr[4];
  node_content ct;

  fseek(fd, off, SEEK_SET);
  //r_len = XDB_MAXKLEN+17;
  //if(r_len > len) r_len = len;
  //fread(buff, r_len, 1, fd);  

  fread(&l_offset, sizeof(unsigned int), 1, fd );
  fread(&l_len, sizeof(unsigned int), 1, fd );
  fread(&r_offset, sizeof(unsigned int), 1, fd );
  fread(&r_len, sizeof(unsigned int), 1, fd );
  fread(&k_len, sizeof(unsigned char), 1, fd );

  memset(key_name,0, sizeof(key_name));
  //key_name[0] = '\"';
  //fread(&key_name[0], sizeof(unsigned char), k_len, fd );
  fread(key_name, sizeof(unsigned char), k_len, fd );
  //key_name[k_len+1] = '\"';
  //key_name[k_len+2] = '\n';
  //fputs((const char*)key_name, gLog);
  fread(&ct, sizeof(node_content), 1, fd );

  if( (l_offset > 0 ) && (l_len > 0) ) {
    get_record(fd, l_offset, l_len, 1,level+1,(char*)key_name );
  }

  if( (r_offset > 0 ) && (r_len > 0) ) {
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
    ( 0 == ct.attr[0]) ? 0x01:ct.attr[0],
    ( 0 == ct.attr[1]) ? 0x01:ct.attr[1],
    ( 0 == ct.attr[2]) ? 0x01:ct.attr[2],
    key_name,
    _get_index(key_name,gHashBase, gPrime )
    );
    fputs(szLog, gLogDetail );
  // Detail log
#endif

  memset(attr,0,4);
#ifdef _XDB_GEN_TOOL_EXPORT_
  // OWN toolsusage
  memcpy(attr, ct.attr, 3);
  sprintf(szLog, "%s\t%f\t%f\t%d\t%s\n",key_name, ct.tf, ct.idf, ct.flag, attr);
  fputs(szLog, gLog );
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
  memset(normal_key_name,0,sizeof(normal_key_name));

  key_len = strlen((char*)key_name);

  for(int z =0; z < key_len; )
  {
    char_utf8_len = _mblen_table_utf8[key_name[z]];

    if( 3 != char_utf8_len ) 
    {
      memcpy(&normal_key_name[z], &key_name[z], char_utf8_len );
    } 
    else 
    {
      memset(cht_utf8,0,sizeof(cht_utf8));
      memcpy(cht_utf8, &key_name[z], char_utf8_len );

      char_utf8.clear();
      char_utf8 += (char*)cht_utf8;

      findIter = g_normalize_hash.find(char_utf8);
      if( findIter == g_normalize_hash.end() ) 
      {
        printf("NOT Found  [%x][%x][%x]...................\n", cht_utf8[0], cht_utf8[1], cht_utf8[2]);
        memcpy(&normal_key_name[z], &key_name[z], char_utf8_len );
      } 
      else 
      {
        memcpy(&normal_key_name[z], g_normalize_hash[char_utf8].c_str(), char_utf8_len );
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
      sprintf(szLog, "%s<=%s\t%f\t%f\t%d\t%s\n",nor_str.c_str(), first_nor_to_ori_map[nor_str].c_str(), nor_map_it->second.tf, nor_map_it->second.idf, nor_map_it->second.flag, nor_map_it->second.attr);
      
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
  sprintf(szLog, "%s\t%f\t%f\t%d\t%s\n",normal_key_name, ct.tf, ct.idf, ct.flag, attr);
  fputs(szLog, gNormalizeLog );
#endif //_CONVERT_NORMALIZE_
}

int main(int argc, char* argv[])
{
  FILE *fd;
  struct xdb_header xdb_header;
  int trav_index;
  unsigned int f_offset;
  unsigned int d_offset;
  unsigned int d_length;

#ifdef _DETAIL_EXPORT_
  gLogDetail = fopen(DETAIL_EXP_FILE, "w" );
  if( NULL == gLogDetail)
  {
    printf("Error open file=%s\n", DETAIL_EXP_FILE );
    return -1;
  }
#endif
#ifdef _XDB_GEN_TOOL_EXPORT_
  gLog = fopen(SIMPLE_EXP_FILE, "w" );
#endif

  if( NULL == gLog )
  {
    printf("Error open file=%s\n", SIMPLE_EXP_FILE);
    return -1;
  }

  if( NULL == (fd= fopen(SRC_XDB_FILE,"rb")) )
  {
    printf("Error open file=%s\n", SRC_XDB_FILE);
    return -1;
  }

#ifdef _CONVERT_NORMALIZE_
  // Read mapping into tables
  FILE *normalize_fd;
  char szLine[256], *pFind;
  int token_idx,szLine_Len;
  char szSrc[4], szDst[4];

  gNormalizeLog = fopen(NORMAL_EXP_FILE, "w" ); if( NULL == gNormalizeLog )
    return -1;

  normalize_fd = fopen(NORMAL_MAP_FILE, "r" );
  if( NULL == normalize_fd )
    return -1;

  // For repeat normalized tokens log.
  nor_map.clear();
  nor_repeat_vector.clear();
  gNormalizeRepeatLog = fopen(NORMAL_EXP_REPEAT, "w" );
  if( NULL == gNormalizeRepeatLog )
    return -1;
  
  while( fgets(szLine,sizeof(szLine), normalize_fd )) 
  {
    szLine_Len = strlen(szLine);
    szLine_Len--;

    while( ('\r' == szLine[szLine_Len]) || ('\n' == szLine[szLine_Len]) )
      szLine_Len--;

    szLine[szLine_Len+1] = 0;

    token_idx = 0;
    pFind = strtok(szLine, ",");
    while( pFind ) 
    {
      if(1 == token_idx ) 
      {
        memset(szSrc,0,sizeof(szSrc));
        strcpy(szSrc, pFind);
      } 
      else if(2 == token_idx ) 
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

  printf("XDB header size=%ld\n", sizeof(struct xdb_header) );
  fread(&xdb_header, sizeof(struct xdb_header),1,fd);

  //printf("unsigned int size=%d\n", sizeof(unsigned int) );
  printf("unsigned base=%d\n", xdb_header.base );
  printf("unsigned size=%d\n", xdb_header.fsize );
  printf("unsigned prime=%d\n", xdb_header.prime );

  gPrime = xdb_header.prime;
  gHashBase = xdb_header.base;

  //sprintf(szLog, "DB base=%d size=%d prime=%d\n", xdb_header.base, xdb_header.fsize, xdb_header.prime );
  //fputs(szLog, gLog );

  trav_index = 0;

  do 
  {
    f_offset = trav_index*8+32;
    fseek(fd, f_offset, SEEK_SET);
    fread(&d_offset, sizeof(unsigned int), 1, fd );
    fread(&d_length, sizeof(unsigned int), 1, fd );

    if( 0 != d_length ) 
    {
      printf("trav_index=%d d_offset=%d d_length=%d\n", trav_index, d_offset, d_length );
      get_record(fd, d_offset, d_length,0,0, "");
      word_count = 0;

      //sprintf(szLog, "+++++++++++++++++++Prime_index=%d\n", trav_index );
      //fputs(szLog, gLog );
    }
    trav_index++;
  } while( trav_index < xdb_header.prime );

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

  return 0;
}
