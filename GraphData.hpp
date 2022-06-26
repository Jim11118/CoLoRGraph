#ifndef GRAPH_DATA_H__
#define GRAPH_DATA_H__

#include <vector>

#define dimension 10

struct Secret_Edge 
{
	uint32_t id_u;  	// 64 bit, [1..2^64]  ~ 4 byte
	uint32_t id_v;  	// 64 bit, [1..2^64]  ~ 4 byte
	uint32_t profile_u[dimension]; 	// 10*64 bit [0..1]
	uint32_t profile_v[dimension];	// 10*64 bit, [0..1]
	uint32_t rating; 	// 8 bit, [0..5]     	~ 1 byte
	uint32_t isReal; 	// 8 bit, [true,false]  ~ 1 byte
};
struct TSecret_Edge 
{
	uint32_t id_u1,id_u2;  	// 64 bit, [1..2^64]  ~ 4 byte
	uint32_t id_v1,id_v2;  	// 64 bit, [1..2^64]  ~ 4 byte
	uint32_t profile_u1[dimension],profile_u2[dimension]; 	// 10*64 bit [0..1]
	uint32_t profile_v1[dimension],profile_v2[dimension];	// 10*64 bit, [0..1]
	uint32_t rating1,rating2; 	// 8 bit, [0..5]     	~ 1 byte
	uint32_t isReal1,isReal2; 	// 8 bit, [true,false]  ~ 1 byte
};


//typedef Secret_Edge SecretEdgeMAC;



// secret shared values
struct Half_Secret_Edge {
	uint32_t profile_u[dimension];
	uint32_t profile_v[dimension];
	uint32_t rating;
	uint32_t isReal;
	//SecretEdgeMAC bMACs;
};
struct THalf_Secret_Edge {
	uint32_t profile_u1[dimension],profile_u2[dimension];;
	uint32_t profile_v1[dimension],profile_v2[dimension];
	uint32_t rating1,rating2;
	uint32_t isReal1,isReal2;
	//SecretEdgeMAC bMACs;
};


//四方半边
struct FHalf_Secret_Edge {
	uint32_t profile_u1[dimension],profile_u2[dimension],profile_u3[dimension];
	uint32_t profile_v1[dimension],profile_v2[dimension],profile_v3[dimension];
	uint32_t rating1,rating2,rating3;
	uint32_t isReal1,isReal2,isReal3;
	//SecretEdgeMAC bMACs;
};




struct Secret_Node 
{
	uint32_t vertexID;  // 16 bit, [1..2^16-1]
	uint32_t Profile[dimension]; // 10*64 bit [0..1]
	uint32_t newProfile[dimension]; // 10*64 bit, [0..1]
	
	std::vector<Half_Secret_Edge> halfEdge;
};

struct TSecret_Node 
{
	uint32_t vertexID1,vertexID2;  // 16 bit, [1..2^16-1]
	uint32_t Profile1[dimension],Profile2[dimension]; // 10*64 bit [0..1]
	uint32_t newProfile1[dimension],newProfile2[dimension]; // 10*64 bit, [0..1]
	
	std::vector<THalf_Secret_Edge> halfEdge;
};

//四方节点数据
struct FSecret_Node 
{
	uint32_t  vertexID1,vertexID2,vertextID3;  // 16 bit, [1..2^16-1]
	uint32_t Profile1[dimension],Profile2[dimension],Profile3[dimension]; // 10*64 bit [0..1]
	uint32_t newProfile1[dimension],newProfile2[dimension],newProfile3[dimension]; // 10*64 bit, [0..1]
	
	std::vector<FHalf_Secret_Edge> halfEdge;
};




struct UpdateNode
{
	uint32_t newProfile[dimension];
};
struct TUpdateNode
{
	uint32_t newProfile1[dimension];
	uint32_t newProfile2[dimension];
};


struct FUpdateNode
{
	uint32_t newProfile1[dimension];
	uint32_t newProfile2[dimension];
	uint32_t newProfile3[dimension];
};

#endif

