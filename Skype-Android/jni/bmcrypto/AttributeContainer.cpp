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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "AttributeContainer.hpp"
#include <android/log.h>

#define CheckMemory(a) if(memory_left<(a)) return false; memory_left-=(a);

void FatalError(char const *format,...)
{ 
        va_list ap;
        va_start(ap,format);
        vprintf(format,ap);
        va_end(ap);
        printf("\n");
        exit(0); 
}

void ResizePtr(void **ptr,uint len){
    if(*ptr)
    {
      uchar *p;
      p=new uchar[len];
      if (p)
        memcpy(p,(uchar *)*ptr,len);
      delete (uchar *)*ptr;
      *ptr=p;
    }
    else 
      *ptr=new uchar[len];
    if(!*ptr)
      FatalError("Out of memory, needed %u bytes",len);
}

void FreePtr(void *ptr){
    if (ptr)
      delete (uchar *)ptr;
}
            
void MEMBLOCK::SetSize(int len){
    if(!len){
        FreePtr(Ptr);
        Ptr=NULL;
    } else
        ResizePtr(&Ptr,len);
    Len=len;
}

int MEMBLOCK::AppendMem(const void *mem,int len){
    SetSize(Len+len);
    memcpy((uchar*)Ptr+Len-len,mem,len);
    return Len-len;
}
                                    

uint AttributeContainer::AttributeCount(uint key) const {
	uint result(0);
	for(int i=0;i<Attrs.Items;i++)
		if(Attrs[i].Key==key)
			++result;
	return result;
}

const AttributeContainer::Attribute *AttributeContainer::FindAttribute(uint key,uint nr) const {
	for(int i=0;i<Attrs.Items;i++)
		if(Attrs[i].Key==key)
			if(!nr--) return &Attrs[i];
	return NULL;
}

AttributeContainer::Attribute *AttributeContainer::FindAttribute(uint key,uint nr) {
	for(int i=0;i<Attrs.Items;i++)
		if(Attrs[i].Key==key)
			if(!nr--) return &Attrs[i];
	return NULL;
}

const AttributeContainer::Attribute *AttributeContainer::FindTypedAttribute(uint key,Attribute::TYPE type) const {
	const AttributeContainer::Attribute * const attr=FindAttribute(key);
	if (attr == NULL)
		return NULL;

	if (attr->GetType() != type)
		return NULL;

	return attr;
}

AttributeContainer::Attribute *AttributeContainer::FindTypedAttribute(uint key,Attribute::TYPE type) {
	AttributeContainer::Attribute * const attr=FindAttribute(key);
	if (attr == NULL)
		return NULL;

	if (attr->GetType() != type)
		return NULL;

	return attr;
}

bool AttributeContainer::AttributeExists(uint key,Attribute::TYPE type) const {
	const AttributeContainer::Attribute *attr=FindAttribute(key);
	return attr && attr->GetType()==type;
}										 

uint AttributeContainer::GetInteger(uint key) const {
	const AttributeContainer::Attribute *attr=FindAttribute(key);
	if(!attr)  FatalError("AttributeContainer: Expected key %u not present",key);
	if(attr->GetType()!=attr->TYPE_INTEGER) 
		FatalError("AttributeContainer: Invalid key %u type %u expected %u",key,attr->GetType(),attr->TYPE_INTEGER);
	return attr->GetInteger();
}

ulonglong AttributeContainer::GetInt64(uint key) const {
	const AttributeContainer::Attribute *attr=FindAttribute(key);
	if(!attr) FatalError("AttributeContainer: Expected key %u not present",key);
	if(attr->GetType()!=attr->TYPE_INT64) 
		FatalError("AttributeContainer: Invalid key %u type %u expected %u",key,attr->GetType(),attr->TYPE_INT64);
	return attr->GetInt64();
}

uint AttributeContainer::GetOptInteger(uint key,uint default_value) const {
	const AttributeContainer::Attribute *attr=FindAttribute(key);
	if (attr == NULL)
		return default_value;
	if (attr->GetType() != Attribute::TYPE_INTEGER) 
		return default_value;
	return attr->GetInteger();
}

