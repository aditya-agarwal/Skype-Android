#ifndef TRSA_HPP_INCLUDED
#define TRSA_HPP_INCLUDED

/*
This module offers two layers of functionality:

* bignum<BITS> class with optimized large-integer arithmetics functions,
	including modular exponentiation that forms the basis of RSA and
	Diffie-Hellman algorithms

* RSA functionality built on top of bignum<BITS>. It has key generation
	and private-key base operations, but not any padding functionality.
	Thus it is unsafe if used directly, and additional padding should
	be applied by the user for proper usage.

RSA public-key operation is simple enough to be implemented directly with
	bignum<BITS> class, for example as follows:

	uchar data_big_endian[2048/8]={...};
	const uchar public_key_big_endian[2048/8]={...};

	bignum<2048> data_bignum(data_big_endian,1);
	const bignum<2048> public_key_bignum(public_key_big_endian,1);

	data_bignum.mod_exp(65537,public_key_bignum);

	data_bignum.export_as_bytes(data_big_endian,2048/8);

RSA key generation and private key operations (using public exponent 65537)
	are done with rsa_crt_private_key<BITS> class, for example as follows:

	rsa_crt_private_key<2048> private_key;
	const bignum<2048> public_key=private_key.make(my_random_func,NULL,5);

	bignum<2048> data(...);
	private_key.do_private_key_operation(data,public_key);

	After this, the corresponding public-key operation
	( data.mod_exp(65537,public_key) ) should yield the same plaintext
	as before the private-key operation.
	
	If private key storage is needed, it can be implemented using
	get_export_size(), export_as_bytes(), import_from_bytes() functions.
*/

#include "bignum-funcs.hpp"

template <int KEY_BITS> class bignum : public bignum_funcs {

	element_type m[(KEY_BITS/8) / sizeof(element_type)];		// [N]

	friend class bignum<KEY_BITS - ELEMENT_BITS>;
	friend class bignum<KEY_BITS/2>;

	public:

	bignum(void) throw() {};
	bignum(const element_type d) throw();
	bignum(const uchar * const src,const uint use_big_endian=0) throw();
								// src must have KEY_BITS/8 uchar's

	void export_as_bytes(uchar *dest,const uint nr_of_bytes,
								const uint use_big_endian=0) const throw();

	element_type & operator [] (const uint idx) throw() { return m[idx]; }

	uint elements(void) const throw();

	element_type operator % (const element_type v) const throw();
	void operator /= (const element_type v) throw();
	void operator *= (const element_type v) throw();

	void set_mod(bignum<KEY_BITS*2> &x,const bignum &modulus) throw()
			// the high bit of modulus must be set
			// x < (modulus<<KEY_BITS)
			// garbles x
		{ mod_div(lenof(m),m,x.m,modulus.m); }

	void mod_mult(const bignum &b,const bignum &modulus) throw();
	void mod_square(const bignum &modulus) throw();
	void mod_exp(const bignum &exponent,const bignum &modulus) throw();
		// the high bit of modulus must be set
		// *this < modulus

	uint mod_inverse(bignum v) throw();
		// sets *this to (v**-1) mod (*this)
		// returns zero if implementation could not handle such input values
		// *this > 1
		// v > 0
		// v < *this

	bignum & operator += (element_type b) throw();
	bignum & operator -= (element_type b) throw();
	bignum & operator ++ (void) throw() { return (*this)+=(element_type)1; }
	void operator ++ (int) throw() { (*this)+=(element_type)1; }
	bignum & operator -- (void) throw() { return (*this)-=(element_type)1; }

	element_type subtract(const bignum &b) throw();
		// returns carry
	element_type add(const bignum &b) throw();
		// returns carry
	uint set_linear_combination(const bignum &u,const bignum &v,
				const uint u_elems,const uint v_elems,
				const element_type umult,const element_type vmult) throw();
		// returns nonzero on overflow
	uint set_linear_combination(const bignum &u,const bignum &v,
				const uint u_elems,const uint v_elems,
				const element_type umult,const element_type vmult,
				const uint subtract_direction) throw();
		// returns nonzero on overflow

	void mod_subtract(const bignum &b,const bignum &modulus) throw();
		// *this < modulus

	uint operator < (const bignum &b) const throw()
			{ return is_less_than(lenof(m),&m[0],&b.m[0]); }

	uint operator != (const bignum &b) const throw();

	uint get_bit(const uint bit_nr) const throw();
	void operator >>= (const uint j) throw();

	void multiply(bignum<KEY_BITS*2> &dest,const bignum &b) const throw()
		{ do_multiply(lenof(m),&dest.m[0],&m[0],&b.m[0]); }

	uint is_divisible_by(const element_type v) throw() { return !(*this % v); }
	void gen_prime(	uchar (* const random_func)(void *data),void *data=NULL,
					const element_type public_exponent=3,
					const uint miller_rabin_repetitions=25);
	void gen_random(const uint nr_of_elements,
					uchar (* const random_func)(void *data),void *data=NULL);
	};

template <int KEY_BITS> struct rsa_ciphertext {
	uchar data[KEY_BITS/8];
	};

template <int KEY_BITS> struct rsa_plaintext {
	uchar data[KEY_BITS/8 - 1];
	};

