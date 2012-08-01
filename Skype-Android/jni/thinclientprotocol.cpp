#include "thinclientprotocol.hpp"

#include "rootkeys.cpp"

bool ThinClientProtocol::CopyAttribute(int key,AttributeContainer& src,AttributeContainer& dst)
{
  const void *s;
  uint l;
  const uint *a;
  AttributeContainer::Attribute* at;
  at=src.FindAttribute(key);
  if (at) {
    if (dst.FindAttribute(key))
      dst.RemoveAttribute(key);
    switch(at->GetType()) {
      case ATTRTYPE_INTEGER:
          dst.AddInteger(key,at->GetInteger());
          break;
      case ATTRTYPE_INT64:
          dst.AddInt64(key,at->GetInt64());
          break;
      case ATTRTYPE_ADDRESS:
          dst.AddAddress(key,at->GetAddress());
          break;
      case ATTRTYPE_STRING:
          dst.AddString(key,at->GetString());
          break;
      case ATTRTYPE_BINARY:
          s=at->GetBinary(l);
          dst.AddBinary(key,s,l);
          break;
      case ATTRTYPE_CONTAINER:
          dst.AddContainer(key,at->GetContainer());
          break;
      case ATTRTYPE_INTARRAY:
          a=at->GetIntArray(l);
          dst.AddIntArray(key,a,l);
          break;
    }
    return true;
  }
  return false;
}

bool ThinClientProtocol::SendPacket(AttributeContainer& packet)
{
  uint len=packet.SerializeSize(),ll;
  MEMBLOCK m;
  CRC32 crc;
  ll=EncodedUintSize(len); 	// calculate the size of resulting packet
  m.SetSize(ll+len+4);     	// allocate buffer space (length of varlen encoded len+serialized containter+crc)
  uchar *p=(uchar *)m.Ptr;
  EncodeUint(p,len);       	// encode data length
  packet.Serialize(p);      	// serialize payload
  p+=len;
  if (encryption) {          	// if encryption is on, seed the CRC32
      crc.Update(hmac_key,20);
  }
  crc.Update((uchar *)m.Ptr+ll,len);
  uint icrc=~(uint)crc;
  p[0]=(uint)icrc&0xff;     	// add inverted CRC32, LSB first
  p[1]=((uint)icrc>>8)&0xff;
  p[2]=((uint)icrc>>16)&0xff;
  p[3]=((uint)icrc>>24)&0xff;
  //Debug::Log("Sending %d bytes\n",m.Len);
  //HexDump((uchar *)m.Ptr,m.Len);
  PacketDump("Sending",packet);
  if (encryption)          // if encryption is active, encrypt outgoing data
      write_rc4.ProcessString((uchar *)m.Ptr,m.Len);
  packetsout++;
  return transport->Write(m.Ptr,m.Len);
}

void ThinClientProtocol::Disconnect()
{
  Debug::Log("Disconnecting\n");
  if (!isDisconnected()) {
    if (encryption)
      SendDisconnect();
  }
  transport->Disconnect();
  protocolstate=IDLE;
  receiverstate=RESET;
}

ThinClientProtocol::~ThinClientProtocol()
{
  Debug::Log("Destructing ThinClientProtol\n");
  state.Store();
  transport->Disconnect();
  delete transport;
}

ThinClientProtocol::ThinClientProtocol(const char* platformid,const char *uilanguage)
{
  MEMBLOCK m;
  m.SetSize(sizeof(primary_rootkey));
  memcpy(m.Ptr,primary_rootkey,m.Len);
  primaryrootkey.Deserialize(m);
  if (!primaryrootkey.AttributeExists(ATTR_ROOT_PUBLIC_KEY,ATTRTYPE_BINARY))
    Debug::Log("Primary root key does not contain valid arguments\n");
  memcpy(m.Ptr,secondary_rootkey,m.Len);
  secondaryrootkey.Deserialize(m);
  if (!primaryrootkey.AttributeExists(ATTR_ROOT_PUBLIC_KEY,ATTRTYPE_BINARY))
    Debug::Log("Primary root key does not contain valid arguments\n");
  transport=new TCPTransport;
  protocolvalues.Clear();
  protocolvalues.AddString(TCPROTO::PLATFORMID,platformid);
  protocolvalues.AddString(TCPROTO::LANGUAGE,uilanguage);
  encryption=false;
  protocolstate=IDLE;
  receiverstate=RESET;
  packetsout=packetsin=0;
  connectionstart=0;
  presencebookmark=0;
  buddybookmark = 0;
  state.Load();
}

