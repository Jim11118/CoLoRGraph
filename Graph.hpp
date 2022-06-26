#ifndef GRAPH_H__
#define GRAPH_H__

#pragma once

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <math.h>
#include <cstring>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <bitset>
#include <set>
// #ifdef UNIX_PLATFORM
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>

#include "Utility/Range.h"
#include "Utility/Timer.h"
#include "Utility/Communicator.h"
#include "Utility/CommunicatorBuilder.h"
#include "Utility/ISecureRNG.h"
#include "Utility/CryptoUtility.h"

#include "Circuit/ArithmeticGate.h"
#include "Circuit/ArithmeticCircuit.h"
#include "Circuit/ArithmeticCircuitBuilder.h"
#include "Circuit/LayeredArithmeticCircuit.h"
#include "Circuit/RandomArithmeticCircuitBuilder.h"
#include "Circuit/LayeredArithmeticCircuitBuilder.h"
#include "Circuit/PlainArithmeticCircuitEvaluator.h"
#include "Circuit/GradientDescendCircuitBuilder.h"


#include "TwoPartyMaskedEvaluation/PreprocessingBuilder.h"
#include "TwoPartyMaskedEvaluation/SimplePreprocessingBuilder.h"
#include "TwoPartyMaskedEvaluation/MaskedEvaluation.h"

#include "CrossCheck/PreprocessingParty.h"
#include "CrossCheck/Player.h"

#include "Machine.hpp"
#include "GraphData.hpp"

using namespace std;
using namespace std::chrono;
using namespace Circuit;
using namespace Utility;
using namespace CrossCheck;
using namespace TwoPartyMaskedEvaluation;

// const int dimension = 10;

double GAMMA = 0.001;
double LAMBDA = 0.0;
double MU = 0.02;
	
class Graph 
{ 
public:
	int totalMachines;
	int totalParties;
	int machineId;
	int party;
	int group;
	//在这里定义两个partner
	int partner1;
	int partner2;
	int totalEdges;
	int totalUsers;
	int totalItems;	
	double epsilon;

	int nUsers, nItems, nNodes, nEdges;
	int firstLightMachineID_User;
	int firstLightMachineID_Item;

	Machine* machine;
	Communicator *com;
	Player *player;
	//在这里定义两个rng
	AESRNG *rng1,*rng2;
	SPDZ2kMAC *MACGen;
	
	Range *inputRange;
	Range *sizeRange;
	ArithmeticCircuit *circuit;
	LayeredArithmeticCircuit *lc;

	//uint32_t sharedBeta;

	std::vector<unsigned char> alpha;// = machine->getSharedSeedForPeerAndParty(party, partner, com);
	__uint128_t alphaVal;

    //初始化方法
	Graph (int totalMachines, int machineId, int party, int totalEdges, int totalUsers, int totalItems, double epsilon, Machine* machine) {
		this->totalMachines = totalMachines;
		this->totalParties = 6;
		this->machineId = machineId;
		this->party = party;
		cout<<"********************Graph开始*****************************"<<endl;
		
		//定义各自的伙伴
		if (party == Alice || party == Bob || party == Charlie){
			partner1 = (party +1)%3;
			partner2 = (party +2)%3;
		} 
		else{
			partner1 = (party +1)%6+3;
			partner2 = (party +2)%6+3;

		}

				//定义各自的伙伴
		if (party == Alice || party == Bob || party == Charlie){
			partner1 = (party +1)%3;
			group = 1;
		} 
		else{
			group =2;
		}
		this->totalEdges = totalEdges;
		this->totalUsers = totalUsers;
		this->totalItems = totalItems;
		this->epsilon = epsilon;
		this->machine = machine;
		
		//构造通信器
		CommunicatorBuilder::BuildCommunicator(party, machine, com);
		com->machineId = machineId;
		cout << "com->machineId: " << com->machineId << " machineId: " << machineId << endl;
		
		// 初始化随机数生成器，rand1是和partener1共享的，rand2适合partener2共享的。
		//std::vector<vector<unsigned char>> rand= machine->getPartnerSharedRandomSeed(party, partner1, partner2,com);
		std::vector<unsigned char> rand1(32,0);
		std::vector<unsigned char> rand2(32,0);
		//AESRNG的构造参数中rand1是随机数种子
		rng1 = new AESRNG(rand1.data());
		rng2 = new AESRNG(rand2.data());
		
		// Each group agree on parameters for MAC computation
// 		alpha = machine->getSharedSeedForPeerAndParty(party, partner, com);
// 		alphaVal = *((__uint128_t *)(alpha.data()));
// 		alphaVal = alphaVal % 0x10000000000;

		//randoms1是一个长度为2的数组。
		// std::vector<uint32_t> randoms1 = rng1->GetUInt32Array(2);
		// std::vector<uint32_t> randoms2 = rng2->GetUInt32Array(2);
		

		//我的框架中不需要MAC
		//MACGen = new SPDZ2kMAC(randoms[0] + randoms[1], com);
		
		// if(party == Charlie || party == David)
		// {
		// 	sharedBeta = randoms[1];
		// 	com->SendAlice((unsigned char *)(&randoms[0]), sizeof(uint32_t));
		// 	com->SendBob((unsigned char *)(&sharedBeta), sizeof(uint32_t));
		// }
		// else
		// {
		// 	uint32_t s1, s2;
		// 	com->AwaitAlice((unsigned char *)(&s1), sizeof(uint32_t));
		// 	com->AwaitBob((unsigned char *)(&s2), sizeof(uint32_t));
		// 	assert(s1 == s2);
		// 	sharedBeta = s1;
		// }
		cout<<"***********************Graph初始化完成********************";
	}

	~Graph(){
		delete rng1,rng2;
		//delete MACGen;
		delete machine;
	}