template <int KEY_BITS> class rsa_public_key {
	protected:
	bignum<KEY_BITS> common_modulus;

	void do_operation(const uchar * const src,uchar * const dest,
						const uint src_is_plaintext,
						const bignum<KEY_BITS> &exponent) const throw();
	public:

	void set(const uchar * const src) throw();
		// src must have RSA_KEY_BYTES uchar's
	void export_as_bytes(uchar *dest,const uint use_big_endian=0) const throw();
							// dest must have RSA_KEY_BYTES uchar's
	rsa_ciphertext<KEY_BITS> encrypt(const rsa_plaintext<KEY_BITS> &buf) const throw();
	uint verify_signature(const rsa_ciphertext<KEY_BITS> &buf,
				const rsa_plaintext<KEY_BITS> &correct_hash) const throw();
		// returns 1 if signature was correct, otherwise returns 0
	};

template <int KEY_BITS> class rsa_key_pair : public rsa_public_key<KEY_BITS> {
	protected:
	bignum<KEY_BITS> private_exponent;
	public:

	typedef typename bignum<KEY_BITS>::element_type element_type;
	typedef typename bignum<KEY_BITS>::two_element_type two_element_type;

	void set_private_part(const uchar * const src) throw();
							// src must have RSA_KEY_BYTES uchar's
	void export_private_part_as_bytes(uchar *dest,const uint use_big_endian=0) const throw();
							// dest must have RSA_KEY_BYTES uchar's
	void make(uchar (* const random_func)(void *data),void *data=NULL,
									const element_type public_exponent=3,
									const uint miller_rabin_repetitions=25);
	rsa_plaintext<KEY_BITS> decrypt(const rsa_ciphertext<KEY_BITS> &buf) const throw();
	rsa_ciphertext<KEY_BITS> sign(const rsa_plaintext<KEY_BITS> &buf) const throw();
	};

template <int KEY_BITS> class rsa_crt_private_key {
	protected:
	bignum<KEY_BITS/2> p,q,dP,dQ,invP;
	public:

		// uses public exponent 65537

	bignum<KEY_BITS> make(uchar (* const random_func)(void *data),
			void *data=NULL,const uint miller_rabin_repetitions=25) throw();
		// returns public key (common modulus)

	void do_private_key_operation(bignum<KEY_BITS> &block,
						const bignum<KEY_BITS> &public_key) const throw();
		// the high bit of public_key must be set
		// block must be < public_key

	uint import_from_bytes(const uchar * const src,const uint nr_of_bytes)
																	throw();
		// returns nonzero on success

	static uint get_export_size(void) throw() { return KEY_BITS*5/(2*8); }

	void export_as_bytes(uchar * const dest) const throw();
		// dest must have get_export_size() uchar's
	};

#define N 				(KEY_BITS/ELEMENT_BITS)		// must be even

/**************************************************************************/
/*********************************          *******************************/
/********************************* bignum:: *******************************/
/*********************************          *******************************/
/**************************************************************************/

template <int KEY_BITS> bignum<KEY_BITS>::bignum(const element_type d) throw()
{
	for (uint i=0;i < lenof(m);i++)
		m[i]=0;
	m[0]=d;
	}

template <int KEY_BITS> bignum<KEY_BITS>::bignum(
						const uchar * const src,const uint use_big_endian) throw()
{						// src must have KEY_BITS/8 uchar's
	const uchar *p=src;
	sint p_increment=+1;
	if (use_big_endian) {
		p+=KEY_BITS/8 - 1;
		p_increment=-1;
		}

	for (uint i=0;i < lenof(m);i++) {
		element_type value=0;
		for (uint j=0;j < ELEMENT_BITS;j+=8) {
			value|=(element_type)(((element_type)*p) << j);
			p+=p_increment;
			}
		m[i]=value;
		}
	}

template <int KEY_BITS> void bignum<KEY_BITS>::export_as_bytes(uchar *dest,
			const uint nr_of_bytes,const uint use_big_endian) const throw()
{
	sint dest_increment=+1;
	if (use_big_endian) {
		dest+=nr_of_bytes - 1;
		dest_increment=-1;
		}

	for (uint i=0;i < nr_of_bytes;i++,dest+=dest_increment)
		*dest=(uchar)(m[i / (ELEMENT_BITS/8)] >> ((i % (ELEMENT_BITS/8))*8));
	}

template <int KEY_BITS> uint bignum<KEY_BITS>::elements(void) const throw()
{
	uint count;
	for (count=lenof(m);count;count--)
		if (m[count-1])
			break;
	return count;
	}

template <int KEY_BITS> uint bignum<KEY_BITS>::operator != (
											const bignum &b) const throw()
{
	for (uint i=0;i < lenof(m);i++)
		if (m[i] != b.m[i])
			return 1;
	return 0;
	}

template <int KEY_BITS> bignum_funcs::element_type
						bignum<KEY_BITS>::add(const bignum &b) throw()
{
	element_type carry=0;
	for (uint i=0;i < lenof(m);i++) {
		const element_type a=m[i];
		m[i]+=b.m[i] + carry;
		carry=(m[i] < a || (m[i] == a && carry));
		}

	return carry;
	}

template <int KEY_BITS> bignum_funcs::element_type
						bignum<KEY_BITS>::subtract(const bignum &b) throw()
{
	element_type carry=0;
	for (uint i=0;i < lenof(m);i++) {
		const element_type a=m[i];
		m[i]-=b.m[i] + carry;
		carry=(m[i] > a || (m[i] == a && carry));
		}

	return carry;
	}

template <int KEY_BITS> uint bignum<KEY_BITS>::set_linear_combination(
				const bignum &u,const bignum &v,
				const uint u_elems,const uint v_elems,
				const element_type umult,const element_type vmult) throw()
{		// returns nonzero on overflow

	bignum<KEY_BITS + ELEMENT_BITS> uu(0U),vv(0U);
	multiply_add_one_line(uu.m,umult,u.m,u_elems);
	multiply_add_one_line(vv.m,vmult,v.m,v_elems);

	if (uu.add(vv))
		return 1;

	for (uint i=0;i < lenof(m);i++)
		m[i]=uu[i];

	return !!uu[lenof(m)];
	}

