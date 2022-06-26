#ifndef CRYPTO_UTILITY_H__
#define CRYPTO_UTILITY_H__

#pragma once
#include <cassert>
#include <stdint.h>
#include <vector>
#include <bitset>
#include "Communicator.h"
#include "../GraphData.hpp"

#define PRECISION_BIT_LENGTH 16
#define DIM 10

namespace Utility
{
	std::ostream& operator<<( std::ostream& dest, __int128_t value );
	std::ostream& operator<<( std::ostream& dest, __uint128_t value );
	
	class VectorOperation
	{
	public:
		static std::vector<uint32_t> Add(const std::vector<uint32_t>& x, const std::vector<uint32_t>& y);
		static std::vector<uint32_t> Sub(const std::vector<uint32_t>& x, const std::vector<uint32_t>& y);
		static std::vector<uint32_t> Mul(const uint32_t x, const std::vector<uint32_t>& y);
		static std::vector<uint32_t> Mul(const std::vector<uint32_t>& x, const uint32_t y);
		static std::vector<uint32_t> Mul(const std::vector<uint32_t>& x, const std::vector<uint32_t>& y);
		static std::vector<uint32_t> Dot(const std::vector<uint32_t>& x, const std::vector<uint32_t>& y);

		static std::vector<int32_t> Add(const std::vector<int32_t>& x, const std::vector<int32_t>& y);
		static std::vector<int32_t> Sub(const std::vector<int32_t>& x, const std::vector<int32_t>& y);
		static std::vector<int32_t> Mul(const int32_t x, const std::vector<int32_t>& y);
		static std::vector<int32_t> Mul(const std::vector<int32_t>& x, const int32_t y);
		static std::vector<int32_t> Mul(const std::vector<int32_t>& x, const std::vector<int32_t>& y);
		static std::vector<int32_t> Dot(const std::vector<int32_t>& x, const std::vector<int32_t>& y);
	};
	
	class CryptoUtility
	{
	public:
		static std::vector<uint32_t> SampleSmallInput(int length);
		
		static std::vector<unsigned char> SampleByteArray(int length);

		static std::vector<unsigned char> ComputeHash(std::vector<unsigned char> msg);
		
		static int32_t * SampleInt32Array(int length);
		
		static std::vector<uint32_t>  SampleUInt32Array(int length);
		
		static std::vector<int> buildMaskIndex(const std::vector<int>& itemsPerUser);
		
		template<typename T>
		static void shuffle(std::vector<T>& data, const std::vector<unsigned char>& seed);

		template<typename T>
		static void shuffle(std::vector<T>& data, __int128& seed);
	};
	
	class ArithmeticOperation
	{
	public:
		static uint32_t add(uint32_t x, uint32_t y){return x + y;}
		static uint32_t sub(uint32_t x, uint32_t y){return x - y;}
		static uint32_t mul(uint32_t x, uint32_t y){return (x * y) >> PRECISION_BIT_LENGTH;}
		static uint32_t div(uint32_t x, uint32_t y){return (x << PRECISION_BIT_LENGTH) / y;}
	};
	
	class ArrayEncoder
	{
	public:
		static std::vector<uint32_t> UCharVec2UInt32tVec(std::vector<unsigned char> array);
		
		static std::vector<unsigned char> Encode(std::vector<uint32_t> array);	
		static std::vector<unsigned char> Hash(std::vector<uint32_t> array);
		
		static std::vector<uint32_t>  Decodeuint32_t(std::vector<unsigned char> &array);
		//编码一个二维的vectorS数组，返回一个char类型的vector。
		static std::vector<unsigned char> EncodeUInt32Array(std::vector<std::vector<uint32_t>> array);
		static std::vector<std::vector<uint32_t> > DecodeUInt32Array(std::vector<unsigned char> array);
		
		/// Convert a 4-byte array to unsigned int value
		static int byteArrayToInt32(unsigned char *byteArray);

		/// Convert an unsigned int to 4-byte array
		static unsigned char * int32ToByteArray(unsigned int val);
		
		static std::vector<unsigned char> MacVectorToByteArray(std::vector<uint32_t> MACedEdges);
		
		static std::vector<unsigned char> MacVectorToByteArray(uint32_t *MACedEdges, int size);
		
		static std::vector<uint32_t> ByteArrayToMacVector(std::vector<unsigned char> byteArray);
	};
	
	// 40:  	87, 167, 195, 203, 213, 285, 293, 299, 389, 437
	class DistributedMACGenerator
	{
	public:
		DistributedMACGenerator(uint32_t alpha, uint32_t beta);
		
		uint32_t computeMAC(int party, std::vector<uint32_t> &data, uint32_t random);
		
		__int128 Z64;
	private:
		const uint32_t p = 0x10000000000 - 293;
		
		uint32_t alpha;
		uint32_t beta;
		
		std::vector<uint32_t> coeff;
	};
	
	class SPDZ2kMAC
	{
	public:
		SPDZ2kMAC(uint32_t alpha, Communicator *com);
		uint32_t computeMAC(int party, uint32_t data, uint32_t random);
		uint32_t singleCheck(int party, uint32_t mac, uint32_t share);
// 		uint32_t multipleCheck(int party, std::vector<uint32_t> mac, std::vector<uint32_t> share);
// 	private:
		uint32_t alpha;
		Communicator *com;
	};
	
	class TestUtility
	{
	public:
		static void PrintByteArray(const std::vector<unsigned char>& array, const std::string& str);
		static void PrintVector(const std::vector<int16_t>& input, const std::string& str);
		static void PrintVector(const std::vector<uint16_t>& input, const std::string& str);
		static void PrintVector(const std::vector<uint32_t>& input, const std::string& str);
		static void PrintVector(const std::vector<std::vector<uint32_t> >& input, const std::string& str, float scale);
		static void PrintMask(const std::vector<uint32_t>& masks, const std::vector<int>& maskIndex, const std::vector<int>& itemsPerUser, const std::string& str);
		static std::vector<std::vector<uint32_t> > GenerateInput(const std::vector<int>& itemsPerUser, int inputLength);
	};
}

#endif
