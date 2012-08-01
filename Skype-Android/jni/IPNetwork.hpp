#ifndef __network_hpp__
#define __network_hpp__

#include "stdincludes.hpp"

#ifndef COUNTOF
#define COUNTOF(c) ((int)(sizeof(c)/sizeof(c[0])))
#endif

//#ifdef ANDROID
//#include <android/log.h>
//#define printf(fmt,args...)  __android_log_print(ANDROID_LOG_INFO, "Skype IP", fmt, ##args)
//#endif
#define zprintf printf

unsigned char * HexDump(void *p,int n);

class IPADDR {
private:
  unsigned int addr;
  unsigned short int port;
  char nbuf[3+1+3+1+3+1+3+1+5+1];
  
public:
  IPADDR() {
    addr=0;
    port=0;
  }
  
  IPADDR(struct sockaddr& s) {
    SetAddr(s);
  }

  void SetAddr(struct sockaddr& s) {
    SetAddr((struct sockaddr_in&)s);
  }
  
  void SetAddr(struct sockaddr_in& sa) {
    addr=ntohl(sa.sin_addr.s_addr);
    port=ntohs(sa.sin_port);
    printf("IP address set to %s\n",(char *)*this);
  }

  void SetAddr(struct in_addr& a) {
    addr=ntohl(a.s_addr);
  }
  
  bool SetAddr(char *s) {
    unsigned int u=0;
    printf("Setting ip address from %s\n",s);
    for (int i=3;i>=0;i--)
    {
      u=u|((strtoul(s,&s,10)&255)<<(i*8));
      if (*s!='.' && *s!=':' && *s!='\0')
        return false;
      if (*s==':') {
        s++;
        if (!*s)
          return false;
        addr=u;
        port=atoi(s)&0xffff;
        return true;
      }
      s++;
    }
    addr=u;
    return true;
  }
  
  void SetAddr(unsigned long a) {
    addr=a;
    printf("IP address set to %s\n",(char *)*this);
  }
  
  void SetPort(int p) {
    port=p;
    printf("IP address set to %s\n",(char *)*this);
  }

  void Clear() {
    addr=0;
    port=0;
  }
  
  unsigned int GetAddr() {
    return addr;
  }
  
  unsigned int GetPort() {
    return port;
  }
  
  void GetAddr(struct sockaddr_in& a) {
    a.sin_family=AF_INET;            
    a.sin_addr.s_addr=htonl(addr);
    a.sin_port=htons(port);
  }
  
  void GetAddr(struct sockaddr& a) {
    GetAddr((struct sockaddr_in&)a);
  }

  operator char *() {
    sprintf(nbuf,"%d.%d.%d.%d:%u",(addr>>24),(addr>>16)&255,(addr>>8)&255,addr&255,port&0xffff);
    return nbuf;
  }  
  
  operator unsigned int() {
    return addr;
  }
  
  bool operator =(unsigned int a) {
    addr=a;
    return true;
  }
  
};

class Network
{
public:
  static int GetProtocolNumber(char *protoname) {
    struct protoent *ptrp;
    //printf("Resolving protocol '%s'\n",protoname);
    ptrp=getprotobyname(protoname); 
    if(ptrp==NULL)
      return -1;
    return (int)ntohs(ptrp->p_proto);
  }
    
  static int GetServiceNumber(char *servicename) {
    struct servent *sptr;
    //printf("Resolving service '%s'\n",servicename);
    sptr=getservbyname(servicename,"tcp");
    if (sptr==NULL) {
      //printf("Could not resolve service '%s' (%s)\n",servicename,strerror(errno));
      return -1; 
    }
    //printf(" resolved to port %d\n",ntohs(sptr->s_port));
    return (int)ntohs(sptr->s_port);  
  }

  static bool ResolveName(char *hostname,IPADDR& addr) {
    struct hostent *ptrh;
    struct sockaddr_in sad;
    printf("Resolving host '%s'\n",hostname);
    memset((char *)&sad,0,sizeof(sad));
    sad.sin_family=AF_INET;            
    sad.sin_addr.s_addr=inet_addr(hostname);
    if (sad.sin_addr.s_addr==INADDR_NONE) {
      //printf(" was not numeric IP, trying DNS\n");
      ptrh=gethostbyname(hostname);
      if(ptrh==NULL)
      {
        //printf("Could not resolve host '%s' (%s)\n",hostname,strerror(errno));
        return false;
      }
      addr.SetAddr((struct in_addr&)*ptrh->h_addr);
    }
    else
      addr.SetAddr(sad);
    printf("Resolved to %s\n",(char *)addr);
    return true;
  }

