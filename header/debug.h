#ifndef __DEBUG__  
#define __DEBUG__  

#define deprintf(format,...) printf("File: "__FILE__"(L%d) Func: %s() : "format"", __LINE__,__FUNCTION__, ##__VA_ARGS__)  
  
#endif