// notification handlers, overload these to your taste
void ThinClientProtocol::onErrorReceived(uint errorcode,const char *errormessage)
{
  Debug::Log("Protocol error %u received (%s)\n",errorcode,errormessage);
}

void ThinClientProtocol::onWarningReceived(uint warningcode,const char *warningmessage)
{
  Debug::Log("Protocol warning %u received (%s)\n",warningcode,warningmessage);
}

bool ThinClientProtocol::onUpgradeInfoReceived()
{
  Debug::Log("Upgrading not implemented yet\n");
  return false; // return true if upgraded and must reconnect
}

void ThinClientProtocol::onPeerAssistedPush(Address node,Address target,Address initiator,uchar *cookie,uint cookielen)
{
  Debug::Log("Peer assisted push not implemented yet\n");
}

void ThinClientProtocol::onLoggedIn()
{
  Debug::Log("Logged in.\n");
}

void ThinClientProtocol::onConnected()
{
	Debug::Log("Connected.\n");
}

bool ThinClientProtocol::ProcessIncomingData()
{
  static MEMBLOCK m;
  static uint len=0;
  static uint shift=0;
  static uint remaining=0;
  static uchar *readptr;
  uchar c;
  CRC32 crc;
  switch (receiverstate) {
    case DELIVER:
      return true; // dont allow reentry
    case RESET:
      len=0;
      shift=0;
      receiverstate=READLEN;
    case READLEN:
      while (receiverstate==READLEN && transport->Poll()) {
        if (!transport->Read(&c,1)) {
          Debug::Log("broken connection? could not read packet length\n");
          protocolstate=DISCONNECTING;
          Disconnect();
          return false;
        }
        if (encryption)
          read_rc4.ProcessString(&c,1);
        len|=((uint)c&0x7f)<<shift;
        shift+=7;
        if (!(c&0x80)) {
          if (len>32768) {
            Debug::Log("Corrupt data, packet size (%u) exceeds maximum allowed size\n",len);
            protocolstate=DISCONNECTING;
            Disconnect();
            return false;
          }
          remaining=len+4;
          m.SetSize(remaining);
          readptr=(uchar *)m.Ptr;
          receiverstate=READDATA;
        }
      }
      if (receiverstate==READLEN)
        return true;
    case READDATA:
      while (receiverstate==READDATA && transport->Poll()) {
        len=transport->Read(readptr,remaining);
        if (len>0) {
          readptr+=len;
          remaining-=len;
          if (!remaining)
            receiverstate=CHECK;
        }
        else {
          Debug::Log("broken connection, could not read packet data\n");
          protocolstate=DISCONNECTING;
          Disconnect();
          return false;
        }
      }
      if (receiverstate==READDATA)
        return true;
    case CHECK:
      if (encryption) {
        crc.Update(hmac_key,sizeof(hmac_key));
        read_rc4.ProcessString((uchar *)m.Ptr,m.Len);
      }
      crc.Update((uchar *)m.Ptr,m.Len-4);
      readptr=(uchar *)m.Ptr+(m.Len-4);
      uint rcrc=(((uint)readptr[0])|((uint)readptr[1])<<8|((uint)readptr[2])<<16|((uint)readptr[3])<<24);
      if (~(uint)crc==rcrc) {
        AttributeContainer packet;
        receiverstate=DELIVER;
        packet.Deserialize(m);
        packetsin++;
        PacketDump("Received",packet);
        onPacketReceived(packet);
        receiverstate=RESET;
        return true;
      }
      Debug::Log("CRC check failed on packet (expected value %08x, got %08x)\n",~(uint)crc,rcrc);
      protocolstate=DISCONNECTING;
      Disconnect();
      return false;
  }
  return true;
}  