uint AttributeContainer::GetOptIntegerInRange(uint key,uint defval,uint minval,uint maxval) const {
	const AttributeContainer::Attribute *attr=FindAttribute(key);
	if (attr == NULL || attr->GetType() != Attribute::TYPE_INTEGER)
		return defval;
	uint rt=attr->GetInteger();
	if(rt<minval) return minval;
	if(rt>maxval) return maxval;
	return rt;
}

ulonglong AttributeContainer::GetOptInt64(uint key,ulonglong default_value,bool fallback_to_int) const {
	const AttributeContainer::Attribute *attr=FindAttribute(key);
	if (attr == NULL)
		return default_value;
	if (attr->GetType() == Attribute::TYPE_INT64)
		return attr->GetInt64();
	if (fallback_to_int && attr->GetType() == Attribute::TYPE_INTEGER)
		return attr->GetInteger();
	return default_value;
}

Address AttributeContainer::GetAddress(uint key) const 
{
	const AttributeContainer::Attribute *attr=FindAttribute(key);
	if(!attr) FatalError("AttributeContainer: Expected key %u not present",key);
	if(attr->GetType()!=attr->TYPE_ADDRESS) 
		FatalError("AttributeContainer: Invalid key %u type %u expected %u",key,attr->GetType(),attr->TYPE_ADDRESS);
	return attr->GetAddress();
}

const char *AttributeContainer::GetString(uint key) const 
{
	const AttributeContainer::Attribute *attr=FindAttribute(key);
	if(!attr)
	  return NULL;
	if(attr->GetType()!=attr->TYPE_STRING) 
		FatalError("AttributeContainer: Invalid key %u type %u expected %u",key,attr->GetType(),attr->TYPE_STRING);
	return attr->GetString();
}

const char *AttributeContainer::GetOptString(uint key,const char *default_value) const 
{
	const AttributeContainer::Attribute *attr=FindAttribute(key);
	if (attr == NULL)
		return default_value;
	if (attr->GetType() != Attribute::TYPE_STRING) 
		return default_value;
	return attr->GetString();
}

const void *AttributeContainer::GetBinary(uint key,uint &len) const 
{
	const AttributeContainer::Attribute *attr=FindAttribute(key);
	if(!attr)
	  return NULL;
	if(attr->GetType()!=attr->TYPE_BINARY) 
		FatalError("AttributeContainer: Invalid key %u type %u expected %u",key,attr->GetType(),attr->TYPE_BINARY);
	return attr->GetBinary(len);
}

const AttributeContainer &AttributeContainer::GetContainer(uint key) const 
{
	const AttributeContainer::Attribute *attr=FindAttribute(key);
	if(!attr) FatalError("AttributeContainer: Expected key %u not present",key);
	if(attr->GetType()!=attr->TYPE_CONTAINER) 
		FatalError("AttributeContainer: Invalid key %u type %u expected %u",key,attr->GetType(),attr->TYPE_CONTAINER);
	return attr->GetContainer();
}

const uint *AttributeContainer::GetIntArray(uint key,uint &count) const 
{
	const AttributeContainer::Attribute *attr=FindAttribute(key);
	if(!attr)
	  return NULL;
	if(attr->GetType()!=attr->TYPE_INTARRAY) 
		FatalError("AttributeContainer: Invalid key %u type %u expected %u",key,attr->GetType(),attr->TYPE_INTARRAY);
	return attr->GetIntArray(count);
}

AttributeContainer::~AttributeContainer(void)
{
}

void AttributeContainer::Clear(void)
{
	Attrs.DeleteAll();
}

AttributeContainer::Attribute &AttributeContainer::Add(const Attribute &attr)
{
	Attrs.Append(attr);
	return Attrs[Attrs.Items-1];
}

AttributeContainer::Attribute &AttributeContainer::AddInteger(uint key,uint value)
{
	Attribute *attr=Attrs.Append();
	attr->Type=attr->TYPE_INTEGER;
	attr->Key=key;
	attr->IntValue=value;
	return *attr;
}

AttributeContainer::Attribute &AttributeContainer::AddInt64(uint key,ulonglong value)
{
	Attribute *attr=Attrs.Append();
	attr->Type=attr->TYPE_INT64;
	attr->Key=key;
	attr->Int64Value=value;
	return *attr;
}