template <int KEY_BITS> bignum_funcs::element_type
								bignum<KEY_BITS>::set_linear_combination(
						const bignum &u,const bignum &v,
						const uint u_elems,const uint v_elems,
						const element_type umult,const element_type vmult,
						const uint subtract_direction) throw()
{		// returns nonzero on overflow

	bignum<KEY_BITS + ELEMENT_BITS> uu(0U),vv(0U);
	multiply_add_one_line(uu.m,umult,u.m,u_elems);
	multiply_add_one_line(vv.m,vmult,v.m,v_elems);

	bignum<KEY_BITS + ELEMENT_BITS> *positive=subtract_direction ? &vv : &uu;
	bignum<KEY_BITS + ELEMENT_BITS> *negative=subtract_direction ? &uu : &vv;

	if (positive->subtract(*negative))
		return 1;

	for (uint i=0;i < lenof(m);i++)
		m[i]=(*positive)[i];

	return !!(*positive)[lenof(m)];
	}

template <int KEY_BITS> void bignum<KEY_BITS>::mod_subtract(
						const bignum &b,const bignum &modulus) throw()
{		// *this < modulus

	element_type orig_carry=subtract(b);
	while (orig_carry)
		orig_carry-=add(modulus);
	}

template <int KEY_BITS> bignum<KEY_BITS> & bignum<KEY_BITS>::operator +=
													(element_type b) throw()
{
	for (uint i=0;i < lenof(m);i++) {
		const element_type a=m[i];
		m[i]+=b;
		if (m[i] >= a)
			break;
		b=1;
		}
	return *this;
	}

template <int KEY_BITS> bignum<KEY_BITS> & bignum<KEY_BITS>::operator -=
													(element_type b) throw()
{
	for (uint i=0;i < lenof(m);i++) {
		const element_type a=m[i];
		m[i]-=b;
		if (m[i] <= a)
			break;
		b=1;
		}
	return *this;
	}

template <int KEY_BITS> void bignum<KEY_BITS>::operator >>= (const uint j) throw()
{
		// 0 < j < ELEMENT_BITS

	for (uint i=0;i < lenof(m);i++) {
		m[i] >>= j;
		if (i < lenof(m)-1)
			m[i]+=m[i+1] << (ELEMENT_BITS-j);
		}
	}

template <int KEY_BITS> void bignum<KEY_BITS>::gen_random(
						const uint nr_of_elements,
						uchar (* const random_func)(void *data),void *data)
{
	for (uint i=0;i < lenof(m);i++) {
		m[i]=0;
		if (i < nr_of_elements) {
			for (uint j=0;j < ELEMENT_BITS;j+=8)
				m[i]|=(element_type)(((element_type)random_func(data)) << j);
			}
		}
	}

/***************************************************************************/
/****************************                  *****************************/
/**************************** bignum * integer *****************************/
/****************************                  *****************************/
/***************************************************************************/

template <int KEY_BITS>
			void bignum<KEY_BITS>::operator *= (const element_type v) throw()
{
	element_type carry=0;
	for (uint i=0;i < N;i++) {
		const two_element_type big_value=m[i] * (two_element_type)v + carry;
		m[i]=(element_type)(big_value & ELEMENT_ALL_BITS_SET);
		carry=(element_type)(big_value >> ELEMENT_BITS);
		}
	}

/***************************************************************************/
/****************************                  *****************************/
/**************************** bignum / integer *****************************/
/****************************                  *****************************/
/***************************************************************************/

template <int KEY_BITS> void bignum<KEY_BITS>::operator /=
											(const element_type v) throw()
{
	element_type remainder=0;
	for (uint i=N;i--;) {
		const two_element_type big_value=m[i] +
							(((two_element_type)remainder) << ELEMENT_BITS);
		remainder=(element_type)(big_value % v);
		m[i]=(element_type)(big_value / v);
		}
	}

/***************************************************************************/
/****************************                  *****************************/
/**************************** bignum % integer *****************************/
/****************************                  *****************************/
/***************************************************************************/

template <int KEY_BITS> typename bignum<KEY_BITS>::element_type
			bignum<KEY_BITS>::operator % (const element_type v) const throw()
{
	uint i=N;

	for (;i >= 8;i-=8)
		if ((m[i-1] | m[i-2] | m[i-3] | m[i-4] |
			 m[i-5] | m[i-6] | m[i-7] | m[i-8]) != 0)
			break;

	element_type remainder=0;
	for (;i--;)
		remainder=(element_type)
			(((((two_element_type)remainder) << ELEMENT_BITS) + m[i]) % v);

	return remainder;
	}

/***************************************************************************/
/*******************************            ********************************/
/******************************* mod_mult() ********************************/
/*******************************            ********************************/
/***************************************************************************/

template <int KEY_BITS> void bignum<KEY_BITS>::mod_mult(
		const bignum<KEY_BITS> &b,const bignum<KEY_BITS> &modulus) throw()
{
		// must handle both N and half_N moduli

	const uint n=(modulus.m[N-1]) ? N : (N/2);

	element_type mult_result[2*N];
	do_multiply(n,&mult_result[0],&m[0],&b.m[0]);

	mod_div(n,&m[0],&mult_result[0],&modulus.m[0]);
	for (uint i=n;i < N;i++)
		m[i]=0;
	}