	void GraphComputation() {

		int alpha = 1;  /* modify alpha */		
		// double p = 1 - (1/exp(epsilon));  // p
		// int biasedCoinBits = (int) ceil(log2(1/p)); // k
		// int alpha = (int) ceil(-39 * log(2) / log(1 - p) - (log(totalItems) / log(1 - p)));  // d or alpha
        //这里的nUsers,nItems代表每个机器本身处理的用户数目和项目数目。
		int nUsers, nItems;
		//轻机器计算的项目比重机器少哦
		int firstLightMachineID_User = totalUsers % totalMachines;
		int firstLightMachineID_Item = totalItems % totalMachines;

		/* Compute nUsers: number of Users in each machine */
		//计算nUsers,nItems,
		if (machineId < firstLightMachineID_User)
		{
			// no. of users in heavy load machine;
			nUsers = (totalUsers + totalMachines - 1)/totalMachines; 
		}
		else
		{
			// no of types in light load machine;
			nUsers = totalUsers/totalMachines; 
		}
		
		/* Compute nItems: number of Items in each machine */
		if (machineId < firstLightMachineID_Item)
		{
			nItems = (totalItems + totalMachines - 1)/totalMachines;
		}
		else
		{
			nItems = totalItems/totalMachines;
		}
		
		/* Compute nNodes & nEdges in each machine */
		//注意Nodes等于nusers和nItems的和。
		int nNodes = nUsers + nItems;
		//边数等于总边数处以机器数
		int nEdges = totalEdges / totalMachines;

		cout << "************ Input ************" << endl;
		cout << "nEdges: " << nEdges << " , nNodes:" << nNodes << " , nUsers:" << nUsers << " , nItems:" << nItems << endl;
		
		TSecret_Node * secret_nodes = new TSecret_Node[nNodes];
		TSecret_Edge * secret_edges = new TSecret_Edge[nEdges];

		//SecretEdgeMAC *MACedEdges;

		if ((secret_nodes == nullptr) || (secret_edges  == nullptr))
		{
			cout << "Error: memory could not be allocated";
		}

		/* Alice & Bob&Charlie generate data */
		if (party == Alice || party == Bob || party == Charlie)
		{
			SecretSharing(party, partner1,partner2, secret_edges, nEdges,nNodes, secret_nodes, nUsers, nItems, alpha);
		}

		// std::cout << "--------------- Original Nodes -----------------" << std::endl;
		// //printNodes(secret_nodes, nNodes);
		// //printNodes(secret_nodes, nNodes);
		// std::cout << "--------------- Original Edges -----------------" << std::endl;
		// // printEdges(secret_edges, nEdges);
		cout<<"********************原始数据***********"<<endl;
		cout<<secret_edges[0].id_v1<<"****"<<secret_edges[0].id_v2<<endl;

		//**************************  Shuffle **********************************************************************************************************************************
		auto start = high_resolution_clock::now();
		Timer t;

		// std::cout << "---------------" << machineId << " Start MACs -----------------" << std::endl;
		// if (party == Alice || party == Bob)
		// {
		// 	MACedEdges = computeMAC(party, secret_edges, nEdges);
		// 	com->SendVerificationPartner((unsigned char *)secret_edges, nEdges*sizeof(Secret_Edge));
		// 	com->SendVerificationPartner((unsigned char *)MACedEdges, nEdges*sizeof(SecretEdgeMAC));
		// }
		// else
		// {
		// 	MACedEdges = new SecretEdgeMAC[nEdges];
		// 	com->AwaitVerificationPartner((unsigned char *)secret_edges, nEdges*sizeof(Secret_Edge));
		// 	com->AwaitVerificationPartner((unsigned char *)MACedEdges, nEdges*sizeof(SecretEdgeMAC));
		// }

		// std::cout << machineId << ": finished MAC....." << std::endl;


   		/* Charlie & David shuffle edges and Mac, re-randomize them and send them back to Alice & Bob */
		//Secret_Edge   *shuffledEdges;
		TSecret_Edge   *shuffledEdges;
	
		//Alice Bob Charlie shuffle edges send to  David Erik Frank
		if (party == Alice || party == Bob || party ==Charlie )
		{
			/* Generate & Distribuite permutation seeds to the other party */
			//生成shuffule的种子给其他参与方
			std::cout << machineId << ": start seed " << endl;
			//这里的seed是所有机器都要使用的同一个种子。
			std::vector<unsigned char> seed(32,0);

			__uint128_t permSeed = *((__uint128_t *)(seed.data()));
			std::cout << machineId << " end seed" << endl;
			//定义了一个全部边的shuffule数组，注意是int类型
			std::vector<int> shuffledArray(totalEdges);
			//iota 依次填充
			std::iota (std::begin(shuffledArray), std::end(shuffledArray), 0);  //Fills the range [first, last) with sequentially increasing values, starting with value 0.

			std::mt19937 g(permSeed);  // permutation function
			std::shuffle(shuffledArray.begin(), shuffledArray.end(), g);  // permute an integer string
			
			std::cout << machineId << ": Shuffling edges....." << std::endl;
			shuffledEdges = Shuffle<TSecret_Edge>(party, partner1,partner2, shuffledArray, secret_edges, nEdges);
			
			// std::cout << machineId << ": Shuffling macs....." << std::endl;
			// shuffledMACs = Shuffle<SecretEdgeMAC>(party, partner, shuffledArray, MACedEdges, nEdges);
			
			//std::cout << machineId << ": Re-randomize shares....." << std::endl;
			//rerandomize(party, shuffledEdges, nEdges);
			//rerandomize(party, shuffledMACs, nEdges);
			//cout<<shuffledEdges[0].id_v1<<"****"<<shuffledEdges[0].id_v2<<endl;

			// Send to verification partner
			//此处验证的伙伴对应关系为Alice对应David,Bob 对应Eric,Charlie 对应 Frank
			auto startsenddata= high_resolution_clock::now();
			com->SendOtherGroupPartner(party,(unsigned char *)(shuffledEdges),nEdges*sizeof(TSecret_Edge));
			//com->SendVerificationPartner((unsigned char *)(shuffledMACs), nEdges*sizeof(SecretEdgeMAC));
			//需要把shuffleEdges中的数据的1和2拿出来，做一个hash，发送给对应的参与方，先把大框架写好，在写这个.
		    auto endsenddata= high_resolution_clock::now();
			auto SendDuration = duration_cast<milliseconds>(endsenddata - startsenddata);
		     cout << "phase" << "SendDuration " << SendDuration.count() <<endl;
		}
		else
		{ 
			shuffledEdges = new TSecret_Edge[nEdges];
			//shuffledMACs  = new SecretEdgeMAC[nEdges];
			auto startreceivedata= high_resolution_clock::now();
			com->AwaitOtherGroupPartner(party,(unsigned char *)(shuffledEdges), nEdges*sizeof(TSecret_Edge));
			auto endreceivedata= high_resolution_clock::now();
			auto ReceiveDuration = duration_cast<milliseconds>(endreceivedata- startreceivedata);
	         cout << "phase" << "ReceiveDuration " << ReceiveDuration.count() <<endl;
		}

		delete[] secret_edges;
		//delete[] MACedEdges;

		std::cout << machineId << ": --------------- Shuffled Edges -----------------" << std::endl;
		// printEdges(shuffledEdges, nEdges);

		auto ShuffleDone = high_resolution_clock::now(); 
		auto ShuffleDuration = duration_cast<milliseconds>(ShuffleDone - start);
         cout << "phase" << "ShuffleDuration " << ShuffleDuration.count() <<endl;
		

   // **************************  Gather **********************************************************************************************************************************

		// // generate macs and send them to Alice and Bob
		// if (party == Charlie || party == David)日
		// {
		// 	// std::cout << machineId << ": Mac the id_u, id_v, and data" << std::endl;		
		// 	bMACs = computeMAC(party, shuffledEdges, nEdges);
		// 	com->SendVerificationPartner((unsigned char *)bMACs, nEdges*sizeof(SecretEdgeMAC));
		// }
		// else
		// { 
		// 	bMACs = new SecretEdgeMAC[nEdges];
		// 	com->AwaitVerificationPartner((unsigned char *)bMACs, nEdges*sizeof(SecretEdgeMAC));
		// }


		bool direction = true;
		std::vector<int> elementsPerNode;
		std::vector<std::vector<uint32_t> > input;
		std::vector<uint32_t> OpenedVertexIds(nEdges);
		// std::vector<uint32_t> MACId(nEdges);
		// std::vector<std::vector<uint32_t> > inputMAC;


		//
		if(party == David || party == Eric || party == Frank)
		{
			// D, E, F gather the data。
			cout << machineId << ": ------- Gathered Edges -------" << endl;
			OpenedVertexIds = Gather(party, partner1 ,partner2, shuffledEdges,nEdges, secret_nodes, nUsers, nItems, direction);
			
			// Send re-randomized OpenedId & MACId to Charlie & David so that they can verify the MAC
			// Alice & Bob get the same PRNG and re-rand Ids & MACId
			// std::vector<uint32_t> Id1(nEdges), Id2(nEdges);
			// std::vector<unsigned char> seed = machine->getSharedRandomSeed(party, partner, com);
			// AESRNG *rng = new AESRNG(seed.data());
			
			// std::vector<uint32_t> randUInt32 = rng->GetUInt32Array(nEdges);
			
			// for(int idx = 0; idx < nEdges; idx++)
			// {
			// 	Id1[idx] = randUInt32[idx];
			// 	Id2[idx] = OpenedVertexIds[idx] - Id1[idx];
			// }
			
			// randUInt32 = rng->GetUInt32Array(nEdges);
			
			// for(int idx = 0; idx < nEdges; idx++)
			// {
			// 	if(party % 2 == 0)
			// 	{
			// 		MACId[idx] += randUInt32[idx];
			// 	}
			// 	else
			// 	{
			// 		MACId[idx] -= randUInt32[idx];
			// 	}
			// }
			
			// // Send Id1 to Charlie, Id2 to David, MACId to Verification partner
			// if(party == ALICE)
			// {
			// 	com->SendVerificationPartner((unsigned char *)(Id1.data()), nEdges*sizeof(uint32_t));
			// 	std::vector<unsigned char> hashedId2 = ArrayEncoder::Hash(Id2);
			// 	com->SendNonPartner(hashedId2.data(), hashedId2.size());
			// }
			// else
			// {
			// 	// Send the hash of the vector
			// 	com->SendVerificationPartner((unsigned char *)(Id2.data()), nEdges*sizeof(uint32_t));
			// 	std::vector<unsigned char> hashedId1 = ArrayEncoder::Hash(Id1);
			// 	com->SendNonPartner(hashedId1.data(), hashedId1.size());
			// }
			
			// com->SendVerificationPartner((unsigned char *)(MACId.data()), nEdges*sizeof(uint32_t));
		}
		// else
		// {
		// 	// Receive Ids, hashed ids, and MACId
		// 	std::vector<uint32_t> Id(nEdges);
		// 	std::vector<unsigned char> hashedId(32);
			
		// 	com->AwaitVerificationPartner((unsigned char *)(Id.data()), nEdges*sizeof(uint32_t));
		// 	com->AwaitNonPartner(hashedId.data(), 32);
			
		// 	com->AwaitVerificationPartner((unsigned char *)(MACId.data()), nEdges*sizeof(uint32_t));
			
		// 	std::vector<unsigned char> hashedId2 = ArrayEncoder::Hash(Id);
		// 	assert(hashedId == hashedId2);
		// 	verifyMAC(party, MACId, Id);

		// 	int size;
		// 	com->AwaitVerificationPartner((unsigned char *)(&size), sizeof(int));
		// 	elementsPerNode.resize(size);
		// 	com->AwaitVerificationPartner((unsigned char *)elementsPerNode.data(), size*sizeof(int));
		// }
		cout<<"************************Gather完成******************"<<endl;


		auto GatherDone = high_resolution_clock::now();
		auto GatherDuration = duration_cast<milliseconds>(GatherDone - ShuffleDone);


   // **************************  Transform**********************************************************************************************************************************
   //在应用阶段前首先三个人的share转为四个人share，假设选择ALice作为转换对象。
		//std::cout << "------------------transform to four party share-------------------" << std::endl;
		//以true为准
		// FSecret_Node * f_secret_nodes;
		if(direction){
			if(party == David || party == Eric || party == Frank){
			//需要三个人把各自的数据发送给Alice,用于参与四方计算
			// share_transform(party,secret_nodes,f_secret_nodes,nNodes,nUsers,nItems,direction);
				//这里生成中间数据
			std::cout << machineId << ": --------- Generate circuit metadata -------------" << std::endl;
			for(int idx = 0; idx < nItems; idx++)
			{
				//这里判断大于0的原因?
				if(secret_nodes[idx+nUsers].halfEdge.size() > 0)
				{
					//elementsPerNode节点存储每个节点halfedge的大小
					elementsPerNode.push_back(secret_nodes[idx+nUsers].halfEdge.size());
					// std::cout << idx << " " << secret_nodes[idx].halfEdge.size() << " " << secret_nodes[idx].vertexID << endl;
				}
			}
			if(party == David){
				int size = elementsPerNode.size();
			    com->SendEvaluationAlice((unsigned char *)(&size), sizeof(int));
			    com->SendEvaluationAlice((unsigned char *)elementsPerNode.data(), size*sizeof(int));
			}
		}
		if(party == Alice){
			std::cout << machineId << ": --------- Receive circuit metadata -------------" << std::endl;
		 	int size;
		    com->AwaitEvaluationAlice((unsigned char *)(&size), sizeof(int));
		    elementsPerNode.resize(size);
		    com->AwaitEvaluationAlice((unsigned char *)elementsPerNode.data(), size*sizeof(int));

		}
		}
		else{
			cout<<"transform_error"<<endl;
		}

   // **************************  Apply **********************************************************************************************************************************
   //在应用阶段首先构造电路
	
		std::cout << "------------------- Running 4 Party Protocol --------------------" << std::endl;
		//定义了一个项目数目的变量，因为在gather阶段公开的是vid。
		if(direction){
			if(party == Alice || party == David || party == Eric || party == Frank){

			////定义了一个表示项目数目的变量，每个数组中的元素的含义是每个item对应的用户个数
			int numUsers = elementsPerNode.size();
			int sumNumItems = 0;
			for(int idx = 0; idx < elementsPerNode.size(); idx++)
			{
				sumNumItems += elementsPerNode[idx];
			}
			//定义mask数量
			int numMasks = 6*DIM*numUsers + (2*DIM + 4)*sumNumItems;

			//定义线的数目
			int numWires = 6*numUsers + 6*sumNumItems;
			//把参与方和机器id写入text文件。
			std::stringstream ss;
			ss << "circuit"; 
			ss << party; 
			ss << machineId; 
			ss << ".txt";
			

			/*
			double GAMMA = 0.001;
			double LAMBDA = 0.0;
			double MU = 0.02;
			*/
			//算术电路构建
			ArithmeticCircuitBuilder circuitBuilder;
			//层电路构建
			LayeredArithmeticCircuitBuilder layerBuilder;
			//电路等于梯度下降构建
			circuit = GradientDescendCircuitBuilder::BuildMultipleUsersByLayers(elementsPerNode, GAMMA, MU);
			//lc为创建的按层的电路
			lc = layerBuilder.CreateLayeredCircuit(circuit);

			cout<<"********************电路创建完成***************"<<endl;
			// std::cout << lc->ToString() << std::endl;
			
			int inputSize = circuit->InputLength;
			int outputSize = circuit->OutputLength;;
			int circuitSize = circuit->NumWires;
			//输入范围
			inputRange = new Range(0, inputSize);
			//输出范围
			Range *outputRange = new Range(numWires-numUsers, numUsers);


			
		
			// // input = TestUtility::GenerateInput(elementsPerNode, inputSize);
			// std::cout << "Get shared masked input: " << inputSize << std::endl;
			std::vector<std::vector<uint32_t>> input1(inputSize);
			std::vector<std::vector<uint32_t>> input2(inputSize);
			std::vector<std::vector<uint32_t>> input3(inputSize);
			cout<<"**************获得电路输入*************"<<endl;

			getFourSharedInput(party,secret_nodes,nNodes, nUsers, inputSize, direction,input1,input2,input3);
			// std::cout << "Get betamac " << inputSize << std::endl;
			//假设到这里input1,input2,input3，已经赋值，可能存在问题的地方包括传输node数据时候可能会出错


			// inputMAC = getBetaMAC(secret_nodes, nNodes, nUsers, inputSize, direction);
			std::cout << "getencode " << inputSize << std::endl;
			//定义不同的参与方
			AESRNG *rng_124;
			AESRNG *rng_234;
			AESRNG *rng_134;

			//生成rng124
			if(party == David || party == Eric || party == Alice){
				vector<unsigned char> rand(32,0);
		            //AESRNG的构造参数中rand1是随机数种子
					rng_124 = new AESRNG(rand.data());
					cout<<"获得的rng_124:"<<endl;
					cout<<rng_124<<endl;
			}
			//生成rnd234
			if(party == Eric || party == Frank || party == Alice){
				vector<unsigned char> rand(32,0);
		            //AESRNG的构造参数中rand1是随机数种子
					rng_234 = new AESRNG(rand.data());
					cout<<"获得的rng_234:"<<endl;
					cout<<rng_234<<endl;
	
			}
			//生成rnd134
			if(party == David || party == Frank || party == Alice){
				vector<unsigned char> rand(32,0);
		            //AESRNG的构造参数中rand1是随机数种子
					rng_134 = new AESRNG(rand.data());
					cout<<"获得的rng_134:"<<endl;
					cout<<rng_134<<endl;

			}

			if (party == David)   
			player = new PlayerOne(com, inputRange,rng_124,rng_234,rng_134);
			if (party == Eric)     
			player = new PlayerTwo(com, inputRange,rng_124,rng_234,rng_134);
			if (party == Frank) 
			player = new PlayerThree(com, inputRange,rng_124,rng_234,rng_134);
			if (party == Alice)   
			player = new PlayerFour(com, inputRange,rng_124,rng_234,rng_134);
				
			std::cout << "Executing the circuit" << std::endl;
			// For the testing purpose, the output is the reconstructed one, not the shared one
			//重点编写这里的run函数
			//lc是电路，基本不变，input是输入数据，可能需要调整。
			std::vector<std::vector<uint32_t>> output1;
			std::vector<std::vector<uint32_t>> output2;
			std::vector<std::vector<uint32_t>> output3;
			player->Run(lc, input1,input2,input3,outputRange,elementsPerNode,output1,output2,output3);

			// // Print the output
			// for(int idx = 0; idx < 10; idx++) std::cout << output[0][idx] << " ";
			// std::cout << std::endl;

			} 
		}
		auto ApplyDone = high_resolution_clock::now();
		auto ApplyDuration = duration_cast<milliseconds>(ApplyDone - GatherDone);


   // **************************  Scatter **********************************************************************************************************************************
		cout << "------- Scattered Edges -------" << endl;
		TSecret_Edge * ScatteredEdges = new TSecret_Edge[nEdges];
		if (party == David || party == Eric || party==Frank)
		{
			ScatteredEdges = Scatter(party, partner1,partner2, OpenedVertexIds, shuffledEdges, nEdges, secret_nodes, nUsers, nItems, direction);
		}
		// cout << "------- Scattered Edges -------" << endl;
		// printEdges(ScatteredEdges, nEdges, party, Alice, Bob);

		auto ScatterDone = high_resolution_clock::now();
		auto ScatterDuration = duration_cast<milliseconds>(ScatterDone - ApplyDone);
		
		delete[] secret_nodes;
		// delete[] secret_edges;


   // **************************  Timing **********************************************************************************************************************************
		
		// std::cout << "Total data sent:      " << com->sendingSize << " bytes" << std::endl;
		// std::cout << "Total data received : " << com->receivingSize << " bytes" << std::endl;

		std::cout << "dataSentReceived" << " " << com->sendingSize << " " << com->receivingSize << std::endl;
		cout << "phase" << " " << ShuffleDuration.count() << " " << GatherDuration.count() << " " << ApplyDuration.count() <<" " << ScatterDuration.count() << endl;
		t.Tick("GraphComputation() ");
		cout << "phase" << "ShuffleDuration " << ShuffleDuration.count() <<endl;
	}

	// void share_transform(int party, TSecret_Node * secret_nodes,FSecret_Node *  f_secret_nodes,int nNodes,int nUsers,int nItems,bool isEdgeIncoming){
	// 	//这个算法中，把三人秘密共享转换成四人秘密共享，首先三个人协商一个rng，用来生成随机数，也就是x4。
	// 	vector<unsigned char>  seed = machine->getSingleComSharedRandomSeed(party, partner1,partner2, com);
	// 	AESRNG *rng = new AESRNG(seed.data());
	// 	//填充u.v	
	// 	std::vector<uint32_t> randUInt32_1 = rng->GetUInt32Array((2*dimension + 2)*nNodes);
	// 	if(isEdgeIncoming){
	// 		//f_secret_nodes 新建一个nItems大小的数据，存储用于参与四方计算的数据。
	// 		f_secret_nodes = new FSecret_Node[nItems];
	// 		std::vector<uint32_t> randUInt16_1 = rng->GetUInt32Array(nItems);
	// 		//std::vector<uint32_t> randUInt32_1 = rng->GetUInt32Array(2*nItems*dimension);
	// 		int counter16=0;
	// 		//int counter64=0;
	// 		if(party==Eric){
	// 			Secret_Node  * t_nodedata = new Secret_Node[nItems];
	// 			for(uint32_t idx = nUsers; idx < nNodes; idx++){
	// 			f_secret_nodes[idx-nUsers].vertexID1=secret_nodes[idx].vertexID1;
	// 			t_nodedata[idx-nUsers].vertexID = f_secret_nodes[idx].vertexID1;
	// 			f_secret_nodes[idx-nUsers].vertexID2=secret_nodes[idx].vertexID2;
	// 			f_secret_nodes[idx-nUsers].vertextID3=randUInt16_1[counter16];counter16++;
	// 			//先不修改Profile的内容。
	// 			// for(uint32_t k=0; k<dimension;k++){
	// 			// 	f_secret_nodes[idx].Profile1[k]=secret_nodes[idx].Profile1[k];
	// 			// 	f_secret_nodes[idx].Profile2[k]=secret_nodes[idx].Profile2[k];
	// 			// 	f_secret_nodes[idx].Profile3[k]=randUInt32_1[counter64];counter64++;

	// 			// 	f_secret_nodes[idx].newProfile1[k]=secret_nodes[idx].newProfile1[k]