bool ThinClientProtocol::onConnectedReceived(AttributeContainer& a)
{
  CopyAttribute(TCPROTO::CLIENT_PUBLIC_IP_ADDRESS,a,protocolvalues);
  CopyAttribute(TCPROTO::LATEST_VERSION,a,protocolvalues);
  CopyAttribute(TCPROTO::UPGRADEURL,a,protocolvalues);
  CopyAttribute(TCPROTO::DEVICE_PSTNNUMBER,a,protocolvalues);
  CopyAttribute(TCPROTO::DEVICE_MCC,a,protocolvalues);
  CopyAttribute(TCPROTO::DEVICE_MNC,a,protocolvalues);
  CopyAttribute(TCPROTO::SERVER_UTC_TIME,a,protocolvalues);
  CopyAttribute(TCPROTO::SUBSCRIPTION_ERROR_MESSAGE,a,protocolvalues);
  CopyAttribute(TCPROTO::SUBSCRIPTION_LSK,a,protocolvalues);
  CopyAttribute(TCPROTO::SUBSCRIPTION_URL,a,protocolvalues);
  CopyAttribute(TCPROTO::UPGRADE_MESSAGE,a,protocolvalues);
  CopyAttribute(TCPROTO::UPGRADE_LSK,a,protocolvalues);
  PacketDump("Protocol values: ",protocolvalues);
  return true;
}

bool ThinClientProtocol::onBuddyListReceived(AttributeContainer& a)
{
  Debug::Log("onBuddyListReceived");

  buddybookmark = a.GetInteger(TCPROTO::BOOKMARK);
  uint count = a.GetInteger(TCPROTO::RECORDCOUNT);
  AttributeContainer records = a.GetContainer(TCPROTO::BUDDYRECORDS);
  for(int i = 0; i < count; i++) {

	  AttributeContainer buddy = records.GetContainer(i);
	  int buddyIndex = buddy.GetInteger(TCPROTO::BUDDYINDEX);
	  const char* name = buddy.GetString(TCPROTO::SKYPENAME);
	  const char* fullname = buddy.GetString(TCPROTO::FULLNAME);
	  int availability = buddy.GetInteger(TCPROTO::CONTACTAVAILABILITY);

	  onBuddyReceived(count, buddyIndex, name, fullname, availability);
  }

  return true;
}

void ThinClientProtocol::onBuddyReceived(uint total, uint buddyIndex,
		const char* name, const char* fullname, uint availability) {

	Debug::Log("Buddy index: %d name: %s fullname: %s availability: %d",
			buddyIndex, name, fullname, availability);
}

void ThinClientProtocol::onPresenceUpdated(uint buddyIndex, uint availability) {

	Debug::Log("Buddy updated %d status: %d", buddyIndex, availability);
}

bool ThinClientProtocol::onPresenceUpdateReceived(AttributeContainer& a)
{
	presencebookmark = a.GetInteger(TCPROTO::BOOKMARK);
	uint count;
	const uint* statuses = a.GetIntArray(TCPROTO::ONLINESTATUSES, count);
	for(int i = 0; i < count; i++) {
		int status = statuses[i];
		int availability = AVAILABILITYFROMSTATUS(status);
		int index = INDEXFROMSTATUS(status);
		onPresenceUpdated(index, availability);
	}
	return true;
}

bool ThinClientProtocol::onCallSetupReceived(AttributeContainer& a)
{
  SetCallMethods(NULL,0);
  CopyAttribute(TCPROTO::CALLMETHOD,a,protocolvalues);
  CopyAttribute(TCPROTO::CALLMETHODS,a,protocolvalues);
  CopyAttribute(TCPROTO::DIALIN_NUMBER_PREFIX,a,protocolvalues);
  CopyAttribute(TCPROTO::DIALIN_NUMBER_RANGE_SIZE,a,protocolvalues);
  CopyAttribute(TCPROTO::VOICEMAIL_NUMBER_PREFIX,a,protocolvalues);
  CopyAttribute(TCPROTO::VOICEMAIL_NUMBER_RANGE_SIZE,a,protocolvalues);
  CopyAttribute(TCPROTO::SI_GATEWAY_NUMBER,a,protocolvalues);
  CopyAttribute(TCPROTO::SI_GATEWAY_NAME,a,protocolvalues);
  CopyAttribute(TCPROTO::SO_ENABLED,a,protocolvalues);
  CopyAttribute(TCPROTO::CALLING_URI_EXTENSIONS_ENABLED,a,protocolvalues);
  PacketDump("Protocol values: ",protocolvalues);
  return true;
}