template <int KEY_BITS> void bignum<KEY_BITS>::mod_square(
									const bignum<KEY_BITS> &modulus) throw()
{
		// must handle both N and half_N moduli

	const uint n=(modulus.m[N-1]) ? N : (N/2);

	element_type mult_result[2*N];
	element_type tarr[2*N-2];
	bignum_funcs::mod_square(&m[0],&modulus.m[0],mult_result,tarr,n);

	for (uint i=n;i < N;i++)
		m[i]=0;
	}

/***************************************************************************/
/********************************           ********************************/
/******************************** mod_exp() ********************************/
/********************************           ********************************/
/***************************************************************************/

template <int KEY_BITS> uint bignum<KEY_BITS>::get_bit(
											const uint bit_nr) const throw()
{				// bit_nr = 0 .. KEY_BITS-1
	return (m[bit_nr / ELEMENT_BITS] >> (bit_nr % ELEMENT_BITS)) & 1;
	}

template <int KEY_BITS> void bignum<KEY_BITS>::mod_exp(
	const bignum<KEY_BITS> &exponent,const bignum<KEY_BITS> &modulus) throw()
{
		// the high bit of modulus must be set, either in N or N/2
		// *this < modulus

	uint exponent_bits;
	for (exponent_bits=KEY_BITS;exponent_bits;exponent_bits--)
		if (exponent.get_bit(exponent_bits-1))
			break;

		/*********************************/
		/*****                       *****/
		/***** handle tiny exponents *****/
		/*****                       *****/
		/*********************************/

#ifndef RSA_MODEXP_BITS_PER_ITERATION
#define RSA_MODEXP_BITS_PER_ITERATION	5
#endif

	bignum<KEY_BITS> odd_x_powers[(1 << RSA_MODEXP_BITS_PER_ITERATION)/2];	// x^1, x^3, x^5, ...
	odd_x_powers[0]=*this;

	if (exponent_bits < 60) {
		if (!exponent_bits) {
			*this=bignum<KEY_BITS>(1);
			return;
			}

		/* On average, this method does
				(exponent_bits-1)/2 mults and (exponent_bits-1) squares

			On average, the odd_x_powers method does
				(exponent_bits-1)/RSA_MODEXP_BITS_PER_ITERATION mults and
					(exponent_bits-1)*(RSA_MODEXP_BITS_PER_ITERATION+1)/
										RSA_MODEXP_BITS_PER_ITERATION squares

			Test results with KEY_BITS=1536 and full N:

			exponent_bits	simple method	odd_x_powers method
				32				4.29 ms			4.77 ms
				64				8.5  ms			7.86 ms
				96				13 ms			11.1 ms
				128				17 ms			14 ms
			*/

		for (uint bits_left=exponent_bits-1;bits_left;) {
			mod_square(modulus);
			bits_left--;
			if (exponent.get_bit(bits_left))
				mod_mult(odd_x_powers[0],modulus);
			}

		return;
		}

		/**********************************/
		/*****                        *****/
		/***** compute odd_x_powers[] *****/
		/*****                        *****/
		/**********************************/

	{ bignum<KEY_BITS> x_square=*this;
	x_square.mod_square(modulus);

	for (uint i=1;i < lenof(odd_x_powers);i++) {
		odd_x_powers[i]=odd_x_powers[i-1];
		odd_x_powers[i].mod_mult(x_square,modulus);
		}}

		/*****************************/
		/*****                   *****/
		/***** mod_exp main loop *****/
		/*****                   *****/
		/*****************************/

	for (uint bits_left=exponent_bits-1;bits_left;) {
		mod_square(modulus);

		bits_left--;
		if (!exponent.get_bit(bits_left))
			continue;

		uint bits=1;
		uint bit_count=1;
		for (;bit_count < RSA_MODEXP_BITS_PER_ITERATION &&
													bits_left;bit_count++)
			bits=2*bits + exponent.get_bit(--bits_left);

		uint reduction_count=0;
		while (!(bits & 1)) {
			bits>>=1;
			reduction_count++;
			bit_count--;
			}

		while (--bit_count)
			mod_square(modulus);

		mod_mult(odd_x_powers[bits >> 1],modulus);

		while (reduction_count--)
			mod_square(modulus);
		}
	}

/***************************************************************************/
/**********************                              ***********************/
/********************** multiplicative inverse mod M ***********************/
/**********************                              ***********************/
/***************************************************************************/