AttributeContainer::Attribute &AttributeContainer::AddAddress(uint key,Address value)
{
	Attribute *attr=Attrs.Append();
	attr->Type=attr->TYPE_ADDRESS;
	attr->Key=key;
	attr->AddressValue.IP=value.IPAddress;
	attr->AddressValue.Port=value.Port;
	return *attr;
}

AttributeContainer::Attribute &AttributeContainer::AddString(uint key,const char *value,uint max_len)
{
	Attribute *attr=Attrs.Append();
	attr->Type=attr->TYPE_STRING;
	attr->Key=key;

	attr->AllocedValue.ValueLen=max_len + 1;

	if (max_len) {
		attr->AllocedValue.ValueLen=strlen(value)+1;
		//const void * const string_end=memchr(value,'\0',max_len);
		//if (string_end != NULL)
		//	attr->AllocedValue.ValueLen=((const char *)string_end) - value + 1;
		}

	attr->AllocedValue.Value=new char[attr->AllocedValue.ValueLen];
	memcpy(attr->AllocedValue.Value,value,attr->AllocedValue.ValueLen);
	((char *)attr->AllocedValue.Value)[attr->AllocedValue.ValueLen-1]='\0';
	return *attr;
}

AttributeContainer::Attribute &AttributeContainer::AddBinary(uint key,const void *value,uint len)
{
	Attribute *attr=Attrs.Append();
	attr->Type=attr->TYPE_BINARY;
	attr->Key=key;
	attr->AllocedValue.ValueLen=len;
	attr->AllocedValue.Value=new char[len];
	memcpy(attr->AllocedValue.Value,value,len);
	return *attr;
}

AttributeContainer::Attribute &AttributeContainer::AddIntArray(uint key,const uint *value,uint count)
{
	Attribute *attr=Attrs.Append();
	attr->Type=attr->TYPE_INTARRAY;
	attr->Key=key;
	attr->AllocedValue.ValueLen=sizeof(uint)*count;
	uint *ptr=new uint[count];
	attr->AllocedValue.Value=ptr;
	for(uint i=0;i<count;i++)
		ptr[i]=value[i];
	return *attr;
}

AttributeContainer &AttributeContainer::AddContainer(uint key,const AttributeContainer &value)
{
	AttributeContainer &a=AddContainer(key);
	a=value;
	return a;
}

AttributeContainer &AttributeContainer::AddContainer(uint key)
{
	Attribute *attr=Attrs.Append();
	attr->Type=attr->TYPE_CONTAINER;
	attr->Key=key;
	AttributeContainer *c=new AttributeContainer;
	attr->AllocedValue.Value=c;
	return *c;
}

void AttributeContainer::RemoveAttribute(uint key,uint nr) {
	for(int i=0;i<Attrs.Items;i++)
		if(Attrs[i].Key==key)
			if(!nr--) {
				Attrs.Delete(i);
				break;
			}
}

void AttributeContainer::RemoveAttribute(const Attribute *attr) {
	for(int i=0;i<Attrs.Items;i++) {
		if(&Attrs[i]==attr) {
			Attrs.Delete(i);
			break;
        }
    }
}

/*
serialized format:
	signature 'A' 1 byte (used for versioning
	varlen addribute count
	count*(
		type 1 byte
		varlen key;
		varlen intval | address 6 bytes | varlen len, data | asciiz string | serialized container
	)
*/

uint EncodedUintSize(uint value)
{
	uint size;
	for(size=1;value>0x7f;value>>=7,size++)
		;
	return size;
}

void EncodeUint(uchar *&buf,uint value)
{
	for(;value>0x7f;value>>=7)
		*buf++=(value&0x7f)|0x80;
	*buf++=value;
}

bool DecodeUint(uint &val,const uchar *&buf,uint &len_left)
{
	val=0;
	uchar c;
	uint shift=0;
	do {
		if(!len_left--) return false;
		c=*buf++;
		val|=((uint)c&0x7f)<<shift;
		shift+=7;
	} while(c&0x80);
	return true;
}