bool ThinClientProtocol::onPresenceSettingsReceived(AttributeContainer& a)
{
  CopyAttribute(TCPROTO::FG_POLL_INTERVAL,a,protocolvalues);
  CopyAttribute(TCPROTO::BG_POLL_INTERVAL,a,protocolvalues);
  PacketDump("Protocol values: ",protocolvalues);
  return true;
}

bool ThinClientProtocol::onRedirectReceived(AttributeContainer& a)
{
Address adr;
char abuf[128];
  CopyAttribute(TCPROTO::LATEST_VERSION,a,protocolvalues);
  CopyAttribute(TCPROTO::UPGRADEURL,a,protocolvalues);
  CopyAttribute(TCPROTO::UPGRADE_MESSAGE,a,protocolvalues);
  CopyAttribute(TCPROTO::UPGRADE_LSK,a,protocolvalues);
  if (a.AttributeExists(TCPROTO::NEW_SERVER,ATTRTYPE_ADDRESS)) {
    adr=a.GetAddress(TCPROTO::NEW_SERVER);
    if (CLIENTPROTOCOLVERSION<3)
      adr.Port=((adr.Port<<8)&0xff00)|((adr.Port>>8)&255);
    sprintf(abuf,"%u.%u.%u.%u:%u",
      (adr.IPAddress>>24),
      (adr.IPAddress>>16)&0xff,
      (adr.IPAddress>>8)&0xff,
      adr.IPAddress&0xff,
      adr.Port);
    Debug::Log("Redirected to %s\n",abuf);
    state.SetLastUsedHost(abuf);
    return true;
  }
  return false;
}

bool ThinClientProtocol::onServerIdentityReceived(AttributeContainer& a)
{
uchar *s;
uint i;
  s=(uchar *)a.GetBinary(TCPROTO::SERVER_RANDOM,i);
  if (s && i==sizeof(server_random)) {
    memcpy(server_random,s,sizeof(server_random));
    RemoveProtocolValue(TCPROTO::SERVER_NAME);
    protocolvalues.AddString(TCPROTO::SERVER_NAME,a.GetString(TCPROTO::SERVER_NAME));
    s=(uchar *)a.GetBinary(TCPROTO::SERVER_IDENTITY,i);
    if (s && i==ROOTKEYSIZE/8+2) {
      serverkey.Clear();
      serverkey.AddBinary(ATTR_SERVER_SIGNED_PUBLIC_KEY,s,i);
      if (tc_crypto::VerifyServerKey(serverkey,primaryrootkey) ||
          tc_crypto::VerifyServerKey(serverkey,secondaryrootkey) ) {
        Debug::Log("Server key is valid\n");
        return true;
      }
      else {
        Debug::Log("Server signature check failed\n");
      }
    }
    else {
      Debug::Log("SERVERIDENTITY malformed, server identity invalid (l=%u vs. %u)\n",i,ROOTKEYSIZE/8+2);
    }
  }
  else {
    Debug::Log("SERVERIDENTITY malformed, server random data invalid\n");
  }               
  return false;
}

bool ThinClientProtocol::onSaveDataReceived(AttributeContainer &a)
{
  if (a.GetOptInteger(TCPROTO::PACKETTYPE)==TCPROTO::SAVEDATA) {
    Debug::Log("SAVEDATA received, storing into state\n");
    state.SetSaveData(a.GetContainer(TCPROTO::SAVEDDATA));
    return true;
  }
  return false;
}

