#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
	int count = 0;
	while (*s++ != '\0')
	{
		count++;
	}
	return (size_t)count;
}

char *strcpy(char *dst, const char *src) {
	while (*src++ != '\0')
	{
		*dst = *src;
	}
	return dst;
}


char *strncpy(char *dest, const char *src, size_t len)
{
	assert(dest != NULL && src != NULL);
	char *res = dest;
	int offset = 0;
	if (strlen(src) < len)//src长度小于len
	{
		offset = len - strlen(src);
		len = strlen(src);
	}

	while (len--)
	{
		*dest++ = *src++;
	}
	while (offset--)
	{
		*dest++ = '\0';
	}
	return res;
}



char *strcat(char *dst, const char *src) {
  char *ret = dst;
  assert(dst && src);
	while (*dst)
	{
		dst++;
	}
	while (*dst++ = *src++)
	{
		;
	}
	return ret;
}

int strcmp(const char *s1, const char *s2) {
      char*   p1;  
      char*   p2;  
      p1=s1;  
      p2=s2;  
      while((*p1)&&(*p2))  
      {  
          if(*p1==*p2)  
          {  
              p1++;p2++;  
          }else{  
              return   (*p1-*p2);  
          }  
      }  
      return   (*p1-*p2);  
}

int strncmp(const char *s1, const char *s2, size_t n) {
      int i = 0;
      char*   p1;  
      char*   p2;  
      p1=s1;  
      p2=s2;  
      while((*p1)&&(*p2))  
      {  
          if(*p1==*p2)  
          {  
              p1++;p2++;  
          }else{  
              return   (*p1-*p2);  
          } 
          i++;
          if(i == n){
            break;
          } 
      }  
      return   (*p1-*p2);  
}

void *(memset) (void *s,int c,size_t n)
{
	const unsigned char uc = c;    //unsigned char占1字节，意味着只截取c的后八位
	unsigned char *su;
	for(su = s;0 < n;++su,--n)
		*su = uc;
	return s;
}

void *memmove(void* dest, const void* src, size_t n)
{
	assert(dest && src);
	void* ret = dest;
	if (dest < src)
	{
		while (n--)//前->后
		{
			*(char*)dest = *(char*)src;
			dest = (char*)dest + 1;
			src = (char*)src + 1;
		}
	}
	else
	{
		while (n--)//后->前
		{
			*((char*)dest + n) = *((char*)src +n);
		}
	}
	return ret;
}


void* memcpy(void* dst, const void* src, size_t n)
{
	if (NULL == dst || NULL == src) {
		return NULL;
	}

	void* ret = dst;

	if (dst <= src || (char*)dst >= (char*)src + n) {
		//没有内存重叠，从低地址开始复制
		while (n--) {
			*(char*)dst = *(char*)src;
			dst = (char*)dst + 1;
			src = (char*)src + 1;
		}
	}
	else {
		//有内存重叠，从高地址开始复制
		src = (char*)src +n - 1;
		dst = (char*)dst +n - 1;
		while (n--) {
			*(char*)dst = *(char*)src;
			dst = (char*)dst - 1;
			src = (char*)src - 1;
		}
	}
	return ret;
}


int memcmp(const void *s1, const void *s2, size_t n) {
   if (!n)

      return(0);

   while ( --n && *(char *)s1 == *(char *)s2)

   {

       s1 = (char *)s1 + 1;

       s2 = (char *)s2 + 1;

   }

   return( *((unsigned char *)s1) - *((unsigned char *)s2) );
}

#endif