  // http_URL       = "http:" "//" host [ ":" port ] [ abs_path ]
  static char *ParseURL(char *url,char **protocol,char **host,char **port,char **path) {
    char *s,*t;
    //printf("Parsing url '%s'\n",url);
    if (protocol)
      *protocol=NULL;
    if (host)
      *host=NULL;
    if (port)
      *port=NULL;
    if (path)
      *path=NULL;
    // check if protocol is specified
    s=strstr(url,"://");
    if (s)
    {
      s[1]='\0';
      //printf(" protocol='%s'\n",url);
      if (protocol)
        *protocol=url;
      url=s+3;
    }
    // see if there is port
    s=strchr(url,':');
    if (s) {
      *s='\0';
      s++;
      if (port)
        *port=s;
    }
    else
      s=url;
    t=strchr(s,'/');
    if (t)
      *t='\0';
    if (*port) {
      //printf(" port='%s'\n",*port);
    }
    if (host)
      *host=url;
    //printf(" host='%s'\n",url);
    s=strchr(s,'\0')+1;
    t=strchr(s,'\0')-1;
    while (t>=s && isspace(*t))
      *t--='\0';
    //printf(" path='%s'\n",s);
    if (path)
      *path=s;
    return url;
  }
  
};

class Socket
{
private:
  int sd;

public:
  IPADDR localaddr,remoteaddr;

  Socket() {
    sd=-1;
  }

  Socket(int type,int protocol) {
    sd=-1;
    Create(type,protocol);
  }

  ~Socket() {
    Close();
  }  

  bool SetOpt(int opt,int value) {
    if (setsockopt(sd,SOL_SOCKET,opt,&value,sizeof value)==0)
      return true;
    //printf("Could not set socket option - %s\n",strerror(errno));
    return false;
  }

  bool SetOpt(int opt,void *value,int len) {
    if (setsockopt(sd,SOL_SOCKET,opt,value,len)==0)
      return true;
    //printf("Could not set socket option - %s\n",strerror(errno));
    return false;
  }
  
  bool GetOpt(int opt,int& v) {
    socklen_t len = sizeof(v);
    return (getsockopt(sd,SOL_SOCKET,opt,&v,&len)==0);
  }
  
  bool Create(int type=SOCK_STREAM,int protocol=IPPROTO_TCP) {
    if (sd!=-1)
      close(sd);
    sd=socket(PF_INET,type,protocol);
    if (sd!=-1) {
      //printf("Created socket #%d, type %d proto %d\n",sd,type,protocol);
    }
    else {
      //printf("Cannot create socket type %d proto %d - %s\n",type,protocol,strerror(errno));
    }
    return sd!=-1;
  }
  
  void Close() {
    if (sd!=-1) {
      //printf("Closing socket #%d\n",sd);
      close(sd);
    }
    localaddr.Clear();
    remoteaddr.Clear();
    sd=-1;
  }

  void SetAddr(IPADDR& addr) {
    //printf("Setting socket #%d addr to %s\n",sd,(char *)addr);
    localaddr=addr;
  }
  
  void SetSd(int fd) {
    if (sd!=-1)
      close(sd);
    sd=fd;
  }

  int GetSd() {
    return sd;
  }
  
  IPADDR GetName() {
    struct sockaddr name;
    IPADDR a;
    socklen_t len=sizeof(name);
    if (!getsockname(sd,&name,&len))
      a.SetAddr(name);
    //printf("Returning socket #%d name - %s\n",sd,(char *)a);
    return a;
  }

  bool Bind(IPADDR& addr) {
    struct sockaddr a;
    addr.GetAddr(a);
    localaddr=addr;
    if (bind(sd,&a,sizeof a)==0) {
      printf("Socket #%d bound to %s\n",sd,(char *)addr);
      return true;
    }
    printf("Cannot bind socket #%d to %s (%s)\n",sd,(char *)addr,strerror(errno));
    return false;
  }
  
  bool Accept(Socket& s) {
    struct sockaddr a;
    int i;
    socklen_t len=sizeof(a);
    i=TEMP_FAILURE_RETRY(accept(sd,&a,&len));
    if (i!=-1) {
      s.remoteaddr.SetAddr(a);
      s.SetSd(i);
      //printf("Accepted connection to socket #%d from %s\n",sd,(char *)s.remoteaddr);
      return true;
    }
    //printf("Failed to accept connection on #%d - %s\n",sd,strerror(errno));
    return false;    
  }
  