void ThinClientProtocol::HandleCommonAttributes(AttributeContainer& a)
{
Address target,initiator,node;
uchar *cookie,*nodelist;
uint cl,ac;
  // check for peer assisted push
  if (a.AttributeExists(TCPROTO::POLL_TARGET,ATTRTYPE_ADDRESS)
      && a.AttributeExists(TCPROTO::POLL_INITIATOR,ATTRTYPE_ADDRESS)
      && a.AttributeExists(TCPROTO::POLL_COOKIE,ATTRTYPE_BINARY)
      && a.AttributeExists(TCPROTO::NOTIFY_NODES,ATTRTYPE_BINARY)) {
    target=a.GetAddress(TCPROTO::POLL_TARGET);
    initiator=a.GetAddress(TCPROTO::POLL_INITIATOR);
    cookie=(uchar *)a.GetBinary(TCPROTO::POLL_COOKIE,cl);
    nodelist=(uchar *)a.GetBinary(TCPROTO::NOTIFY_NODES,ac);
    if (!nodelist || !cookie)
      return; 
    ac=ac/6; // divide total length with length of one address
    while (ac--) {
      node.IPAddress=(nodelist[0]<<24)|(nodelist[1]<<16)|(nodelist[2]<<8)|nodelist[3];
      node.Port=(nodelist[4]<<16)|nodelist[5]; 
      nodelist+=6;
      onPeerAssistedPush(node,target,initiator,cookie,cl);
    }
  }
}

bool ThinClientProtocol::SendConnect()
{
  AttributeContainer a;
  uint i;
  a.AddInteger(TCPROTO::PACKETTYPE,TCPROTO::CONNECT);
  a.AddInteger(TCPROTO::PROTOCOLVERSION,CLIENTPROTOCOLVERSION);
  for (i=0;i<sizeof(client_random);i++)
    client_random[i]=tc_random_func(NULL);
  a.AddBinary(TCPROTO::CLIENT_RANDOM,client_random,sizeof(client_random));
  GetProtocolValue(TCPROTO::PLATFORMID,a);
  GetProtocolValue(TCPROTO::CAPABILITIES,a);
  GetProtocolValue(TCPROTO::LANGUAGE,a);
  return SendPacket(a);
}


bool ThinClientProtocol::SendMasterKey()
{
AttributeContainer a,ud;
uchar encrypted_secret[SERVERKEYSIZE/8+2];
uint i;
  a.AddInteger(TCPROTO::PACKETTYPE,TCPROTO::MASTERKEY);
  for (i=0;i<sizeof(secret);i++)
    secret[i]=tc_random_func(NULL);
  if (!tc_crypto::EncryptSecret(secret,encrypted_secret,serverkey))
  {
    Debug::Log("master secret encryption failed\n");
    Disconnect();
    return false;
  }
  a.AddBinary(TCPROTO::SECRETMASTER,encrypted_secret,sizeof(encrypted_secret));
  ud.AddInteger(TCPROTO::PACKETTYPE,TCPROTO::USERDATA);
  GetProtocolValue(TCPROTO::NODEID,ud);
  GetProtocolValue(TCPROTO::SKYPENAME,ud);
  GetProtocolValue(TCPROTO::SDFSESSIONID,ud);
  GetProtocolValue(TCPROTO::DEVICE_PSTNNUMBER,ud);
  MEMBLOCK m;
  ud.Serialize(m);
  MakeRC4Key(userdata_rc4,"userdata");
  userdata_rc4.ProcessString((uchar *)m.Ptr,m.Len);
  a.AddBinary(TCPROTO::ENCRYPTED_USERDATA,(uchar *)m.Ptr,m.Len);
  if (SendPacket(a)) {
    EnableEncryption();
    return true;
  }
  return false;
}

bool ThinClientProtocol::SendClientData()
{
AttributeContainer a;
Address privateaddress=transport->GetAddress();
  a.AddInteger(TCPROTO::PACKETTYPE,TCPROTO::CLIENTDATA);
  a.AddAddress(TCPROTO::CLIENT_PRIVATE_IP_ADDRESS,privateaddress);
  GetProtocolValue(TCPROTO::DEVICE_PSTNNUMBER,a);
  GetProtocolValue(TCPROTO::CALLMETHODS,a);
  GetProtocolValue(TCPROTO::PUSHIDENTIFIER,a);
  a.AddString(TCPROTO::PUSHIDENTIFIER,"");
  return SendPacket(a);
}