	// 			// }
	// 			uint32_t halfedge_len=secret_nodes[idx].halfEdge.size();
	// 			std::vector<uint32_t> randUInt32_2 = rng->GetUInt32Array(halfedge_len*2+10+halfedge_len*dimension);
	// 			int count32_1 = 0;
	// 			for(uint32_t cdx=0;k<halfedge_len;k++){
	// 				// Get v (v is the same for all edges)
	// 				FHalf_Secret_Edge  half_edgedata;
	// 				Half_Secret_Edge t_half_edgedata;
	// 				if(cdx==0){
	// 					for(uint32_t kdx = 0; kdx < dimension; kdx++){
	// 						half_edgedata.profile_v1[kdx]=secret_nodes[idx].halfEdge[cdx].profile_v1[kdx];
	// 						t_half_edgedata.profile_v[kdx]=half_edgedata.profile_v1[kdx];
	// 						half_edgedata.profile_v2[kdx]=secret_nodes[idx].halfEdge[cdx].profile_v2[kdx];
	// 						half_edgedata.profile_v3[kdx]=randUInt32_2[count32_1];
	// 						count32_1++;
	// 					}	
	// 				}
	// 				for(uint32_t kdx = 0; kdx < dimension; kdx++){
	// 					half_edgedata.profile_u1[kdx]=secret_nodes[idx].halfEdge[cdx].profile_u1[kdx];
	// 					t_half_edgedata.profile_u[kdx]=half_edgedata.profile_u1[kdx];
	// 					half_edgedata.profile_u2[kdx]=secret_nodes[idx].halfEdge[cdx].profile_u2[kdx];
	// 					half_edgedata.profile_u3[kdx]=randUInt32_2[count32_1];
	// 					count32_1++;
	// 				}
	// 				half_edgedata.isReal1=secret_nodes[idx].halfEdge[cdx].isReal1;
	// 				t_half_edgedata.isReal=half_edgedata.isReal1;
	// 				half_edgedata.isReal2=secret_nodes[idx].halfEdge[cdx].isReal2;
	// 				half_edgedata.isReal3=randUInt32_2[count32_1];
	// 				count32_1++;

	// 				half_edgedata.rating1=secret_nodes[idx].halfEdge[cdx].rating1;
	// 				t_half_edgedata.rating=half_edgedata.rating1;
	// 				half_edgedata.rating2=secret_nodes[idx].halfEdge[cdx].rating2;
	// 				half_edgedata.rating3=randUInt32_2[count32_1];
	// 				count32_1++;
					
	// 				f_secret_nodes[idx-nUsers].halfEdge.push_back(half_edgedata);
	// 				t_nodedata[idx-nUsers].halfEdge.push_back(t_half_edgedata);
	// 			}
	// 		}
	// 		com->SendEvaluationHelper((unsigned char *)(t_nodedata),nItems*sizeof(Secret_Node),isEdgeIncoming);


	// 		}
	// 		//如果等于David的话
	// 		else if(party== David){
	// 			Secret_Node  * t_nodedata = new Secret_Node[nItems];
	// 			for(uint32_t idx = nUsers; idx < nNodes; idx++){
				
	// 			f_secret_nodes[idx-nUsers].vertexID1=secret_nodes[idx].vertexID1-randUInt16_1[counter16];
	// 			t_nodedata[idx-nUsers].vertexID = f_secret_nodes[idx].vertexID1;
	// 			f_secret_nodes[idx-nUsers].vertexID2=secret_nodes[idx].vertexID2;
	// 			f_secret_nodes[idx-nUsers].vertextID3=randUInt16_1[counter16];counter16++;
	// 			//先不修改Profile的内容。
	// 			// for(uint32_t k=0; k<dimension;k++){
	// 			// 	f_secret_nodes[idx].Profile1[k]=secret_nodes[idx].Profile1[k];
	// 			// 	f_secret_nodes[idx].Profile2[k]=secret_nodes[idx].Profile2[k];
	// 			// 	f_secret_nodes[idx].Profile3[k]=randUInt32_1[counter64];counter64++;

	// 			// 	f_secret_nodes[idx].newProfile1[k]=secret_nodes[idx].newProfile1[k]

	// 			// }
	// 			uint32_t halfedge_len=secret_nodes[idx].halfEdge.size();
	// 			std::vector<uint32_t> randUInt32_2 = rng->GetUInt32Array(halfedge_len*2+10+halfedge_len*dimension);
	// 			int count32_1 = 0;
	// 			for(uint32_t cdx=0;k<halfedge_len;k++){
	// 				// Get v (v is the same for all edges)
	// 				FHalf_Secret_Edge  half_edgedata;
	// 				Half_Secret_Edge t_half_edgedata;
	// 				if(cdx==0){
	// 					for(uint32_t kdx = 0; kdx < dimension; kdx++){
	// 						half_edgedata.profile_v1[kdx]=secret_nodes[idx].halfEdge[cdx].profile_v1[kdx]-randUInt32_2[count32_1];
	// 						t_half_edgedata.profile_v[kdx]=half_edgedata.profile_v1[kdx];
	// 						half_edgedata.profile_v2[kdx]=secret_nodes[idx].halfEdge[cdx].profile_v2[kdx];
	// 						half_edgedata.profile_v3[kdx]=randUInt32_2[count32_1];
	// 						count32_1++;
	// 					}
						
	// 				}
					
	// 				for(uint32_t kdx = 0; kdx < dimension; kdx++){

	// 					half_edgedata.profile_u1[kdx]=secret_nodes[idx].halfEdge[cdx].profile_u1[kdx]-randUInt32_2[count32_1];
	// 					t_half_edgedata.profile_u[kdx]=half_edgedata.profile_u1[kdx];
	// 					half_edgedata.profile_u2[kdx]=secret_nodes[idx].halfEdge[cdx].profile_u2[kdx];
	// 					half_edgedata.profile_u3[kdx]=randUInt32_2[count32_1];
	// 					count32_1++;
	// 				}
	// 				half_edgedata.isReal1=secret_nodes[idx].halfEdge[cdx].isReal1-randUInt32_2[count32_1];
	// 				t_half_edgedata.isReal=half_edgedata.isReal1;
	// 				half_edgedata.isReal2=secret_nodes[idx].halfEdge[cdx].isReal2;
	// 				half_edgedata.isReal3=randUInt32_2[count32_1];
	// 				count32_1++;

	// 				half_edgedata.rating1=secret_nodes[idx].halfEdge[cdx].rating1-randUInt32_2[count32_1];
	// 				t_half_edgedata.rating=half_edgedata.rating1;
	// 				half_edgedata.rating2=secret_nodes[idx].halfEdge[cdx].rating2;
	// 				half_edgedata.rating3=randUInt32_2[count32_1];
	// 				count32_1++;
					
	// 				f_secret_nodes[idx-nUsers].halfEdge.push_back(half_edgedata);
	// 				t_nodedata[idx-nUsers].halfEdge.push_back(t_half_edgedata);
	// 			}

	// 		}
	// 		com->SendEvaluationHelper((unsigned char *)(t_nodedata),nItems*sizeof(Secret_Node),isEdgeIncoming);

	// 		}else if(party==Frank){
	// 			Secret_Node  * t_nodedata = new Secret_Node[nItems];
	// 			for(uint32_t idx = nUsers; idx < nNodes; idx++){
				
	// 			f_secret_nodes[idx-nUsers].vertexID1=secret_nodes[idx].vertexID1-randUInt16_1[counter16];
	// 			t_nodedata[idx-nUsers].vertexID = f_secret_nodes[idx].vertexID1;
	// 			f_secret_nodes[idx-nUsers].vertexID2=secret_nodes[idx].vertexID2;
	// 			f_secret_nodes[idx-nUsers].vertextID3=randUInt16_1[counter16];counter16++;
	// 			//先不修改Profile的内容。
	// 			// for(uint32_t k=0; k<dimension;k++){
	// 			// 	f_secret_nodes[idx].Profile1[k]=secret_nodes[idx].Profile1[k];
	// 			// 	f_secret_nodes[idx].Profile2[k]=secret_nodes[idx].Profile2[k];
	// 			// 	f_secret_nodes[idx].Profile3[k]=randUInt32_1[counter64];counter64++;

	// 			// 	f_secret_nodes[idx].newProfile1[k]=secret_nodes[idx].newProfile1[k]

	// 			// }
	// 			uint32_t halfedge_len=secret_nodes[idx].halfEdge.size();
	// 			std::vector<uint32_t> randUInt32_2 = rng->GetUInt32Array(halfedge_len*2+10+halfedge_len*dimension);
	// 			int count32_1 = 0;
	// 			for(uint32_t cdx=0;k<halfedge_len;k++){
	// 				// Get v (v is the same for all edges)
	// 				FHalf_Secret_Edge  half_edgedata;
	// 				Half_Secret_Edge t_half_edgedata;
	// 				if(cdx==0){
	// 					for(uint32_t kdx = 0; kdx < dimension; kdx++){
	// 						half_edgedata.profile_v1[kdx]=secret_nodes[idx].halfEdge[cdx].profile_v1[kdx];
	// 						t_half_edgedata.profile_v[kdx]=half_edgedata.profile_v1[kdx];
	// 						half_edgedata.profile_v2[kdx]=secret_nodes[idx].halfEdge[cdx].profile_v2[kdx]-randUInt32_2[count32_1];
	// 						half_edgedata.profile_v3[kdx]=randUInt32_2[count32_1];
	// 						count32_1++;
	// 					}
	// 				}
	// 				for(uint32_t kdx = 0; kdx < dimension; kdx++){

	// 					half_edgedata.profile_u1[kdx]=secret_nodes[idx].halfEdge[cdx].profile_u1[kdx];
	// 					t_half_edgedata.profile_u[kdx]=half_edgedata.profile_u1[kdx];
	// 					half_edgedata.profile_u2[kdx]=secret_nodes[idx].halfEdge[cdx].profile_u2[kdx]-randUInt32_2[count32_1];
	// 					half_edgedata.profile_u3[kdx]=randUInt32_2[count32_1];
	// 					count32_1++;
	// 				}
	// 				half_edgedata.isReal1=secret_nodes[idx].halfEdge[cdx].isReal1;
	// 				t_half_edgedata.isReal=half_edgedata.isReal1;
	// 				half_edgedata.isReal2=secret_nodes[idx].halfEdge[cdx].isReal2-randUInt32_2[count32_1];
	// 				half_edgedata.isReal3=randUInt32_2[count32_1];
	// 				count32_1++;

	// 				half_edgedata.rating1=secret_nodes[idx].halfEdge[cdx].rating1;
	// 				t_half_edgedata.rating=half_edgedata.rating1;
	// 				half_edgedata.rating2=secret_nodes[idx].halfEdge[cdx].rating2-randUInt32_2[count32_1];
	// 				half_edgedata.rating3=randUInt32_2[count32_1];
	// 				count32_1++;
					
	// 				f_secret_nodes[idx-nUsers].halfEdge.push_back(half_edgedata);
	// 				t_nodedata[idx-nUsers].halfEdge.push_back(t_half_edgedata);
	// 			}

	// 		}
	// 		com->SendEvaluationHelper((unsigned char *)(t_nodedata),nItems*sizeof(Secret_Node),isEdgeIncoming);

	// 	}
	// 	if(party == Alice){
	// 		Secret_Node  * t_nodedata1 = new Secret_Node[nItems];
	// 		Secret_Node  * t_nodedata2 = new Secret_Node[nItems];
	// 		Secret_Node  * t_nodedata3 = new Secret_Node[nItems];
	// 		com->AwaitAlice((unsigned char *)(t_nodedata1), nItems*sizeof(Secret_Node));
	// 		com->AwaitBob((unsigned char *)(t_nodedata2), nItems*sizeof(Secret_Node));
	// 		com->AwaitCharlie((unsigned char *)(t_nodedata3), nItems*sizeof(Secret_Node));
	// 		for(uint32_t idx = 0; idx < nItems; idx++){
	// 			f_secret_nodes[idx].vertexID1=t_nodedata1[idx].vertexID;
	// 			f_secret_nodes[idx].vertexID2=t_nodedata2[idx].vertexID;
	// 			f_secret_nodes[idx].vertexID3=t_nodedata3[idx].vertexID;

	// 			uint32_t halfedge_len=t_nodedata1[idx].halfEdge.size();
	// 			for(uint32_t cdx=0;cdx<halfedge_len;cdx++){
	// 				FHalf_Secret_Edge half_data;
	// 				if(cdx==0){
	// 					for(uint32_t kdx=0;kdx<dimension:kdx++){
	// 						half_data.profile_v1[kdx]=t_nodedata1->halfEdge[cdx].profile_v[kdx];
	// 						half_data.profile_v2[kdx]=t_nodedata2->halfEdge[cdx].profile_v[kdx];
	// 						half_data.profile_v3[kdx]=t_nodedata3->halfEdge[cdx].profile_v[kdx];
	// 					}
	// 				}
	// 				for(uint32_t kdx=0;kdx<dimension:kdx++){
	// 						half_data.profile_u1[kdx]=t_nodedata1->halfEdge[cdx].profile_u[kdx];
	// 						half_data.profile_u2[kdx]=t_nodedata2->halfEdge[cdx].profile_u[kdx];
	// 						half_data.profile_u3[kdx]=t_nodedata3->halfEdge[cdx].profile_u[kdx];
	// 					}
	// 				half_data.isReal1=t_nodedata1->halfEdge[cdx].isReal;
	// 				half_data.isReal2=t_nodedata2->halfEdge[cdx].isReal;
	// 				half_data.isReal3=t_nodedata3->halfEdge[cdx].isReal;

	// 				half_data.rating1=t_nodedata1->halfEdge[cdx].rating;
	// 				half_data.rating2=t_nodedata2->halfEdge[cdx].rating;
	// 				half_data.rating3=t_nodedata3->halfEdge[cdx].rating;

	// 				f_secret_nodes[idx].halfEdge.push_back(half_data);
	// 			}

	// 		}
	// 	} //Alice
	// 	}  //isEdgeIscoming
	// 	else{
	// 		cout<< "share_transform_error!!"<<endl;

	// 	}
	// }
	