  bool Listen(int backlog=4) {
    if (listen(sd,backlog)==0) {
      //printf("Listening on socket #%d\n",sd);
      return true;
    }
    //printf("Failed to listen on socket #%d - %s\n",sd,strerror(errno));
    return false;
  }
  
  bool Connect(IPADDR server)
  {
    remoteaddr=server;
    struct sockaddr a;
    server.GetAddr(a);
    if (TEMP_FAILURE_RETRY(connect(sd,&a,sizeof a))==0) {
      printf("Connected #%d to %s\n",sd,(char *)server);
      return true;
    }
    //printf("Failed to connect #%d to %s - %s\n",sd,(char *)server,strerror(errno));
    return false;
  }

  bool DataReady()
  {
#ifdef _SYMBIAN
	int wait = 200;

	fd_set rfds;
	timeval tv;
	int retval;

	FD_ZERO(&rfds);
	FD_SET(GetSd(), &rfds);

	tv.tv_sec = wait / 1000;
	tv.tv_usec = (wait % 1000) * 1000;

	retval = select(GetSd() + 1, &rfds, NULL, NULL, &tv);

	// select return value is buggy on symbian
	if(FD_ISSET(GetSd(), &rfds))
		return true;
	return false;
#else
    struct pollfd pfd;
    pfd.fd=sd;
    pfd.events=POLLIN;
    if (!poll(&pfd,1,100))
      return false; // only return false if nothing to return
    return true;	// errors are reported as data available, reading the socket later will get error
#endif
  }
    
};

class UDPConnection : public Socket
{
public:

  UDPConnection() {
    if (Create(SOCK_DGRAM,IPPROTO_UDP)) {
      SetOpt(SO_BROADCAST,1);
#ifdef SO_BSDCOMPAT
      SetOpt(SO_BSDCOMPAT,1);
#endif
    }
  }

  int SendTo(IPADDR dest,const void *buf,size_t len,int flags=0) {
    int i;
    struct sockaddr a;
    dest.GetAddr(a);
    //printf("Sending %d bytes to %s\n",len,(char *)dest);
    TEMP_FAILURE_RETRY(i=sendto(GetSd(),buf,len,flags,&a,sizeof a));
    if (i==-1)
      perror("SendTo");
    return i;
  }

  int Recv(void *buf,size_t len,IPADDR& fromaddr,int flags=0)
  {
    struct sockaddr a;
    int i;
    socklen_t l=sizeof(a);
    TEMP_FAILURE_RETRY(i=recvfrom(GetSd(),buf,len,flags,&a,&l));
    if (i!=-1) {
    //printf("Received %d bytes from %s\n",i,(char *)fromaddr);
      fromaddr.SetAddr(a);
    }
    else {
      perror("Recv");
    }
    return i;    
  }

};

class TCPConnection : public Socket
{
private:
  char *m_url,*m_protocol,*m_host,*m_path;

public:
  char *GetUrl() { return m_url; }
  char *GetHost() { return m_host; }
  char *GetPath() { return m_path; }
  char *GetProtocol() { return m_protocol; }

  TCPConnection() {
    //printf("Creating TCPConnection\n");
    m_url=NULL;
    m_protocol=NULL;
    m_host=NULL;
    m_path=NULL;
  }

  ~TCPConnection() {
    //printf("Destructing TCPConnection\n");
    Close();
  }
  
  void Close() {
    if (GetSd()!=-1) {
      //printf("Closing connection to %s\n",(char *)remoteaddr);
      Socket::Close();
    }
    if (m_url)
      free(m_url);
    m_url=NULL;
    m_protocol=NULL;
    m_host=NULL;
    m_path=NULL;
  };
  
  bool Open() {
    if (Socket::Create()) {
      if (Socket::Connect(remoteaddr)) {
        printf("TCPConnection Connected to %s\n",(char *)remoteaddr);
        return true;
      }
      else {
        printf("TCPConnection Socket::Connect() failed - %s\n",strerror(errno));
      }
    }
    else {
      printf("TCPConnection Socket::Create failed - %s\n",strerror(errno));
    }
    return false;
  }  
  
