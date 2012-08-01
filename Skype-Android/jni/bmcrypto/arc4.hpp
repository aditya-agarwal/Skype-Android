#ifndef __ARC4_HPP
#define __ARC4_HPP

#include "def.h"

class ARC4 {
public:
	void SetKey(const uchar *key, uint keylen){
		m_x=0;
		m_y=0;

		uint i;
		for(i=0;i<256;i++)
			m_state[i]=i;

		uint keyIndex=0, stateIndex=0;

		for (i=0;i<256;i++){
			uint a = m_state[i];
			stateIndex += key[keyIndex] + a;
			stateIndex &= 0xff;
			m_state[i] = m_state[stateIndex];
			m_state[stateIndex] = a;
			if (++keyIndex >= keylen)
				keyIndex = 0;
		}
	}

    ARC4(void){}
    ARC4(const uchar *key, uint keylen){
		SetKey(key,keylen);
	}
    ~ARC4(void){
		m_x=0;
		m_y=0;
	}

	uchar GenerateByte(){
		m_x = (m_x+1) & 0xff;
		uint a = m_state[m_x];
		m_y = (m_y+a) & 0xff;
		uint b = m_state[m_y];
		m_state[m_x] = b;
		m_state[m_y] = a;
		return m_state[(a+b) & 0xff];
	}

	uchar ProcessByte(uchar input){
		return input ^ GenerateByte();
	}

	void ProcessString(uchar *outString, const uchar *inString, uint length){
		uchar *const s=m_state;
		uint x = m_x;
		uint y = m_y;
		while(length--){
			x = (x+1) & 0xff;
			uint a = s[x];
			y = (y+a) & 0xff;
			uint b = s[y];
			s[x] = b;
			s[y] = a;
			*outString++ = *inString++ ^ s[(a+b) & 0xff];
		}
		m_x = x;
		m_y = y;
	}

	void ProcessString(uchar *inoutString, uint length){
		uchar *const s=m_state;
		uint x = m_x;
		uint y = m_y;
		while(length--)	{
			x = (x+1) & 0xff;
			uint a = s[x];
			y = (y+a) & 0xff;
			uint b = s[y];
			s[x] = b;
			s[y] = a;
			*inoutString++ ^= s[(a+b) & 0xff];
		}
		m_x = x;
		m_y = y;
	}

	static bool IsWeakKey(const uchar *key, uint keylen){
		return keylen>1 && (key[0]+key[1])%256==0;
	}

private:
    uchar m_state[256];
    uchar m_x, m_y;
};

#endif