bool ThinClientProtocol::SendDisconnect()
{
AttributeContainer a;
  a.AddInteger(TCPROTO::PACKETTYPE,TCPROTO::DISCONNECT);
  return SendPacket(a);
}

bool ThinClientProtocol::SendGetBuddyList()
{
AttributeContainer a;
  a.AddInteger(TCPROTO::PACKETTYPE,TCPROTO::GETBUDDYLIST);
  a.AddInteger(TCPROTO::BOOKMARK, buddybookmark);
  return SendPacket(a);
}

bool ThinClientProtocol::SendGetPresenceUpdate()
{
AttributeContainer a;
  a.AddInteger(TCPROTO::PACKETTYPE,TCPROTO::GETPRESENCEUPDATE);
  a.AddInteger(TCPROTO::BOOKMARK, presencebookmark);
  return SendPacket(a);
}

bool ThinClientProtocol::SendLogin(int savepassword)
{
AttributeContainer a,s;
  a.AddInteger(TCPROTO::PACKETTYPE,TCPROTO::LOGIN);
  a.AddInteger(TCPROTO::SAVEPASSWORD,savepassword);
  GetProtocolValue(TCPROTO::SESSION_ID,a);
  GetProtocolValue(TCPROTO::SKYPENAME,a);
  if (!GetProtocolValue(TCPROTO::PASSWORD,a)) {
    if (!state.GetSaveData(s)) {
      Debug::Log("Password not supplied but no SAVEDATA either, cannot login\n");
      return false;
    }
    a.AddContainer(TCPROTO::SAVEDDATA,s);
    Debug::Log("Logging in with saved credentials\n");
  }
  else {
    Debug::Log("Logging in with password\n");
  }
  GetProtocolValue(TCPROTO::DEVICE_PSTNNUMBER,a);
  return SendPacket(a);  
}

bool ThinClientProtocol::onLoggedinReceived(AttributeContainer& packet)
{
uchar *s;
uint l;
  s=(uchar *)packet.GetBinary(TCPROTO::SESSION_ID,l);
  if (!s)
    return false;
  RemoveProtocolValue(TCPROTO::SESSION_ID);
  protocolvalues.AddBinary(TCPROTO::SESSION_ID,s,l);
  RemoveProtocolValue(TCPROTO::HELP_URL);
  if (packet.AttributeExists(TCPROTO::HELP_URL,ATTRTYPE_STRING))
    protocolvalues.AddString(TCPROTO::HELP_URL,packet.GetString(TCPROTO::HELP_URL));
  protocolstate=LOGGEDIN;
  onLoggedIn();
  return true;
}

void ThinClientProtocol::onLoggedOut(TCPROTO::ACCOUNT_LOGOUTREASON reason)
{
  Debug::Log("Logged out, ");
  switch (reason) {
    case TCPROTO::LOGOUTREASON_UNKNOWN:
      Debug::Log("unknown reason\n");
      break;
    case TCPROTO::LOGOUT_CALLED:
      Debug::Log("normal logout\n");
      break;
    case TCPROTO::INVALID_SKYPENAME: 
      Debug::Log("invalid skypename\n");
      break;
    case TCPROTO::INVALID_EMAIL:
      Debug::Log("invalid email\n");
      break;
    case TCPROTO::UNACCEPTABLE_PASSWORD: 
      Debug::Log("unacceptable password\n");
      break;
    case TCPROTO::SKYPENAME_TAKEN: 
      Debug::Log("skypename taken\n");
      break;
    case TCPROTO::REJECTED_AS_UNDERAGE:
      Debug::Log("rejected as underage\n");
      break;
    case TCPROTO::NO_SUCH_IDENTITY: 
      Debug::Log("no such identity\n");
      break;
    case TCPROTO::INCORRECT_PASSWORD: 
      Debug::Log("incorrect password\n");
      break;
    case TCPROTO::TOO_MANY_LOGIN_ATTEMPTS:
      Debug::Log("too many login attempts\n");
      break;
    case TCPROTO::PASSWORD_HAS_CHANGED: 
      Debug::Log("password has changed\n");
      break;
    case TCPROTO::INTERNAL_LIBRARY_ERROR:
      Debug::Log("internal library error\n");
      break;
  }
}

