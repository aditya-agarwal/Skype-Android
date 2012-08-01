#ifndef __stateinfo_hpp_included__
#define __stateinfo_hpp_included__

#include "stdincludes.hpp"
#include "bmcrypto.hpp"

class StateInfo {
private:
AttributeContainer container;

enum STATEINFO { LASTSERVER,SAVEDATA };

public:


  void SetSaveData(const AttributeContainer& a)
  {
    container.RemoveAttribute(StateInfo::SAVEDATA);
    container.AddContainer(StateInfo::SAVEDATA,a);
    Store();
  }
  
  bool GetSaveData(AttributeContainer& a)
  {
    if (!container.AttributeExists(StateInfo::SAVEDATA,ATTRTYPE_CONTAINER))
      return false;
    a=container.GetContainer(StateInfo::SAVEDATA);
    return true;
  }
  
  const char * GetLastUsedHost(void)
  {
    return container.GetString(StateInfo::LASTSERVER);
  }
  
  void SetLastUsedHost(const char *host)
  {
    if (container.AttributeExists(StateInfo::LASTSERVER,ATTRTYPE_STRING))
      container.RemoveAttribute(StateInfo::LASTSERVER);
    container.AddString(StateInfo::LASTSERVER,host);
    Store();
  }
  
  void Load()
  {
  MEMBLOCK m;
    FILE *fp=fopen("clientstate.dat","rb");
    if (fp) {
      fseek(fp,0,SEEK_END);
      m.SetSize(ftell(fp));
      rewind(fp);
      if (fread(m.Ptr,m.Len,1,fp)) {
        container.Clear();
        container.Deserialize(m);
        printf("State loaded\n");
      }
      fclose(fp);
    }
    
  }
  
  void Store()
  {
  MEMBLOCK m;
    printf("Storing state\n");
    FILE *fp=fopen("clientstate.dat","wb");
    if (fp) {
      container.Serialize(m);
      fwrite(m.Ptr,m.Len,1,fp);
      fclose(fp);
    }
  }
};

#endif