uint AttributeContainer::SerializeSize() const 
{
	uint total_size=1+EncodedUintSize(Attrs.Items);
	for(int i=0;i<Attrs.Items;i++)
		total_size+=Attrs[i].OldSerializeSize();
	return total_size;
}

uint AttributeContainer::Serialize(void *buf,uint buf_len) const 
{
	if(buf_len < NIL){
		const uint size=SerializeSize();
		if(buf_len < size)
			return size;
	}
	uchar *dest=(uchar *)buf;
	*dest++='A';
	EncodeUint(dest,Attrs.Items);
	for(int i=0;i<Attrs.Items;i++){
		Attrs[i].OldSerialize(dest);
		dest+=Attrs[i].OldSerializeSize();
	}
	return dest - (uchar *)buf;
}	

void AttributeContainer::Serialize(MEMBLOCK &dest) const 
{
	dest.SetSize(SerializeSize());
	Serialize(dest.Ptr);
}

bool AttributeContainer::Deserialize(const MEMBLOCK &memblock,uint max_depth,uint max_memory)
{
	const uchar *data=(const uchar *)memblock.Ptr;
	uint len=memblock.Len;
	return DeserializeInternal(data,len,max_depth,max_memory);
}

bool AttributeContainer::DeserializeInternal(const uchar *&data,uint &len_left,uint max_depth,uint &memory_left)
{
	if(!max_depth) return false;
	if(!len_left) return false;
	if(*data!='A')
	    return false;
	len_left--;
	data++;
	uint attrs;
	if(!DecodeUint(attrs,data,len_left)) return false;
	Attrs.EnsureOverhead ((attrs<100)?attrs:100);
	for(uint i=0;i<attrs;i++)
	{
		CheckMemory(sizeof(Attribute));
		Attribute *a=Attrs.Append();
		if(!a->OldDeserialize(data,len_left,max_depth,memory_left)) return false;
	}
	return true;
}

bool AttributeContainer::Validate(const RequiredKeys *req,uint items) const 
{
	for(uint i=0;i<items;i++,req++){
		const Attribute *attr=FindAttribute(req->Key);
		if(!attr || attr->GetType()!=req->Type)
			return false;
	}
	return true;
}

/* ====================================================================
 * AttributeContainer::Attribute functions
 *   
 */

AttributeContainer::Attribute::Attribute(uint key,TYPE type,ulonglong value)
{
	Clear();
	Type=type;
	Key=key;
	if(type==TYPE_INTEGER) IntValue=(uint)value;
	else if(type==TYPE_INT64) Int64Value=value;
}

void AttributeContainer::Attribute::Clear(void)
{
	Type=TYPE_INTEGER;
	Key=0;
	IntValue=(uint)0;
}


AttributeContainer::Attribute &AttributeContainer::Attribute::operator =(const Attribute &attr)
{
	Free();
	Clear();
	Type=attr.Type;
	Key=attr.Key;
	if(Type==TYPE_INTEGER)
		IntValue=attr.IntValue;
	else if(Type==TYPE_INT64)
		Int64Value=attr.Int64Value;
	else if(Type==TYPE_ADDRESS)
		AddressValue=attr.AddressValue;
	else if(Type==TYPE_STRING || Type==TYPE_BINARY || Type==TYPE_INTARRAY){
		AllocedValue.Value=new char[AllocedValue.ValueLen=attr.AllocedValue.ValueLen];
		memcpy(AllocedValue.Value,attr.AllocedValue.Value,AllocedValue.ValueLen);
	} else if(Type==TYPE_CONTAINER){
		AllocedValue.Value=new AttributeContainer(attr.GetContainer());
	}
	return *this;
}

AttributeContainer::Attribute::Attribute(const Attribute &attr)
{
	Clear();
	*this=attr;
}

void AttributeContainer::Attribute::Free(void)
{
	if(Type==TYPE_STRING || Type==TYPE_BINARY || Type==TYPE_INTARRAY)
		delete [] (char *)AllocedValue.Value;
	else if(Type==TYPE_CONTAINER)
		delete (AttributeContainer*)AllocedValue.Value;
}

AttributeContainer::Attribute::~Attribute(void)
{
	Free();
}

