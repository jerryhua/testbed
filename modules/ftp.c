#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winbase.h>

typedef struct Hoststruct
{
 char HostName[128];
 unsigned int HostIp; 
 char Hostport[32];
 char UserName[128];
 char PassWord[128];
 char CurrentDir[128];
 SOCKET HostSocketCmd;
 SOCKET HostSocketData;
 int npsupport;
 int HostState;
 struct Hoststruct *next;
}Host;

typedef struct _RESUME
{
 char dest[128];
 char tmp[128];
 char file[128];
 char hostSite[128];
 char UserName[128];
 char PassWord[128];
 int size;
 int alreadydone;
 int hostip;
 unsigned short hostPort;
 int state;
 struct _RESUME *next;
 SOCKET sockcmd;
 SOCKET sockdata;
 HANDLE hThread;
 HANDLE hTmpFile;
}RESUME;

typedef struct _upload
{
 SOCKET cmdSock;
 SOCKET sockdata;
 char file[128];
 char dest[128];
 char hostSite[128];
 int size;
 int alreadydone;
 HANDLE hThread;
 struct _upload *next;
}UPLOAD;

Host HostList;
Host *CurrentHost;
RESUME ResumeFile;
UPLOAD Up;
char numResumeFiles;
int PasvOrPort; 

int resumeList();
int _scanf(char *s);
int Login();
int ListDir();
int receive(SOCKET s,char *buf,int len);
int post(SOCKET s,char *buf,int len);
int addToHostList(Host *h);
int changeDirectory(char *directory);
int showCurrentDirectory();
int downLoad(char *file,char *directory);
int resumeDownLoad(RESUME *profile);
int upLoad(char *file,char *dest);
int DeleteDown(RESUME *r);
int DeleteHost(Host *h);
int DetectCurrentHostHost();