   //生成节点
	void GenerateNodes(Secret_Node * nodes, int nUsers, int nItems) 
	{
		// std::cout << "Generate nodes:" << std::endl;
		/* vertex populating */
		// User nodes
		int firstLightMachineID_User = totalUsers % totalMachines;
		for (int i = 0; i < nUsers; i++)
		{
			if (machineId < firstLightMachineID_User)
			{
				nodes[i].vertexID = machineId*nUsers + i + 1;
			}
			else
			{
				// Light machine has 1 less user: offset = machineId*nUsers + firstLightMachineID_User
				nodes[i].vertexID = (machineId*nUsers + firstLightMachineID_User) + i + 1;
			}
			
			// std::cout << "idx: " << i << "\t" << nodes[i].vertexID << std::endl;
		}

		// Item nodes
		int firstLightMachineID_Item = totalItems % totalMachines;
		for (int i = 0; i < nItems; i++)
		{
			if (machineId < firstLightMachineID_Item)
			{
				nodes[i + nUsers].vertexID = totalUsers + machineId*nItems + i + 1;
			}
			else
			{
				nodes[i + nUsers].vertexID = totalUsers + (machineId*nItems + firstLightMachineID_Item) + i + 1;
			}
			
			// std::cout << "idx: " << i << "\t" << nodes[i + nUsers].vertexID << std::endl;
		}
	}
	//生成边
	void GenerateEdges(Secret_Edge* edges, int nEdges, int nItems, int alpha) 
	{

		unsigned seed =  std::chrono::system_clock::now().time_since_epoch().count();
		default_random_engine  generator_int(seed);
		uniform_int_distribution<uint32_t>  randUserId(1, totalUsers);
		uniform_int_distribution<uint32_t>  randItemId(totalUsers+1, totalUsers+totalItems);
		uniform_int_distribution<uint32_t>  randRate(1, 5);

		/* edge populating */
		int q = 0;  // index of all edges, first dummies then reals
		for (int i = 0; i < nItems; i++) // fill the first 2alpha edges with dummy ones
		{	
			for (int j = 0; j < (2*alpha); j++)
			{
				edges[q].id_u = randUserId(generator_int);
				edges[q].id_v = randItemId(generator_int);//nodeID[i + nUsers];  // we could generate them as random as well, but to avoid redundancy we do this
				edges[q].rating = randRate(generator_int);
				edges[q].isReal = 0;   // false ~ dummy edges
				for(int idx = 0; idx < dimension; idx++)
				{
					edges[q].profile_u[idx] = (1<<15);
					edges[q].profile_v[idx] = ((idx % 2) - 1)*(1<<15);
				}
				q++;
			}
		}

		for (int k = q; k < nEdges; k++) // fill the rest of the edges with real ones
		{
			edges[k].id_u = randUserId(generator_int);
			edges[k].id_v = randItemId(generator_int);
			edges[k].rating = randRate(generator_int);
			edges[k].isReal = (1/* << 20*/);   // true ~ real edges
			for(int idx = 0; idx < dimension; idx++)
			{
				edges[k].profile_u[idx] = (1<<15);
				edges[k].profile_v[idx] = ((idx % 2) - 1)*(1<<15);
			}
		}
	}
  //对边重新随机化
	void rerandomize(int party, TSecret_Edge *secret_edges, int nEdges)
	{
		vector<vector<unsigned char>>  seed = machine->getComSharedRandomSeed(party, partner1,partner2, com);

		AESRNG *rng1 = new AESRNG(seed[0].data());
		AESRNG *rng2 = new AESRNG(seed[1].data());
		
		std::vector<uint32_t> randUInt16_1 = rng1->GetUInt32Array(2*nEdges);
		std::vector<uint32_t> randUInt32_1 = rng1->GetUInt32Array((2*dimension + 2)*nEdges);

		std::vector<uint32_t> randUInt16_2 = rng2->GetUInt32Array(2*nEdges);
		std::vector<uint32_t> randUInt32_2 = rng2->GetUInt32Array((2*dimension + 2)*nEdges);
		
		int counter16 = 0;
		int counter64 = 0;
		
		if (party % 3 == 0)
		{
			for (int edx = 0; edx < nEdges; edx++) 
			{
				secret_edges[edx].id_u1 += randUInt16_1[counter16] + randUInt16_2[counter16] ; 
				secret_edges[edx].id_u2 -=  randUInt16_1[counter16];
				counter16++;
				secret_edges[edx].id_v1 += randUInt16_1[counter16] + randUInt16_2[counter16] ; 
				secret_edges[edx].id_v2 -=  randUInt16_1[counter16];
				counter16++;
				
				for(int kdx = 0; kdx < dimension; kdx++)
				{
					secret_edges[edx].profile_u1[kdx] += randUInt32_1[counter64]+ randUInt32_2[counter64]; 
					secret_edges[edx].profile_u2[kdx] -= randUInt32_1[counter64];
					counter64++;
					secret_edges[edx].profile_v1[kdx] +=randUInt32_1[counter64]+ randUInt32_2[counter64]; 
					secret_edges[edx].profile_v2[kdx] -= randUInt32_1[counter64];
					counter64++;
				}
				secret_edges[edx].rating1 += randUInt32_1[counter64]+ randUInt32_2[counter64];
				secret_edges[edx].rating2 -=  randUInt32_1[counter64];
				counter64++;
				secret_edges[edx].isReal1 += randUInt32_1[counter64]+ randUInt32_2[counter64];
				secret_edges[edx].isReal2 -=  randUInt32_1[counter64];
				counter64++;
			}
		}
		else if(party%3==1)
		{
			for (int edx = 0; edx < nEdges; edx++) 
			{
				secret_edges[edx].id_u1 -= randUInt16_1[counter16]; 
				secret_edges[edx].id_u2 -= randUInt16_2[counter16]; 
				counter16++;
				secret_edges[edx].id_v1 -= randUInt16_1[counter16];
				secret_edges[edx].id_v2 -= randUInt16_2[counter16];
				 counter16++;
				
				for(int kdx = 0; kdx < dimension; kdx++)
				{
					secret_edges[edx].profile_u1[kdx] -= randUInt32_1[counter64]; 
					secret_edges[edx].profile_u2[kdx] -= randUInt32_2[counter64]; 
					counter64++;
					secret_edges[edx].profile_v1[kdx] -= randUInt32_1[counter64]; 
					secret_edges[edx].profile_v2[kdx] -= randUInt32_2[counter64]; 
					counter64++;
				}
				secret_edges[edx].rating1 -= randUInt32_1[counter64]; 
				secret_edges[edx].rating2 -= randUInt32_2[counter64]; 
				counter64++;
				secret_edges[edx].isReal1 -= randUInt32_1[counter64]; 
				secret_edges[edx].isReal2 -= randUInt32_2[counter64]; 
				counter64++;
			}
		}
		else{

			for (int edx = 0; edx < nEdges; edx++) 
			{
				secret_edges[edx].id_u1 -= randUInt16_2[counter16]; 
				secret_edges[edx].id_u2 +=randUInt16_1[counter16]+ randUInt16_2[counter16]; 
				counter16++;
				secret_edges[edx].id_v1 -= randUInt16_2[counter16];
				secret_edges[edx].id_v2 += randUInt16_1[counter16]+randUInt16_2[counter16];
				 counter16++;
				
				for(int kdx = 0; kdx < dimension; kdx++)
				{
					secret_edges[edx].profile_u1[kdx] -= randUInt32_2[counter64]; 
					secret_edges[edx].profile_u2[kdx] += randUInt32_1[counter64]+randUInt32_2[counter64]; 
					counter64++;
					secret_edges[edx].profile_v1[kdx] -= randUInt32_2[counter64]; 
					secret_edges[edx].profile_v2[kdx] += randUInt32_1[counter64]+randUInt32_2[counter64]; 
					counter64++;
				}
				secret_edges[edx].rating1 -= randUInt32_2[counter64]; 
				secret_edges[edx].rating2 +=randUInt32_1[counter64]+ randUInt32_2[counter64]; 
				counter64++;
				secret_edges[edx].isReal1 -= randUInt32_2[counter64]; 
				secret_edges[edx].isReal2 += randUInt32_1[counter64]+randUInt32_2[counter64]; 
				counter64++;
			}

		}
	}

	
	void SecretSharing(int party, int partner1,int partner2, TSecret_Edge* secret_edges, int nEdges, int nNodes,TSecret_Node* secret_nodes, int nUsers, int nItems, int alpha)
	{

		// cout << "secret_edges: " << secret_edges[0].profile_u[0] << endl;
		
		// 		std::vector<unsigned char> seed = machine->getSharedRandomSeed(party, partner);
		
		// 		AESRNG *rng = new AESRNG(seed.data());
		
		std::vector<uint32_t> randUInt16_1 = rng1->GetUInt32Array(nUsers + nItems + 2*nEdges);	
		std::vector<uint32_t> randUInt16_2 = rng2->GetUInt32Array(nUsers + nItems + 2*nEdges);
		std::vector<uint32_t> randUInt32_1 = rng1->GetUInt32Array(2*dimension*(nUsers + nItems) + (2*dimension + 2)*nEdges);
		std::vector<uint32_t> randUInt32_2 = rng2->GetUInt32Array(2*dimension*(nUsers + nItems) + (2*dimension + 2)*nEdges);
		// 		TestUtility::PrintVector(randUInt32, "rand 64");
		//由Alice把生成的数据发送给其他两个参与方。
		cout<<"*****************随机数据**************"<<endl;
		cout<<randUInt32_1[0]<<endl;
		cout<<randUInt32_2[0]<<endl;
		if (party == Alice)
		{
			Secret_Node * nodes_data= new Secret_Node[nNodes];
			Secret_Edge * edges_data = new Secret_Edge[nEdges];
			//nodes_3,edges_3中存储要发送给Bob和Charlie的元素
			Secret_Node * nodes_3 = new  Secret_Node[nNodes];
			Secret_Edge *  edges_3 = new Secret_Edge[nEdges];
			//    生成数据
			GenerateNodes(nodes_data, nUsers, nItems);
			GenerateEdges(edges_data, nEdges, nItems, alpha);

			int counter16 = 0;
			int counter64 = 0;
			
			for (int i = 0; i < (nUsers+nItems); i++)
			{
				secret_nodes[i].vertexID1 = randUInt16_2[counter16]; 
				secret_nodes[i].vertexID2 = randUInt16_1[counter16]; 
				nodes_3[i].vertexID = nodes_data[i].vertexID - secret_nodes[i].vertexID1 - secret_nodes[i].vertexID2 ;

				counter16++;
				
				for(int kdx = 0; kdx < dimension; kdx++)
				{
					secret_nodes[i].Profile1[kdx] = randUInt32_2[counter64];
					secret_nodes[i].Profile2[kdx] = randUInt32_1[counter64]; 
					nodes_3[i].Profile[kdx] = nodes_data[i].Profile[kdx] - secret_nodes[i].Profile1[kdx] -	secret_nodes[i].Profile2[kdx] ;
					counter64++;

					secret_nodes[i].newProfile1[kdx] = randUInt32_2[counter64]; 
					secret_nodes[i].newProfile2[kdx] = randUInt32_1[counter64]; 
					nodes_3[i].newProfile[kdx]=nodes_data[i].newProfile[kdx] - secret_nodes[i].newProfile1[kdx] - secret_nodes[i].newProfile2[kdx];
					counter64++;
				}
			}
			
			for (int i = 0; i < nEdges; i++) 
			{
				secret_edges[i].id_u1= randUInt16_2[counter16]; 
				secret_edges[i].id_u2= randUInt16_1[counter16]; 
				edges_3[i].id_u = edges_data[i].id_u - secret_edges[i].id_u1 - secret_edges[i].id_u2;

				counter16++;

				secret_edges[i].id_v1= randUInt16_2[counter16]; 
				secret_edges[i].id_v2= randUInt16_1[counter16]; 
				edges_3[i].id_v = edges_data[i].id_v - secret_edges[i].id_v1 - secret_edges[i].id_v2;


				counter16++;
				
				for(int kdx = 0; kdx < dimension; kdx++)
				{
					secret_edges[i].profile_u1[kdx] = randUInt32_2[counter64]; 
					secret_edges[i].profile_u2[kdx] = randUInt32_1[counter64]; 
					edges_3[i].profile_u[kdx] = edges_data[i].profile_u[kdx] - secret_edges[i].profile_u1[kdx]  -  secret_edges[i].profile_u2[kdx] ;

					counter64++;

					secret_edges[i].profile_v1[kdx] = randUInt32_2[counter64]; 
					secret_edges[i].profile_v2[kdx] = randUInt32_1[counter64]; 
					edges_3[i].profile_v[kdx] = edges_data[i].profile_v[kdx] - secret_edges[i].profile_v1[kdx] - secret_edges[i].profile_v2[kdx] ;
					counter64++;
				}
				secret_edges[i].rating1= randUInt32_2[counter64]; 
				secret_edges[i].rating2= randUInt32_1[counter64]; 
				edges_3[i].rating = edges_data[i].rating - secret_edges[i].rating1-secret_edges[i].rating2;
				
				counter64++;

				secret_edges[i].isReal1 = randUInt32_2[counter64]; 
				secret_edges[i].isReal2= randUInt32_1[counter64]; 
				edges_3[i].isReal = edges_data[i].isReal-secret_edges[i].isReal1-secret_edges[i].isReal2;
				counter64++;
			}
			//把edge_3,node_3发送Bob,
				// 	com->SendVerificationPartner((unsigned char *)MACedEdges, nEdges*sizeof(SecretEdgeMAC));
			
			cout << "send edges_3,nodes_3 to Bob"<< endl;
			com->SendEvaluationPartner1((unsigned char *)edges_3,nEdges*sizeof(Secret_Edge ));
			cout<<"kkkkkkk"<<endl;
			com->SendEvaluationPartner1((unsigned char *)nodes_3,nNodes*sizeof(Secret_Node  ));

			
			//把edge_3,node_3发送给Charlie
			cout << "send edges_3,nodes_3 to Charlie"<< endl;
			com->SendEvaluationPartner2((unsigned char *)edges_3,nEdges*sizeof(Secret_Edge ));
			com->SendEvaluationPartner2((unsigned char *)nodes_3,nNodes*sizeof(Secret_Node  ));

		}
		else if (party == Bob)
		{
			Secret_Node * nodes_3 = new  Secret_Node[nNodes];
			Secret_Edge *  edges_3 = new Secret_Edge[nEdges];
			cout << "accept edges_3,nodes_3 from Alice"<< endl;
			com->AwaitEvaluationPartner2((unsigned char *)edges_3,nEdges*sizeof(Secret_Edge ));
			com->AwaitEvaluationPartner2((unsigned char *)nodes_3,nNodes*sizeof(Secret_Node ));
			int counter16 = 0;
			int counter64 = 0;
			for (int i = 0; i < (nUsers+nItems); i++)
			{
				secret_nodes[i].vertexID1 = randUInt16_2[counter16]; 
				secret_nodes[i].vertexID2 = nodes_3[i].vertexID; 
				counter16++;

				for (int j = 0; j < dimension; j++)
				{
					secret_nodes[i].Profile1[j] =  randUInt32_2[counter64]; 
					secret_nodes[i].Profile2[j] = nodes_3[i].Profile[j];
					counter64++;

					secret_nodes[i].newProfile1[j] = randUInt32_2[counter64]; 
					secret_nodes[i].newProfile2[j] = nodes_3[i].newProfile[j];
					counter64++;
				}
			}

			for (int i = 0; i < nEdges; i++) 
			{
				secret_edges[i].id_u1 =  randUInt16_2[counter16]; 
				secret_edges[i].id_u2 = edges_3[i].id_u; 
				counter16++;
				secret_edges[i].id_v1 = randUInt16_2[counter16]; 
				secret_edges[i].id_v2 = edges_3[i].id_v; 
				counter16++;
				
				for (int j = 0; j < dimension; j++)
				{
					secret_edges[i].profile_u1[j] =  randUInt32_2[counter64];
					secret_edges[i].profile_u2[j] = edges_3[i].profile_u[j];
					 counter64++;
					secret_edges[i].profile_v1[j] = randUInt32_2[counter64]; 
					secret_edges[i].profile_v2[j] = edges_3[i].profile_v[j]; 
					counter64++;
				}
				
				secret_edges[i].rating1 = randUInt32_2[counter64]; 
				secret_edges[i].rating2 = edges_3[i].rating; 
				counter64++;

				secret_edges[i].isReal1 =  randUInt32_2[counter64]; 
				secret_edges[i].isReal2 = edges_3[i].isReal; 
				counter64++;
			}
		}
		//Charlie
		else if (party == Charlie)
		{
			Secret_Node * nodes_3 = new  Secret_Node[nNodes];
			Secret_Edge *  edges_3 = new Secret_Edge[nEdges];
			cout << "accept edges_3,nodes_3 from Alice"<< endl;
			com->AwaitEvaluationPartner1((unsigned char *)edges_3,nEdges*sizeof(Secret_Edge ));
			com->AwaitEvaluationPartner1((unsigned char *)nodes_3,nNodes*sizeof(Secret_Node ));

			int counter16 = 0;
			int counter64 = 0;
			for (int i = 0; i < (nUsers+nItems); i++)
			{
				secret_nodes[i].vertexID1 = nodes_3[i].vertexID;
				secret_nodes[i].vertexID2 =  randUInt16_1[counter16];
				 counter16++;

				for (int j = 0; j < dimension; j++)
				{
					secret_nodes[i].Profile1[j] =  nodes_3[i].Profile[j];
					secret_nodes[i].Profile2[j] = randUInt32_1[counter64]; 
					counter64++;

					secret_nodes[i].newProfile1[j] = nodes_3[i].newProfile[j];
					secret_nodes[i].newProfile2[j] = randUInt32_1[counter64]; 
					counter64++;
				}
			}
				for (int i = 0; i < nEdges; i++) 
			{
				secret_edges[i].id_u1 =  edges_3[i].id_u; 
				secret_edges[i].id_u2 = randUInt16_1[counter16]; 
				counter16++;
				secret_edges[i].id_v1 =edges_3[i].id_v;  
				secret_edges[i].id_v2 = randUInt16_1[counter16]; 
				counter16++;
				
				for (int j = 0; j < dimension; j++)
				{
					secret_edges[i].profile_u1[j] =  edges_3[i].profile_u[j];
					secret_edges[i].profile_u2[j] = randUInt32_1[counter64];
					 counter64++;
					secret_edges[i].profile_v1[j] = edges_3[i].profile_v[j]; 
					secret_edges[i].profile_v2[j] = randUInt32_1[counter64]; 
					counter64++;
				}
				
				secret_edges[i].rating1 = edges_3[i].rating; 
				secret_edges[i].rating2 = randUInt32_1[counter64]; 
				counter64++;

				secret_edges[i].isReal1 =  edges_3[i].isReal; 
				secret_edges[i].isReal2 = randUInt32_1[counter64]; 
				counter64++;
			}
		}
	}