template <int KEY_BITS> uint bignum<KEY_BITS>::mod_inverse(bignum v) throw()
{		// sets *this to (v**-1) mod (*this)
		// returns zero if implementation could not handle such input values
		// *this > 1
		// v > 0
		// v < *this

	bignum<KEY_BITS> u=*this;
	bignum<KEY_BITS + ELEMENT_BITS> s0(0U), s1(1U);
	uint negate_s0=1;

	uint u_elems,v_elems;

	while (1) {

			// Implementation of Lehmer's algorithm (Knuth algorithm 4.5.2L)

		u_elems=u.elements();
		v_elems=v.elements();
		if (v_elems < 2)
			break;

			/****************************/
			/*****                  *****/
			/***** Set uhigh, vhigh *****/
			/*****                  *****/
			/****************************/

			// at this point always u > v

		bignum_funcs::element_type uhigh=u.m[u_elems-1];
		bignum_funcs::element_type vhigh=v.m[u_elems-1];

		{ uint additional_bits=0;
		while (uhigh <= ELEMENT_ALL_BITS_SET/2) {
			uhigh<<=1;
			vhigh<<=1;
			additional_bits++;
			}
		if (additional_bits) {
			uhigh+=u.m[u_elems-2] >> (ELEMENT_BITS-additional_bits);
			vhigh+=v.m[u_elems-2] >> (ELEMENT_BITS-additional_bits);
			}}

			/***************************************************/
			/*****                                         *****/
			/***** Iterate with A,B,C,D as far as possible *****/
			/*****                                         *****/
			/***************************************************/

		bignum_funcs::element_type A=1,B=0,C=0,D=1;
		uint negate_a=0;

		while (1) {
			if (vhigh - C == 0 || vhigh + D == 0)
				break;

			{ const bignum_funcs::element_type q=(uhigh + A) / (vhigh - C);
			if (q != (uhigh - B) / (vhigh + D))
				break;

			{ const bignum_funcs::element_type tmp=A+q*C; A=C; C=tmp; }
			{ const bignum_funcs::element_type tmp=B+q*D; B=D; D=tmp; }
			{ const bignum_funcs::element_type tmp=uhigh-q*vhigh;
												uhigh=vhigh; vhigh=tmp; }}

			if (vhigh - D == 0) {
				negate_a=1;
				break;
				}

			{ const bignum_funcs::element_type q=(uhigh - A) / (vhigh + C);
			if (q != (uhigh + B) / (vhigh - D)) {
				negate_a=1;
				break;
				}

			{ const bignum_funcs::element_type tmp=A+q*C; A=C; C=tmp; }
			{ const bignum_funcs::element_type tmp=B+q*D; B=D; D=tmp; }
			{ const bignum_funcs::element_type tmp=uhigh-q*vhigh;
												uhigh=vhigh; vhigh=tmp; }}
			}

			/**************************************/
			/*****                            *****/
			/***** Apply A,B,C,D to u,v,s0,s1 *****/
			/*****                            *****/
			/**************************************/

		negate_s0^=negate_a;

		if (!B)
			return 0;	// This difficult infrequent case is not implemented

			// u <-- u*A + v*B (with negate_a)
			// v <-- u*C + v*D (with negate_a)
			//
			// s0 <-- s0*A + s1*B
			// s1 <-- s0*C + s1*D

		bignum<KEY_BITS> new_u,new_v;
		bignum<KEY_BITS + ELEMENT_BITS> new_s0,new_s1;

		const uint s0_elems=s0.elements();
		const uint s1_elems=s1.elements();

		if (!!(new_u.set_linear_combination(u,v,u_elems,v_elems,
														A,B,negate_a) |
				new_v.set_linear_combination(u,v,u_elems,v_elems,
														C,D,negate_a^1) |
				new_s0.set_linear_combination(s0,s1,s0_elems,s1_elems,A,B) |
				new_s1.set_linear_combination(s0,s1,s0_elems,s1_elems,C,D)))
			return 0;

		u=new_u;
		v=new_v;
		s0=new_s0;
		s1=new_s1;
		}

	if (!v_elems) {
		if (u_elems != 1 || u.m[0] != 1)
			return 0;		// U and V are not relatively prime
		}
	  else {
		element_type vl=u % v.m[0];
		u/=v.m[0];

		{ bignum<KEY_BITS + ELEMENT_BITS> tmp(0U);

		const uint s1_elems=s1.elements();
		if (u_elems + s1_elems > lenof(tmp.m))
			return 0;	// values unexpectedly large, this should never happen

		for (uint i=0;i < u_elems;i++)
			multiply_add_one_line(tmp.m + i,u.m[i],s1.m,s1_elems);

		if (tmp.add(s0))
			return 0;	// values unexpectedly large, this should never happen

		s0=s1;
		s1=tmp;
		negate_s0^=1; }

		element_type ul=v.m[0];
		while (vl) {
			const element_type q=ul / vl;
			const element_type r=ul - q*vl;

			{ bignum<KEY_BITS + ELEMENT_BITS> tmp(s0);
			multiply_add_one_line(tmp.m,q,s1.m,lenof(m));
			s0=s1;
			s1=tmp;
			negate_s0^=1; }

			ul=vl;
			vl=r;
			}

		if (ul != 1)
			return 0;		// U and V are not relatively prime
		}

	bignum<KEY_BITS> s;
	{ for (uint i=0;i < lenof(m);i++)
		s.m[i]=s0[i]; }
	if (negate_s0)
		subtract(s);
	  else
		*this=s;

	return 1;
	}

/***************************************************************************/
/*******************************             *******************************/
/******************************* gen_prime() *******************************/
/*******************************             *******************************/
/***************************************************************************/

