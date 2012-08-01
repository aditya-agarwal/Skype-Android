#ifndef __THINCLIENTPROTOCOL_HPP_INCLUDED__
#define __THINCLIENTPROTOCOL_HPP_INCLUDED__

#ifdef ANDROID
#include <android/log.h>
#endif
#include "bmcrypto/tc_crypto.hpp"
#include "IPNetwork.hpp"
#include "tcprotoV3.hpp"
#include "packetdump.hpp"
#include "stateinfo.hpp"
#include "transport.hpp"

#define CLIENTPROTOCOLVERSION 3

/* ThinClientProtocol is a very simple networking implementation. It contains no threading and, thus, callers are responsible for initiating polling */
class ThinClientProtocol
{
private:
Transport * transport;
AttributeContainer protocolvalues, serverkey, primaryrootkey, secondaryrootkey; 
uchar client_random[20],server_random[20],hmac_key[20],secret[20];
ARC4 read_rc4, write_rc4, userdata_rc4;
bool encryption;
StateInfo state;
enum PROTOCOLSTATE { IDLE, DISCONNECTING, REDIRECT, STARTCONNECTING, WAITINGIDENTITY, WAITINGCONNECTED, CONNECTED, LOGGEDOUT, LOGGEDIN };
enum RECEIVERSTATE { RESET=0,READLEN,READDATA,CHECK,DELIVER };
PROTOCOLSTATE protocolstate;
RECEIVERSTATE receiverstate;
uint packetsout,packetsin;
time_t connectionstart;
uint presencebookmark;
uint buddybookmark;

  bool CopyAttribute(int key,AttributeContainer& src,AttributeContainer& dst);

  // universal packets
  void HandleCommonAttributes(AttributeContainer& a);
  
  // built-in packet handlers
  bool onSaveDataReceived(AttributeContainer &a);
  bool onServerIdentityReceived(AttributeContainer &a);
  bool onConnectedReceived(AttributeContainer& a);
  bool onRedirectReceived(AttributeContainer& a);
  bool onLoggedinReceived(AttributeContainer& packet);
  bool onLoggedoutReceived(AttributeContainer& packet);
  bool onCallSetupReceived(AttributeContainer& packet);
  bool onPresenceSettingsReceived(AttributeContainer& packet);
    
  bool onBuddyListReceived(AttributeContainer& packet);
  bool onPresenceUpdateReceived(AttributeContainer& packet);

  // protocol handshake packets
  bool SendConnect();
  bool SendMasterKey();
  bool SendClientData();
  bool SendDisconnect();
  bool SendLogin(int savepassword);
  bool SendLogout();



  // crypto helpers
  void MakeRC4Key(ARC4& rc4,char *constant);
  void MakeMACKey(uchar *key);
  bool EnableEncryption(void);


  void RemoveProtocolValue(int key)
  {
    if (protocolvalues.FindAttribute(key))
      protocolvalues.RemoveAttribute(key);
  }

  
public:

  class Debug {
    public:
      static void Log(const char *format,...) {
        va_list ap;
        va_start(ap,format);
        vprintf(format,ap);
#ifdef ANDROID
        __android_log_vprint(ANDROID_LOG_INFO, "Skype client", format, ap);
#endif
        va_end(ap);
      }
  };

  AttributeContainer& GetProtocolValues()
  {
    return protocolvalues;
  }

  void Stats()
  {
    uint sent,sentm,received,receivedm;
    if (transport) {
      transport->GetStats(sent,sentm,received,receivedm);
      Debug::Log("Transport stats: %u sent (%u bytes/min) %u received (%u bytes/min)\n",
        sent,sentm,received,receivedm);
      Debug::Log("Connected %u seconds, packets sent %u, received %u\n",
        connectionstart?time(NULL)-connectionstart:0,packetsout,packetsin);
    }
  }

  ThinClientProtocol(const char *platformid,const char *uilanguage);
  virtual ~ThinClientProtocol();

  bool isConnected() {return protocolstate==CONNECTED;}
  bool isDisconnected() {return protocolstate==IDLE;}
  bool isBusy() {return protocolstate!=IDLE;}
  bool isLoggedIn() {return protocolstate==LOGGEDIN;}
  
  /* Initiate the connection to Skype's servers */
  bool Connect(const char *host=NULL);
  /* Initiate the login procedure */
  bool Login(int savepassword);
  /* Poll network to see if a packet is ready to be read in and processed */
  bool ProcessIncomingData();
  bool SendPacket(AttributeContainer& packet);
  /* Initiate the logout procedure */
  bool Logout();
  /* Initiate the disconnection to Skype's servers */
  void Disconnect();

  bool SendGetBuddyList();
  bool SendGetPresenceUpdate();

  virtual void onPacketReceived(AttributeContainer& packet);
  virtual void onClientPacketReceived(AttributeContainer& packet);
  virtual void onErrorReceived(uint errorcode,const char *errormessage);
  virtual void onWarningReceived(uint warningcode,const char *warningmessage);
  virtual bool onUpgradeInfoReceived();
  virtual void onPeerAssistedPush(Address node,Address target,Address initiator,uchar *cookie,uint cookielen);
  virtual void onLoggedIn();
  virtual void onLoggedOut(TCPROTO::ACCOUNT_LOGOUTREASON reason);
  virtual void onConnected();
  virtual void onBuddyReceived(uint total, uint buddyIndex, const char* name, const char* fullname, uint availability);
  virtual void onPresenceUpdated(uint buddyIndex, uint availability);

  void SetCapabilities(uint capabilities);
  void SetUserName(const char *username);
  void SetSystemID(uchar systemid[8]);
  void SetPSTNNumber(char *pstnnumber);
  void SetPassword(const char *password);
  void SetPushIdentifier(char *identifier);
  void SetCallMethods(uint *m,uint count=0);

  bool GetProtocolValue(int key,AttributeContainer& c)
  {
    return CopyAttribute(key,protocolvalues,c);
  }


};

#endif
