#ifndef __TRANSPORT_HPP_INCLUDED__
#define __TRANSPORT_HPP_INCLUDED__

#include "IPNetwork.hpp"

class Transport {
public:
  virtual ~Transport() {};
  virtual bool Connect(const char *host=NULL)=0;
  virtual void Disconnect()=0;
  virtual int Read(void *buf,int len)=0;
  virtual int Write(void *buf,int len)=0;
  virtual bool Poll()=0;
  virtual Address GetAddress()=0;
  virtual void GetStats(uint& bytessent,uint& bytesperminsent,
                        uint& bytesreceived,uint& bytesperminreceived)=0;
};



class TCPTransport : public Transport 
{

  TCPConnection tcpconnection;
  unsigned int sent,received;
  time_t starttime;

public:

  virtual void GetStats(uint& bytessent,uint& bytesperminsent,
                uint& bytesreceived,uint& bytesperminreceived)
  {
    bytessent=sent;
    bytesreceived=received;
    time_t t=(time(NULL)-starttime)/60;
    if (starttime && t>0) {
      bytesperminsent=sent/t;
      bytesperminreceived=received/t;
    }
    else {
      bytesperminsent=0;
      bytesperminreceived=0;
    }
  }

  virtual bool Connect(const char *host) {
    MEMBLOCK m;
    int port;
    m.SetSize(strlen(host)+1);
    strcpy((char *)m.Ptr,host);
    char *s=strchr((char *)m.Ptr,':');
    if (s) {
      *s='\0';
      port=atoi(s+1);
    }
    else
      port=80;
    printf("TCPTransport connecting to %s:%d\n",(char *)m.Ptr,port);
    starttime=time(NULL);
    sent=received=0;
    return tcpconnection.Open((char *)m.Ptr,port);
  }
  
  virtual void Disconnect() {
    tcpconnection.Close();
  }
  
  virtual bool Poll()
  {
	return tcpconnection.DataReady();
  }

  virtual int Read(void *buf,int len)
  {
    int i;
    i=tcpconnection.Read((uchar *)buf,len);
    if (i>0)
      received+=i;
    return i;
  }
  
  virtual int Write(void *buf,int len)
  {
    int i;
    i=tcpconnection.Write((uchar *)buf,len);
    if (i>0)
      sent+=i;
    return i;
  }
  
  virtual Address GetAddress()
  {
    Address a;
    IPADDR n;
    n=tcpconnection.GetName();
    a.IPAddress=n.GetAddr();
    a.Port=n.GetPort();
    return a;
  }
  
};

#endif