template <int KEY_BITS> void bignum<KEY_BITS>::gen_prime(
						uchar (* const random_func)(void *data),void *data,
						const element_type public_exponent,
						const uint miller_rabin_repetitions)
{
	// prime-1 should not be divisible by public exponent

	gen_random(N,random_func,data);

	while (1) {

			// set its highest 2 bits to 11
			// set its lowest  2 bits to 11

		(*this)[N-1]|=((element_type)3) << (ELEMENT_BITS - 2);
		(*this)[0  ]|=(element_type)3;

		{ const uint r=(*this) % (3*5*7*11*13U);
		if (!(r%3) || !(r%5) || !(r%7) || !(r%11) || !(r%13))
			goto next_prime; }

		{ element_type primes[15];
		uint nr_of_primes=0;
		element_type primes_product=1;

		for (element_type cur_prime=17;cur_prime < 800;cur_prime+=2) {
					// 1200 seems to be optimal for AMD, 800 for PXA270
			uint is_prime=0;
			for (uint divisor=3;;divisor+=2) {
				const uint q=cur_prime / divisor;
				if (q < divisor) {
					is_prime=1;
					break;
					}
				if (cur_prime <= q*divisor)
					break;
				}

			if (!is_prime)
				continue;

			const two_element_type big_value=
								primes_product * (two_element_type)cur_prime;
			if (big_value > ELEMENT_ALL_BITS_SET ||
											nr_of_primes >= lenof(primes)) {
				const uint r=(*this) % primes_product;

				for (uint j=0;j < nr_of_primes;j++)
					if (!(r % primes[j]))
						goto next_prime;

				primes_product=cur_prime;
				nr_of_primes=0;
				}
			  else
				primes_product=(element_type)big_value;

			primes[nr_of_primes++]=cur_prime;
			}}

			// set u_minus_1

		{ bignum<KEY_BITS> u_minus_1=*this;
		u_minus_1[0]--;

		if (u_minus_1.is_divisible_by(public_exponent))
			goto next_prime;

			/*********************************/
			/*****                       *****/
			/***** perform a Fermat test *****/
			/*****                       *****/
			/*********************************/

		{ bignum<KEY_BITS> xx(210);
		xx.mod_exp(u_minus_1,*this);
		if (xx.elements() != 1 || xx.m[0] != 1)
			goto next_prime; }

			// set u_minus_1_div_2

		{ bignum<KEY_BITS> u_minus_1_div_2=u_minus_1;
		u_minus_1_div_2 >>= 1;

			// do algorithm P

		for (uint i=0;i < miller_rabin_repetitions;i++) {
			bignum<KEY_BITS> x;

			do {
				x.gen_random(N,random_func,data);
				} while (!(x < u_minus_1) ||
									(x.elements() <= 1 && x.m[0] <= 1));

				// compute x = (x ^ u_minus_1_div_2) mod u

			x.mod_exp(u_minus_1_div_2,*this);

			if ((x.elements() != 1 || x.m[0] != 1) && x != u_minus_1)
				goto next_prime;
			}}}

		return;		// we found a probable prime

		next_prime:

		(*this)+=4;
		}
	}

/**************************************************************************/
/**********************************     ***********************************/
/********************************** rsa ***********************************/
/**********************************     ***********************************/
/**************************************************************************/

template <int KEY_BITS> void rsa_key_pair<KEY_BITS>::make(
					uchar (* const random_func)(void *data),void *data,
					const element_type public_exponent,
					const uint miller_rabin_repetitions)
{
	bignum<KEY_BITS/2> p,q;

	do {
		p.gen_prime(random_func,data,public_exponent,miller_rabin_repetitions);
		q.gen_prime(random_func,data,public_exponent,miller_rabin_repetitions);
		p.multiply(rsa_public_key<KEY_BITS>::common_modulus,q);

		--p;
		--q;
		p.multiply(private_exponent,q);
		} while (!private_exponent.mod_inverse(public_exponent));
	}

template <int KEY_BITS> void rsa_public_key<KEY_BITS>::do_operation(
					const uchar * const src,uchar * const dest,
					const uint src_is_plaintext,
					const bignum<KEY_BITS> &exponent) const throw()
{
	uchar x_buf[KEY_BITS/8];
	memcpy(x_buf,src,src_is_plaintext ? (KEY_BITS/8-1) : KEY_BITS/8);

	if (src_is_plaintext) {
		// ensure that x < common_modulus && x^3 > common_modulus

		x_buf[lenof(x_buf)-1]=1;
		}

	bignum<KEY_BITS> x(x_buf);

	if (common_modulus.get_bit(KEY_BITS-1))
		x.mod_exp(exponent,common_modulus);

	x.export_as_bytes(dest,src_is_plaintext ? KEY_BITS/8 : (KEY_BITS/8-1));
	}

template <int KEY_BITS> rsa_ciphertext<KEY_BITS> rsa_public_key<KEY_BITS>::
				encrypt(const rsa_plaintext<KEY_BITS> &buf) const throw()
{
	rsa_ciphertext<KEY_BITS> dest;
	do_operation(buf.data,dest.data,1,3);
	return dest;
	}

template <int KEY_BITS> rsa_plaintext<KEY_BITS> rsa_key_pair<KEY_BITS>::
				decrypt(const rsa_ciphertext<KEY_BITS> &buf) const throw()
{
	rsa_plaintext<KEY_BITS> dest;
	do_operation(buf.data,dest.data,0,private_exponent);
	return dest;
	}

template <int KEY_BITS> rsa_ciphertext<KEY_BITS> rsa_key_pair<KEY_BITS>::
				sign(const rsa_plaintext<KEY_BITS> &buf) const throw()
{
	rsa_ciphertext<KEY_BITS> dest;
	do_operation(buf.data,dest.data,1,private_exponent);
	return dest;
	}

template <int KEY_BITS> uint rsa_public_key<KEY_BITS>::verify_signature(
				const rsa_ciphertext<KEY_BITS> &buf,
				const rsa_plaintext<KEY_BITS> &correct_hash) const throw()
{		// returns 1 if signature was correct, otherwise returns 0

	rsa_plaintext<KEY_BITS> dest;
	do_operation(buf.data,dest.data,0,3);
	return !memcmp(dest.data,correct_hash.data,sizeof(correct_hash.data));
	}

template <int KEY_BITS> void rsa_public_key<KEY_BITS>::set(
											const uchar * const src) throw()
{									// src must have KEY_BITS uchar's
	const bignum<KEY_BITS> x(src);
	common_modulus=x;
	}

template <int KEY_BITS> void rsa_public_key<KEY_BITS>::export_as_bytes(
						uchar *dest,const uint use_big_endian) const throw()
{									// dest must have KEY_BITS uchar's
	common_modulus.export_as_bytes(dest,KEY_BITS/8,use_big_endian);
	}