	template <typename T>
	T * Shuffle(int party, int partner1, int partner2, const std::vector<int>& shuffledArray, T* data, int size)
	{
		/* Shuffling starts */
		T * shuffledData = new T[size];
		int srcMachine = 0, srcIndex = 0, dstMachine = 0, dstIndex = 0;
		//定义相关缓冲区
		std::vector<std::vector<int> > recvIndex(totalMachines);
		std::vector<std::vector<T> > sendBuffer(totalMachines);
		std::vector<std::vector<T> > recvBuffer(totalMachines);


		cout << machineId << " inside shuffle " << endl;


		for (int i = 0; i < totalEdges; i++) 
		{
			srcMachine = i / size;
			srcIndex   = i % size;
			dstMachine = (shuffledArray[i] / size);  // find where is the destination machine
			dstIndex   = (shuffledArray[i] % size);  // find where is the local index in destination machine


			//cout << machineId << " srcMachine " << srcMachine << " srcIndex " << srcIndex << " dstMachine " << dstMachine << " dstIndex " << dstIndex << " size " << size << endl;			
			
			if (machineId == srcMachine && machineId == dstMachine)
			{
				shuffledData[dstIndex] = data[srcIndex];
			}
			else {
				if (machineId == srcMachine)
				{
					sendBuffer[dstMachine].push_back(data[srcIndex]);
				}
				else if (machineId == dstMachine)
				{
					recvIndex[srcMachine].push_back(dstIndex);
				}
			}
		}

		/* Sending to Lower Machine & Receiving from Ups to avoid DOSing the machines */
		for (int m = 0; m < machineId; m++)
		{
			// cout << "Sending halfedge info to Upper Machines... " << counter[m] << endl;
			if(sendBuffer[m].size() > 0)
			{
				cout << machineId << " ----> " << m << endl;
				machine->sendToPeer(machineId, m, (unsigned char *)(sendBuffer[m].data()), sendBuffer[m].size()*sizeof(T));
			}
		}



		for (int n = (machineId+1); n < totalMachines; n++) 
		{
			// cout << "Receiving halfedge info from Lower Machines..." << endl;
			// if (machineId==0) cout << "recvIndex[n].size()..." << recvIndex[n].size() << endl;
			if (recvIndex[n].size() > 0)
			{	
				std::vector<T> recvData(recvIndex[n].size());
				cout << machineId << " <---- " << n << endl;
				machine->receiveFromPeer(machineId, n, (unsigned char *)(recvData.data()), recvIndex[n].size()*sizeof(T));
				
				

				for(int cdx = 0; cdx < recvIndex[n].size(); cdx++)
				{
					// if(recvIndex[n][cdx] >= size) std::cout << machineId << ": recvIndex[n] = " << recvIndex[n][cdx] << "***********************" << endl;
					shuffledData[recvIndex[n][cdx]] = recvData[cdx];
				}
			}
		}

		for (int q = 0; q < machineId; q++) 
		{
			// cout << "Receiving halfedge info from Lower Machines..." << endl;
			if (recvIndex[q].size() > 0)
			{	
				std::vector<T> recvData(recvIndex[q].size());
				cout << machineId << " <---- " << q << endl;
				machine->receiveFromPeer(machineId, q, (unsigned char *)(recvData.data()), recvIndex[q].size()*sizeof(T));
				
				for(int cdx = 0; cdx < recvIndex[q].size(); cdx++)
				{
					// if(recvIndex[q][cdx] >= size) std::cout << machineId << ": recvIndex[q] = " << recvIndex[q][cdx] << "***********************" << endl;
					shuffledData[recvIndex[q][cdx]] = recvData[cdx];
				}
			}
		}

		/* Sending to Upper Machine & Receiving from Downs to avoid DOSing the machines */
		for (int p = (machineId+1); p < totalMachines; p++)
		{
			// cout << "Sending halfedge info to Upper Machines... " << counter[p] << endl;
			// cout << "Sending halfedge info to Upper Machines... " << counter[m] << endl;
			if(sendBuffer[p].size() > 0)
			{
				cout << machineId << " ----> " << p << endl;
				machine->sendToPeer(machineId, p, (unsigned char *)(sendBuffer[p].data()), sendBuffer[p].size()*sizeof(T));
			}
		}

		return shuffledData;


		// for (int i = 0; i < totalEdges; i++) 
		// {
		// 	srcMachine = i / size;
		// 	srcIndex   = i % size;
		// 	dstMachine = (shuffledArray[i] / size);  // find where is the destination machine
		// 	dstIndex   = (shuffledArray[i] % size);  // find where is the local index in destination machine
			
		// 	if (machineId == srcMachine && machineId == dstMachine)
		// 	{
		// 		shuffledData[dstIndex] = data[srcIndex];
		// 	}
		// 	else {
		// 		if (machineId == srcMachine)
		// 		{
		// 			machine->sendToPeer(srcMachine, dstMachine, (unsigned char *)(&data[srcIndex]), sizeof(T));
		// 		}
		// 		else if (machineId == dstMachine)
		// 		{
		// 			int size = machine->receiveFromPeer(dstMachine, srcMachine, (unsigned char *)(&shuffledData[dstIndex]), sizeof(T));
		// 			assert(size == sizeof(T));
		// 		}
		// 	}
		// }
		
		// return shuffledData;
	}
	
	
	std::vector<uint32_t> Gather(int party, int partner1,int partner2, TSecret_Edge* edges, int nEdges, TSecret_Node* nodes, int nUsers, int nItems, bool isEdgeIncoming)
	{
		//第一轻量机器id。  37%3  ===2
		int firstLightMachineID_Item = nItems % totalMachines;
		int firstLightMachineID_User = nUsers % totalMachines;
		//   111/3=37
		int nHeavy_Item = (int)(ceil ( (double)totalItems / (double)totalMachines)); // no of types in heavy load machine;重载机器型号不限;  
		int nLight_Item = (int)(floor( (double)totalItems / (double)totalMachines)); // no of types in light load machine;
		
		int nHeavy_User = (int)(ceil ( (double)totalUsers / (double)totalMachines)); // no of types in heavy load machine;
		int nLight_User = (int)(floor( (double)totalUsers / (double)totalMachines)); // no of types in light load machine;

		/// Store the gather data
		THalf_Secret_Edge edgeData;
		
		uint32_t vertexId = -1; //, receiverMachine = -1, receiverLocalIndex = -1;
         //定义了一个计数器
		uint32_t counter[totalMachines] = {0};
		//定义边追踪者
		vector<vector<uint32_t>> edgeTracker (totalMachines);
		//定义目录追踪者
		vector<vector<uint32_t>> indexTracker (totalMachines);
        //定义公开顶点的id
		vector<uint32_t> OpenedVertexIds(nEdges);
		//MACId.resize(nEdges);
		//定义接受人本地目录，接受机器
		vector<uint32_t> receiverLocalIndex(nEdges), receiverMachine(nEdges);
		//初始化为-1
		for(int idx = 0; idx < nEdges; idx++)
		{
			receiverLocalIndex[idx] = -1;
			receiverMachine[idx] = -1;
		}
         //判断是否是入边
		if(isEdgeIncoming)
		{
			std::vector<uint32_t> edgesIdv(nEdges), theirIdv(nEdges);
			
			for(int idx = 0; idx < nEdges; idx++)
			{
				edgesIdv[idx] = edges[idx].id_v1;
				//MACId[idx] = bMACs[idx].id_v;
			}

			com->SendEvaluationPartner1((unsigned char *)(edgesIdv.data()), nEdges*sizeof(uint32_t));
			com->AwaitEvaluationPartner2((unsigned char *)(theirIdv.data()), nEdges*sizeof(uint32_t));
			cout<<"************第一次发送数据无误***************"<<endl;
			
			

			// if (machineId == 16) 
				// cout << machineId << ": A" << endl;

			// Reconstructed item Vertex Ids
			for(int idx = 0; idx < nEdges; idx++)
			{
				//重构id，重构了vid
				OpenedVertexIds[idx] =  edges[idx].id_v1+ edges[idx].id_v2 + theirIdv[idx];
				// cout<<edges[idx].id_v1<<endl;
				// cout<<edges[idx].id_v2 <<endl;
				// cout<<theirIdv[idx]<<endl;
                
				//vld    等于公开顶点ID减去总共的用户数，这里是因为项目的id应该是生成顶点的时候直接和用户的一起生成了把
				uint32_t vId = OpenedVertexIds[idx] - totalUsers;
				// if (machineId == 16) 
					// cout << machineId << ": vId =" << vId<< " , OpenedVertexIds = " << OpenedVertexIds[idx] << endl;

				//vld大于第一轻机器id*重机数量，也就是后面那批数据
				if (vId > firstLightMachineID_Item*nHeavy_Item)
				{
					//减去其那面那匹数据的最大值，获得新的id
					vId = vId - (firstLightMachineID_Item*nHeavy_Item);
					// i 代表遍历每个机器
					for (int i = 0; i < (totalMachines-firstLightMachineID_Item); i++) {
						if (vId <= nLight_Item) 
						{
							//接受机器id
							receiverMachine[idx] = i+firstLightMachineID_Item;
							if (receiverMachine[idx] < firstLightMachineID_User)
							//接受者本地id,为何加上nHeavy_User。
								receiverLocalIndex[idx] = nHeavy_User + vId - 1;
							else
								receiverLocalIndex[idx] = nLight_User + vId - 1;
							break;
						}
						else vId -= nLight_Item;
					}
					// if (machineId == 16) 
				 // 		cout << machineId << ": if" << endl;
				}
				else 
				{ 
					for (int j = 0; j < firstLightMachineID_Item; j++) {
						if (vId <= nHeavy_Item) {
							receiverMachine[idx] = j;
							if (receiverMachine[idx] < firstLightMachineID_User)
								receiverLocalIndex[idx] = nHeavy_User + vId - 1;
							else
								receiverLocalIndex[idx] = nLight_User + vId - 1;
							break;
						}
						else vId -= nHeavy_Item;
					}
					// if (machineId == 16) 
					// 	cout << machineId << ": else" << endl;
				}

				// if (machineId == 16) 
				// 	cout << machineId << ": idx= " << idx << endl;
			}

		}
		//如果是出边，暂时不改后面在说
		// else if(!isEdgeIncoming)
		// {
		// 	// std::vector<uint32_t> edgesIdu(nEdges), theirIdu(nEdges);
			
		// 	// for(int idx = 0; idx < nEdges; idx++)
		// 	// {
		// 	// 	edgesIdu[idx] = edges[idx].id_u;
		// 	// 	//MACId[idx] = bMACs[idx].id_u;
		// 	// }
			
		// 	// if(party % 2 == 0)
		// 	// {
		// 	// 	com->SendEvaluationPartner((unsigned char *)(edgesIdu.data()), nEdges*sizeof(uint32_t));
		// 	// 	com->AwaitEvaluationPartner((unsigned char *)(theirIdu.data()), nEdges*sizeof(uint32_t));
		// 	// }
		// 	// else
		// 	// {
		// 	// 	com->AwaitEvaluationPartner((unsigned char *)(theirIdu.data()), nEdges*sizeof(uint32_t));
		// 	// 	com->SendEvaluationPartner((unsigned char *)(edgesIdu.data()), nEdges*sizeof(uint32_t));
		// 	// }
			
		// 	// // Reconstructed user Vertex Ids
		// 	// for(int idx = 0; idx < nEdges; idx++)
		// 	// {
		// 	// 	OpenedVertexIds[idx] = edgesIdu[idx] + theirIdu[idx];

		// 	// 	uint32_t uId = OpenedVertexIds[idx];
				
		// 	// 	if (uId > firstLightMachineID_User*nHeavy_User)
		// 	// 	{
		// 	// 		uId = uId - (firstLightMachineID_User*nHeavy_User);
		// 	// 		receiverMachine[idx] = firstLightMachineID_User + (uId-1)/nLight_User;
		// 	// 		receiverLocalIndex[idx] = (uId-1) % nLight_User;

		// 	// 		// for (int i = 0; i < (totalMachines-firstLightMachineID_User); i++) {
		// 	// 		// 	if (uId <= nLight_User) {
		// 	// 		// 		receiverMachine[idx] = i+firstLightMachineID_User;
		// 	// 		// 		receiverLocalIndex[idx] = uId - 1;
		// 	// 		// 		break;
		// 	// 		// 	}
		// 	// 		// 	else uId -= nLight_User;
		// 	// 		// }
		// 	// 	}
		// 	// 	else 
		// 	// 	{ 
		// 	// 		receiverMachine[idx] =  (uId-1)/nHeavy_User;
		// 	// 		receiverLocalIndex[idx] = (uId-1) % nHeavy_User;

		// 	// 		// for (int j = 0; j < firstLightMachineID_User; j++) {
		// 	// 		// 	if (uId <= nHeavy_User) {
		// 	// 		// 		receiverMachine[idx] = j;
		// 	// 		// 		receiverLocalIndex[idx] = uId - 1;
		// 	// 		// 		break;
		// 	// 		// 	}
		// 	// 		// 	else uId -= nHeavy_Item;
		// 	// 		// }
		// 	// 	}
		// 	// }
			 
		// }

		
		
        cout<<"********************继续对每条边遍历********************"<<endl;
		for(int idx = 0; idx < nEdges; idx++)
		{
			// //如果当前机器id等于接受id
			// cout<<idx<<endl;
			// cout<<receiverMachine[idx]<<endl;
			if (machineId == receiverMachine[idx])
			{
				for (int i = 0; i < dimension; i++)
				{
					edgeData.profile_u1[i] = edges[idx].profile_u1[i];
					edgeData.profile_u2[i] = edges[idx].profile_u2[i];
					edgeData.profile_v1[i] = edges[idx].profile_v1[i];
					edgeData.profile_v2[i] = edges[idx].profile_v2[i];
				}
				if (receiverLocalIndex[idx]== -1) cout << machineId << ": receiverLocalIndex[idx]= " << receiverLocalIndex[idx] << endl;
				edgeData.rating1 = edges[idx].rating1;
				edgeData.rating2 = edges[idx].rating2;
				edgeData.isReal1 = edges[idx].isReal1;
				edgeData.isReal2 = edges[idx].isReal2;
				//edgeData.bMACs  = bMACs[idx];
				nodes[receiverLocalIndex[idx]].halfEdge.push_back(edgeData);
			}
			else
			{	
				if (receiverMachine[idx]== -1) 
				cout << machineId << ": receiverMachine[idx]= " << receiverMachine[idx] << endl;
				if (receiverLocalIndex[idx]== -1) 
				cout << machineId << ": receiverLocalIndex[idx]= " << receiverLocalIndex[idx] << endl;
				++counter[receiverMachine[idx]];
				edgeTracker[receiverMachine[idx]].push_back(idx);
				indexTracker[receiverMachine[idx]].push_back(receiverLocalIndex[idx]);
			}
		}
		cout<<"*****************为接受机器发送缓存***********"<<endl;

		// Allocate buffers for each receiver machine
		std::vector<THalf_Secret_Edge *> buffers(totalMachines);
		for(int idx = 0; idx < totalMachines; idx++)
		{
			if(machineId != idx)
			{
				buffers[idx] = new THalf_Secret_Edge[counter[idx]];
			
				for(int kdx = 0; kdx < counter[idx]; kdx++)
				{
					int index = edgeTracker[idx][kdx];
					
					for(int i = 0; i < dimension; i++)
					{
						edgeData.profile_u1[i] = edges[index].profile_u1[i];
						edgeData.profile_u2[i] = edges[index].profile_u2[i];
						edgeData.profile_v1[i] = edges[index].profile_v1[i];
						edgeData.profile_v2[i] = edges[index].profile_v2[i];
					}
					edgeData.rating1 = edges[index].rating1;
					edgeData.rating2 = edges[index].rating2;
					edgeData.isReal1 = edges[index].isReal1;
					edgeData.isReal2 = edges[index].isReal2;
					//edgeData.bMACs  = bMACs[index];
					
					buffers[idx][kdx] = edgeData;
				}
			}
		}


		cout<<"************发送同机器集群无误***************"<<endl;
		/* Sending to Upper Machine to avoid DOSing the machines */
		for (uint32_t m = 0; m < machineId; m++)
		{
			cout << machineId << " -----> " << m << endl;
			machine->sendToPeer(machineId, m, (unsigned char *)(&counter[m]), sizeof(uint32_t));
			if(counter[m] > 0)
			{
				machine->sendToPeer(machineId, m, (unsigned char *)(indexTracker[m].data()), indexTracker[m].size()*sizeof(uint32_t));
				machine->sendToPeer(machineId, m, (unsigned char *)(buffers[m]), counter[m]*sizeof(THalf_Secret_Edge));
			}
		}

		/* Receiving from Lowers  */
		for (uint32_t n = (machineId+1); n < totalMachines; n++) 
		{
			cout << machineId << " <----- " << n << endl;
			uint32_t count;// = -1;
			machine->receiveFromPeer(machineId, n, (unsigned char *)(&count), sizeof(uint32_t));
			if (count > 0)
			{	
				std::vector<uint32_t> localIndex(count);
				THalf_Secret_Edge *hse = new THalf_Secret_Edge[count];
				
				machine->receiveFromPeer(machineId, n, (unsigned char *)(localIndex.data()), count*sizeof(uint32_t));
				machine->receiveFromPeer(machineId, n, (unsigned char *)(hse), count*sizeof(THalf_Secret_Edge));
				
				for(uint32_t cdx = 0; cdx < count; cdx++)
				{
					nodes[localIndex[cdx]].halfEdge.push_back(hse[cdx]);
				}
				delete[] hse;
			}
		}

		/* Receiving from Upper */
		for (uint32_t q = 0; q < machineId; q++) 
		{
			cout << machineId << " <----- " << q << endl;
			uint32_t count; // = -1;
			machine->receiveFromPeer(machineId, q, (unsigned char *)(&count), sizeof(uint32_t));
			if (count > 0)
			{	
				std::vector<uint32_t> localIndex(count);
				THalf_Secret_Edge *hse = new THalf_Secret_Edge[count];
				
				machine->receiveFromPeer(machineId, q, (unsigned char *)(localIndex.data()), count*sizeof(uint32_t));
				machine->receiveFromPeer(machineId, q, (unsigned char *)(hse), count*sizeof(THalf_Secret_Edge));
				
				for(int cdx = 0; cdx < count; cdx++)
				{
					nodes[localIndex[cdx]].halfEdge.push_back(hse[cdx]);
				}
				
				delete[] hse;
			}
		}

		/* Sending to Lower Machine to avoid DOSing the machines */
		for (uint32_t p = (machineId+1); p < totalMachines; p++)
		{
			cout << machineId << " -----> " << p << endl;
			machine->sendToPeer(machineId, p, (unsigned char *)(&counter[p]), sizeof(uint32_t));
			if(counter[p] > 0)
			{
				machine->sendToPeer(machineId, p, (unsigned char *)(indexTracker[p].data()), indexTracker[p].size()*sizeof(uint32_t));
				machine->sendToPeer(machineId, p, (unsigned char *)(buffers[p]), counter[p]*sizeof(THalf_Secret_Edge));
			}
		}



		return OpenedVertexIds;
		
		// **************************************************************************************************//
	}
	void getFourSharedInput(int party,TSecret_Node *nodes,int nNodes, int nUsers, int inputSize, bool isEdgeIncoming,std::vector<std::vector<uint32_t>> &input1,std::vector<std::vector<uint32_t>> &input2,std::vector<std::vector<uint32_t>> &input3){
		if(isEdgeIncoming){
			if(party == David||party ==  Eric||party == Frank){
				cout<<"**************获取seed*************"<<endl;
				vector<unsigned char>  seed = machine->getSingleComSharedRandomSeed(party, partner1,partner2, com);
				cout<<"1111111111111111"<<endl;
		        AESRNG *rng = new AESRNG(seed.data());
		        //填充u.v	
				cout<<"22222222222222222222222"<<endl;
		        //std::vector<uint32_t> randUInt32_1 = rng->GetUInt32Array((2*dimension + 2)*nNodes);
				uint32_t count = 0;
				cout<<"*********** 遍历item**********"<<endl;
				cout<<"***************nNodes**************"<<endl;
				cout<<nNodes<<endl;
			for(uint32_t idx = nUsers; idx < nNodes; idx++){
				// Get v (v is the same for all edges)
				// cout<<Half_Secret_Edge
				uint32_t halfedge_len=nodes[idx].halfEdge.size();
				cout<<"***************半边长度*******************8"<<endl;

				cout<<halfedge_len<<endl;
				std::vector<uint32_t> randUInt32_1 = rng->GetUInt32Array(halfedge_len*dimension+dimension+halfedge_len*2);
				int counter64=0;
				cout<<"***************idx*******************8"<<endl;
				std::cout<<idx<<endl;
				input1[count].resize(dimension);
				input2[count].resize(dimension);
				input3[count].resize(dimension);
				if(party == David){
					for(uint32_t kdx = 0; kdx < dimension; kdx++)
					{
						input1[count][kdx] = (uint32_t)(nodes[idx].halfEdge[0].profile_v1[kdx])-randUInt32_1[counter64];
						input2[count][kdx] = (uint32_t)(nodes[idx].halfEdge[0].profile_v2[kdx]);
						input3[count][kdx] = randUInt32_1[counter64];
						counter64++;
					}
					count++;
					
					// Get u, rating, isReal on each edge
					for(uint32_t cdx = 0; cdx < nodes[idx].halfEdge.size(); cdx++)
					{
						input1[count].resize(dimension);
						input2[count].resize(dimension);
						input3[count].resize(dimension);
						for(int kdx = 0; kdx < dimension; kdx++)
						{
							input1[count][kdx] = nodes[idx].halfEdge[cdx].profile_u1[kdx]-randUInt32_1[counter64];
							input2[count][kdx] = nodes[idx].halfEdge[cdx].profile_u2[kdx];
							input3[count][kdx] = randUInt32_1[counter64];
							counter64++;
						}	
						count++;
						
						input1[count].resize(1);
						input2[count].resize(1);
						input3[count].resize(1);
						input1[count][0] = nodes[idx].halfEdge[cdx].rating1-randUInt32_1[counter64];
						input2[count][0] = nodes[idx].halfEdge[cdx].rating2;
						input3[count][0] = randUInt32_1[counter64];
						counter64++;
						count++;
						
						input1[count].resize(1);
						input2[count].resize(1);
						input3[count].resize(1);
						input1[count][0] = nodes[idx].halfEdge[cdx].isReal1-randUInt32_1[counter64];
						input2[count][0] = nodes[idx].halfEdge[cdx].isReal2;
						input3[count][0] = randUInt32_1[counter64];
						counter64++;
						count++;
					}

				}
				if(party == Eric){
					for(uint32_t kdx = 0; kdx < dimension; kdx++)
					{
						input1[count][kdx] = (uint32_t)(nodes[idx].halfEdge[0].profile_v1[kdx]);
						input2[count][kdx] = (uint32_t)(nodes[idx].halfEdge[0].profile_v2[kdx]);
						input3[count][kdx] = randUInt32_1[counter64];
						counter64++;
					}
					count++;
					
					// Get u, rating, isReal on each edge
					for(uint32_t cdx = 0; cdx < nodes[idx].halfEdge.size(); cdx++)
					{
						
						input1[count].resize(dimension);
						input2[count].resize(dimension);
						input3[count].resize(dimension);
						for(int kdx = 0; kdx < dimension; kdx++)
						{
							input1[count][kdx] = nodes[idx].halfEdge[cdx].profile_u1[kdx];
							input2[count][kdx] = nodes[idx].halfEdge[cdx].profile_u2[kdx];
							input3[count][kdx] = randUInt32_1[counter64];
							counter64++;
						}	
						count++;
						
						input1[count].resize(1);
						input2[count].resize(1);
						input3[count].resize(1);
						input1[count][0] = nodes[idx].halfEdge[cdx].rating1;
						input2[count][0] = nodes[idx].halfEdge[cdx].rating2;
						input3[count][0] = randUInt32_1[counter64];
						counter64++;
						count++;
						
						input1[count].resize(1);
						input2[count].resize(1);
						input3[count].resize(1);
						input1[count][0] = nodes[idx].halfEdge[cdx].isReal1;
						input2[count][0] = nodes[idx].halfEdge[cdx].isReal2;
						input3[count][0] = randUInt32_1[counter64];
						counter64++;
						count++;
					}
				}
				if(party==Frank){
					for(uint32_t kdx = 0; kdx < dimension; kdx++)
					{
						input1[count][kdx] = (uint32_t)(nodes[idx].halfEdge[0].profile_v1[kdx]);
						input2[count][kdx] = (uint32_t)(nodes[idx].halfEdge[0].profile_v2[kdx])-randUInt32_1[counter64];
						input3[count][kdx] = randUInt32_1[counter64];
						counter64++;
					}
					count++;
					
					// Get u, rating, isReal on each edge
					for(uint32_t cdx = 0; cdx < nodes[idx].halfEdge.size(); cdx++)
					{
						input1[count].resize(dimension);
						input2[count].resize(dimension);
						input3[count].resize(dimension);
						for(int kdx = 0; kdx < dimension; kdx++)
						{
							input1[count][kdx] = nodes[idx].halfEdge[cdx].profile_u1[kdx];
							input2[count][kdx] = nodes[idx].halfEdge[cdx].profile_u2[kdx]-randUInt32_1[counter64];
							input3[count][kdx] = randUInt32_1[counter64];
							counter64++;
						}	
						count++;
						
						input1[count].resize(1);
						input2[count].resize(1);
						input3[count].resize(1);
						input1[count][0] = nodes[idx].halfEdge[cdx].rating1;
						input2[count][0] = nodes[idx].halfEdge[cdx].rating2-randUInt32_1[counter64];
						input3[count][0] = randUInt32_1[counter64];
						counter64++;
						count++;
						
						input1[count].resize(1);
						input2[count].resize(1);
						input3[count].resize(1);
						input1[count][0] = nodes[idx].halfEdge[cdx].isReal1;
						input2[count][0] = nodes[idx].halfEdge[cdx].isReal2-randUInt32_1[counter64];
						input3[count][0] = randUInt32_1[counter64];
						counter64++;
						count++;
					}

				}
				
			}
			//把input1的数据转换成一维的。
			std::vector<unsigned char> temp = ArrayEncoder::EncodeUInt32Array(input1);
			if(party == David){
				int size = temp.size();
				cout<<"****************发送size***************"<<endl;
				com->SendEvaluationAlice((unsigned char *)(&size),sizeof(int));
			}
			//把各自的input1发送Alice。
			cout<<"****************发送input***************"<<endl;
			cout<<temp.size()<<endl;
			com->SendEvaluationAlice(temp.data(),temp.size());
			}
			if(party == Alice){
				int size=0;
				//先接受数据的大小
				cout<<"****************接受size***************"<<endl;
		        com->AwaitEvaluationAlice((unsigned char *)(&size), sizeof(int));
				cout<<size<<endl;
			   //设置接受数据大小的
			    std::vector<unsigned char> recv1(size);
				std::vector<unsigned char> recv2(size);
				std::vector<unsigned char> recv3(size);
                //接受一维数据

				cout<<"****************接受input***************"<<endl;
				cout<<"****************接受Alice_input***************"<<endl;
				com->AwaitEvaluationAlice(recv1.data(),recv1.size());
				cout<<"****************接受Bob_input***************"<<endl;
				com->AwaitEvaluationBob(recv2.data(),recv2.size());
				cout<<"****************接受Charlie_input***************"<<endl;
				com->AwaitEvaluationCharlie(recv3.data(),recv3.size());
				//把数据转换成二维的
				cout<<"***************数据转换为二维***************"<<endl;
				input1=ArrayEncoder::DecodeUInt32Array(recv1);
				input2=ArrayEncoder::DecodeUInt32Array(recv2);
				input3=ArrayEncoder::DecodeUInt32Array(recv3);
			}
			
		}
		else{
			cout<<"getFourSharedInput_error"<<endl;

		}

		cout<<"************四方都获得输入******"<<endl;
	}