  bool Open(char *hostname,int portnumber)
  {
    Close();
    printf("Opening connection to '%s:%d'\n",hostname,portnumber);
    if (Network::ResolveName(hostname,remoteaddr)) {
      remoteaddr.SetPort(portnumber);
      return Open();
    }
    return false;
  };

  bool CreateListener(int portnumber,int queuesize)
  {
    IPADDR a;
    a.SetPort(portnumber);
    a.SetAddr(INADDR_ANY);
    Close();
    if (Socket::Create()) {
      Socket::SetOpt(SO_REUSEADDR,1);
      if (Socket::Bind(a)) {
        if (Socket::Listen(queuesize)) {
          //printf("Listening for TCP connections on port %d\n",portnumber);
          return true;
        }
        else {
          //printf("CreateListener() failed to listen (%s)\n",strerror(errno));
        }
      }
      else {
        //printf("CreateListener() failed to bind (%s)\n",strerror(errno));
      }
      Close();
    }
    else {
      //printf("CreateListener() failed to create socket (%s)\n",strerror(errno));
    }
    return false;
  }

  bool OpenUrl(char *openurl)
  {
    char *p="";
    printf("Opening URL '%s'\n",openurl);
    if (!openurl) {
      printf("NULL URL passed to OpenUrl()\n");
      return false;
    }
    while (*openurl && isspace(*openurl))
      openurl++;
    if (m_url)
      free(m_url);
    m_url=(char *)malloc(strlen(openurl)+1);
    if (m_url) {
      strcpy(m_url,openurl);
      Network::ParseURL(m_url,&m_protocol,&m_host,&p,&m_path);
      int port=atoi(p);
      if (!port)
        port=80;
      return Open(m_host,port);
    }
    return false;
  }

  int Read(void *buf,int bufsize,int flags=0)
  {
    int i;
    printf("reading up to %d bytes\n",bufsize);
    TEMP_FAILURE_RETRY(i=recv(GetSd(),buf,bufsize,flags));
    return i;
  }
  
  int Write(void *buf,int bufsize,int flags=0)
  {
    int i;
    printf("writing %d bytes\n",bufsize);
    TEMP_FAILURE_RETRY(i=send(GetSd(),buf,bufsize,flags));
    return i;
  }

  int ReadLine(char *obuf,int bufsize)
  {
    char *buf=obuf;
    bufsize--;
    int c=0;
    while (bufsize && Read(buf,1)==1) {
      bufsize--;
      c++;
      if (*buf=='\n') {
        buf++;
        break;
      }
      buf++;
    }
    *buf='\0';
    //printf("Read line '%s'\n",obuf);
    return c;
  }
  
};

#if __linux

class NetworkInterface
{
public:
  char name[IFNAMSIZ];
  IPADDR localaddr,bcastaddr,gateway,netmask;

  NetworkInterface() {
    Clear();
  }

  NetworkInterface(char *nm) {
    Query(nm);
  }
  
  void Clear() {
    *name='\0';
    localaddr.Clear();
    bcastaddr.Clear();
    gateway.Clear();
    netmask.Clear();
  }
  
  bool Query(char *nm)
  {
    struct ifreq addr;
    Clear();
    strncpy(name,nm,sizeof(name)-1);
    name[sizeof(name)-1]='\0';
    memset(&addr,0,sizeof addr);
    strcpy(addr.ifr_ifrn.ifrn_name,name);
    Socket s(SOCK_DGRAM,IPPROTO_IP);
    if (!ioctl(s.GetSd(),SIOCGIFADDR,&addr))
      localaddr.SetAddr(addr.ifr_ifru.ifru_addr);
    else {
      //printf("could not get local addr for %s - %s\n",name,strerror(errno));
      return false;
    }
    if (!ioctl(s.GetSd(),SIOCGIFBRDADDR,&addr))
      bcastaddr.SetAddr(addr.ifr_ifru.ifru_broadaddr);
    else {
      //printf("could not get bcast addr for %s - %s\n",name,strerror(errno));
    }
    if (!ioctl(s.GetSd(),SIOCGIFNETMASK,&addr))
      netmask.SetAddr(addr.ifr_ifru.ifru_netmask);
    else {
      //printf("could not get netmask for %s - %s\n",name,strerror(errno));
    }
    if (!ioctl(s.GetSd(),SIOCGIFDSTADDR,&addr))
      gateway.SetAddr(addr.ifr_ifru.ifru_dstaddr);
    else {
      //printf("could not get gateway addr for %s - %s\n",name,strerror(errno));
    }
    return true;
  }

};
#endif

#endif