template <int KEY_BITS> void rsa_key_pair<KEY_BITS>::set_private_part(
											const uchar * const src) throw()
{									// src must have KEY_BITS uchar's
	const bignum<KEY_BITS> x(src);
	private_exponent=x;
	}

template <int KEY_BITS> void rsa_key_pair<KEY_BITS>::export_private_part_as_bytes(
						uchar *dest,const uint use_big_endian) const throw()
{			// dest must have KEY_BITS uchar's
	private_exponent.export_as_bytes(dest,KEY_BITS/8,use_big_endian);
	}

template <int KEY_BITS> bignum<KEY_BITS> rsa_crt_private_key<KEY_BITS>::
							make(uchar (* const random_func)(void *data),
					void *data,const uint miller_rabin_repetitions) throw()
{		// returns public key (common modulus)

	p.gen_prime(random_func,data,65537,miller_rabin_repetitions);
	bignum<KEY_BITS/2> *next_pq_to_generate=&q;

	bignum<KEY_BITS> common_modulus;

	do {
		do {
			next_pq_to_generate->gen_prime(
							random_func,data,65537,miller_rabin_repetitions);
			} while (p[0] == q[0]);		// just a sloppy equality test

		if (p < q) {
			const bignum<KEY_BITS/2> tmp=p;
			p=q;
			q=tmp;
			}
		  else
			next_pq_to_generate=(next_pq_to_generate == &q) ? &p : &q;

		p.multiply(common_modulus,q);

		--p;
		--q;
		bignum<KEY_BITS> private_exponent;
		p.multiply(private_exponent,q);

		private_exponent.mod_inverse(65537);

		{ bignum<KEY_BITS> tmp(private_exponent); dP.set_mod(tmp,p); }
		dQ.set_mod(private_exponent,q);

		++p;
		++q;

		invP=p;

		} while (!invP.mod_inverse(q));

	return common_modulus;
	}

template <int KEY_BITS> void rsa_crt_private_key<KEY_BITS>::
							do_private_key_operation(bignum<KEY_BITS> &block,
									const bignum<KEY_BITS> &public_key) const throw()
{		// the high bit of public_key must be set
		// block must be < public_key

	bignum<KEY_BITS/2> mP;
	{ bignum<KEY_BITS> tmp(block);
	mP.set_mod(tmp,p); }
	mP.mod_exp(dP,p);

	bignum<KEY_BITS/2> mQ;
	mQ.set_mod(block,q);
	mQ.mod_exp(dQ,q);

	mP.mod_subtract(mQ,p);
	mP.mod_mult(invP,p);

	q.multiply(block,mP);

	{ bignum_funcs::element_type carry=0;
	for (uint i=0;i < N;i++) {
		const bignum_funcs::element_type a=block[i];
		if (i < N/2)
			block[i]+=mQ[i];
		block[i]+=carry;
		carry=(block[i] < a || (block[i] == a && carry));
		}}

	if (!(block < public_key))
		block.subtract(public_key);
	}

template <int KEY_BITS> uint rsa_crt_private_key<KEY_BITS>::
	import_from_bytes(const uchar * const src,const uint nr_of_bytes) throw()
{		// returns nonzero on success

	if (nr_of_bytes < get_export_size())
		return 0;

	p=   bignum<KEY_BITS/2>(src                   ,1);
	q=   bignum<KEY_BITS/2>(src +   KEY_BITS/(2*8),1);
	invP=bignum<KEY_BITS/2>(src + 2*KEY_BITS/(2*8),1);
	dP=  bignum<KEY_BITS/2>(src + 3*KEY_BITS/(2*8),1);
	dQ=  bignum<KEY_BITS/2>(src + 4*KEY_BITS/(2*8),1);

	return (p.get_bit(KEY_BITS/2-1) && q.get_bit(KEY_BITS/2-1));
	}

template <int KEY_BITS> void rsa_crt_private_key<KEY_BITS>::
						export_as_bytes(uchar * const dest) const throw()
{		// dest must have get_export_size() uchar's

	p.   export_as_bytes(dest                   ,KEY_BITS/(2*8),1);
	q.   export_as_bytes(dest +   KEY_BITS/(2*8),KEY_BITS/(2*8),1);
	invP.export_as_bytes(dest + 2*KEY_BITS/(2*8),KEY_BITS/(2*8),1);
	dP.  export_as_bytes(dest + 3*KEY_BITS/(2*8),KEY_BITS/(2*8),1);
	dQ.  export_as_bytes(dest + 4*KEY_BITS/(2*8),KEY_BITS/(2*8),1);
	}

#undef N

#ifdef TRSA_TEST_PROGRAM
#include <stdio.h>
#include <stdlib.h>
#include <lcs.hpp>
#include <Crc.cpp>
#include <Timer.cpp>
#define KEY_BITS			1536
#define PRIVATEOP_KEY_BITS	1024
#define INNER_LOOP_COUNT	1000

#define MODULUS_N			(KEY_BITS/(sizeof(bignum_funcs::element_type)*8))

static lcs rng;
static uchar my_random_func(void *) { return rng.get_uint(256); }

#if defined(__arm__) || defined(_ARM_)
#define OUTER_LOOP_COUNT	1
#define CORRECT_CRC			0x4917ffd0
#else
#define OUTER_LOOP_COUNT	40
#define CORRECT_CRC			0x16cb2a3c
#endif

