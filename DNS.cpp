#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
int main (int argc, char *argv[])
{
struct hostent *h;
/* 检测命令行中的参数是否存在 */
if (argc != 2){
/* 如果没有参数，给出使用方法 */ 
fprintf (stderr,"usage: getip address\n");
/* 然后退出 */
exit(1);
}
/* 取得主机信息 */ 
if((h=gethostbyname(argv[1])) == NULL) {
/* 如果 gethostbyname 失败，则给出错误信息 */ 
herror("gethostbyname");
/* 然后退出 */
exit(1); 
}
/* 列印程序取得的信息 */
printf("Host name : %s\n", h->h_name);
printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *)h->h_addr))); /* 返回 */
return 0;
}