	std::vector<std::vector<uint32_t> > getSharedMaskedInput(Secret_Node *nodes, int nNodes, int nUsers, int inputSize, bool isEdgeIncoming)
	{
		std::vector<std::vector<uint32_t> > input(inputSize);
		
		if(isEdgeIncoming)
		{
			uint32_t count = 0;
			
			for(uint32_t idx = nUsers; idx < nNodes; idx++)
			{
				// Get v (v is the same for all edges)
				std::cout<<idx<<endl;
				input[count].resize(dimension);
				for(uint32_t kdx = 0; kdx < dimension; kdx++)
				{
					input[count][kdx] = (uint32_t)(nodes[idx].halfEdge[0].profile_v[kdx]);
				}
				count++;
				
				// Get u, rating, isReal on each edge
				for(uint32_t cdx = 0; cdx < nodes[idx].halfEdge.size(); cdx++)
				{
					input[count].resize(dimension);
					for(int kdx = 0; kdx < dimension; kdx++)
					{
						input[count][kdx] = nodes[idx].halfEdge[cdx].profile_u[kdx];
					}	
					count++;
					
					input[count].resize(1);
					input[count][0] = nodes[idx].halfEdge[cdx].rating;
					count++;
					
					input[count].resize(1);
					input[count][0] = nodes[idx].halfEdge[cdx].isReal;
					count++;
				}
			}
		}
		else
		{
			int count = 0;
			
			for(int idx = 0; idx < nUsers; idx++)
			{
				// Get u (u is the same for all edges)
				input[count].resize(dimension);
				for(int kdx = 0; kdx < dimension; kdx++)
				{
					input[count][kdx] = nodes[idx].halfEdge[0].profile_u[kdx];
				}
				count++;
				
				// Get v, rating, isReal on each edge
				for(int cdx = 0; cdx < nodes[idx].halfEdge.size(); cdx++)
				{
					input[count].resize(dimension);
					for(int kdx = 0; kdx < dimension; kdx++)
					{
						input[count][kdx] = nodes[idx].halfEdge[cdx].profile_v[kdx];
					}	
					count++;
					
					input[count].resize(1);
					input[count][0] = nodes[idx].halfEdge[cdx].rating;
					count++;
					
					input[count].resize(1);
					input[count][0] = nodes[idx].halfEdge[cdx].isReal;
					count++;
				}
			}
		}
		
		return input;
	}

