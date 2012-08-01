/*
 * Copyright (C) 2006, Skype Limited
 *
 * All intellectual property rights, including but not limited to copyrights,
 * trademarks and patents, as well as know how and trade secrets contained
 * in, relating to, or arising from the internet telephony software of Skype
 * Limited (including its affiliates, "Skype"), including without limitation
 * this source code, Skype API and related material of such software
 * proprietary to Skype and/or its licensors ("IP Rights") are and shall
 * remain the exclusive property of Skype and/or its licensors. The recipient
 * hereby acknowledges and agrees that any unauthorized use of the IP Rights
 * is a violation of intellectual property laws.
 *
 * Skype reserves all rights and may take legal action against infringers of
 * IP Rights.
 *
 * The recipient agrees not to remove, obscure, make illegible or alter any
 * notices or indications of the IP Rights and/or Skype's rights and ownership
 * thereof.
 */

#ifndef _ATTRIBUTECONTAINER_HPP_INCLUDED
#define _ATTRIBUTECONTAINER_HPP_INCLUDED

#include "types.hpp"

void ResizePtr(void **ptr,uint len);
void FreePtr(void *ptr);

extern uint EncodedUintSize(uint value);
extern void EncodeUint(uchar *&buf,uint value);
extern bool DecodeUint(uint &val,const uchar *&buf,uint &len_left);

template <class T> class arrayptr {
    public:

    T * const ptr;
    const uint len;

    inline       T & operator[] (const uint idx)       { return ptr[idx]; }
    inline const T & operator[] (const uint idx) const { return ptr[idx]; }

    arrayptr<T>(T * const _ptr,const uint _len) : ptr(_ptr), len(_len) {}
    operator arrayptr<const T> (void) const
                                    { return arrayptr<const T>(ptr,len); }

    uint find_item(const T &elem) const;
        // returns NIL if item not found
    };

template <class T> uint arrayptr<T>::find_item(const T &elem) const {
    for (uint i=0;i < len;i++)
        if (ptr[i] == elem)
            return i;
    return NIL;
    }


template <class T,int Overhead=32> class LIST {

	template<class _Ty> inline void _DestroyListItem(_Ty *p){
		p->~_Ty();
	}
                
protected:
	T       *Ptr;
	int     ItemsAlloced;
    int CalcOverhead(void) const;
                        
public:
    void Construct(void){ItemsAlloced=Items=0;Ptr=NULL;}
    inline int Size(void) const {return Items*sizeof(T);}
            
    T *Add(int pos);
    void Add(T const &elem,int pos);
    inline T *Append(void){return Add(Items);}
    inline void Append(T const &elem){Add(elem,Items);}
    void DeleteAll(void);
	void Delete(int nr);
    void DoNothing(void *){}        
    void EnsureOverhead(int overhead);
    
    inline const T & operator[](int nr) const {return Ptr[nr];}
    inline const T & operator[](unsigned int nr) const {return Ptr[nr];}
    inline const T & operator[](long nr) const {return Ptr[nr];}
    inline const T & operator[](unsigned long nr) const {return Ptr[nr];}
    inline const T & operator[](short nr) const {return Ptr[nr];}
    inline const T & operator[](unsigned short nr) const {return Ptr[nr];}

    inline T & operator[](int nr){return Ptr[nr];}
    inline T & operator[](unsigned int nr){return Ptr[nr];}
    inline T & operator[](long nr){return Ptr[nr];}
    inline T & operator[](unsigned long nr){return Ptr[nr];}
    inline T & operator[](short nr){return Ptr[nr];}
    inline T & operator[](unsigned short nr){return Ptr[nr];}

    inline operator T* (void) const {return Ptr;}
    inline operator arrayptr<T> (void) const {return arrayptr<T>(Ptr,Items);}
    LIST<T,Overhead>(void){Construct();}
    void operator =(const arrayptr<T> &ap);
    void operator =(const LIST<T,Overhead> &l) { *this=(arrayptr<T>)l; }
    LIST<T,Overhead>(const arrayptr<T> &ap){Construct(); *this=ap;}
    LIST<T,Overhead>(const LIST<T,Overhead> &l){Construct(); *this=l;}
    ~LIST(void){DeleteAll();}
    void operator +=(const T &elem){Append(elem);}
	void operator +=(const arrayptr<T> &ap);
        
	int     Items;
};

template <class T,int Overhead> void LIST<T,Overhead>::operator =(const arrayptr<T> &ap){
    if (ap.len == (uint)Items && ap.ptr == Ptr)
        return;     // ensure that list=samelist works

    DeleteAll();
    for(uint i=0;i<ap.len;i++)
        *Append()=ap[i];
}