void AttributeContainer::Attribute::ReplaceInteger(const uint value)
{
	Free();
	Type=TYPE_INTEGER;
	IntValue=value;
}

void AttributeContainer::Attribute::ReplaceInt64(const ulonglong value)
{
	Free();
	Type=TYPE_INT64;
	Int64Value=value;
}

void AttributeContainer::Attribute::ReplaceAddress(const Address value)
{
	Free();
	Type=TYPE_ADDRESS;
	AddressValue.IP=value.IPAddress;
	AddressValue.Port=value.Port;
}

void AttributeContainer::Attribute::ReplaceBinary(const void * const value,const uint len)
{
	Free();
	Type=TYPE_BINARY;
	AllocedValue.ValueLen=len;
	AllocedValue.Value=new char[len];
	memcpy(AllocedValue.Value,value,len);
}

void AttributeContainer::Attribute::ReplaceString(const char * const value,const uint max_len)
{
	Free();
	Type=TYPE_STRING;
	AllocedValue.ValueLen=max_len + 1;
	if(max_len) {
		const void * const string_end=memchr(value,'\0',max_len);
		if(string_end != NULL)
			AllocedValue.ValueLen=((const char *)string_end) - value + 1;
	}
	AllocedValue.Value=new char[AllocedValue.ValueLen];
	memcpy(AllocedValue.Value,value,AllocedValue.ValueLen);
	((char *)AllocedValue.Value)[AllocedValue.ValueLen-1]='\0';
}

void AttributeContainer::Attribute::OldSerialize(void *buf) const {
	uchar *dest=(uchar *)buf;
	*dest++=Type;
	EncodeUint(dest,Key);
	if(Type==TYPE_INTEGER)
		EncodeUint(dest,IntValue);
	else if(Type==TYPE_INT64){
		for(int i=64-8;i>=0;i-=8)
			*dest++=(uchar)(Int64Value>>i);
	} 
	else if(Type==TYPE_ADDRESS) {
	    dest[0]=AddressValue.IP>>24;
	    dest[1]=(AddressValue.IP>>16)&0xff;
	    dest[2]=(AddressValue.IP>>8)&0xff;
	    dest[3]=AddressValue.IP&0xff;
	    dest[4]=(AddressValue.Port>>8)&0xff;
	    dest[5]=AddressValue.Port&0xff;
	} 
	else if(Type==TYPE_STRING)
		memcpy(dest,AllocedValue.Value,AllocedValue.ValueLen);
	else if(Type==TYPE_BINARY) {
		EncodeUint(dest,AllocedValue.ValueLen);
		memcpy(dest,AllocedValue.Value,AllocedValue.ValueLen);
	} 
	else if(Type==TYPE_CONTAINER)
		GetContainer().Serialize(dest);
	else if(Type==TYPE_INTARRAY) {
		const uint cnt=AllocedValue.ValueLen/sizeof(uint);
		EncodeUint(dest,cnt);
		const uint *ptr=(uint *)AllocedValue.Value;
		for(uint i=0;i<cnt;i++)
			EncodeUint(dest,ptr[i]);
	}
}

uint AttributeContainer::Attribute::OldSerializeSize(void) const 
{
	uint size=1+EncodedUintSize(Key);
	if(Type==TYPE_INTEGER)
		size+=EncodedUintSize(IntValue);
	else if(Type==TYPE_INT64)
		size+=64/8;
	else if(Type==TYPE_ADDRESS)
		size+=6;
	else if(Type==TYPE_STRING)
		size+=AllocedValue.ValueLen;
	else if(Type==TYPE_BINARY)
		size+=EncodedUintSize(AllocedValue.ValueLen)+AllocedValue.ValueLen;
	else if(Type==TYPE_CONTAINER)
		size+=GetContainer().SerializeSize();
	else if(Type==TYPE_INTARRAY) {
		const uint cnt=AllocedValue.ValueLen/sizeof(uint);
		const uint *ptr=(uint *)AllocedValue.Value;
		size+=EncodedUintSize(cnt);
		for(uint i=0;i<cnt;i++)
			size+=EncodedUintSize(ptr[i]);
	}
	return size;
}