	// std::vector<std::vector<uint32_t> > getBetaMAC(Secret_Node *nodes, int nNodes, int nUsers, int inputSize, bool isEdgeIncoming)
	// {
	// 	std::vector<std::vector<uint32_t> > input(inputSize);
		
	// 	if(isEdgeIncoming)
	// 	{
	// 		int count = 0;
			
	// 		for(int idx = nUsers; idx < nNodes; idx++)
	// 		{
	// 			// Get bMACs for profile_v (v is the same for all edges)
	// 			input[count].resize(dimension);
	// 			for(int kdx = 0; kdx < dimension; kdx++)
	// 			{
	// 				input[count][kdx] = (nodes[idx].halfEdge[0].bMACs.profile_v[kdx]);
	// 			}
	// 			count++;
				
	// 			// Get bMACs for u, rating, isReal on each edge
	// 			for(int cdx = 0; cdx < nodes[idx].halfEdge.size(); cdx++)
	// 			{
	// 				input[count].resize(dimension);
	// 				for(int kdx = 0; kdx < dimension; kdx++)
	// 				{
	// 					input[count][kdx] = nodes[idx].halfEdge[cdx].bMACs.profile_u[kdx];
	// 				}	
	// 				count++;
					
	// 				input[count].resize(1);
	// 				input[count][0] = nodes[idx].halfEdge[cdx].bMACs.rating;
	// 				count++;
					
	// 				input[count].resize(1);
	// 				input[count][0] = nodes[idx].halfEdge[cdx].bMACs.isReal;
	// 				count++;
	// 			}
	// 		}
	// 	}
	// 	else
	// 	{
	// 		int count = 0;
			
	// 		for(int idx = 0; idx < nUsers; idx++)
	// 		{
	// 			// Get bMACs for u (u is the same for all edges)
	// 			input[count].resize(dimension);
	// 			for(int kdx = 0; kdx < dimension; kdx++)
	// 			{
	// 				input[count][kdx] = nodes[idx].halfEdge[0].bMACs.profile_u[kdx];
	// 			}
	// 			count++;
				
	// 			// Get v, rating, isReal on each edge
	// 			for(int cdx = 0; cdx < nodes[idx].halfEdge.size(); cdx++)
	// 			{
	// 				input[count].resize(dimension);
	// 				for(int kdx = 0; kdx < dimension; kdx++)
	// 				{
	// 					input[count][kdx] = nodes[idx].halfEdge[cdx].bMACs.profile_v[kdx];
	// 				}	
	// 				count++;
					
	// 				input[count].resize(1);
	// 				input[count][0] = nodes[idx].halfEdge[cdx].bMACs.rating;
	// 				count++;
					
	// 				input[count].resize(1);
	// 				input[count][0] = nodes[idx].halfEdge[cdx].bMACs.isReal;
	// 				count++;
	// 			}
	// 		}
	// 	}
		
	// 	return input;
	// }
	