int main(int argc, char* argv[])
{
 int ret;
 WSADATA wsa;
 RESUME *r;
 HANDLE hout;
 COORD cord;
 struct in_addr in;
 char *cmd;
 char commandLine[1024]={0};
 char param1[1024]={0};
 char param2[1024]={0};
 char command[1024]={0};
 char answer;
 struct sockaddr_in hostaddr;
 char data[1024]={0};
 SOCKET cmdSocket;
 resumeList();
  CreateDirectory("incompletes",0);
 CreateDirectory("downloads",0);
 hout = GetStdHandle(STD_OUTPUT_HANDLE);
 cord.X=128;
 cord.Y=8192;
 SetConsoleScreenBufferSize(hout,cord);
 if(!WSAStartup(0x0101,&wsa))
 {
  printf("winsock initialization done! wait for command.../n");
  printf("type /" ? /"for help");
 }
 else
 {
  printf("fail to initialize winsock!/n");
  return 0;
 }
typeCommand:
 printf("/n");
 printf("ftp> ");
 ZeroMemory(commandLine,1024);
 ZeroMemory(param1,1024);
 ZeroMemory(param2,1024);
 _scanf(commandLine);
//process commandling...
 cmd=commandLine;
 while(*(cmd)==' ')cmd++;
 if(strncmp(cmd,"login",5)==0)
 {
  Login();
 }
 else if(strncmp(cmd,"quit",4)==0)
 {
  printf("bye^_^");
  WSACleanup();
  return 1;
 }
 else if(strncmp(cmd,"show site",9)==0)
 {
  int d=0;
  Host *phosts;
  phosts=HostList.next;
  printf("/n");
  while(phosts!=NULL)
  {
   d++;
   in.S_un.S_addr=phosts->HostIp;
   printf("%d. Site:%s Ip:%s/n",d,phosts->HostName,inet_ntoa(in));
   phosts=phosts->next;
  }
 }
 else if(strncmp(cmd,"site",4)==0)
 {
  int psite=0;
  Host *phost;
  phost=HostList.next;
  cmd=cmd+5;
  ZeroMemory(param1,sizeof(param1));
  while(*(cmd)!=0)
  {
   param1[psite]=*(cmd);
   cmd++;
   psite++;
  }
  psite=atoi(param1);
  psite--;
  while(psite && phost!=NULL)
  {
   psite--;
   phost=phost->next;
  }
  if(phost!=NULL)
  {
   CurrentHost=phost;
   printf("/nCurrentHost:%s/n",CurrentHost->HostName);
  }
 }
 else if(strncmp(cmd,"resume",6)==0)
 {
  int k=0,s=1;
  cmd=cmd+7;
  ZeroMemory(param1,sizeof(param1));
  while(*(cmd)!=0)
  {
   param1[k]=*(cmd);
   cmd++;
   k++;
  }
  k=atoi(param1);
  r=ResumeFile.next;
  while(r!=NULL)
  {
   if(s==k)goto resume;
   s++;
   r=r->next;
  }
resume:
  if(r!=NULL)
  {
   if(r->alreadydone>=r->size)
    printf("/ndownload file %s already complete/n,",r->file);
   else
   resumeDownLoad(r);
  }
 }
 else if(strncmp(cmd,"show down",9)==0)
 {
  int c=0;
  r=&ResumeFile;
  r=r->next;
  printf("/n");
  while(r!=NULL)
  {
   c++;
   in.S_un.S_addr=r->hostip;
   if(r->state==1)
   {printf("%d.",c);printf("file %s from site %s,total %d,already done: %d/nstate: on/n",r->file,inet_ntoa(in),r->size,r->alreadydone);}
   else
   {printf("%d.",c);printf("file %s from site %s,total %d,already done: %d/nstate: off/n",r->file,inet_ntoa(in),r->size,r->alreadydone);}
   r=r->next;
  }
 }
 else if(strncmp(cmd,"show up",7)==0)
 {
  int c=0;
  UPLOAD *u;
  u=&Up;
  u=u->next;
  printf("/n");
  while(u!=NULL)
  {
   c++;
   if(u->alreadydone==1)
   {printf("%d.",c);printf("upload file %s to site %s/%s succeeded/n",u->file,u->hostSite,u->dest);}
   else if(u->alreadydone==0)
   {printf("%d.",c);printf("upload file %s to site %s/%s is being done",u->file,u->hostSite,u->dest);}
   u=u->next;
  }
 }
 else if(strncmp(cmd,"cancel",6)==0)
 {
  int pr=0;
  RESUME *res;
  res=ResumeFile.next;
  cmd=cmd+7;
  ZeroMemory(param1,sizeof(param1));
  while(*(cmd)!=0)
  {
   param1[pr]=*(cmd);
   cmd++;
   pr++;
  }
  pr=atoi(param1);
  pr--;
  while(pr && res!=NULL)
  {
   pr--;
   res=res->next;
  }
  if(res!=NULL)
  {
   TerminateThread(res->hThread,0);
   closesocket(res->sockcmd);
   closesocket(res->sockdata);
   CloseHandle(res->hTmpFile);
   res->state=0;
   DeleteFile(res->tmp);
   DeleteDown(res);
  }
 }
 else if(strncmp(cmd,"stop",4)==0)
 {
  int pfile=0;
  RESUME *re;
  re=ResumeFile.next;
  cmd=cmd+5;
  ZeroMemory(param1,sizeof(param1));
  while(*(cmd)!=0)
  {
   param1[pfile]=*(cmd);
   cmd++;
   pfile++;
  }
  pfile=atoi(param1);
  pfile--;
  while(pfile && re!=NULL)
  {
   pfile--;
   re=re->next;
  }
  if(re!=NULL)
  {
   TerminateThread(re->hThread,0);
   closesocket(re->sockcmd);
   closesocket(re->sockdata);
   if(re->alreadydone!=re->size)re->state=0;
  }
 }
 if(CurrentHost==NULL)goto typeCommand;
 if(strncmp(cmd,"dir",3)==0)
 {
  if(CurrentHost!=NULL)
  {
   ListDir();
  }
 }
 else if(strncmp(cmd,"cd",2)==0)
 {
  int i=0;
  cmd=cmd+3;
  while(*(cmd)!=0)
  {
   param1[i]=*(cmd);
   cmd++;
   i++;
  }
  changeDirectory(param1);
 }
 else if(strncmp(cmd,"pos",3)==0)
 {
  showCurrentDirectory();
 }
 else if(strncmp(cmd,"download",8)==0)
 {
  int j=0;
  cmd=cmd+9;
  while(*(cmd)!=0)
  {
   param1[j]=*(cmd);
   cmd++;
   j++;
  }
  printf("/n保存路径(默认为//downloads//):/n");
  _scanf(param2);
  if(param2[0]!=0)
  {
   int err;
   CreateDirectory(param2,0);
   err=GetLastError();
   if(err!=183 && err!=0)
   {
    printf("/n路径错误/n");
    goto typeCommand;
   }
  }
  else
  {
   strcpy(param2,"downloads/0");
  }
  downLoad(param1,param2);
 }
 else if(strncmp(cmd,"upload",6)==0)
 {
  printf("/n源文件路径:/n");
  _scanf(param1);
  printf("/n目标:/n");
  _scanf(param2);
  upLoad(param1,param2);
 }
 else if(strncmp(cmd,"show current",12)==0)
 {
  printf("/nCurrentHost: %s/n",CurrentHost->HostName);
 }
 else if(strncmp(cmd,"pasv",4)==0)
 {
  printf("Enter pasv mode.../n");
  PasvOrPort=0;
 }
 else if(strncmp(cmd,"port",4)==0)
 {
  PasvOrPort=1;
  printf("Enter port mode.../n");
 }
 else if(strncmp(cmd,"logoff",6)==0)
 {
  if(CurrentHost!=NULL)
  {
   closesocket(CurrentHost->HostSocketCmd);
   closesocket(CurrentHost->HostSocketData);
   printf("/nHost:%s,logoff!/n",CurrentHost->HostName);
   DeleteHost(CurrentHost);
   CurrentHost=NULL;
  }
 }
 if(CurrentHost!=NULL)
  ret=DetectCurrentHostHost();
 if(ret<0 && CurrentHost!=NULL)
 {
  answer='y';
  printf("/ncurrent connection closed by remote host,reconnecting.../n");
  if(answer=='y')
  {
   hostaddr.sin_family=AF_INET;
   hostaddr.sin_port=(unsigned short)htons((unsigned short)atoi(CurrentHost->Hostport));
   hostaddr.sin_addr.s_addr=CurrentHost->HostIp;
   cmdSocket=socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
   ret=connect(cmdSocket,&hostaddr,sizeof hostaddr);
   if(ret!=0)
   {
    printf("/nConnect error!/n");
    return -1;
   }
   ret=receive(cmdSocket,data,1024);
   if(ret!=1)
   {
    printf("/nConnect error!/n");
    return -1;
   }
   if(strncmp(data,"220",3)==0)
   {
    printf("/nConnect success,send UserName");
   }
   else
   {
    printf("/nConnect error!/n");
    return -1;
   }
   ZeroMemory(command,1024);
   strcat(command,"USER ");
   strcat(command,CurrentHost->UserName);
   strcat(command,"/r/n/0");
   ret=post(cmdSocket,command,strlen(command));
   if(ret!=1)
   {
    printf("/nConnect error!/n");
    return -1;
   }
   ZeroMemory(data,1024);
   ret=receive(cmdSocket,data,1024);
   if(strncmp(data,"331",3)==0)
   {
    printf("/nConnect success,send PassWord");
   }
   else
   {
    printf("/nConnect error!/n");
    return -1;
   }
   ZeroMemory(command,1024);
   strcat(command,"PASS ");
   strcat(command,CurrentHost->PassWord);
   strcat(command,"/r/n/0");
   ret=post(cmdSocket,command,strlen(command));
   if(ret!=1)
   {
    printf("/nConnect error!/n");
    return-1;
   }
   ret=receive(cmdSocket,data,1024);
   if(strncmp(data,"230",3)==0)
   {
    printf("/nLogin success,wait for command.../n");
   } 
   CurrentHost->HostSocketCmd=cmdSocket;
  }
 }
 ZeroMemory(commandLine,1024);
 goto typeCommand;
 printf("/n");
 return 1;
}

int _scanf(char *s)
{
 char c=0;
 int i=0;
 while(c!='/n')
 {
  scanf("%c",&c);
  *(s+i)=c;
  i++;
 }
 *(s+i-1)=0;
 *(s+i)=0;
 return 1;
}

int receive(SOCKET s,char *buf,int len)
{
 FD_SET ReadSet;
 struct timeval time;
 int iret;
 time.tv_sec=1;
 FD_ZERO(&ReadSet);
 FD_SET(s,&ReadSet);
 time.tv_usec=0;
 iret=select(0,&ReadSet,0,0,&time);
 GetLastError();
 if(iret==SOCKET_ERROR)
  return -1;
 else if(iret==0)
  return -2;
 recv(s,buf,len,0);
 return 1;
}

int _receive(SOCKET s,char *buf,int len)
{
 FD_SET ReadSet;
 struct timeval time;
 int iret;
 time.tv_sec=1;
 FD_ZERO(&ReadSet);
 FD_SET(s,&ReadSet);
 time.tv_usec=0;
 iret=select(0,&ReadSet,0,0,&time);
 GetLastError();
 if(iret==SOCKET_ERROR)
  return -1;
 else if(iret==0)
  return -2;
 iret=recv(s,buf,len,0);
 return iret;
}