template <class T,int Overhead> int LIST<T,Overhead>::CalcOverhead(void) const {
    if(Items < 8) return Overhead<4?Overhead:4;
    if(Items < 32) return Overhead<8?Overhead:8;
    return Overhead;
}
            
template <class T,int Overhead> T *LIST<T,Overhead>::Add(int pos){
    if(Items>=ItemsAlloced){
        const int overhead=CalcOverhead();
        ResizePtr((void **)&Ptr,(Items+overhead)*sizeof(T));
        ItemsAlloced+=overhead;
    }
    memcpy(Ptr+pos+1,Ptr+pos,(Items-pos)*sizeof(T));
    Items++;
    T tmp;
    DoNothing(&tmp);
    memcpy(Ptr+pos,&tmp,sizeof(T));
    //Ptr[pos](tmp);
    return &Ptr[pos];
}

template <class T,int Overhead> void LIST<T,Overhead>::EnsureOverhead(int overhead)
{
    if(ItemsAlloced>=Items+overhead)
        return;

    ItemsAlloced=Items+overhead;
    ResizePtr((void **)&Ptr,ItemsAlloced*sizeof(T));
}
                        
template <class T,int Overhead> void LIST<T,Overhead>::Add(T const &elem,int pos){
    T *t=Add(pos);
    *t=elem;
}

template <class T,int Overhead> void LIST<T,Overhead>::Delete(int nr){
    _DestroyListItem(&Ptr[nr]);
    memcpy(Ptr+nr,Ptr+nr+1,(Items-nr-1)*sizeof(T));
    Items--;
    if(!Items) return;
    if(ItemsAlloced-Items>Overhead){
        ItemsAlloced-=(Overhead+1)/2;
        ResizePtr((void **)&Ptr,ItemsAlloced*sizeof(T));
    }
}

template <class T,int Overhead> void LIST<T,Overhead>::DeleteAll(void){
      for(int i=0;i<Items;i++)
        _DestroyListItem(&Ptr[i]);
    if (Ptr)
      FreePtr((void *)Ptr);
    Ptr=NULL;
    ItemsAlloced=Items=0;
}


#define ATTRTYPE_INTEGER	AttributeContainer::Attribute::TYPE_INTEGER
#define ATTRTYPE_INT64		AttributeContainer::Attribute::TYPE_INT64
#define ATTRTYPE_ADDRESS	AttributeContainer::Attribute::TYPE_ADDRESS
#define ATTRTYPE_STRING		AttributeContainer::Attribute::TYPE_STRING
#define ATTRTYPE_BINARY		AttributeContainer::Attribute::TYPE_BINARY
#define ATTRTYPE_CONTAINER	AttributeContainer::Attribute::TYPE_CONTAINER
#define ATTRTYPE_INTARRAY	AttributeContainer::Attribute::TYPE_INTARRAY

#pragma pack(4)


struct Address {
	uint IPAddress;
	uint Port;
	
	Address(void){}
	Address(uint ip,uint port):IPAddress(ip),Port(port){}
	bool operator ==(const Address &a) const {return IPAddress==a.IPAddress && Port==a.Port;}

};

struct MEMBLOCK {
  	void *Ptr;
  	int Len;
    void SetSize(int len);
    void Delete(void){SetSize(0);}
    MEMBLOCK(void){Ptr=NULL;Len=0;}
    ~MEMBLOCK(void){Delete();}
    operator uchar *(void){return (uchar *)Ptr;}
    int AppendMem(const void *mem,int len);
    void operator =(const MEMBLOCK &m){
        Delete();AppendMem(m.Ptr,m.Len);
    }
};

template <class T> int operator +=(MEMBLOCK &m,const T &l){
                return m.AppendMem(&l,sizeof(l));}
template <class T> int operator +=(MEMBLOCK &m,const LIST<T> &l){
                return m.AppendMem(&l[0],l.Size());}



class AttributeContainer {
	public:
	class Attribute {
		public:
		enum TYPE {TYPE_INTEGER,TYPE_INT64,TYPE_ADDRESS,TYPE_STRING,TYPE_BINARY,TYPE_CONTAINER,TYPE_INTARRAY};
		Attribute(uint key=0,TYPE type=TYPE_INTEGER,ulonglong value=0);
		Attribute(const Attribute &attr);
		~Attribute(void);
		Attribute &operator =(const Attribute &attr);

		protected:
		TYPE Type;
		uint Key;
		union {
			uint IntValue;
			ulonglong Int64Value;
			struct {
				void *Value;
				uint ValueLen;
			} AllocedValue;
			struct {
				uint IP;
				uint Port;
			} AddressValue;
		};