bool AttributeContainer::Attribute::OldDeserialize(const uchar *&data,uint &len_left,uint max_depth,uint &memory_left)
{
	if(!len_left--) 
	    return false;
	Type=(TYPE)*data++;
	AllocedValue.Value=NULL;
	if(!DecodeUint(Key,data,len_left)) 
	    return false;
	if(Type==TYPE_INTEGER) {
		if(!DecodeUint(IntValue,data,len_left)) 
		    return false;
	} 
	else if(Type==TYPE_INT64) {
		if(len_left<64/8) 
		    return false;
		Int64Value=0;
		for(int i=64-8;i>=0;i-=8)
			Int64Value|=((ulonglong)(*data++))<<i;
		len_left-=64/8;
	} 
	else if(Type==TYPE_ADDRESS) {
		if(len_left<6) return false;
		AddressValue.IP=(data[0]<<24)|(data[1]<<16)|(data[2]<<8)|data[3];
		AddressValue.Port=(data[4]<<8)|data[5];
		data+=6;
		len_left-=6;
	} 
	else if(Type==TYPE_STRING)
	{
		const uchar *end=(const uchar *)memchr(data,'\0',len_left);
		if(!end) return false;
		AllocedValue.ValueLen=end+1-data;
		CheckMemory(AllocedValue.ValueLen);
		AllocedValue.Value=new char[AllocedValue.ValueLen];
		memcpy(AllocedValue.Value,data,AllocedValue.ValueLen);
		data+=AllocedValue.ValueLen;
		len_left-=AllocedValue.ValueLen;
	} 
	else if(Type==TYPE_BINARY)
	{
		if(!DecodeUint(AllocedValue.ValueLen,data,len_left) || len_left<AllocedValue.ValueLen) 
			return false;
		CheckMemory(AllocedValue.ValueLen);
		AllocedValue.Value=new char[AllocedValue.ValueLen];
		memcpy(AllocedValue.Value,data,AllocedValue.ValueLen);
		data+=AllocedValue.ValueLen;
		len_left-=AllocedValue.ValueLen;
	} 
	else if(Type==TYPE_CONTAINER)
	{
		CheckMemory(sizeof(AttributeContainer));
		AttributeContainer *c=new AttributeContainer;
		AllocedValue.Value=c;
		return c->DeserializeInternal(data,len_left,max_depth-1,memory_left);
	} 
	else if(Type==TYPE_INTARRAY)
	{
		uint cnt;
		if(!DecodeUint(cnt,data,len_left) || cnt>len_left || cnt>=0x40000000U) 
			return false;
		AllocedValue.ValueLen=cnt*sizeof(uint);
		CheckMemory(AllocedValue.ValueLen);
		AllocedValue.Value=new char[AllocedValue.ValueLen];
		uint *ptr=(uint *)AllocedValue.Value;
		for(uint i=0;i<cnt;i++)
			if(!DecodeUint(ptr[i],data,len_left)) return false;
	} else 
		return false;
	return true;
}

bool AttributeContainer::operator==(const AttributeContainer &other) const 
{
MEMBLOCK tmp1,tmp2;
    Serialize(tmp1);
    other.Serialize(tmp2);
    return tmp1.Len == tmp2.Len && !memcmp(tmp1.Ptr,tmp2.Ptr,tmp1.Len);
}

bool AttributeContainer::Attribute::operator==(const AttributeContainer::Attribute &other) const 
{
    if(Key != other.Key || Type != other.Type) 
        return false;
    switch(Type){
        case TYPE_INTEGER: 
            return IntValue == other.IntValue;
        case TYPE_INT64:   
            return Int64Value == other.Int64Value;
        case TYPE_ADDRESS: 
            return GetAddress() == other.GetAddress();
        case TYPE_STRING:  
        case TYPE_BINARY:
        case TYPE_INTARRAY:
            return AllocedValue.ValueLen == other.AllocedValue.ValueLen &&
                !memcmp(AllocedValue.Value,other.AllocedValue.Value,AllocedValue.ValueLen);
        case TYPE_CONTAINER:
            return GetContainer() == other.GetContainer();
    }
    return false;
}