int post(SOCKET s,char *buf,int len)
{
 FD_SET WriteSet;
 struct timeval time;
 int iret;
 time.tv_sec=1;
 FD_ZERO(&WriteSet);
 FD_SET(s,&WriteSet);
 time.tv_usec=0;
 iret=select(0,0,&WriteSet,0,&time);
 GetLastError();
 if(iret==SOCKET_ERROR)
  return -1;
 else if(iret==0)
  return -2;
 send(s,buf,len,0);
 return 1;
}

int addToHostList(Host *h)
{
 Host *host;
 host= &HostList;
 while(host->next!=NULL)
 {
  host=host->next;
 }
 host->next=h;
 h->next=NULL;
 return 1;
}

int Login()
{
 SOCKET cmdSocket;
 struct sockaddr_in hostaddr;
 Host *host;
 int ret;
 unsigned short port;
 char **ptr=NULL;
 int *addr=NULL;
 struct hostent *iphost=NULL;
 char data[1024]={0};
 char command[1024]={0};
 host=(Host *)malloc(sizeof(Host));
 printf("Enter HostName:");
 _scanf(host->HostName);
 iphost=gethostbyname(host->HostName);
 if(iphost==NULL)
 {
  printf("/nConnect error!/n");
  return -1;
 }
 printf("Enter port(Default is 21):");
 _scanf(host->Hostport);
 port=atoi(host->Hostport);
 if(port==0 || port>65535) 
  port=21;
  ptr=iphost->h_addr_list;
 addr=(int *)(*(ptr));
 hostaddr.sin_family=AF_INET;
 hostaddr.sin_port=(unsigned short)htons(port);
 hostaddr.sin_addr.s_addr=*(addr);
 cmdSocket=socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
 ret=connect(cmdSocket,&hostaddr,sizeof hostaddr);
 if(ret!=0)
 {
  printf("/nConnect error!/n");
  return -1;
 }
 host->HostSocketCmd=cmdSocket;
 ret=receive(host->HostSocketCmd,data,1024);
 if(ret!=1)
 {
  printf("/nConnect error!/n");
  return -1;
 }
 if(strncmp(data,"220",3)==0)
 {
  printf("/nConnect success,need UserName:");
 }
 else
 {
  printf("/nConnect error!/n");
  return -1;
 }
 _scanf(host->UserName);
 ZeroMemory(command,1024);
 strcat(command,"USER ");
 strcat(command,host->UserName);
 strcat(command,"/r/n/0");
 ret=post(host->HostSocketCmd,command,strlen(command));
 if(ret!=1)
 {
  printf("/nConnect error!/n");
  return -1;
 }
 ZeroMemory(data,1024);
 ret=receive(host->HostSocketCmd,data,1024);
 if(strncmp(data,"331",3)==0)
 {
  printf("/nConnect success,need PassWord:");
 }
 else
 {
  printf("/nConnect error!/n");
  return -1;
 }
 _scanf(host->PassWord);
 ZeroMemory(command,1024);
 strcat(command,"PASS ");
 strcat(command,host->PassWord);
 strcat(command,"/r/n/0");
 ret=post(host->HostSocketCmd,command,strlen(command));
 if(ret!=1)
 {
  printf("/nConnect error!/n");
  return -1;
 }
 ret=receive(host->HostSocketCmd,data,1024);
 if(strncmp(data,"230",3)==0)
 {
  printf("/nLogin success!");
  host->HostIp=hostaddr.sin_addr.s_addr;
  ZeroMemory(command,1024);
  strcat(command,"REST 100/r/n/0");
  ret=post(host->HostSocketCmd,command,strlen(command));
  if(ret!=1)
  {
   printf("/nlose connection./n");
   return -1;
  }
  ZeroMemory(data,1024);
  ret=receive(host->HostSocketCmd,data,1024);
  if(ret!=1)
  {
   printf("/nlose connection./n");
   return -1;
  }
  if(strstr(data,"350 Restarting at 100")!=0)
  {
   host->npsupport=1;
   printf("/n站点支持断点续传/n");
  }
  else
  {
   host->npsupport=0;
   printf("/n站点不支持断点续传/n");
  }
  ZeroMemory(command,1024);
  strcat(command,"REST 0/r/n/0");
  ret=post(host->HostSocketCmd,command,strlen(command));
  if(ret!=1)
  {
   printf("/nlose connection./n");
   return -1;
  }
  ZeroMemory(data,1024);
  ret=receive(host->HostSocketCmd,data,1024);
  strcpy(command,"PWD");
  strcat(command,"/r/n");
  ret=post(host->HostSocketCmd,command,strlen(command));
  if(ret!=1)
  {
   printf("/nConnect error!/n");
   return -1;
  }
  ret=receive(host->HostSocketCmd,data,128);
  if(ret!=1)
  {
   printf("/nConnect error!/n");
   return -1;
  }
  else
  {
   if(strncmp(data,"257",3)==0)
   {
    int i=0;
    char *ptr1;
    ptr1=data+5;
    while(*(ptr1)!='/"')
    {
     *(host->CurrentDir+i)=*(ptr1);
     ptr1++;
     i++;
    }
    *(host->CurrentDir+i)='/0';
    printf("Current directory is: %s",host->CurrentDir);
   }
  }
  addToHostList(host);
  CurrentHost=host;
 }
 else
 {
  printf("/nConnect error!/n");
  return -1;
 }
 return 1;
}

DWORD WINAPI ListenProc(LPVOID lParam)
{
 SOCKET dataSock;
 SOCKET ss;
 SOCKET *s;
 int ret;
 s=lParam;
 dataSock=*(s);
 ret=listen(dataSock,1);
 if(ret!=0)
 {
  printf("/nfail to listen.../n");
  return -1;
 }
 ss=accept(dataSock,NULL,0);
 *(s)=ss;
 if(ss==INVALID_SOCKET)
 {
  printf("/nhost fail to connect.../n");
  return -1;
 }
 return 1; 
}