		void OldSerialize(void *buf) const;
		uint OldSerializeSize(void) const;
		bool OldDeserialize(const uchar *&data,uint &len_left,uint max_depth,uint &max_memory);
		void Clear(void);
		void Free(void);
		
		friend class AttributeContainer;

		public:
		TYPE GetType(void) const {return Type;}
		uint GetKey(void) const {return Key;}
		
		uint GetInteger(void) const {return IntValue;}
		ulonglong GetInt64(void) const {return Int64Value;}
		Address GetAddress(void) const {return Address(AddressValue.IP,AddressValue.Port);}
		const char *GetString(void) const {return (const char *)AllocedValue.Value;}
		const void *GetBinary(uint &len) const {len=AllocedValue.ValueLen;return AllocedValue.Value;}
		const AttributeContainer &GetContainer(void) const {return *(AttributeContainer*)AllocedValue.Value;}
		const uint *GetIntArray(uint &count) const {count=AllocedValue.ValueLen/sizeof(uint);return (uint *)AllocedValue.Value;}

		void ReplaceString(const char * const value,const uint max_len=NIL);
		void ReplaceInteger(const uint value);
		void ReplaceInt64(const ulonglong value);
		void ReplaceAddress(const Address value);
		void ReplaceBinary(const void * const value,const uint len);
		
		bool operator==(const Attribute &other) const; 
	};

	struct RequiredKeys {
		uint Key;
		Attribute::TYPE Type;
	};

	protected:
	LIST<Attribute> Attrs;

	bool DeserializeInternal(const uchar *&data,uint &len_left,uint max_depth,uint &max_memory);
	friend class Attribute;

	public:
	virtual bool IsValid(void) const {return true;}
	bool Validate(const RequiredKeys *req,uint items) const;
	virtual ~AttributeContainer(void);

	// functions for reading attributes
	uint AttributeCount(void) const {return Attrs.Items;}
	uint AttributeCount(uint key) const;
	const Attribute *GetAttribute(uint nr) const {return nr<(uint)Attrs.Items?&Attrs[nr]:NULL;}
	Attribute *GetAttribute(uint nr) {return nr<(uint)Attrs.Items?&Attrs[nr]:NULL;}
	const Attribute *FindAttribute(uint key,uint nr=0) const;
	Attribute *FindAttribute(uint key,uint nr=0);
	const Attribute *FindTypedAttribute(uint key,Attribute::TYPE type) const;
	Attribute *FindTypedAttribute(uint key,Attribute::TYPE type);
	bool AttributeExists(uint key,Attribute::TYPE type) const;

	uint GetInteger(uint key) const;
	ulonglong GetInt64(uint key) const;
	Address GetAddress(uint key) const;
	const char *GetString(uint key) const;
	const void *GetBinary(uint key,uint &len) const;
	const AttributeContainer &GetContainer(uint key) const;
	const uint *GetIntArray(uint key,uint &count) const;

	const char *GetOptString(uint key,const char *default_value=NULL) const;
	uint GetOptInteger(uint key,uint default_value=0) const;
	uint GetOptIntegerInRange(uint key,uint defval=0,uint minval=0,uint maxval=0xffffffffU) const;
	ulonglong GetOptInt64(uint key,ulonglong default_value=0,bool fallback_to_int=false) const;
	
	// functions for modifying attributes
	void Clear(void);
	Attribute &Add(const Attribute &attr);
	Attribute &AddInteger(uint key,uint value);
	Attribute &AddInt64(uint key,ulonglong value);
	Attribute &AddAddress(uint key,Address value);
	Attribute &AddString(uint key,const char *value,uint max_len=NIL);
	Attribute &AddBinary(uint key,const void *value,uint len);
	Attribute &AddIntArray(uint key,const uint *value,uint count);
	AttributeContainer &AddContainer(uint key,const AttributeContainer &value);
	AttributeContainer &AddContainer(uint key);
	void RemoveAttribute(uint key,uint nr=0);
	void RemoveAttribute(const Attribute *attr);

	// serialization
	uint SerializeSize() const;
	uint Serialize(void *buf,uint buf_len=NIL) const;
			// returns SerializeSize(), even if it's larger than buf_len
	void Serialize(MEMBLOCK &dest) const;

	bool Deserialize(const uchar *&data,uint &len_left,uint max_depth=8,uint max_memory=300*1024){
				return DeserializeInternal(data,len_left,max_depth,max_memory);}
	bool Deserialize(const MEMBLOCK &memblock,uint max_depth=8,uint max_memory=300*1024);
	
	bool operator==(const AttributeContainer &other) const;
};

#pragma pack()

#endif //_ATTRIBUTECONTAINER_HPP_INCLUDED
