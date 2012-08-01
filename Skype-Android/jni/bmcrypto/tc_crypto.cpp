#include "tc_crypto.hpp"

// find a binary attribute from container. If format is given, verify format. If len is also
// given, then verify len as well. If the attribute is not found, or either of verifications fail returns NULL
// if the attribute is found then a pointer to binary data is returned. if non-NULL lptr is given, then
// the actual length of binary data is stored at *lptr if the return value is non-NULL
//
uchar * tc_crypto::GetContainerBlob(AttributeContainer& c,uint key,uint *lptr,char *format,uint len)
{
const AttributeContainer::Attribute * const a=c.FindTypedAttribute(key,ATTRTYPE_BINARY);
uchar *ptr;
uint l;
	if (a == NULL)
			return NULL; 	// no private key in src attributecontainer
	ptr=(uchar *)a->GetBinary(l);
	if (len && len!=l)
		return NULL;
	if (l>2 && format && (ptr[0]!=format[0] || ptr[1]!=format[1]))
		return NULL;
	if (lptr)
		*lptr=l;
	return ptr;  
}

//
// generate a key pair for rootkey, and return it in attributecontainer
//
void tc_crypto::GenerateRootKey(AttributeContainer& dest)
{
rsa_crt_private_key<ROOTKEYSIZE> private_key;
const bignum<ROOTKEYSIZE> public_key=private_key.make(tc_random_func,NULL,5);
MEMBLOCK m;
	m.SetSize(private_key.get_export_size()+2);
	uchar *private_key_buf=(uchar *)m;
	private_key_buf[0]='R';
	private_key_buf[1]='P'; 
	private_key.export_as_bytes(private_key_buf+2);
	dest.AddBinary(ATTR_ROOT_PRIVATE_KEY,(const uchar *)m.Ptr,m.Len); 
	uchar big_endian_public_root_key[ROOTKEYSIZE/8+2];
	big_endian_public_root_key[0]='R';
	big_endian_public_root_key[1]='K';
	public_key.export_as_bytes(big_endian_public_root_key+2,ROOTKEYSIZE/8,1);
	dest.AddBinary(ATTR_ROOT_PUBLIC_KEY,big_endian_public_root_key,sizeof(big_endian_public_root_key)); 
}

// generate a key pair for server, sign it with root key, and return result in attributecontainer
// return value is false if signing key is not in valid format
//
bool tc_crypto::GenerateAndSignServerKey(AttributeContainer& dest,AttributeContainer& SigningKey)
{
rsa_crt_private_key<SERVERKEYSIZE> server_private_key;
const bignum<SERVERKEYSIZE> server_public_key=server_private_key.make(tc_random_func,NULL,5);
rsa_crt_private_key<ROOTKEYSIZE> private_key;
bignum<ROOTKEYSIZE> public_key;
uchar *ptr;
uint len;
	// extract private key from signingkey container
	ptr=GetContainerBlob(SigningKey,ATTR_ROOT_PRIVATE_KEY,&len,"RP");
	if (!ptr)
		return false; // no valid private key
	if (!private_key.import_from_bytes(ptr+2,len-2))
		return false; // invalid private key in src attributecontainer
	// extract public key from signingkey container
	ptr=GetContainerBlob(SigningKey,ATTR_ROOT_PUBLIC_KEY,NULL,"RK",ROOTKEYSIZE/8+2);
	if (!ptr)
		return false;
	public_key=bignum<ROOTKEYSIZE>(ptr+2,1); 
	// sign the public half of server key, and store the result
	uchar signing_block[ROOTKEYSIZE/8];
	uchar big_endian_public_server_key[SERVERKEYSIZE/8+2];
	server_public_key.export_as_bytes(big_endian_public_server_key+2,SERVERKEYSIZE/8,1);
	big_endian_public_server_key[0]='S';
	big_endian_public_server_key[1]='U';
	const uint remainder_len=signature_padding::encode(big_endian_public_server_key,SERVERKEYSIZE/8+2,ROOTKEYSIZE/8,signing_block);
	if (remainder_len) {
		// key does not fit into signature, should never happen if signature contains only the public key and no other data
		printf("key does not fit into signature!\n");
		return false;	
	}
	bignum<ROOTKEYSIZE> signing_block_bignum(signing_block,1);
	private_key.do_private_key_operation(signing_block_bignum,public_key);
	MEMBLOCK m;
	m.SetSize(server_private_key.get_export_size()+2);
	uchar * private_key_buf=(uchar *)m;
	private_key_buf[0]='S';
	private_key_buf[1]='P'; 
	server_private_key.export_as_bytes(private_key_buf+2);
	dest.AddBinary(ATTR_SERVER_PRIVATE_KEY,(const uchar *)m.Ptr,m.Len);
	uchar signed_public_key[ROOTKEYSIZE/8+2];
	signed_public_key[0]='S';
	signed_public_key[1]='K';
	signing_block_bignum.export_as_bytes(signed_public_key+2,ROOTKEYSIZE/8,1);
	dest.AddBinary(ATTR_SERVER_SIGNED_PUBLIC_KEY,signed_public_key,sizeof(signed_public_key)); 
	return true;
}