bool ThinClientProtocol::onLoggedoutReceived(AttributeContainer& packet)
{
  onLoggedOut((TCPROTO::ACCOUNT_LOGOUTREASON)packet.GetOptInteger(TCPROTO::LOGOUTREASON));
  protocolstate=LOGGEDOUT;
  return true;
}

void ThinClientProtocol::onPacketReceived(AttributeContainer& packet)
{
  // handle optional attributes that can be in any packet 
  HandleCommonAttributes(packet);  
  // these can arrive irrespective of state
  switch (packet.GetOptInteger(TCPROTO::PACKETTYPE)) {
    case TCPROTO::SAVEDATA:
      onSaveDataReceived(packet);
      return;
    case TCPROTO::ERROR:
      onErrorReceived(packet.GetOptInteger(TCPROTO::ERROR_CODE),packet.GetOptString(TCPROTO::ERROR_MESSAGE));
      return;
    case TCPROTO::WARNING:
      onWarningReceived(packet.GetOptInteger(TCPROTO::WARNING_CODE),packet.GetOptString(TCPROTO::WARNING_MESSAGE));
      return;
    case TCPROTO::LOGGEDIN:
      onLoggedinReceived(packet);
      return;
    case TCPROTO::LOGGEDOUT:
      onLoggedoutReceived(packet);
      return;
  }
  // we need to handle the rest depending on connection state
  switch (protocolstate) {
    case IDLE:
      return;
    case DISCONNECTING:
    case LOGGEDOUT:
      Disconnect();      
      return;
    case REDIRECT:
      Disconnect();
      protocolstate=STARTCONNECTING;
    case STARTCONNECTING:
      serverkey.Clear();
      encryption=false;
      if (transport->Connect(state.GetLastUsedHost())) {
        connectionstart=time(NULL);
        SendConnect();
        protocolstate=WAITINGIDENTITY;
      }
      else {
        protocolstate=IDLE;
        Debug::Log("connect failed"); //np log
      }
      return;
    // connect has been sent, identity must arrive
    case WAITINGIDENTITY:
      if (packet.GetOptInteger(TCPROTO::PACKETTYPE)==TCPROTO::SERVERIDENTITY) {
        onServerIdentityReceived(packet);
        SendMasterKey();
        protocolstate=WAITINGCONNECTED;
      }
      else {
        Debug::Log("Protocol error, expected packettype %d got %d\n",TCPROTO::SERVERIDENTITY,packet.GetOptInteger(TCPROTO::PACKETTYPE)); 
        Disconnect();
      }
      return;
    // masterkey has been sent, must get CONNECTED or REDIRECT now
    case WAITINGCONNECTED:
      switch (packet.GetOptInteger(TCPROTO::PACKETTYPE))
      {
        case TCPROTO::CONNECTED:
          onConnectedReceived(packet);
          if (onUpgradeInfoReceived()) {
            Debug::Log("Upgrade handled, disconnecting\n");
            Disconnect();
            return;
          }
          SendClientData();
          protocolstate=CONNECTED;
          onConnected();
          break;
        case TCPROTO::REDIRECT:
          onRedirectReceived(packet);
          if (onUpgradeInfoReceived()) {
            Debug::Log("Upgrade handled, disconnecting\n");
            Disconnect();
            return;
          }
          {
            AttributeContainer a;
            protocolstate=REDIRECT;
            onPacketReceived(a);
          }
          break;
        default:
          Debug::Log("Protocol error, expected packettype %d or %d, got %d\n",TCPROTO::CONNECTED,TCPROTO::REDIRECT,packet.GetOptInteger(TCPROTO::PACKETTYPE));
          Disconnect();
          return;
      }
      break;
    // in connected state, just process everything that arrives, in any order
    case CONNECTED:
    case LOGGEDIN:
      switch (packet.GetOptInteger(TCPROTO::PACKETTYPE))
      {
        case TCPROTO::CALLSETUP:
          onCallSetupReceived(packet);
          break;
        case TCPROTO::PRESENCESETTINGS:
          onPresenceSettingsReceived(packet);
          break;
        case TCPROTO::BUDDYLIST:
          onBuddyListReceived(packet);
          break;
        case TCPROTO::PRESENCEUPDATE:
          onPresenceUpdateReceived(packet);
          break;
      }
      
      break;
  }
}    
    