int ListDir()
{
 char command[1024]={0};
 char _data[128]={0};
 char *data;
 char numBuf[8]={0};
 char attrib[32]={0};
 char Size[32]={0};
 char param1[64]={0};
 char fileName[128]={0};
 char *ptr1,*ptr2;
 int  flag=0,ip=0,jp=0;
 SOCKET dataSock;
 struct sockaddr_in hostaddr;
 unsigned short i,j,port;
 int ret;
 data=(char *)malloc(sizeof(char)*128000);
 ZeroMemory(data,128000);
 ZeroMemory(command,1024);
 if(PasvOrPort==0)
 {
  strcat(command,"PASV /r/n/0");
  ret=post(CurrentHost->HostSocketCmd,command,strlen(command));
  if(ret!=1)
  {
   printf("/nConnect error!/n");
   free(data);
   return -1;
  }
  ret=receive(CurrentHost->HostSocketCmd,data,1024);
  if(strncmp(data,"227",3)==0)
  {
   printf("%s/n",data);
  }
  else
  {
   free(data);
   return -1;
  }
  ptr1=data+strlen(data);
  while(*(ptr1)!=')')
   ptr1--;
  ptr2=ptr1;
  while(*(ptr2)!=',')
   ptr2--;
  ZeroMemory(numBuf,8);
  strncpy(numBuf,ptr2+1,ptr1-ptr2-1);
  j=atoi(numBuf);
  ptr2--;
  ptr1=ptr2;
  ptr2++;
  while(*(ptr1)!=',')
   ptr1--;
  ZeroMemory(numBuf,8);
  strncpy(numBuf,ptr1+1,ptr2-ptr1-1);
  i=atoi(numBuf);
  port=i*256+j;
  dataSock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  hostaddr.sin_addr.s_addr=CurrentHost->HostIp;
  hostaddr.sin_family=AF_INET;
  hostaddr.sin_port=htons(port);
  ret=connect(dataSock,&hostaddr,sizeof hostaddr);
  if(ret!=0)
  {
   printf("/nConnect error!/n");
   free(data);
   return -1;
  }
 }
 else
 {
  dataSock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  hostaddr.sin_family=AF_INET;
  hostaddr.sin_addr.s_addr=INADDR_ANY;
  hostaddr.sin_port=0;
  ret=bind(dataSock,&hostaddr,sizeof(hostaddr));
  CreateThread(0,0,ListenProc,&dataSock,0,0);
  if(ret==SOCKET_ERROR)
  {
   free(data);
   printf("/nfail to bind a port for listenning.../n");
   return -1;
  }
  ip=sizeof(struct sockaddr);
  ret=getsockname(dataSock,(struct sockaddr *)&hostaddr,&ip);
  if(ret==SOCKET_ERROR)
  {
   free(data);
   printf("/nfail to decide the port number.../n");
   return -1;
  }
  ip=ntohs(hostaddr.sin_port)/256;
  jp=ntohs(hostaddr.sin_port)%256;
  strcat(command,"PORT ");
  ZeroMemory(param1,128);
  ret=sizeof(struct sockaddr_in);
  getsockname(CurrentHost->HostSocketCmd,&hostaddr,&ret);
  strcpy(_data,inet_ntoa(hostaddr.sin_addr));
  ptr1=_data;
  while(*(ptr1)!=0)
  {
   if(*(ptr1)=='.')
   {
    *(ptr1)=',';
   }
   ptr1++;
  }
  strcat(command,_data);
  strcat(command,",");
  strcat(command,itoa(ip,numBuf,10));
  strcat(command,",");
  strcat(command,itoa(jp,numBuf,10));
  strcat(command,"/r/n/0/0");
  ret=post(CurrentHost->HostSocketCmd,command,strlen(command));
  if(ret!=1)
  {
   printf("/nConnect error!/n");
   free(data);
   return -1;
  }
  ret=receive(CurrentHost->HostSocketCmd,data,1024);
  if(strncmp(data,"200",3)==0)
  {
   printf("%s/n",data);
  }
  else
  {
   free(data);
   return -1;
  }
 }
 ZeroMemory(command,1024);
 strcat(command,"LIST");
 strcat(command,"/r/n/0");
 ret=post(CurrentHost->HostSocketCmd,command,strlen(command));
 if(ret!=1)
 {
  printf("/nConnect error!/n");
  free(data);
  return -1;
 }
receiveData:
 Sleep(20);
 ZeroMemory(data,128000);
 ret=_receive(dataSock,data,128000);
 if(ret==0)
 {
  closesocket(dataSock);
  goto ending;
 }
 printf("%s",data);
 if(flag==0)
 {
  ret=_receive(CurrentHost->HostSocketCmd,_data,128);
  if(strstr(_data,"226")!=NULL)
  {
   flag=1;
   goto receiveData;
  }
 }
 goto receiveData;
ending:
 if(flag!=1)
  ret=_receive(CurrentHost->HostSocketCmd,_data,128);
 free(data);
 return 1;
}

int changeDirectory(char *directory)
{
 char command[128]="CWD ";
 char data[128]="/0";
 int ret;
 strcat(command,directory);
 strcat(command,"/r/n");
 ret=post(CurrentHost->HostSocketCmd,command,strlen(command));
 if(ret!=1)
 {
  printf("/nConnect error!/n");
  return -1;
 }
 ret=receive(CurrentHost->HostSocketCmd,data,128);
 if(ret!=1)
 {
  printf("/nConnect error!/n");
  return -1;
 }
 printf("%s/n",data);
 if(strncmp(data,"250",3)==0)
 {
  strcpy(command,"PWD /r/n/0");
  ret=post(CurrentHost->HostSocketCmd,command,strlen(command));
  if(ret==-1)
  {
   printf("/nConnect error!/n");
   return -1;
  }
  ret=receive(CurrentHost->HostSocketCmd,data,128);
  if(ret!=1)
  {
   printf("/nConnect error!/n");
   return -1;
  }
  else
  {
   if(strncmp(data,"257",3)==0)
   {
    int i=0;
    char *ptr1;
    ptr1=data+5;
    while(*(ptr1)!='/"')
    {
     *(CurrentHost->CurrentDir+i)=*(ptr1);
     ptr1++;
     i++;
    }
    *(CurrentHost->CurrentDir+i)='/0';
    printf("Current directory is: %s",CurrentHost->CurrentDir);
   }
  }
 }
 return 1;
}

int showCurrentDirectory()
{
 char command[128]="PWD";
 char data[128]="/0";
 int ret;
 strcat(command,"/r/n");
 ret=post(CurrentHost->HostSocketCmd,command,strlen(command));
 if(ret!=1)
 {
  printf("/nConnect error!/n");
  return -1;
 }
 ret=receive(CurrentHost->HostSocketCmd,data,128);
 if(ret!=1)
 {
  printf("/nConnect error!/n");
  return -1;
 }
 printf("%s/n",data);
 return 1;
}

