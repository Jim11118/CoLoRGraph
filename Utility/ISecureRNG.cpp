#include <cassert>
#include <iostream>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include "CryptoUtility.h"
#include "ISecureRNG.h"

#include <emp-tool/utils/block.h>
#include <emp-tool/utils/prp.h>
#include <emp-tool/utils/prg.h>

#if !defined (__AES__)
    #error "AES-NI instructions not enabled"
#endif


namespace Utility
{
// 	unsigned char *InsecureSeededRNG::GetByteArray(int length)
// 	{
// 		unsigned char *ret = new unsigned char[length];
// 		RAND_bytes(ret, length);;
// 		return ret;
// 	}

// // 	unsigned char *InsecureSeededRNG::GetInt64Array(int length)
// // 	{
// // 		unsigned char *ret = new unsigned char[length*sizeof(int64_t)];
// // 		RAND_bytes(ret, length);;
// // 		return ret;
// // 	}
	
	/// Initialize an AES encryptor/decryptor
	AESRNG::AESRNG(unsigned char * seed)
	{
		key = new unsigned char[32];
		for(int idx = 0; idx < 32; idx++)
		{
			key[idx] = seed[idx];
		}
		
		prg = new emp::PRG(seed);
		counter = 0;
	}

	unsigned char * AESRNG::GetByteArray()
	{
		counter++;
		unsigned char *temp = (unsigned char *)(&counter);
		
		return Generate(temp, 4*sizeof(unsigned char), key);
	}
	
	std::vector<uint16_t> AESRNG::GetUInt16Array(int length)
	{
		std::vector<uint16_t> ret(length);
		
		for(int idx = 0; idx < length/8; idx++)
		{
			uint16_t *prng = (uint16_t *)GetByteArray();
			
			ret[8*idx] = prng[0];
			ret[8*idx + 1] = prng[1];
			ret[8*idx + 2] = prng[2];
			ret[8*idx + 3] = prng[3];
			ret[8*idx + 4] = prng[4];
			ret[8*idx + 5] = prng[5];
			ret[8*idx + 6] = prng[6];
			ret[8*idx + 7] = prng[7];
			
			delete [] prng;
		}
		
		if(length - 8*(length/8) > 0)
		{
		    uint16_t *prng = (uint16_t *)GetByteArray();

		    for(int idx = 0; idx < length - 8*(length/8); idx++)
		    {
			    ret[8*(length/8) + idx] = prng[idx];
		    }
		    delete [] prng;
		}
		
		return ret;
	}
	std::vector<uint32_t> AESRNG::GetUInt32Array(int length)
	{
		std::vector<uint32_t> ret(length);
		
		for(int idx = 0; idx < length/4; idx++)
		{
			uint32_t *prng = (uint32_t *)GetByteArray();
			
			ret[4*idx] = prng[0];
			ret[4*idx + 1] = prng[1];
			ret[4*idx + 2] = prng[2];
			ret[4*idx + 3] = prng[3];
			
			delete [] prng;
		}
		
		if(length - 4*(length/4) > 0)
		{
		    uint32_t *prng = (uint32_t *)GetByteArray();

		    for(int idx = 0; idx < length - 4*(length/4); idx++)
		    {
			    ret[4*(length/4) + idx] = prng[idx];
		    }
		    delete [] prng;
		}
		
		return ret;
	}
	uint32_t AESRNG::GetUInt32Array_1(){
		// std::vector<uint64_t> temp = GetUInt64Array(1);
		// cout<<"??????temp"<<endl;
		// cout<<temp[0]<<endl;
		// cout<<"??????temp"<<endl;
		uint32_t c = 0;
		//delete temp;
		return c;
	}
	
	uint64_t AESRNG::GetUInt64Array_1(){
		// std::vector<uint64_t> temp = GetUInt64Array(1);
		// cout<<"??????temp"<<endl;
		// cout<<temp[0]<<endl;
		// cout<<"??????temp"<<endl;
		uint64_t c = 0;
		//delete temp;
		return c;
	}
	std::vector<uint64_t> AESRNG::GetUInt64Array(int length)
	{
		//cout<<"????????????64?????????"<<endl;
		std::vector<uint64_t> ret(length);
#if !defined (__AES__)
		//cout<<"AES????????????"<<endl;																																																																																																																																																																																																																																																																																																																																																																				
		for(int idx = 0; idx < length; idx++)
		{
			unsigned char *prng = GetByteArray();
			ret[idx] = *((unsigned int *)prng);
			
			delete [] prng;
		}
#else
		//cout<<"AES????????????"<<endl;	
		emp::block *data = new emp::block[(1+length)/2];
		cout<<"?????????prg"<<endl;
		//?????????????????????????????????????????????
		prg->random_block(data, length/2);
		cout<<"temp"<<endl;
		uint64_t *temp = (uint64_t *)data;
		cout<<"??????"<<endl;
		for(int idx = 0; idx < length; idx++)
		{
			ret[idx] = temp[idx];
		}
		cout<<"??????"<<endl;
		delete [] data;
		cout<<"????????????"<<endl;
#endif
         cout<<"????????????"<<endl;
		return ret;
	}
	
// 	std::vector<unsigned __int128> AESRNG::GetUInt128Array(int length)
// 	{
// 		std::vector<unsigned __int128> ret(length);
// 		
// 		for(int idx = 0; idx < length; idx++)
// 		{
// 			unsigned __int128 *prng = (unsigned __int128 *)GetByteArray();
// 			
// 			ret[idx] = prng[0];
// 			
// 			delete [] prng;
// 		}
// 		
// 		return ret;
// 	}
	
	std::vector<uint32_t> AESRNG::GetMaskArray(int length)
	{
		return GetUInt32Array(length);
	}

	unsigned char * AESRNG::Generate(unsigned char * plaintext, int plaintext_len, unsigned char * key)
	{
		unsigned char *ciphertext = new unsigned char[16];
		int ciphertext_len;
		
		EVP_CIPHER_CTX *ctx;

		int len;

		/* Create and initialise the context */
		if(!(ctx = EVP_CIPHER_CTX_new())){
// 			handleErrors();
		}

		/* Initialise the encryption operation. IMPORTANT - ensure you use a key
		* In this example we are using 256 bit AES (i.e. a 256 bit key). 
		*/
		if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, key, NULL))
		{
// 			handleErrors();
		}

		/* Provide the message to be encrypted, and obtain the encrypted output.
		* EVP_EncryptUpdate can be called multiple times if necessary
		*/
		if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) 
		{
// 			handleErrors();
		}
		
		ciphertext_len = len;

		/* Finalise the encryption. Further ciphertext bytes may be written at
		* this stage.
		*/
		if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
		{
// 			handleErrors();
		}
		
		ciphertext_len += len;

		/* Clean up */
		EVP_CIPHER_CTX_free(ctx);
		
		return ciphertext;
	}
}