// verify signature of server key. signed server key is in src container, and
// public root key in VerificationKey container. If the validation fails, returns false.
// if validation succeeds, then the verified server key with signature stripped is inserted
// back into src container
//
bool tc_crypto::VerifyServerKey(AttributeContainer& src,AttributeContainer& VerificationKey)
{
rsa_crt_private_key<SERVERKEYSIZE> server_private_key;
bignum<ROOTKEYSIZE> public_key;
uchar *ptr;
	// get public half of root key from container
	ptr=GetContainerBlob(VerificationKey,ATTR_ROOT_PUBLIC_KEY,NULL,"RK",ROOTKEYSIZE/8+2);
	if (!ptr)
		return false;
	public_key=bignum<ROOTKEYSIZE>(ptr+2,1);
	// get signed server key from container and validate signature
	ptr=GetContainerBlob(src,ATTR_SERVER_SIGNED_PUBLIC_KEY,NULL,"SK",ROOTKEYSIZE/8+2);
	if (!ptr)
		return false;
	bignum<ROOTKEYSIZE> signed_block_bignum((const uchar *)ptr+2,1);
	// decrypt the data
	if (public_key.get_bit(ROOTKEYSIZE-1))
		signed_block_bignum.mod_exp(65537,public_key);
	else
		return false;
	uchar signed_block[ROOTKEYSIZE/8];
	signed_block_bignum.export_as_bytes(signed_block,ROOTKEYSIZE/8,1);
	uchar buf[ROOTKEYSIZE/8];
	uint public_key_len;
	// check the signature and strip padding
	if (!signature_padding::decode(signed_block,ROOTKEYSIZE/8,NULL,0,buf,public_key_len))
		return false;	// invalid signature
	if (public_key_len != SERVERKEYSIZE/8+2 || buf[0]!='S' || buf[1]!='U')
		return false;	// invalid signature
	src.AddBinary(ATTR_SERVER_PUBLIC_KEY,buf,public_key_len); // insert verified and stripped server key back into source container
	return true;
}

// encrypt 20 bytes using servers public key. The result is stored in buffer. flase is returned if encryption key
// is invalid.
//
bool tc_crypto::EncryptSecret(const uchar data[20],uchar encrypted_data[SERVERKEYSIZE/8+2],AttributeContainer& EncryptionKey)
{
bignum<SERVERKEYSIZE> server_public_key;
uchar *ptr;
	// extract servers public key
	ptr=GetContainerBlob(EncryptionKey,ATTR_SERVER_PUBLIC_KEY,NULL,"SU",SERVERKEYSIZE/8+2);
	if (!ptr)
		return false;
	server_public_key=bignum<SERVERKEYSIZE>((const uchar *)ptr+2,1); 
	// pad and encrypt the data
	const uint remainder_len=signature_padding::encode(data,20,SERVERKEYSIZE/8,encrypted_data);
	if (remainder_len) {
		// should never happen if data[] is just 20 bytes and server key is at least 336 bits
		printf("Secret is too big for server key\n");
		return false;	
	}
	bignum<SERVERKEYSIZE> block_bignum(encrypted_data,1);
	if (server_public_key.get_bit(SERVERKEYSIZE-1))
		block_bignum.mod_exp(65537,server_public_key);
	else 
		return false;
	block_bignum.export_as_bytes(encrypted_data+2,SERVERKEYSIZE/8,1);
	encrypted_data[0]='P';
	encrypted_data[1]='S';
	return true;
}