DWORD WINAPI downLoadProcA(LPVOID lpParam)
{
 RESUME *r;
 int ii;
 unsigned short i,j,port;
 char *ptr1,*ptr2;
 char command[64]={0};
 char already[64]={0};
 char _data[128]={0};
 char param1[128]={0};
 char *data;
// start pasv  
 char numBuf[8]={0};
 HANDLE hFile;
 HANDLE hTmpFile;
 HANDLE hListenThread;
 SOCKET dataSock;
 struct sockaddr_in hostaddr;
 int ret,_ret,ip,jp;
 int flag=0;
 r=lpParam;
 r->state=1;
 data=VirtualAlloc(NULL,1024*1024,MEM_COMMIT,PAGE_READWRITE);
 if(data==NULL){VirtualFree(data,1024*1024,MEM_DECOMMIT);r->state=0; return-1;}
 ZeroMemory(data,1024*1024);
// set pointer
 ZeroMemory(command,64);
 strcat(command,"REST ");
 ii=r->alreadydone;
 _itoa(ii,already,10);
 strcat(command,already);
 strcat(command,"/r/n/0/0");
 ret=post(r->sockcmd,command,strlen(command));
 if(ret!=1)
 {
  return -1;
 }
 ZeroMemory(data,1024*1024);
 ret=receive(r->sockcmd,data,1024*1024);
 if(ret!=1)
 {
  return -1;
 }
 if(strstr(data,"350 Restarting")==0 && strncmp(data,"501",3)!=0)
  return -1;
// set pasv
 if(PasvOrPort==0)
 {
  ZeroMemory(command,64);
  strcat(command,"PASV /r/n/0");
  ret=post(r->sockcmd,command,64);
  if(ret!=1)
  {
   {VirtualFree(data,1024*1024,MEM_DECOMMIT);r->state=0; return-1;}
  }
  ZeroMemory(data,1024*1024);
  ret=receive(r->sockcmd,data,1024*1024);
  if(strncmp(data,"227",3)!=0)
   {VirtualFree(data,1024*1024,MEM_DECOMMIT);r->state=0; return-1;}
  ptr1=data+strlen(data);
  while(*(ptr1)!=')')
   ptr1--;
  ptr2=ptr1;
  while(*(ptr2)!=',')
   ptr2--;
  strncpy(numBuf,ptr2+1,ptr1-ptr2-1);
  j=atoi(numBuf);
  ptr2--;
  ptr1=ptr2;
  ptr2++;
  while(*(ptr1)!=',')
   ptr1--;
  strncpy(numBuf,ptr1+1,ptr2-ptr1-1);
  i=atoi(numBuf);
  port=i*256+j;
  dataSock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  hostaddr.sin_addr.s_addr=r->hostip;
  hostaddr.sin_family=AF_INET;
  hostaddr.sin_port=htons(port);
  ret=connect(dataSock,&hostaddr,sizeof hostaddr);
  if(ret!=0)
   {VirtualFree(data,1024*1024,MEM_DECOMMIT); r->state =0;return-1;}
 }
 else
 {
  r->sockdata=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  hostaddr.sin_family=AF_INET;
  hostaddr.sin_addr.s_addr=INADDR_ANY;
  hostaddr.sin_port=0;
  ret=bind(r->sockdata,&hostaddr,sizeof(hostaddr));
  dataSock=r->sockdata;
  hListenThread=CreateThread(0,0,ListenProc,&(r->sockdata),0,0);
  if(ret==SOCKET_ERROR)
  {
   free(data);
   return -1;
  }
  ip=sizeof(struct sockaddr);
  ret=getsockname(r->sockdata,(struct sockaddr *)&hostaddr,&ip);
  if(ret==SOCKET_ERROR)
  {
   free(data);
   return -1;
  }
  ip=ntohs(hostaddr.sin_port)/256;
  jp=ntohs(hostaddr.sin_port)%256;
  ZeroMemory(command,64);
  strcat(command,"PORT ");
  ZeroMemory(param1,128);
  ret=sizeof(struct sockaddr_in);
  getsockname(r->sockcmd,&hostaddr,&ret);
  strcpy(_data,inet_ntoa(hostaddr.sin_addr));
  ptr1=_data;
  while(*(ptr1)!=0)
  {
   if(*(ptr1)=='.')
   {
    *(ptr1)=',';
   }
   ptr1++;
  }
  strcat(command,_data);
  strcat(command,",");
  strcat(command,itoa(ip,numBuf,10));
  strcat(command,",");
  strcat(command,itoa(jp,numBuf,10));
  strcat(command,"/r/n/0/0");
  ret=post(r->sockcmd,command,strlen(command));
  if(ret!=1)
  {
   free(data);
   return -1;
  }
  ZeroMemory(data,1024*1024);
  ret=receive(r->sockcmd,data,1024*1024);
  if(strncmp(data,"200",3)!=0)
  {
   free(data);
   return -1;
  }
 }
 hFile=CreateFile(r->dest,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
 hTmpFile=CreateFile(r->tmp,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
 r->hTmpFile=hTmpFile;
 if(hFile==INVALID_HANDLE_VALUE || hTmpFile==INVALID_HANDLE_VALUE){VirtualFree(data,1024*1024,MEM_DECOMMIT); return-1;}
 SetFilePointer(hTmpFile,128+128+128+128+128+128,0,FILE_BEGIN);
 SetFilePointer(hFile,r->alreadydone,0,FILE_BEGIN);
 ZeroMemory(command,64);
 strcat(command,"RETR ");
 strcat(command,r->file);
 strcat(command,"/r/n/0");
 ret=post(r->sockcmd,command,strlen(command));
 if(ret!=1)
 {
  CloseHandle(hFile);
  CloseHandle(hTmpFile);
  DeleteFile(r->tmp);
  {VirtualFree(data,1024*1024,MEM_DECOMMIT); r->state=0;return -1;}
 }
 ZeroMemory(data,1024*1024);
 ret=_receive(r->sockcmd,data,1024*1024);
 if(PasvOrPort==1)
 {
waiting:
  if(dataSock==r->sockdata)
  {
   Sleep(20);
   goto waiting;
  }
  dataSock=r->sockdata;
 }
// get data
getdata:
 ZeroMemory(data,1024*1024);
 ret=_receive(dataSock,data,1024*1024);
 if(ret<0){VirtualFree(data,1024*1024,MEM_DECOMMIT); r->state=0;return -1;}
// refresh data 
 WriteFile(hFile,data,ret,&_ret,0);
 FlushFileBuffers(hFile);
 r->alreadydone+=ret;
 WriteFile(hTmpFile,((char *)r+128*6),sizeof(RESUME)-128*6,&_ret,0);
 FlushFileBuffers(hTmpFile);
 SetFilePointer(hTmpFile,128+128+128+128+128+128,0,FILE_BEGIN);
 if(flag==0)
 {
  ret=_receive(r->sockcmd,data,512);
  flag=1;
  goto getdata;
 }
 if(flag==1)
 {
  if(r->size==r->alreadydone)
   goto ending;
  else
   goto getdata;
 }
ending:
 CloseHandle(hFile);
 CloseHandle(hTmpFile);
 closesocket(dataSock);
 VirtualFree(data,1024*1024,MEM_DECOMMIT);
 DeleteFile(r->tmp);
 r->state=0;
 return 1;
}

int addtoDownLoadList(RESUME *r)
{
 r->next=ResumeFile.next;
 ResumeFile.next=r;
 return 1;
}

int downLoad(char *file,char *directory)
{
// login
 HANDLE hFile;
 SOCKET cmdSocket;
 struct sockaddr_in hostaddr;
 Host *host;
 RESUME *r;
 int ret;
 unsigned short port;
 char **ptr=NULL,*_file=NULL;
 int *addr=NULL;
 struct hostent *iphost=NULL;
 char data[1024]={0};
 char command[1024]={0};
 char incomName[80]={0};
 host=CurrentHost;
 _file=file+strlen(file)-1;
 while(_file>=file && *(_file)!='/')
 {
  _file--;
 }
 if(_file<file)_file=file;
 else
  if(*(_file)=='/')
   _file++;
 iphost=gethostbyname(host->HostName);
 r=(RESUME *)malloc(sizeof(RESUME));
 ZeroMemory(r,sizeof(RESUME));
 if(iphost==NULL)
 {
  printf("/nConnect error!/n");
  free(r);return-1;
 }
 port=atoi(host->Hostport);
 if(port==0 || port>65535) 
  port=21;
  ptr=iphost->h_addr_list;
 addr=(int *)(*(ptr));
 hostaddr.sin_family=AF_INET;
 hostaddr.sin_port=(unsigned short)htons(port);
 hostaddr.sin_addr.s_addr=*(addr);
 cmdSocket=socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
 ret=connect(cmdSocket,&hostaddr,sizeof hostaddr);
  if(ret!=0)
 {
  printf("/nConnect error!/n");
  free(r);return-1;
 }
 ret=receive(cmdSocket,data,1024);
 if(ret!=1)
 {
  printf("/nConnect error!/n");
  free(r);return-1;
 }
 if(strncmp(data,"220",3)==0)
 {
  printf("/nConnect success,send UserName");
 }
 else
 {
  printf("/nConnect error!/n");
  free(r);return-1;
 }
 ZeroMemory(command,1024);
 strcat(command,"USER ");
 strcat(command,host->UserName);
 strcat(command,"/r/n/0");
 ret=post(cmdSocket,command,strlen(command));
 if(ret!=1)
 {
  printf("/nConnect error!/n");
  free(r);return-1;
 }
 ZeroMemory(data,1024);
 ret=receive(cmdSocket,data,1024);
 if(strncmp(data,"331",3)==0)
 {
  printf("/nConnect success,send PassWord");
 }
 else
 {
  printf("/nConnect error!/n");
  free(r);return-1;
 }
 ZeroMemory(command,1024);
 strcat(command,"PASS ");
 strcat(command,host->PassWord);
 strcat(command,"/r/n/0");
 ret=post(cmdSocket,command,strlen(command));
 if(ret!=1)
 {
  printf("/nConnect error!/n");
  free(r);return-1;
 }
 ret=receive(cmdSocket,data,1024);
 if(strncmp(data,"230",3)==0)
 {
  printf("/nLogin success,prepare to download/n");
 }
 else
 {
  printf("/nConnect error!download aborted/n");
  free(r);return-1;
 }
 if(strncmp(file,"/",1)!=0)
 {
  strcpy(r->file,CurrentHost->CurrentDir);
  if(*(CurrentHost->CurrentDir+strlen(CurrentHost->CurrentDir)-1)!='/')
   strcat(r->file,"/");
  strcat(r->file,_file);
 }
 else
 {
  strcat(r->file,file);
 }
// get file size
 ZeroMemory(command,1024);
 strcat(command,"SIZE ");
 strcat(command,r->file);
 strcat(command,"/r/n/0");
 ret=post(cmdSocket,command,strlen(command));
 if(ret!=1)
 {
  printf("/nConnect error!/n");
  free(r);return-1;
 }
 ZeroMemory(data,1024);
 ret=receive(cmdSocket,data,1024);
 if(ret!=1)
 {
  printf("/nConnect error!/n");
  free(r);return-1;
 }
 if(strncmp(data,"213",3)!=0)
 {
  printf("/nCan't find the file/n");
  free(r);return-1;
 }
 r->size=atoi(data+4);
 r->alreadydone=0;
 r->sockcmd=cmdSocket;
 r->hostPort=(unsigned short)htons((unsigned short)atoi(CurrentHost->Hostport));
 strcpy(r->dest,directory);
 strcat(r->dest,"//");
 strcat(r->dest,_file);
 r->next=NULL;
// fill profile and create profile and realfile
 strcpy(incomName,"incompletes//");
 strcat(incomName,_file);
 strcat(incomName,".tmp");
 strcpy(r->tmp,incomName);
 hFile=CreateFile(incomName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_DELETE,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
 if(hFile==INVALID_HANDLE_VALUE)
 {
  printf("cannot create profile,download fail.");
  free(r);return-1;
 }
 ret=0;
 r->hostip=hostaddr.sin_addr.s_addr;
 strcpy(r->hostSite,CurrentHost->HostName);
 strcpy(r->PassWord,CurrentHost->PassWord);
 strcpy(r->UserName,CurrentHost->UserName);
 WriteFile(hFile,r,sizeof(RESUME),&ret,0);
 if(!ret)
 {
  printf("/ncannot write profile,download fail/n");
  free(r);return-1;
 }
 CloseHandle(hFile);
 hFile=CreateFile(r->dest,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_DELETE,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
 if(hFile==INVALID_HANDLE_VALUE)
 {
  printf("cannot create file,download fail.");
  free(r);return-1;
 }
 CloseHandle(hFile);
 r->state=0;
 addtoDownLoadList(r);
// start download
 r->hThread=CreateThread(0,0,downLoadProcA,r,0,0);
 return 1; 
}

int resumeDownLoad(RESUME *profile)
{
// login
 SOCKET cmdSocket;
 struct sockaddr_in hostaddr;
 int ret;
 unsigned short port;
 char **ptr=NULL;
 int *addr=NULL;
 struct hostent *iphost=NULL;
 char data[1024]={0};
 char command[1024]={0};
 char incomName[80]={0};
 iphost=gethostbyname(profile->hostSite);
 if(iphost==NULL)
 {
  printf("/nConnect error!/n");
  return -1;
 }
 port=profile->hostPort;
 if(port==0 || port>65535) 
  port=21;
  ptr=iphost->h_addr_list;
 addr=(int *)(*(ptr));
 hostaddr.sin_family=AF_INET;
 hostaddr.sin_port=(unsigned short)port;
 hostaddr.sin_addr.s_addr=*(addr);
 cmdSocket=socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
 ret=connect(cmdSocket,&hostaddr,sizeof hostaddr);
 if(ret!=0)
 {
  printf("/nConnect error!/n");
  return -1;
 }
 ret=receive(cmdSocket,data,1024);
 if(ret!=1)
 {
  printf("/nConnect error!/n");
  return -1;
 }
 if(strncmp(data,"220",3)==0)
 {
  printf("/nConnect success,send UserName");
 }
 else
 {
  printf("/nConnect error!/n");
  return -1;
 }
 ZeroMemory(command,1024);
 strcat(command,"USER ");
 strcat(command,profile->UserName);
 strcat(command,"/r/n/0");
 ret=post(cmdSocket,command,strlen(command));
 if(ret!=1)
 {
  printf("/nConnect error!/n");
  return -1;
 }
 ZeroMemory(data,1024);
 ret=receive(cmdSocket,data,1024);
 if(strncmp(data,"331",3)==0)
 {
  printf("/nConnect success,send PassWord");
 }
 else
 {
  printf("/nConnect error!/n");
  return -1;
 }
 ZeroMemory(command,1024);
 strcat(command,"PASS ");
 strcat(command,profile->PassWord);
 strcat(command,"/r/n/0");
 ret=post(cmdSocket,command,strlen(command));
 if(ret!=1)
 {
  printf("/nConnect error!/n");
  return -1;
 }
 ret=receive(cmdSocket,data,1024);
 if(strncmp(data,"230",3)==0)
 {
  printf("/nLogin success,prepare to download");
 }
 else
 {
  printf("/nConnect error!download aborted/n");
  return -1;
 }
// start download 
 profile->state=0;
 profile->sockcmd=cmdSocket;
// start download
 profile->hThread=CreateThread(0,0,downLoadProcA,profile,0,0);
 return 1;
}

int resumeList()
{
 RESUME *r;
 int ret;
 HANDLE hFind,hFile;
 WIN32_FIND_DATA wfd;
 char name[128];
 hFind=FindFirstFile("incompletes//*.tmp",&wfd);
 if(hFind==INVALID_HANDLE_VALUE)return 0;
 r=(RESUME *)malloc(sizeof(RESUME));
 strcpy(name,"incompletes//");
 strcat(name,wfd.cFileName);
 hFile=CreateFile(name,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
 if(hFile!=INVALID_HANDLE_VALUE)
 {
  ReadFile(hFile,r,sizeof(RESUME),&ret,0);
  CloseHandle(hFile);
  r->state=0;
  addtoDownLoadList(r);
 }
 else
 {
  free(r);
 }
 while(FindNextFile(hFind,&wfd)!=0)
 {
  r=(RESUME *)malloc(sizeof(RESUME));
  strcpy(name,"incompletes//");
  strcat(name,wfd.cFileName);
  hFile=CreateFile(name,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
  if(hFile!=INVALID_HANDLE_VALUE)
  {
   ReadFile(hFile,r,sizeof(RESUME),&ret,0);
   CloseHandle(hFile);
   r->state=0;
   addtoDownLoadList(r);
  }
  else
  {
   free(r);
  }  
 }
 FindClose(hFind);
 return 1;
}
int addtoUpLoadList(UPLOAD *r)
{
 r->next=Up.next;
 Up.next=r;
 return 1;
}

DWORD WINAPI upLoadProcA(LPVOID lpParam)
{
 UPLOAD *r;
 HANDLE hListenThread;
 unsigned short i,j,port;
 char *ptr1,*ptr2;
 char command[64]={0};
 char param1[128]={0};
 char already[64]={0};
 char _data[512]={0};
 char *data;
// start pasv  
 char numBuf[8]={0};
 HANDLE hFile;
 SOCKET dataSock;
 struct sockaddr_in hostaddr;
 int ret,ip,jp;
 int flag=0;
 r=lpParam;
 data=VirtualAlloc(NULL,r->size,MEM_COMMIT,PAGE_READWRITE);
 if(data==NULL){VirtualFree(data,1024*1024,MEM_DECOMMIT); return-1;}
 ZeroMemory(data,r->size);
// set pasv
 if(PasvOrPort==0)
 {
  ZeroMemory(command,64);
  strcat(command,"PASV /r/n/0");
  ret=post(r->cmdSock,command,64);
  if(ret!=1)
   {VirtualFree(data,1024*1024,MEM_DECOMMIT); return-1;}
  ZeroMemory(_data,512);
  ret=receive(r->cmdSock,_data,512);
  if(strncmp(_data,"227",3)!=0)
   {VirtualFree(data,r->size,MEM_DECOMMIT); return-1;}
  ptr1=_data+strlen(_data);
  while(*(ptr1)!=')')
   ptr1--;
  ptr2=ptr1;
  while(*(ptr2)!=',')
   ptr2--;
  strncpy(numBuf,ptr2+1,ptr1-ptr2-1);
  j=atoi(numBuf);
  ptr2--;
  ptr1=ptr2;
  ptr2++;
  while(*(ptr1)!=',')
   ptr1--;
  strncpy(numBuf,ptr1+1,ptr2-ptr1-1);
  i=atoi(numBuf);
  port=i*256+j;
  dataSock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  hostaddr.sin_addr.s_addr=CurrentHost->HostIp;
  hostaddr.sin_family=AF_INET;
  hostaddr.sin_port=htons(port);
  ret=connect(dataSock,&hostaddr,sizeof hostaddr);
  if(ret!=0)
  {
   {VirtualFree(data,r->size,MEM_DECOMMIT);return-1;}
  }
 }
 else
 {
  r->sockdata=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  hostaddr.sin_family=AF_INET;
  hostaddr.sin_addr.s_addr=INADDR_ANY;
  hostaddr.sin_port=0;
  ret=bind(r->sockdata,&hostaddr,sizeof(hostaddr));
  dataSock=r->sockdata;
  hListenThread=CreateThread(0,0,ListenProc,&(r->sockdata),0,0);
  if(ret==SOCKET_ERROR)
  {
   free(data);
   return -1;
  }
  ip=sizeof(struct sockaddr);
  ret=getsockname(r->sockdata,(struct sockaddr *)&hostaddr,&ip);
  if(ret==SOCKET_ERROR)
  {
   free(data);
   return -1;
  }
  ip=ntohs(hostaddr.sin_port)/256;
  jp=ntohs(hostaddr.sin_port)%256;
  ZeroMemory(command,64);
  strcat(command,"PORT ");
  ZeroMemory(param1,128);
  ret=sizeof(struct sockaddr_in);
  getsockname(r->cmdSock,&hostaddr,&ret);
  strcpy(_data,inet_ntoa(hostaddr.sin_addr));
  ptr1=_data;
  while(*(ptr1)!=0)
  {
   if(*(ptr1)=='.')
   {
    *(ptr1)=',';
   }
   ptr1++;
  }
  strcat(command,_data);
  strcat(command,",");
  strcat(command,itoa(ip,numBuf,10));
  strcat(command,",");
  strcat(command,itoa(jp,numBuf,10));
  strcat(command,"/r/n/0/0");
  ret=post(r->cmdSock,command,strlen(command));
  if(ret!=1)
  {
   free(data);
   return -1;
  }
  ZeroMemory(_data,512);
  ret=receive(r->cmdSock,_data,512);
  if(strncmp(_data,"200",3)!=0)
  {
   free(data);
   return -1;
  }
 }
 hFile=CreateFile(r->file,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
 GetLastError();
 if(hFile!=INVALID_HANDLE_VALUE)
 {
  ReadFile(hFile,data,r->size,&ret,0);
  FlushFileBuffers(hFile);
  CloseHandle(hFile);
 }
 else
 {
  return -1;
 }
 ZeroMemory(command,64);
 strcat(command,"STOR ");
 strcat(command,r->dest);
 strcat(command,"/r/n/0/0");
 ret=post(r->cmdSock,command,strlen(command));
 ret=receive(r->cmdSock,_data,512);
 if(strncmp(_data,"150 ",3)==0)
// upLoad!
 {
  if(PasvOrPort==1)
  {
waiting:
   if(dataSock==r->sockdata)
   {
    Sleep(20);
    goto waiting;
   }
   dataSock=r->sockdata;
  }
  send(dataSock,data,r->size,0);
  closesocket(dataSock);
  ret=receive(r->cmdSock,_data,512);
  if(strncmp(_data,"226",3)==0)
   r->alreadydone=1;
  else
   r->alreadydone=-1;
  return 1;
 }
 else
 {
  r->alreadydone=-1;
  return 1;
 }
}

int upLoad(char *file,char *dest)
{
 HANDLE hFile;
 SOCKET cmdSocket;
 struct sockaddr_in hostaddr;
 Host *host;
 UPLOAD *r;
 int ret;
 unsigned short port;
 char **ptr=NULL,*_file=NULL;
 int *addr=NULL;
 struct hostent *iphost=NULL;
 char data[1024]={0};
 char command[1024]={0};
 char incomName[80]={0};
 host=CurrentHost;
 iphost=gethostbyname(host->HostName);
 r=(UPLOAD *)malloc(sizeof(UPLOAD));
 ZeroMemory(r,sizeof(UPLOAD));
 if(iphost==NULL)
 {
  printf("/nConnect error!/n");
  free(r);return-1;
 }
 port=atoi(host->Hostport);
 if(port==0 || port>65535) 
  port=21;
  ptr=iphost->h_addr_list;
 addr=(int *)(*(ptr));
 hostaddr.sin_family=AF_INET;
 hostaddr.sin_port=(unsigned short)htons(port);
 hostaddr.sin_addr.s_addr=*(addr);
 cmdSocket=socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
 ret=connect(cmdSocket,&hostaddr,sizeof hostaddr);
  if(ret!=0)
 {
  printf("/nConnect error!/n");
  free(r);return-1;
 }
 ret=receive(cmdSocket,data,1024);
 if(ret!=1)
 {
  printf("/nConnect error!/n");
  free(r);return-1;
 }
 if(strncmp(data,"220",3)==0)
 {
  printf("/nConnect success,send UserName");
 }
 else
 {
  printf("/nConnect error!/n");
  free(r);return-1;
 }
 ZeroMemory(command,1024);
 strcat(command,"USER ");
 strcat(command,host->UserName);
 strcat(command,"/r/n/0");
 ret=post(cmdSocket,command,strlen(command));
 if(ret!=1)
 {
  printf("/nConnect error!/n");
  free(r);return-1;
 }
 ZeroMemory(data,1024);
 ret=receive(cmdSocket,data,1024);
 if(strncmp(data,"331",3)==0)
 {
  printf("/nConnect success,send PassWord");
 }
 else
 {
  printf("/nConnect error!/n");
  free(r);return-1;
 }
 ZeroMemory(command,1024);
 strcat(command,"PASS ");
 strcat(command,host->PassWord);
 strcat(command,"/r/n/0");
 ret=post(cmdSocket,command,strlen(command));
 if(ret!=1)
 {
  printf("/nConnect error!/n");
  free(r);return-1;
 }
 ret=receive(cmdSocket,data,1024);
 if(strncmp(data,"230",3)==0)
 {
  printf("/nLogin success,prepare to upload");
 }
 else
 {
  printf("/nConnect error!upload aborted/n");
  free(r);return-1;
 }
 r->cmdSock=cmdSocket;
 strcpy(r->dest,dest);
 strcpy(r->file,file);
 r->next=NULL;
 hFile=CreateFile(file,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
 if(hFile==INVALID_HANDLE_VALUE)
 {
  printf("/ncannot open the file for upload!/n");
  closesocket(cmdSocket);
  free(r);
  return -1;
 }
 ret=GetFileSize(hFile,0);
 CloseHandle(hFile);
 r->size=ret;
 r->alreadydone=0;
 strcpy(r->hostSite,CurrentHost->HostName);
 addtoUpLoadList(r);
// start download
 r->hThread=CreateThread(0,0,upLoadProcA,r,0,0);
 return 1; 
}

int DeleteDown(RESUME *r)
{
 RESUME *p;
 p=&ResumeFile;
 while(p->next!=r && p)
 {
  p=p->next;
 }
 if(p->next==r)
 {
  p->next=r->next;
  free(r);
 }
 return 1;
}

int DeleteHost(Host *h)
{
 Host *p;
 p=&HostList;
 while(p && p->next!=h)
 {
  p=p->next;
 }
 if(p->next==h)
 {
  p->next=h->next;
  free(h);
 }
 return 1;
}

int DetectCurrentHostHost()
{
 char command[128]="PWD";
 char data[128]="/0";
 int ret;
 strcat(command,"/r/n");
 ret=post(CurrentHost->HostSocketCmd,command,strlen(command));
 if(ret!=1)
 {
  return -1;
 }
 ret=receive(CurrentHost->HostSocketCmd,data,128);
 if(ret!=1 || strncmp(data,"257",3)!=0)
 {
  return -1;
 }
 return 1;
}