	TSecret_Edge* Scatter(int party, int partner1,int partner2, vector<uint32_t> OpenedVertexIds, TSecret_Edge* edges, int nEdges, TSecret_Node* nodes, int nUsers, int nItems, bool isEdgeIncoming)
	{
		cout<<"start1"<<endl;
		int firstLightMachineID_Item = nItems % totalMachines;
		int firstLightMachineID_User = nUsers % totalMachines;
		
		int nHeavy_Item = (int)(ceil ( (double)totalItems / (double)totalMachines)); // no of types in heavy load machine;
		int nLight_Item = (int)(floor( (double)totalItems / (double)totalMachines)); // no of types in light load machine;
		
		int nHeavy_User = (int)(ceil ( (double)totalUsers / (double)totalMachines)); // no of types in heavy load machine;
		int nLight_User = (int)(floor( (double)totalUsers / (double)totalMachines)); // no of types in light load machine;

		vector<uint32_t> receiverLocalIndex(nEdges), receiverMachine(nEdges);
		cout<<"start2"<<endl;
		for(int idx = 0; idx < nEdges; idx++)
		{
			receiverLocalIndex[idx] = -1;
			receiverMachine[idx] = -1;
		}
		uint32_t index = -1;
		std::vector<TUpdateNode *> updateVecs(totalMachines);
		std::vector <std::vector <uint32_t>> allProfiles1;
		std::vector <std::vector <uint32_t>> allProfiles2;
         cout<<"start3"<<endl;
		if(isEdgeIncoming)
		{
			for(int idx = 0; idx < totalMachines; idx++)
			{
				updateVecs[idx] = new TUpdateNode[nItems];
			}

			// Write updated profiles for nodes on our machine
			for(int idx = 0; idx < nItems; idx++)
			{
				for(int d = 0; d < dimension; d++)
				{
					updateVecs[machineId][idx].newProfile1[d] = nodes[nUsers + 1 + idx].newProfile1[d];
					updateVecs[machineId][idx].newProfile2[d] = nodes[nUsers + 1 + idx].newProfile2[d];
				}
			}

			// Communicate to send our update profiles to others and to receiver from others
			/* Sending to Upper Machine & Receiving from Downs to avoid DOSing the machines */
			cout<<"start4"<<endl;
			for (int m = 0; m < machineId; m++)
			{
				cout << machineId << " ---> " << m << endl;
				// cout << machineId << " send " << sizeof(uint32_t) << "bytes" << endl;

				// machine->sendToPeer(machineId, m, (unsigned char *)(&tempId), sizeof(uint32_t));
				// for (int d = 0; d < dimension; d++)
				// {
					// cout << machineId << " send " << sizeof(uint32_t) << "bytes" << endl;
					machine->sendToPeer(machineId, m, (unsigned char *)(updateVecs[machineId]), nItems*sizeof(TUpdateNode));
				// }
			}

			// cout << machineId << ": test1" << endl;

			for (int n = (machineId+1); n < totalMachines; n++) 
			{
				cout << machineId << " <--- " << n << endl;
				// machine->receiveFromPeer(machineId, n, (unsigned char *)(&index), sizeof(uint32_t));
				// for (int d = 0; d < dimension; d++)
					machine->receiveFromPeer(machineId, n, (unsigned char *)(updateVecs[n]), nItems*sizeof(TUpdateNode));
			}

			// cout << machineId << ": test2" << endl;

			for (int p = 0; p < machineId; p++) 
			{
				// cout << "Receiving vertexID from Upper Machines..." << endl;
				cout << machineId << " <--- " << p << endl;
				// machine->receiveFromPeer(machineId, p, (unsigned char *)(&index), sizeof(uint32_t));
				// for (int d = 0; d < dimension; d++)
					machine->receiveFromPeer(machineId, p, (unsigned char *)(updateVecs[p]), nItems*sizeof(TUpdateNode));
			}

			/* Sending to Lower Machine & Receiving from Uppers to avoid DOSing the machines */
			for (int q = (machineId+1); q < totalMachines; q++)
			{
				cout << machineId << " ---> " << q << endl;
				// machine->sendToPeer(machineId, p, (unsigned char *)(&tempId), sizeof(uint32_t));
				// for (int d = 0; d < dimension; d++)
					machine->sendToPeer(machineId, q, (unsigned char *)(updateVecs[machineId]), nItems*sizeof(TUpdateNode));
			}
		}


			// Reconstructed Vertex Ids
			for(int idx = 0; idx < nEdges; idx++)
			{
				// OpenedVertexIds[idx] = edgesIdv[idx] + theirIdv[idx];

				uint32_t vId = OpenedVertexIds[idx] - totalUsers;
				// if (machineId == 16) 
					// cout << machineId << ": vId =" << vId<< " , OpenedVertexIds = " << OpenedVertexIds[idx] << endl;
				
				if (vId > firstLightMachineID_Item*nHeavy_Item)
				{
					vId = vId - (firstLightMachineID_Item*nHeavy_Item);
					
					for (int i = 0; i < (totalMachines-firstLightMachineID_Item); i++) {
						if (vId <= nLight_Item) 
						{
							receiverMachine[idx] = i+firstLightMachineID_Item;
							if (receiverMachine[idx] < firstLightMachineID_User)
								receiverLocalIndex[idx] = nHeavy_User + vId - 1;
							else
								receiverLocalIndex[idx] = nLight_User + vId - 1;
							break;
						}
						else vId -= nLight_Item;
					}
					// if (machineId == 16) 
				 // 		cout << machineId << ": if" << endl;
				}
				else 
				{ 
				
					for (int j = 0; j < firstLightMachineID_Item; j++) {
						if (vId <= nHeavy_Item) {
							receiverMachine[idx] = j;
							if (receiverMachine[idx] < firstLightMachineID_User)
								receiverLocalIndex[idx] = nHeavy_User + vId - 1;
							else
								receiverLocalIndex[idx] = nLight_User + vId - 1;
							break;
						}
						else vId -= nHeavy_Item;
					}
					// if (machineId == 16) 
					// 	cout << machineId << ": else" << endl;
				}

				
				// if (machineId == 16) 
				// 	cout << machineId << ": idx= " << idx << endl;
			}
		

			if(isEdgeIncoming)
			{
					
				for(int idx = 0; idx < nEdges; idx++)
				{
						
					for (int d = 0; d < dimension; d++)
					{
							
							cout<<d<<endl;
						// edges[idx].profile_v1[d] = updateVecs[receiverMachine[idx]][receiverLocalIndex[idx]-nUsers-1].newProfile1[d];
						// edges[idx].profile_v2[d] = updateVecs[receiverMachine[idx]][receiverLocalIndex[idx]-nUsers-1].newProfile2[d];
					    edges[idx].profile_v1[d]=0;
						edges[idx].profile_v2[d]=0;
					}
				}
			}
			cout << "Scatter Done!" << endl;

		// if (isEdgeIncoming) 
		// {
		// 	// std::vector <std::vector <float>> allProfiles(totalItems, vector< float> (dimension));
		// 	allProfiles.resize(totalItems); //, std::vector<uint32_t>(dimension));
		// 	for(int idx = 0; idx < allProfiles.size(); idx++) allProfiles[idx].resize(dimension);


		// 	for(int t = nUsers; t < (nUsers+nItems); t++)  // [nUsers+0 .. nUsers+nItems] is the range for item indices
		// 	{	
		// 		for (int d = 0; d < dimension; d++)
		// 		{
		// 			allProfiles[OpenedVertexIds[t]-totalUsers-1][d] = nodes[t].newProfile[d];
		// 		}
		// 		uint32_t tempId = OpenedVertexIds[t]-totalUsers-1;
			
		// 		uint32_t temp;
		// 		// cout << "totalItems: " << totalItems << endl;

		// 		/* Sending to Upper Machine & Receiving from Downs to avoid DOSing the machines */
		// 		for (int m = 0; m < machineId; m++)
		// 		{
		// 			cout << machineId << " ---> " << m << endl;
		// 			// cout << machineId << " send " << sizeof(uint32_t) << "bytes" << endl;

		// 			machine->sendToPeer(machineId, m, (unsigned char *)(&tempId), sizeof(uint32_t));
		// 			for (int d = 0; d < dimension; d++)
		// 			{
		// 				// cout << machineId << " send " << sizeof(uint32_t) << "bytes" << endl;
		// 				machine->sendToPeer(machineId, m, (unsigned char *)(&nodes[t].newProfile[d]), sizeof(uint32_t));
		// 			}
		// 		}

		// 		// cout << machineId << ": test1" << endl;

		// 		for (int n = (machineId+1); n < totalMachines; n++) 
		// 		{
		// 			cout << machineId << " <--- " << n << endl;
		// 			machine->receiveFromPeer(machineId, n, (unsigned char *)(&index), sizeof(uint32_t));
		// 			for (int d = 0; d < dimension; d++)
		// 				machine->receiveFromPeer(machineId, n, (unsigned char *)(&allProfiles[index][d]), sizeof(uint32_t));
		// 		}

		// 		// cout << machineId << ": test2" << endl;

		// 		for (int p = 0; p < machineId; p++) 
		// 		{
		// 			// cout << "Receiving vertexID from Upper Machines..." << endl;
		// 			cout << machineId << " <--- " << p << endl;
		// 			machine->receiveFromPeer(machineId, p, (unsigned char *)(&index), sizeof(uint32_t));
		// 			for (int d = 0; d < dimension; d++)
		// 				machine->receiveFromPeer(machineId, p, (unsigned char *)(&allProfiles[index][d]), sizeof(uint32_t));
		// 		}

		// 		/* Sending to Lower Machine & Receiving from Uppers to avoid DOSing the machines */
		// 		for (int p = (machineId+1); p < totalMachines; p++)
		// 		{
		// 			cout << machineId << " ---> " << p << endl;
		// 			machine->sendToPeer(machineId, p, (unsigned char *)(&tempId), sizeof(uint32_t));
		// 			for (int d = 0; d < dimension; d++)
		// 				machine->sendToPeer(machineId, p, (unsigned char *)(&nodes[t].newProfile[d]), sizeof(uint32_t));
		// 		}
		// 		// cout << machineId << ": test4" << endl;
		// 	}
		// }

		// else if (!isEdgeIncoming) 
		// {
		// 	// std::vector <std::vector <float>> allProfiles(totalItems, vector< float> (dimension));
		// 	allProfiles.resize(totalUsers, std::vector<uint32_t>(dimension));

		// 	for(int t = 0; t < nUsers; t++)  // [nUsers+0 .. nUsers+nItems] is the range for item indices
		// 	{	
		// 		for (int d = 0; d < dimension; d++)
		// 			allProfiles[nodes[t].vertexID-1][d] = nodes[t].newProfile[d];

		// 		/* Sending to Upper Machine & Receiving from Downs to avoid DOSing the machines */
		// 		for (int m = 0; m < machineId; m++)
		// 		{
		// 			// cout << "Sending vertexID to Upper Machines... " << endl;
		// 			machine->sendToPeer(machineId, m, (unsigned char *)(&nodes[t].vertexID), sizeof(uint32_t));
		// 			for (int d = 0; d < dimension; d++)
		// 				machine->sendToPeer(machineId, m, (unsigned char *)(&nodes[t].newProfile[d]), sizeof(nodes[t].newProfile[d]));
		// 		}

		// 		for (int m = (machineId+1); m < totalMachines; m++) 
		// 		{
		// 			// cout << "Receiving vertexID from Lower Machines..." << endl;
		// 			int size = machine->receiveFromPeer(machineId, m, (unsigned char *)(&index), sizeof(uint32_t));
		// 			for (int d = 0; d < dimension; d++)
		// 				int size = machine->receiveFromPeer(machineId, m, (unsigned char *)(&allProfiles[index-1][d]), sizeof(uint32_t));
		// 		}

		// 		for (int p = 0; p < machineId; p++) 
		// 		{
		// 			// cout << "Receiving vertexID from Upper Machines..." << endl;
		// 			int size = machine->receiveFromPeer(machineId, p, (unsigned char *)(&index), sizeof(uint32_t));
		// 			for (int d = 0; d < dimension; d++)
		// 				int size = machine->receiveFromPeer(machineId, p, (unsigned char *)(&allProfiles[index-1][d]), sizeof(uint32_t));
		// 		}

		// 		/* Sending to Lower Machine & Receiving from Uppers to avoid DOSing the machines */
		// 		for (int p = (machineId+1); p < totalMachines; p++)
		// 		{
		// 			// cout << "Sending vertexID to Lower Machines... " << endl;
		// 			machine->sendToPeer(machineId, p, (unsigned char *)(&nodes[t].vertexID), sizeof(uint32_t));
		// 			for (int d = 0; d < dimension; d++)
		// 				machine->sendToPeer(machineId, p, (unsigned char *)(&nodes[t].newProfile[d]), sizeof(nodes[t].newProfile[d]));
		// 		}
		// 	}
		// }
		

		return edges;
	}

	// SecretEdgeMAC * computeMAC(int party, Secret_Edge* secret_edges, int nEdges)
	// {
	// 	std::vector<unsigned char> seed = machine->getSharedRandomSeed(party, partner, com);
	// 	AESRNG *rng = new AESRNG(seed.data());
		
	// 	// Each entry of an edge needs a random element
	// 	std::vector<uint32_t> randoms = rng->GetUInt32Array(nEdges*24);

	// 	SecretEdgeMAC *MACedEdges = new SecretEdgeMAC[nEdges];

	// 	int count = 0;
	// 	for (int e = 0; e < nEdges; e++){
	// 		MACedEdges[e].id_u = MACGen->computeMAC(party, secret_edges[e].id_u, randoms[count]); count++;
	// 		MACedEdges[e].id_v = MACGen->computeMAC(party, secret_edges[e].id_v, randoms[count]); count++;
	// 		for(int idx = 0; idx < dimension; idx++)
	// 		{
	// 			MACedEdges[e].profile_u[idx] = MACGen->computeMAC(party, secret_edges[e].profile_u[idx], randoms[count]); count++;
	// 			MACedEdges[e].profile_v[idx] = MACGen->computeMAC(party, secret_edges[e].profile_v[idx], randoms[count]); count++;
	// 		}
	// 		MACedEdges[e].rating = MACGen->computeMAC(party, secret_edges[e].rating, randoms[count]); count++;
	// 		MACedEdges[e].isReal = MACGen->computeMAC(party, secret_edges[e].isReal, randoms[count]); count++;
	// 	}
		
	// 	return MACedEdges;
	// }

	// void verifyMAC(int party, SecretEdgeMAC *MACedEdges, Secret_Edge *secretEdge, int nEdges)
	// {
	// 	std::cout << "--------------- verifyMAC -----------------" << std::endl;
	// 	std::vector<uint32_t> myDifference(nEdges*24), theirDiffference(nEdges*24);
		
	// 	int count = 0;
	// 	for (int e = 0; e < nEdges; e++){
	// 		myDifference[count] = MACedEdges[e].id_u - MACGen->alpha*secretEdge[e].id_u; count++;
	// 		myDifference[count] = MACedEdges[e].id_v - MACGen->alpha*secretEdge[e].id_v; count++;
	// 		for(int idx = 0; idx < dimension; idx++)
	// 		{
	// 			myDifference[count] = MACedEdges[e].profile_u[idx] - MACGen->alpha*secretEdge[e].profile_u[idx]; count++;
	// 			myDifference[count] = MACedEdges[e].profile_v[idx] - MACGen->alpha*secretEdge[e].profile_v[idx]; count++;
	// 		}
	// 		myDifference[count] = MACedEdges[e].rating - MACGen->alpha*secretEdge[e].rating; count++;
	// 		myDifference[count] = MACedEdges[e].isReal - MACGen->alpha*secretEdge[e].isReal; count++;
	// 	}
		
	// 	// Verify that all of them are reconstructed to zero
	// 	if(party % 2 == 0)
	// 	{
	// 		com->SendEvaluationPartner((unsigned char *)myDifference.data(), myDifference.size()*sizeof(uint32_t));
	// 		com->AwaitEvaluationPartner((unsigned char *)theirDiffference.data(), myDifference.size()*sizeof(uint32_t));
	// 	}
	// 	else
	// 	{
	// 		com->AwaitEvaluationPartner((unsigned char *)theirDiffference.data(), myDifference.size()*sizeof(uint32_t));
	// 		com->SendEvaluationPartner((unsigned char *)myDifference.data(), myDifference.size()*sizeof(uint32_t));
	// 	}
		
	// 	for(int idx = 0; idx < myDifference.size(); idx++)
	// 	{
	// 		assert(myDifference[idx] + theirDiffference[idx] == 0);
	// 	}
	// }
	
	// void verifyMAC(int party, std::vector<uint32_t> MACs, std::vector<uint32_t> shares)
	// {
	// 	std::cout << "--------------- verifyMAC -----------------" << std::endl;
	// 	assert(MACs.size() == shares.size());
	// 	int nEdges = MACs.size();
		
	// 	std::vector<uint32_t> myDifference(nEdges), theirDiffference(nEdges);
		
	// 	for (int idx = 0; idx < nEdges; idx++){
	// 		myDifference[idx] = MACs[idx] - MACGen->alpha*shares[idx];
	// 	}
		
	// 	// Verify that all of them are reconstructed to zero
	// 	if(party % 2 == 0)
	// 	{
	// 		com->SendEvaluationPartner((unsigned char *)myDifference.data(), myDifference.size()*sizeof(uint32_t));
	// 		com->AwaitEvaluationPartner((unsigned char *)theirDiffference.data(), myDifference.size()*sizeof(uint32_t));
	// 	}
	// 	else
	// 	{
	// 		com->AwaitEvaluationPartner((unsigned char *)theirDiffference.data(), myDifference.size()*sizeof(uint32_t));
	// 		com->SendEvaluationPartner((unsigned char *)myDifference.data(), myDifference.size()*sizeof(uint32_t));
	// 	}
		
	// 	for(int idx = 0; idx < myDifference.size(); idx++)
	// 	{
	// 		assert(myDifference[idx] + theirDiffference[idx] == 0);
	// 	}
	// }	

	void printNodes(Secret_Node* nodes, int nNodes)
	{
		cout << "----Nodes----" << endl;
		cout << "machine: id Profile newProfile" << endl;
		for (int i = 0; i < nNodes; i++)
		{
			cout << machineId << ": " << nodes[i].vertexID << "\t" << nodes[i].Profile[0] << " " << nodes[i].newProfile[0] <<  " " << nodes[i].halfEdge.size() << endl;
		}
	}

		void printNodes(TSecret_Node* nodes, int nNodes)
	{
		cout << "----Nodes----" << endl;
		cout << "machine: id Profile newProfile" << endl;
		for (int i = 0; i < nNodes; i++)
		{
			cout << machineId << ": " << nodes[i].vertexID1 << "\t" << nodes[i].Profile1[0] << " " << nodes[i].newProfile1[0] <<  " " << endl;
		}
	}


	void printEdges(Secret_Edge* edges, int nEdges)
	{
		cout << "----Edges----" << endl;
		cout << "machine: u v   uProfile   vProfile   rating   isReal" << endl;
		for (int i = 0; i < nEdges; i++)
		{
			cout << machineId << ": " << edges[i].id_u << " " << edges[i].id_v << " " << edges[i].profile_u[0] << " " << edges[i].profile_v[0] << " " << edges[i].rating << " " << edges[i].isReal << endl;
		}
	}
};


#endif



	