// decrypt 20 byte payload that was encrypted with servers public key. returns false if the decryption key is invalid
//
bool tc_crypto::DecryptSecret(const uchar encrypted_data[SERVERKEYSIZE/8+2],uchar data[20],AttributeContainer& DecryptionKey)
{
bignum<SERVERKEYSIZE> public_key;
rsa_crt_private_key<SERVERKEYSIZE> server_private_key;
uchar *ptr;
uint len;
	// extract servers private key from container
	ptr=GetContainerBlob(DecryptionKey,ATTR_SERVER_PRIVATE_KEY,&len,"SP");
	if (!ptr)
		return false;	
	if (!server_private_key.import_from_bytes(ptr+2,len-2))
		return false; // invalid private key in src attributecontainer
	// extract servers public key from container
	ptr=GetContainerBlob(DecryptionKey,ATTR_SERVER_PUBLIC_KEY,NULL,"SU",SERVERKEYSIZE/8+2);
	if (!ptr)
		return false;
	public_key=bignum<SERVERKEYSIZE>(ptr+2,1); 
	if (!public_key.get_bit(SERVERKEYSIZE-1))
		return false;
	// decrypt data and strip padding
	if (encrypted_data[0]!='P' || encrypted_data[1]!='S')
		return false;
	bignum<SERVERKEYSIZE> block_bignum(encrypted_data+2,1);
	server_private_key.do_private_key_operation(block_bignum,public_key);
	uchar buf[SERVERKEYSIZE/8];
	uint datalen;
	uchar block[SERVERKEYSIZE/8];
	block_bignum.export_as_bytes(block,SERVERKEYSIZE/8,1);
	if (!signature_padding::decode(block,SERVERKEYSIZE/8,NULL,0,buf,datalen))
		return false;
	if (datalen!=20)
		return false;
	memcpy(data,buf,20);
	return true;
}

// load keyset from file. if passphrase is non-NULL, then the file is decrypted using passphrase as RC4 key
// a valid keyset must contain at least ATTR_ROOT_PUBLIC_KEY or ATTR_SERVER_SIGNED_PUBLIC_KEY attribute
// true is retuned if keys were successfully loaded
// decryption result validation relies on attributecontainer deserializer
//
bool tc_crypto::LoadKeys(AttributeContainer& container,char* filename,char* passphrase)
{
bool rv=false;
	FILE *fp;
	fp=fopen(filename,"rb");
	if (fp) {
		container.Clear();
		fseek(fp,0,SEEK_END);
		uint ln=ftell(fp);
		rewind(fp);
		MEMBLOCK m;
		m.SetSize(ln);
		if (fread(m.Ptr,ln,1,fp))
		{
			if (passphrase) {
				ARC4 rc4;
				rc4.SetKey((const uchar*)passphrase,strlen(passphrase));
				for (int i=0;i<768;i++)
					rc4.GenerateByte();
				rc4.ProcessString((uchar *)m.Ptr,ln);
			}
			AttributeContainer c;
			if (c.Deserialize(m)) {
				if (GetContainerBlob(c,ATTR_ROOT_PUBLIC_KEY,NULL,"RK",ROOTKEYSIZE/8+2) ||
					GetContainerBlob(c,ATTR_SERVER_SIGNED_PUBLIC_KEY,NULL,"SK",ROOTKEYSIZE/8+2)) {
					container=c;
					rv=true;
				}
			}
		}
		fclose(fp);
	}
	return rv;
}

// store keyset into file. if passphrase is non-NULL, then the file is encrypted with RC4,
// using passphrase as key. returns true if everything is ok. false means that keys were not
// stored. Existing key file may be destroyed on failure.
//
bool tc_crypto::SaveKeys(AttributeContainer& container,char *filename,char* passphrase)
{
bool rv=false;
	FILE *fp;
	fp=fopen(filename,"wb");
	if (fp)
	{
		MEMBLOCK m;
		container.Serialize(m);
		if (passphrase) {
			ARC4 rc4;
			rc4.SetKey((const uchar*)passphrase,strlen(passphrase));
			for (int i=0;i<768;i++)
				rc4.GenerateByte();
			rc4.ProcessString((uchar *)m.Ptr,m.Len);
		}
		rv=fwrite(m.Ptr,m.Len,1,fp)==1;
		fclose(fp);
	}
	if (rv)
		printf("Saved keys to %s\n",filename);
	else
		printf("Failed to save keys to %s\n",filename);
	return rv;
}