int main(sint,char **)
{
	timer tim;
	FILE * const fp=
#ifdef _WIN32_WCE
					fopen("trsa.out","w");
#else
					stdout;
#endif

		/******************************/
		/*****                    *****/
		/***** mod_inverse() test *****/
		/*****                    *****/
		/******************************/

	tim.lap();
	rng.set_seed(34368);
	rng.get_uint(); rng.get_uint();
	{ bignum<PRIVATEOP_KEY_BITS/2> p;
	p.gen_prime(my_random_func,NULL,65537,5);

	uint unsuitable_input_values=0;
	uint i;
	for (i=0;i < OUTER_LOOP_COUNT*300;i++) {
		bignum<PRIVATEOP_KEY_BITS/2> q;
		do {
			q.gen_random(PRIVATEOP_KEY_BITS/
				(sizeof(bignum_funcs::element_type)*8)/2,my_random_func,NULL);
			q[PRIVATEOP_KEY_BITS/
				(sizeof(bignum_funcs::element_type)*8)/2-1]|=
					((bignum_funcs::element_type)1) << (ELEMENT_BITS - 1);
			} while (p < q);

		const bignum<PRIVATEOP_KEY_BITS/2> modulus=p;
		bignum<PRIVATEOP_KEY_BITS/2> x=p;

		if (!x.mod_inverse(q))
			unsuitable_input_values++;
		  else {
			x.mod_mult(q,modulus);
			if (x.elements() != 1 || x[0] != 1) {
				fprintf(fp,"mod_inverse() gave WRONG RESULT at %u\n",i);
				return 1;
				}
			}
		}
	fprintf(fp,"%u-bit mod_inverse() test OK, "
			"%.1fus per mod_inverse(), %.1f%% unsuitable input values\n",
						(uint)(PRIVATEOP_KEY_BITS/2),
						tim.lap()*1000/i,unsuitable_input_values*100.0/ i); }

		/**************************************/
		/*****                            *****/
		/***** keygen and private-op test *****/
		/*****                            *****/
		/**************************************/

	tim.lap();
	{ float keygen_time=0.0f,privateop_time=0.0f;
	uint i;
	const uint innerloop_count=20;
	rsa_crt_private_key<PRIVATEOP_KEY_BITS> private_key;
	uchar *private_key_buf=new uchar[private_key.get_export_size()];
	for (i=0;i < OUTER_LOOP_COUNT;i++) {
		rng.set_seed(i ^ 937567);	// this value gives quite average 1024-bit keygen time for i==0
		rng.get_uint();
		const bignum<PRIVATEOP_KEY_BITS> public_key=
									private_key.make(my_random_func,NULL,5);
		keygen_time+=tim.lap();

		private_key.export_as_bytes(private_key_buf);
		if (!private_key.import_from_bytes(private_key_buf,
										private_key.get_export_size())) {
			fprintf(fp,"PRIVATE-KEY IMPORT FAILED AT %u\n",i);
			return 1;
			}

		bignum<PRIVATEOP_KEY_BITS> block(0U);
		{ for (uint q=0;q < (PRIVATEOP_KEY_BITS/
							(sizeof(bignum_funcs::element_type)*8))-1;q++)
			block[q]=rng.get_uint(); }

		const bignum<PRIVATEOP_KEY_BITS> orig_block=block;

		private_key.do_private_key_operation(block,public_key);

		block.mod_exp(65537,public_key);
		if (block != orig_block) {
			fprintf(fp,"PRIVATE-KEY OP PRODUCED WRONG VALUE AT %u\n",i);
			return 1;
			}

		for (uint j=1;j < innerloop_count;j++)
			private_key.do_private_key_operation(block,public_key);

		privateop_time+=tim.lap();
		}
	delete [] private_key_buf;
	fprintf(fp,"%u-bit private-key op test OK, "
					"keygen time %.0fms, private-key op time %.2fms\n",
				(uint)PRIVATEOP_KEY_BITS,
				keygen_time / i,privateop_time / (i*innerloop_count)); }

		/**************************/
		/*****                *****/
		/***** public-op test *****/
		/*****                *****/
		/**************************/

	tim.lap();
	{ CRC32 crc;
	uint i;
	for (i=0;i < OUTER_LOOP_COUNT;i++) {
		rng.set_seed(i);
		rng.get_uint(); rng.get_uint(); rng.get_uint(); rng.get_uint();

		static uchar output_value[KEY_BITS/8];

		{ bignum<KEY_BITS-32> m,xx(210);
		m.gen_random(MODULUS_N-1,my_random_func,NULL);
		m[MODULUS_N-1-1]|=7U << 29;
		m[0]|=3;
		xx.mod_exp(65537,m);
		xx.export_as_bytes(output_value,(KEY_BITS-32)/8);
		crc.Update(output_value,(KEY_BITS-32)/8); }


		bignum<KEY_BITS> m,xx(210);
		m.gen_random(MODULUS_N,my_random_func,NULL);
		m[MODULUS_N-1]|=7U << 29;
		m[0]|=3;

		for (uint j=0;j < INNER_LOOP_COUNT;j++)
			xx.mod_exp(65537,m);

		xx.export_as_bytes(output_value,KEY_BITS/8);
		crc.Update(output_value,sizeof(output_value));
		}

	char print_buf[200];
	if (((uint)crc) == CORRECT_CRC)
		sprintf(print_buf,"OK");
	  else
		sprintf(print_buf,"WRONG RESULT CRC 0x%08x",(uint)crc);

	fprintf(fp,"%u-bit public-key op test %s, %.1fus per op\n",
		(uint)KEY_BITS,print_buf,tim.lap()*1000 / (i*(INNER_LOOP_COUNT+1))); }
	return 0;
	}

#endif
#endif