bool ThinClientProtocol::Connect(const char *host)
{
AttributeContainer a;
Address adr;
  if (!host) {
    host=state.GetLastUsedHost();
    if (!host) {
      Debug::Log("No hostname given, and none stored in state, cannot connect\n");
      return false;
    }
    Debug::Log("Connecting to last used host %s\n",host);
  }
  else
    state.SetLastUsedHost(host);
  Debug::Log("Host set to %s\n",host);

  protocolstate=STARTCONNECTING;
  packetsout=packetsin=0;
  onPacketReceived(a); // fake call to kick the connection machine going
  return true;
}


bool ThinClientProtocol::Login(int savepassword)
{
  return SendLogin(savepassword);
}

bool ThinClientProtocol::SendLogout()
{
AttributeContainer a;
  a.AddInteger(TCPROTO::PACKETTYPE,TCPROTO::LOGOUT);
  return SendPacket(a);
}

bool ThinClientProtocol::Logout()
{
AttributeContainer a;
  return SendLogout();
}


void ThinClientProtocol::SetCapabilities(uint capabilities)
{
  protocolvalues.RemoveAttribute(TCPROTO::CAPABILITIES);
  protocolvalues.AddInteger(TCPROTO::CAPABILITIES,capabilities);
}

void ThinClientProtocol::SetUserName(const char *username)
{
  protocolvalues.RemoveAttribute(TCPROTO::SKYPENAME);
  if (username)
    protocolvalues.AddString(TCPROTO::SKYPENAME,username);
}

void ThinClientProtocol::SetSystemID(uchar systemid[8])
{
  protocolvalues.RemoveAttribute(TCPROTO::NODEID);
  if (systemid)
    protocolvalues.AddBinary(TCPROTO::NODEID,systemid,8);
}


void ThinClientProtocol::SetPSTNNumber(char *pstnnumber)
{
  protocolvalues.RemoveAttribute(TCPROTO::DEVICE_PSTNNUMBER);
  if (pstnnumber)
    protocolvalues.AddString(TCPROTO::DEVICE_PSTNNUMBER,pstnnumber);
}

void ThinClientProtocol::SetPassword(const char *password)
{
  protocolvalues.RemoveAttribute(TCPROTO::PASSWORD);
  if (password)
    protocolvalues.AddString(TCPROTO::PASSWORD,password);
}
  
void ThinClientProtocol::SetPushIdentifier(char *identifier)
{
  protocolvalues.RemoveAttribute(TCPROTO::PUSHIDENTIFIER);
  protocolvalues.AddString(TCPROTO::PUSHIDENTIFIER,identifier);
}

void ThinClientProtocol::SetCallMethods(uint *m,uint count)
{
  protocolvalues.RemoveAttribute(TCPROTO::CALLMETHODS);
  if (m && count)
    protocolvalues.AddIntArray(TCPROTO::CALLMETHODS,m,count);
}

void ThinClientProtocol::MakeRC4Key(ARC4& rc4,char *constant)
{
SHA1 hash;
uchar s[20];
    hash.Update(secret,20);
    hash.Update(client_random,20);
    hash.Update(server_random,20);
    hash.Update((uchar *)constant,strlen(constant));
    hash.GetBinary(s);
    rc4.SetKey(s,20);
    for (int i=0;i<768;i++)
      rc4.GenerateByte();
    memset(s,0,sizeof(s));
}

void ThinClientProtocol::MakeMACKey(uchar *key)
{
SHA1 hash;
    hash.Update(secret,20);
    hash.Update(client_random,20);
    hash.Update(server_random,20);
    hash.Update("hashkey",7);
    hash.GetBinary(key);
}

bool ThinClientProtocol::EnableEncryption(void)
{
  MakeRC4Key(write_rc4,"client");
  MakeRC4Key(read_rc4,"server");
  MakeMACKey(hmac_key);
  encryption=true;
  Debug::Log("Encryption enabled\n");
  return encryption;
}

void ThinClientProtocol::onClientPacketReceived(AttributeContainer& a)
{
}
