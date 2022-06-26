#include <stdint.h>
#include <vector>
#include <cassert>
#include <iostream>
#include <sstream>
#include <ctime>
#include <chrono>
#include "MaskedEvaluation.h"
#include "PreprocessingShare.h"
#include "MaskedEvaluationException.h"
#include "../Circuit/LayeredArithmeticCircuit.h"
#include "../Utility/Communicator.h"
#include "../Utility/CryptoUtility.h"

using namespace Circuit;
using namespace Utility;

#define CORRECTION_MOD 15
#define VALID_RANGE 4

namespace TwoPartyMaskedEvaluation
{
	MaskedEvaluation::MaskedEvaluation(LayeredArithmeticCircuit *lc, Communicator *communicator,AESRNG *rng_124,AESRNG *rng_234,AESRNG *rng_134) : communicator(communicator), lc(lc),rng_124(rng_124),rng_234(rng_234),rng_134(rng_134)
	{
	}
 
	MaskedEvaluation::MaskedEvaluation(LayeredArithmeticCircuit *lc, PreprocessingShare *share, Communicator *communicator,AESRNG *rng_124,AESRNG *rng_234,AESRNG *rng_134) : communicator(communicator), lc(lc), share(share),rng_124(rng_124),rng_234(rng_234),rng_134(rng_134)
	{
	}

	MaskedEvaluation::MaskedEvaluation(LayeredArithmeticCircuit *lc, PreprocessingShare *share, std::vector<int> *maskIndex, Communicator *communicator) : communicator(communicator), lc(lc), maskIndex(maskIndex), share(share)
	{
	}
	
	// input range: 0 to range->Length
	// Add to maskedEvaluation from range->Start to range->Start + range->Length
	void MaskedEvaluation::AddInput(const std::vector<std::vector<uint32_t> >& input1,const std::vector<std::vector<uint32_t> >& input2,const std::vector<std::vector<uint32_t> >& input3)
	{
		cout<<"************执行添加输入*************"<<endl;
		cout<<"*************input1的数量**********"<<endl;
		cout<<input1.size()<<endl;
		for(int i = 0;i<10;i++){
			cout<<input1[i].size()<<endl;
		}
		maskedEvaluation1 = /*std::move*/input1;

		maskedEvaluation2 = /*std::move*/input2;
		maskedEvaluation3 = /*std::move*/input3;

		//为什么要resize
		cout<<"*************线的数量**********"<<endl;
		cout<<lc->NumWires<<endl;

		cout<<"*************maskedEVALUATION的数量**********"<<endl;
		cout<<maskedEvaluation1.size()<<endl;
		maskedEvaluation1.resize(lc->NumWires);
		cout<<"*************masked resize()之后的数量**********"<<endl;
		cout<<maskedEvaluation1.size()<<endl;
		maskedEvaluation2.resize(lc->NumWires);
		maskedEvaluation3.resize(lc->NumWires);
		inputAdded = true;
	}
	std::vector<std::vector<uint32_t>> MaskedEvaluation::getout1(){
		return maskedEvaluation1;
	}
	std::vector<std::vector<uint32_t>> MaskedEvaluation::getout2(){
		return maskedEvaluation2;
	}
	std::vector<std::vector<uint32_t>> MaskedEvaluation::getout3(){
		return maskedEvaluation3;
	}




	// std::vector<std::vector<uint32_t> > MaskedEvaluation::Decrypt(std::vector<uint32_t> mask, Range *range)
	// {
	// 	if (!evaluated)
	// 	{
	// 		throw std::exception();
	// 	}
		
	// 	int count = 0;
	// 	std::vector<std::vector<uint32_t> > output(range->Length);
		
	// 	for(int idx = range->Start; idx < range->Start + range->Length; idx++)
	// 	{
	// 		output[idx - range->Start].resize(maskedEvaluation[idx].size());
	// 		for(int kdx = 0; kdx < maskedEvaluation[idx].size(); kdx++)
	// 		{
	// 			  output[idx - range->Start][kdx] = maskedEvaluation[idx][kdx] - mask[count++];
	// 		}
	// 	}
		
	// 	return output;
	// }
	


	void MaskedEvaluation::EvaluateCircuit()
	{
// 		if(playerID == DAVID) std::cout << "MaskedEvaluation::EvaluateCircuit()" << std::endl;
		//没有提供输入，抛出无输入错误
		if (!inputAdded)
		{
			throw NoInputAddedException();
		}

		if (evaluated)
		{
			throw AlreadyEvaluatedException();
		}
		//迭代电路的深度。
		cout<<"**************电路深度******"<<endl;
		cout<<lc->Depth<<endl;
		for (int layer = 0; layer < lc->Depth; layer++)
		{
// 			std::cout << playerID << ": layer: " << layer << std::endl;
            //计算加法门,已经修改完成
			cout<<"***********  layer*******************"<<endl;
			cout<<layer<<endl;
			cout<<"**************开始执行电路*************"<<endl;
			cout<<"**************计算加法门*************"<<endl;
			EvaluateAddGate(layer);
			//计算减法门，已经修改完成
			EvaluateSubGate(layer);
			cout<<"**************计算常量乘法门*************"<<endl;

			//计算常量乘法门，已经修改完成
			EvaluateCMulGate(layer);
			cout<<"**************计算普通乘法门*************"<<endl;

			//计算乘法，其实是一个标量乘以一个向量,已经修改完成
			EvaluateMulGate(layer);
			
			cout<<"**************计算点乘门*************"<<endl;
		     
			//计算点乘，已经修改完成
			EvaluateDotGate(layer);
			cout<<"**************计算N点乘门*************"<<endl;
			//计算N点乘,已经修改完成
			EvaluateNDotGate(layer);
		}
		evaluated = true;
	}
    //计算加法门，不需要通信。
	void MaskedEvaluation::EvaluateAddGate(int layer)
	{
		//std::cout << playerID << ": MaskedEvaluation::EvaluateAddGate()" << std::endl;
		auto addGates = lc->operator[](layer)->AddGates;
		int count = addGates.size();
		cout<<"***********加法门数量**********"<<endl;
		cout<<count<<endl;
		for (auto &g : lc->operator[](layer)->AddGates)
		{
			std::vector<uint32_t> leftValue  = maskedEvaluation1[g->LeftWire];
			std::vector<uint32_t> rightValue = maskedEvaluation1[g->RightWire];
			
			maskedEvaluation1[g->OutputWire] = VectorOperation::Add(leftValue, rightValue);

            //cout<<"***************maskedEvalutation************"<<endl;
			//cout<<maskedEvaluation1[g->OutputWire].size()<<endl;

			leftValue  = maskedEvaluation2[g->LeftWire];
		    rightValue = maskedEvaluation2[g->RightWire];
			maskedEvaluation2[g->OutputWire] = VectorOperation::Add(leftValue, rightValue);

			leftValue  = maskedEvaluation3[g->LeftWire];
		    rightValue = maskedEvaluation3[g->RightWire];
			maskedEvaluation3[g->OutputWire] = VectorOperation::Add(leftValue, rightValue);
		}
	}
	
	// void MaskedEvaluation::EvaluateNAddGate(int layer)
	// {
	// 	for (auto &g : lc->operator[](layer)->NAddGates)
	// 	{
	// 		maskedEvaluation[g->OutputWire].resize(DIM);
			
	// 		for(int idx = 0; idx < g->InputWires.size(); idx++)
	// 		{
	// 			std::vector<uint32_t> wireValue  = maskedEvaluation[g->InputWires[idx]];
				
	// 			for(int kdx = 0; kdx < DIM; kdx++)
	// 			{
	// 				maskedEvaluation[g->OutputWire][kdx] += wireValue[kdx];
	// 			}
	// 		}
	// 	}
	// }
	//计算减法门
	void MaskedEvaluation::EvaluateSubGate(int layer)
	{
// 		std::cout << playerID << ": MaskedEvaluation::EvaluateSubGate()" << std::endl;
		for (auto &g : lc->operator[](layer)->SubGates)
		{
			std::vector<uint32_t> leftValue  = maskedEvaluation1[g->LeftWire];
			std::vector<uint32_t> rightValue = maskedEvaluation1[g->RightWire];
			
			maskedEvaluation1[g->OutputWire] = VectorOperation::Add(leftValue, rightValue);

			leftValue  = maskedEvaluation2[g->LeftWire];
		    rightValue = maskedEvaluation2[g->RightWire];
			maskedEvaluation2[g->OutputWire] = VectorOperation::Add(leftValue, rightValue);

			leftValue  = maskedEvaluation3[g->LeftWire];
		    rightValue = maskedEvaluation3[g->RightWire];
			maskedEvaluation3[g->OutputWire] = VectorOperation::Add(leftValue, rightValue);
		}
	}
	
	void MaskedEvaluation::EvaluateCAddGate(int layer)
	{
		for (auto &g : lc->operator[](layer)->CAddGates)
		{
			std::vector<uint32_t> inputValue1 = maskedEvaluation1[g->InputWire];
			std::vector<uint32_t> inputValue2 = maskedEvaluation2[g->InputWire];
			std::vector<uint32_t> inputValue3 = maskedEvaluation3[g->InputWire];
			uint32_t constant = g->constant;
			maskedEvaluation1[g->OutputWire].resize(inputValue1.size());
			maskedEvaluation2[g->OutputWire].resize(inputValue2.size());
			maskedEvaluation3[g->OutputWire].resize(inputValue3.size());
			for(int idx = 0; idx < inputValue1.size(); idx++)
			{
				maskedEvaluation1[g->OutputWire][idx] = constant + inputValue1[idx];
				maskedEvaluation2[g->OutputWire][idx] = constant + inputValue2[idx];
				maskedEvaluation3[g->OutputWire][idx] = constant + inputValue3[idx];
			}
		}
	}
	
	void MaskedEvaluation::EvaluateCMulGate(int layer)
	{
// 		std::cout << playerID << ": MaskedEvaluation::EvaluateCMulGate()" << std::endl;
		auto cMulGates = lc->operator[](layer)->CMulGates;
		auto count = cMulGates.size();
		cout<<"*************cmul门的数量*********"<<endl;
		cout<<count<<endl;
		if(count > 0)
		{   
			//std::vector<uint32_t> beaverShare = share->GetNextBeaverTriples();	// <lz>
			std::vector<uint32_t> myBeaverValuesShare1(DIM*count);	//c*y		// <c*y + lz>
			std::vector<uint32_t> myBeaverValuesShare2(DIM*count);
			std::vector<uint32_t> myBeaverValuesShare3(DIM*count);
			// for(int i =0;i<20;i++){
			// 	cout<<"*************maskedEvaluation1大小******"<<endl;
			// 	cout<<i<<endl;
			// 	cout<<maskedEvaluation1[i].size();
			// }
			
			for (int idx = 0; idx < count; idx++)
			{
				ConstantMultiplicationGate *g = cMulGates[idx];
				//为什么应该是常量换成了int类型不报错
				
				uint32_t mx = g->constant;		           // c
				//cout<<"*************常量*************"<<endl;
		        //cout<<mx<<endl;
				std::vector<uint32_t> my1 = maskedEvaluation1[g->InputWire]; // y + ly
				std::vector<uint32_t> my2 = maskedEvaluation2[g->InputWire]; 
				std::vector<uint32_t> my3 = maskedEvaluation3[g->InputWire]; 
				//cout<<"*************常量*************"<<endl;
				int inputWire   = g->InputWire;
				int outputWire  = g->OutputWire;
				// cout<<maskedEvaluation1.size()<<endl;
				// cout<<my1.size()<<endl;
				for(int kdx = 0; kdx < DIM; kdx++)
				{
						//cout<<"*************for*************"<<endl;
					// uint32_t ly = share->operator[]((*maskIndex)[inputWire] + kdx);
					// uint32_t lz = share->operator[]((*maskIndex)[outputWire] + kdx);
				

					myBeaverValuesShare1[DIM*idx + kdx] = mx*my1[kdx];
                     //cout<<"*************Share1 finish*************"<<endl;
					myBeaverValuesShare2[DIM*idx + kdx] = mx*my2[kdx];
                    //cout<<"*************Share2 finish*************"<<endl;
					myBeaverValuesShare3[DIM*idx + kdx] = mx*my3[kdx];
					//cout<<"*************Share3 finish*************"<<endl;
				}
			}
			
			// Second, reconstruct mz = xy + lz and compute the truncation
			// std::vector<unsigned char> temp = ArrayEncoder::Encode(myBeaverValuesShare);
			// assert(temp.size() > 0);

			// std::vector<unsigned char> recv(DIM*count*sizeof(uint32_t));
			// if (playerID == ALICE || playerID == CHARLIE)
			// {
			// 	communicator->SendEvaluationPartner(temp.data(), temp.size());
			// 	communicator->AwaitEvaluationPartner(recv.data(), recv.size());
			// }
			// else
			// {
			// 	communicator->AwaitEvaluationPartner(recv.data(), recv.size());
			// 	communicator->SendEvaluationPartner(temp.data(), temp.size());
			// }
			
			// std::vector<uint32_t> theirBeaverValues = ArrayEncoder::UCharVec2UInt32tVec(recv);
			
			for (int idx = 0; idx < count; idx++)
			{
				ConstantMultiplicationGate *g = cMulGates[idx];
				maskedEvaluation1[g->OutputWire].resize(DIM);
				maskedEvaluation2[g->OutputWire].resize(DIM);
				maskedEvaluation3[g->OutputWire].resize(DIM);
				for(int kdx = 0; kdx < DIM; kdx++)
				{
					// __uint128_t temp = 0;
					maskedEvaluation1[g->OutputWire][kdx] = myBeaverValuesShare1[DIM*idx + kdx];
					maskedEvaluation2[g->OutputWire][kdx] = myBeaverValuesShare2[DIM*idx + kdx];
					maskedEvaluation3[g->OutputWire][kdx] = myBeaverValuesShare3[DIM*idx + kdx];
					// temp += maskedEvaluation[g->OutputWire][kdx];
					// temp += unTruncatedMasks[(*maskIndex)[g->OutputWire] + kdx];
					// temp = (temp >> PRECISION_BIT_LENGTH) - (masks[(*maskIndex)[g->OutputWire] + kdx]);
					// maskedEvaluation[g->OutputWire][kdx] = (uint32_t)temp;
				}
			}
		}
	}
	
	void MaskedEvaluation::EvaluateMulGate(int layer)
	{
// 		std::cout << playerID << ": MaskedEvaluation::EvaluateMulGate()" << std::endl;
// 		MyDebug::NonNull(beaver);
// 		assert(nullptr != beaver);
		
		auto mulGates = lc->operator[](layer)->MulGates;
		//count数表示乘法门的个数
		auto count = mulGates.size();
		cout<<"****普通乘法门的数量*********"<<endl;
		cout<<count<<endl;
		if (count > 0)
		{   
			int sum = DIM*count;
			std::vector<uint32_t> myBeaverValuesShare1(DIM*count);			
			std::vector<uint32_t> myBeaverValuesShare2(DIM*count);
			std::vector<uint32_t> myBeaverValuesShare3(DIM*count);
			std::vector<uint32_t> rd_1(DIM*count);
			std::vector<uint32_t> rd_2(DIM*count);
			std::vector<uint32_t> rd_3(DIM*count);
			std::vector<uint32_t> r1(sum);
			std::vector<uint32_t> r2(sum);
			std::vector<uint32_t> r(DIM*count);
			std::vector<uint32_t> rd(DIM*count);
			//assert(DIM*count == beaverShare.size());
			//准备用于截断的r数据
			
			if(playerID == ALICE){
				cout<<"***Alice开始获得***"<<endl;
					for(int i = 0; i<sum;i++){
						//cout<<i<<endl;
						r1[i] = rng_124->GetUInt32Array_1();
						//cout<<"获得了一个"<<endl;
					}
					for(int i=0; i< sum;i++){
						rd_1[i]=0;
						rd_2[i]=rng_124->GetUInt32Array_1();
						rd_3[i]=0;
					}
					cout<<"***Alice获得数据***"<<endl;
				}
				else if(playerID == BOB){
					cout<<"***Bob开始获得***"<<endl;
					for(int i = 0; i<sum;i++){
						//cout<<i<<endl;
						r1[i] = rng_124->GetUInt32Array_1();
						//cout<<"获得了一个"<<endl;
					}
					for(int i = 0; i<sum;i++){
						r2[i] = rng_234->GetUInt32Array_1();
					}
					// r1 = rng_124->GetUInt32Array(DIM*count);
					// r2 = rng_234->GetUInt32Array(DIM*count);
					for(int i=0; i<sum;i++){
						r[i]=r1[i]+r2[i];
						rd[i]=r[i]>> PRECISION_BIT_LENGTH;

						rd_1[i]=rng_124->GetUInt32Array_1();
						rd_2[i]=rd[i]-rd_1[i];
						rd_3[i]=0;
					}
					cout<<"******Bob获得数据***"<<endl;
				}
				else if(playerID == CHARLIE)
				{
					cout<<"******Charlie开始获得***"<<endl;
					for(int i = 0; i<sum;i++){
						r2[i] = rng_234->GetUInt32Array_1();
					}
                    // r2 = rng_234->GetUInt32Array(DIM*count);
					for(int i=0; i<sum;i++){
						rd_2[i]=0;
						rd_3[i]=0;
					}
					cout<<"***Charlie获得数据***"<<endl;
				}
				else if(playerID == DAVID){
					cout<<"***David开始获得***"<<endl;
					for(int i = 0; i<sum;i++){
						r1[i] = rng_124->GetUInt32Array_1();
					}
					for(int i = 0; i<sum;i++){
						r2[i] = rng_234->GetUInt32Array_1();
					}
					// r1 = rng_124->GetUInt32Array(DIM*count);
					// r2 = rng_234->GetUInt32Array(DIM*count);
					for(int i=0; i<sum;i++){
						r[i]=r1[i]+r2[i];
						rd[i]=r[i]>> PRECISION_BIT_LENGTH;

						rd_1[i]=0;
						rd_2[i]=rng_124->GetUInt32Array_1();
						rd_3[i]=rd[i]-rd_2[i];
					}
					cout<<"***David获得数据***"<<endl;
				}
			// 准备乘法数据
			for (int i = 0; i < count; i++)
			{
				MultiplicationGate *g = mulGates[i];
				
				uint32_t mx1 = maskedEvaluation1[g->LeftWire][0];  // x + lx
				uint32_t mx2 = maskedEvaluation2[g->LeftWire][0]; 
				uint32_t mx3= maskedEvaluation3[g->LeftWire][0];


				std::vector<uint32_t> my1 = maskedEvaluation1[g->RightWire]; // y + ly
				std::vector<uint32_t> my2 = maskedEvaluation2[g->RightWire];
				std::vector<uint32_t> my3 = maskedEvaluation3[g->RightWire];
				
				int leftWire = g->LeftWire;
				int rightWire = g->RightWire;
				int outputWire = g->OutputWire;
				
				// myBeaverValuesShare1[i] = 0;
				// myBeaverValuesShare2[i] = 0;
				// myBeaverValuesShare3[i] = 0;
			    //1,2,4
				if(playerID == ALICE){
					for(int kdx = 0; kdx < DIM; kdx++){
						myBeaverValuesShare1[DIM*i+kdx]=mx1*my2[kdx]+mx2*my1[kdx]+mx2*my2[kdx]-r1[DIM*i+kdx];  //1,2
					    //myBeaverValuesShare2[i]+=mx2[kdx]*my3[kdx]+mx3[kdx]*my2[kdx];    //2,4
						myBeaverValuesShare2[DIM*i+kdx]=mx1*my1[kdx]+mx2*my2[kdx]+mx3*my3[kdx]+mx1*my2[kdx]+mx2*my1[kdx]+mx1*my3[kdx]+mx3*my1[kdx]+mx2*my3[kdx]+mx3*my2[kdx]-r1[DIM*i+kdx];
					}
				}
				//2,3,4
				else if(playerID == BOB){
					for(int kdx = 0; kdx < DIM; kdx++){
						myBeaverValuesShare1[DIM*i+kdx]=mx1*my2[kdx]+mx2*my1[kdx];  //2,3
						myBeaverValuesShare2[DIM*i+kdx]=mx1*my3[kdx]+mx3*my1[kdx];  //2,4
						myBeaverValuesShare3[DIM*i+kdx]=mx2*my3[kdx]+mx3*my2[kdx];    //3,4
					}
				}
				//3,1,4
				else if(playerID == CHARLIE)
				{
					
					for(int kdx = 0; kdx < DIM; kdx++){
						myBeaverValuesShare1[DIM*i+kdx]=mx1*my2[kdx]+mx2*my1[kdx]+mx1*my1[kdx]-r2[DIM*i+kdx];  //1,3
					    myBeaverValuesShare2[DIM*i+kdx]=mx1*my1[kdx]+mx2*my2[kdx]+mx3*my3[kdx]+mx1*my2[kdx]+mx2*my1[kdx]+mx1*my3[kdx]+mx3*my1[kdx]+mx2*my3[kdx]+mx3*my2[kdx]-r2[DIM*i+kdx];
					}
				}
				//1,2,3
				else if(playerID == DAVID){
					for(int kdx = 0; kdx < DIM; kdx++){
					myBeaverValuesShare1[DIM*i+kdx]=mx1*my2[kdx]+mx2*my1[kdx]+mx2*my2[kdx];  //1,2
					myBeaverValuesShare2[DIM*i+kdx]=mx2*my3[kdx]+mx3*my2[kdx];    //2,3
					myBeaverValuesShare3[DIM*i+kdx]=mx1*my3[kdx]+mx3*my1[kdx];  //1,3
					}
				}
			}

            //传输数据
			if(playerID== ALICE){
				std::vector<unsigned char> temp = ArrayEncoder::Encode(myBeaverValuesShare1);
				assert(temp.size() > 0);
				std::vector<unsigned char> recv1(count*sizeof(uint32_t)*DIM);
				std::vector<unsigned char> recv2(count*sizeof(uint32_t)*DIM);
				std::vector<unsigned char> recv3(count*sizeof(uint32_t)*DIM);
				communicator->SendEvaluationPartner2(temp.data(),temp.size());
				//cout<<"第一次等待"<<endl;
				communicator->AwaitEvaluationPartner1(recv2.data(),recv2.size());
				//cout<<"第二次等待"<<endl;
				communicator->AwaitEvaluationPartner1(recv3.data(),recv3.size());
				communicator->AwaitEvaluationPartner2(recv1.data(),recv1.size());
				std::vector<uint32_t> z_13 = ArrayEncoder::UCharVec2UInt32tVec(recv1);
				std::vector<uint32_t> z_23= ArrayEncoder::UCharVec2UInt32tVec(recv2);
				std::vector<uint32_t> z_34 = ArrayEncoder::UCharVec2UInt32tVec(recv3);
				std::vector<uint32_t> z_sub_r(DIM*count);
				std::vector<uint32_t> z_d(DIM*count);
				//cout<<"开始迭代"<<endl;
				for(int i= 0;i<count;i++){
					for(int kdx=0;kdx<DIM;kdx++){
						z_sub_r[DIM*i+kdx]=myBeaverValuesShare2[DIM*i+kdx]+z_13[DIM*i+kdx]+z_23[DIM*i+kdx]+z_34[DIM*i+kdx];
					    z_d[DIM*i+kdx]=z_sub_r[DIM*i+kdx]>>PRECISION_BIT_LENGTH;
					}
				}
				
				for(int i=0;i<count;i++){
					for(int kdx=0;kdx<DIM;kdx++){
						myBeaverValuesShare1[DIM*i+kdx]=rng_134->GetUInt32Array_1();
						myBeaverValuesShare2[DIM*i+kdx]=0;
						myBeaverValuesShare3[DIM*i+kdx]=z_d[DIM*i+kdx]-myBeaverValuesShare1[DIM*i+kdx];
					}
				}
				std::vector<unsigned char> temp1 = ArrayEncoder::Encode(myBeaverValuesShare3);
				cout<<"发送"<<endl;
				communicator->SendEvaluationPartner1(temp1.data(),temp1.size());
				cout<<"发送成功"<<endl;
				
			}
			else if(playerID == BOB){
				std::vector<unsigned char> temp1 = ArrayEncoder::Encode(myBeaverValuesShare1);
				assert(temp1.size() > 0);
				std::vector<unsigned char> temp2 = ArrayEncoder::Encode(myBeaverValuesShare2);
				assert(temp2.size() > 0);
				std::vector<unsigned char> temp3 = ArrayEncoder::Encode(myBeaverValuesShare3);
				assert(temp3.size() > 0);
				std::vector<unsigned char> temp4 = ArrayEncoder::Encode(rd_2);
				assert(temp4.size() > 0);
				std::vector<unsigned char> recv1(count*sizeof(uint32_t)*DIM);

				communicator->SendEvaluationPartner1(temp1.data(),temp1.size());   //2,3
				communicator->SendEvaluationPartner1(temp2.data(),temp2.size());   //2,4
				communicator->SendEvaluationPartner1(temp4.data(),temp4.size());   //r

				communicator->SendEvaluationPartner2(temp1.data(),temp1.size());   //2,3
				communicator->SendEvaluationPartner2(temp3.data(),temp3.size());   //3,4
				communicator->AwaitEvaluationPartner2(recv1.data(),recv1.size());

				//std::vector<unsigned char> recv1(count*sizeof(uint32_t)*DIM);
				std::vector<uint32_t> zd_3 = ArrayEncoder::UCharVec2UInt32tVec(recv1);
				for(int i=0;i<count;i++){
					for(int kdx=0;kdx<DIM;kdx++){
						myBeaverValuesShare1[DIM*i+kdx]=0;
						myBeaverValuesShare2[DIM*i+kdx]=0;
						myBeaverValuesShare3[DIM*i+kdx]=zd_3[DIM*i+kdx];
					}
				}

			}else if(playerID == CHARLIE){
				std::vector<unsigned char> temp = ArrayEncoder::Encode(myBeaverValuesShare1);
				assert(temp.size() > 0);
				std::vector<unsigned char> recv1(count*sizeof(uint32_t)*DIM);
				std::vector<unsigned char> recv2(count*sizeof(uint32_t)*DIM);
				std::vector<unsigned char> recv3(count*sizeof(uint32_t)*DIM);
				std::vector<unsigned char> recv4(count*sizeof(uint32_t)*DIM);
				communicator->AwaitEvaluationPartner1(recv1.data(),recv1.size());  //1,2
				communicator->SendEvaluationPartner1(temp.data(),temp.size());

				communicator->AwaitEvaluationPartner2(recv2.data(),recv2.size());   //2,3
				communicator->AwaitEvaluationPartner2(recv3.data(),recv3.size());   //2,4
				communicator->AwaitEvaluationPartner2(recv4.data(),recv4.size());   //rd_1
				std::vector<uint32_t> z_12 = ArrayEncoder::UCharVec2UInt32tVec(recv1);
				std::vector<uint32_t> z_23= ArrayEncoder::UCharVec2UInt32tVec(recv2);
				std::vector<uint32_t> z_24 = ArrayEncoder::UCharVec2UInt32tVec(recv3);
				rd_1 = ArrayEncoder::UCharVec2UInt32tVec(recv4);
				std::vector<uint32_t> z_sub_r(count*DIM);
				std::vector<uint32_t> z_d(count*DIM);
				for(int i= 0;i<count;i++){
					for(int kdx=0;kdx<DIM;kdx++){
						z_sub_r[DIM*i+kdx]=myBeaverValuesShare2[DIM*i+kdx]+z_12[DIM*i+kdx]+z_23[DIM*i+kdx]+z_24[DIM*i+kdx];
						z_d[DIM*i+kdx]=z_sub_r[DIM*i+kdx]>>PRECISION_BIT_LENGTH;
					}
				}
				for(int i=0;i<count;i++){
					for(int kdx=0;kdx<DIM;kdx++){
						myBeaverValuesShare1[DIM*i+kdx]=0;
						myBeaverValuesShare2[DIM*i+kdx]=rng_134->GetUInt32Array_1();
						myBeaverValuesShare3[DIM*i+kdx]=z_d[DIM*i+kdx]-myBeaverValuesShare2[DIM*i+kdx];
					}
				}
			}
			else{ //Alice
			for(int i=0;i<count;i++){
				for(int kdx=0;kdx<DIM;kdx++){
					myBeaverValuesShare1[DIM*i+kdx]=rng_134->GetUInt32Array_1();
					myBeaverValuesShare2[DIM*i+kdx]=0;
					myBeaverValuesShare3[DIM*i+kdx]=0;
				}	
				}
			}

			// // Second, reconstruct mz = xy + lz and compute the truncation
			// std::vector<unsigned char> temp = ArrayEncoder::Encode(myBeaverValuesShare);
			// assert(temp.size() > 0);

			// std::vector<unsigned char> recv(DIM*count*sizeof(uint32_t));
			// if (playerID == ALICE || playerID == CHARLIE)
			// {
			// 	communicator->SendEvaluationPartner(temp.data(), temp.size());
			// 	communicator->AwaitEvaluationPartner(recv.data(), recv.size());
			// }
			// else
			// {
			// 	communicator->AwaitEvaluationPartner(recv.data(), recv.size());
			// 	communicator->SendEvaluationPartner(temp.data(), temp.size());
			// }
			
			// std::vector<uint32_t> theirBeaverValues = ArrayEncoder::UCharVec2UInt32tVec(recv);
			//cout<<"开始最后计算"<<endl;
			for (int idx = 0; idx < count; idx++)
			{
				MultiplicationGate *g = mulGates[idx];
				maskedEvaluation1[g->OutputWire].resize(DIM);
				maskedEvaluation2[g->OutputWire].resize(DIM);
				maskedEvaluation3[g->OutputWire].resize(DIM);
				//cout<<"开始迭代"<<endl;
				for(int kdx = 0; kdx < DIM; kdx++)
				{
					__uint128_t temp = 0;
					maskedEvaluation1[g->OutputWire][kdx] = myBeaverValuesShare1[DIM*idx+kdx] + rd_1[DIM*idx+kdx];
					maskedEvaluation2[g->OutputWire][kdx] = myBeaverValuesShare2[DIM*idx+kdx] + rd_2[DIM*idx+kdx];
					maskedEvaluation3[g->OutputWire][kdx] = myBeaverValuesShare3[DIM*idx+kdx] + rd_3[DIM*idx+kdx];
					// temp += maskedEvaluation[g->OutputWire][kdx];
					// temp += unTruncatedMasks[(*maskIndex)[g->OutputWire] + kdx];
					// temp = (temp >> PRECISION_BIT_LENGTH) - (masks[(*maskIndex)[g->OutputWire] + kdx]);
					// maskedEvaluation[g->OutputWire][kdx] = (uint32_t)temp;
				}
			}
		}
	}
	
	void MaskedEvaluation::EvaluateDotGate(int layer)
	{
// 		std::cout << playerID << ": MaskedEvaluation::EvaluateDotGate()" << std::endl;
		auto dotGates = lc->operator[](layer)->DotGates;
		int count = dotGates.size();
		//cout<<"*********层数："+layer <<"***********"<<endl;
		cout<<"*********点乘门个数***************"<<endl;
		cout<<count <<endl;
		if(count > 0)
		{
			// std::vector<uint32_t> beaverShare = share->GetNextBeaverTriples();	// <lx*ly>
			std::vector<uint32_t> myBeaverValuesShare1(count);
			std::vector<uint32_t> myBeaverValuesShare2(count);	
			std::vector<uint32_t> myBeaverValuesShare3(count);				// -<lx*my> - <ly*mx> + <lz> + <lx*ly>
			
			//第一层send前的准备
			//cout<<"***************开始处理随机掩盖玛r***********"<<endl;
			std::vector<uint32_t> rd_1(count);
			std::vector<uint32_t> rd_2(count);
			std::vector<uint32_t> rd_3(count);
			std::vector<uint32_t> r1(count);
			std::vector<uint32_t> r2(count);
			std::vector<uint32_t> r(count);
			std::vector<uint32_t> rd(count);
			//算r。
			if(playerID == ALICE){
				// cout<<"ALICE开始处理数据************"<<endl;
				// cout<<rng_124<<endl;
					r1 = rng_124->GetUInt32Array(count);
					// cout<<"r1获得数据************"<<endl;
					for(int i=0; i<count;i++){
						rd_1[i]=0;
						rd_2[i]=rng_124->GetUInt32Array_1();
						rd_3[i]=0;
					}
					// cout<<"rd获得数据************"<<endl;
				}
			if(playerID == BOB){
					// cout<<"BOB开始处理数据************"<<endl;
					// //这里显示Rng=0???
					// cout<<count<<endl;
					for(int i = 0; i<count;i++){
						r1[i]=rng_124->GetUInt32Array_1();
						//cout<<"获得了r"+i<<endl;
					}
					// cout<<"r1开始处理数据************"<<endl;
					for(int i = 0; i<count;i++){
						r2[i]=rng_124->GetUInt32Array_1();
						//cout<<"获得了r"+i<<endl;
					}
					// cout<<"r2开始处理数据************"<<endl;
					for(int i=0; i<count;i++){
						r[i]=r1[i]+r2[i];
						rd[i]=r[i]>> PRECISION_BIT_LENGTH;

						rd_1[i]=rng_124->GetUInt32Array_1();
						rd_2[i]=rd[i]-rd_1[i];
						rd_3[i]=0;
					}
					// cout<<"BOB获得数据************"<<endl;
				}
				if(playerID == CHARLIE)
				{
					// cout<<"CHARLIE开始处理数据************"<<endl;
					// cout<<rng_234<<endl;
					// cout<<"获得r2"<<endl;
                    for(int i = 0; i<count;i++){
						r2[i]=rng_234->GetUInt32Array_1();
						//cout<<"获得了r"+i<<endl;
					}
					for(int i=0; i<count;i++){
						rd_2[i]=0;
						rd_3[i]=0;
					}
					// cout<<"CHARLIE获得数据************"<<endl;
				}
				if(playerID == DAVID){
					// cout<<"DAVID开始处理数据************"<<endl;
					// cout<<rng_124<<endl;
					// cout<<rng_234<<endl;
					// cout<<"DAVID获得r1"<<endl;
					for(int i = 0; i<count;i++){
						r1[i]=rng_124->GetUInt32Array_1();
						//cout<<"获得了r"+i<<endl;
					}
					for(int i = 0; i<count;i++){
						r2[i]=rng_234->GetUInt32Array_1();
						//cout<<"获得了r"+i<<endl;
					}
					for(int i=0; i<count;i++){
						r[i]=r1[i]+r2[i];
						rd[i]=r[i]>> PRECISION_BIT_LENGTH;

						rd_1[i]=0;
						rd_2[i]=rng_124->GetUInt32Array_1();
						rd_3[i]=rd[i]-rd_2[i];
					}
					// cout<<"David获得数据************"<<endl;
				}

			//assert(beaverShare.size() == count);
			//count表示计算卷积的次数
			// cout<<"***************开始计算需要发送的数据***********"<<endl;

			for (int i = 0; i < count; i++)
			{
				DotProductGate *g = dotGates[i];
				
				std::vector<uint32_t> mx1 = maskedEvaluation1[g->LeftWire];  // x + lx
				std::vector<uint32_t> mx2 = maskedEvaluation2[g->LeftWire];  // x + lx
				std::vector<uint32_t> mx3 = maskedEvaluation3[g->LeftWire];  // x + lx


				std::vector<uint32_t> my1 = maskedEvaluation1[g->RightWire]; // y + ly
				std::vector<uint32_t> my2 = maskedEvaluation2[g->RightWire];
				std::vector<uint32_t> my3 = maskedEvaluation3[g->RightWire];
				
				int leftWire = g->LeftWire;
				int rightWire = g->RightWire;
				int outputWire = g->OutputWire;
				
				myBeaverValuesShare1[i] = 0;
				myBeaverValuesShare2[i] = 0;
				myBeaverValuesShare3[i] = 0;
			    //1,2,4
				if(playerID == ALICE){
					for(int kdx = 0; kdx < DIM; kdx++){
						myBeaverValuesShare1[i]+=mx1[kdx]*my2[kdx]+mx2[kdx]*my1[kdx]+mx2[kdx]*my2[kdx];  //1,2
					    //myBeaverValuesShare2[i]+=mx2[kdx]*my3[kdx]+mx3[kdx]*my2[kdx];    //2,4
						myBeaverValuesShare2[i]+=mx1[kdx]*my1[kdx]+mx2[kdx]*my2[kdx]+mx3[kdx]*my3[kdx]+mx1[kdx]*my2[kdx]+mx2[kdx]*my1[kdx]+mx1[kdx]*my3[kdx]+mx3[kdx]*my1[kdx]+mx2[kdx]*my3[kdx]+mx3[kdx]*my2[kdx];
					}
					myBeaverValuesShare1[i]-=r1[i];
					myBeaverValuesShare2[i]-=r1[i];
				}
				//2,3,4
				else if(playerID == BOB){
					for(int kdx = 0; kdx < DIM; kdx++){
						myBeaverValuesShare1[i]+=mx1[kdx]*my2[kdx]+mx2[kdx]*my1[kdx];  //2,3
						myBeaverValuesShare2[i]+=mx1[kdx]*my3[kdx]+mx3[kdx]*my1[kdx];  //2,4
						myBeaverValuesShare3[i]+=mx2[kdx]*my3[kdx]+mx3[kdx]*my2[kdx];    //3,4
					}
				}
				//3,1,4
				else if(playerID == CHARLIE)
				{
					for(int kdx = 0; kdx < DIM; kdx++){
						myBeaverValuesShare1[i]+=mx1[kdx]*my2[kdx]+mx2[kdx]*my1[kdx]+mx1[kdx]*my1[kdx];  //1,3
					    myBeaverValuesShare2[i]+=mx1[kdx]*my1[kdx]+mx2[kdx]*my2[kdx]+mx3[kdx]*my3[kdx]+mx1[kdx]*my2[kdx]+mx2[kdx]*my1[kdx]+mx1[kdx]*my3[kdx]+mx3[kdx]*my1[kdx]+mx2[kdx]*my3[kdx]+mx3[kdx]*my2[kdx];
					}
				   myBeaverValuesShare1[i]-=r2[i];
				   myBeaverValuesShare2[i]-=r2[i];
				}
				//1,2,3
				else if(playerID == DAVID){
					for(int kdx = 0; kdx < DIM; kdx++){
					myBeaverValuesShare1[i]+=mx1[kdx]*my2[kdx]+mx2[kdx]*my1[kdx]+mx2[kdx]*my2[kdx];  //1,2
					myBeaverValuesShare2[i]+=mx2[kdx]*my3[kdx]+mx3[kdx]*my2[kdx];    //2,3
					myBeaverValuesShare3[i]+=mx1[kdx]*my3[kdx]+mx3[kdx]*my1[kdx];  //1,3
					}
				}
				
				// for(int kdx = 0; kdx < DIM; kdx++)
				// {
				// 	// uint32_t lx = share->operator[]((*maskIndex)[leftWire] + kdx);
				// 	// uint32_t ly = share->operator[]((*maskIndex)[rightWire] + kdx);
					
				// 	// player1
				// 	if (playerID == ALICE){
				// 		//利用随机

				// 		myBeaverValuesShare[i] += (-lx*my[kdx] - ly*mx[kdx]);
				// 	}
				// 	// player2
				// 	else if (playerID == BOB{
				// 		myBeaverValuesShare[i] += (mx[kdx]*my[kdx] -lx*my[kdx] - ly*mx[kdx]);
				// 	}
				// 	// player3
				// 	else if( playerID == CHARLIE){

				// 	}
				// 	// player4
				// 	else{

				// 	}
				// }
				
				//myBeaverValuesShare[i] = myBeaverValuesShare[i] + beaverShare[i];
			}
			// cout<<"***************开始发送数据***********"<<endl;
			if(playerID== ALICE){
				std::vector<unsigned char> temp = ArrayEncoder::Encode(myBeaverValuesShare1);
				assert(temp.size() > 0);
				std::vector<unsigned char> recv1(count*sizeof(uint32_t));
				std::vector<unsigned char> recv2(count*sizeof(uint32_t));
				std::vector<unsigned char> recv3(count*sizeof(uint32_t));
				communicator->SendEvaluationPartner2(temp.data(),temp.size());
				communicator->AwaitEvaluationPartner1(recv2.data(),recv2.size());
				communicator->AwaitEvaluationPartner1(recv3.data(),recv3.size());
				communicator->AwaitEvaluationPartner2(recv1.data(),recv1.size());
				std::vector<uint32_t> z_13 = ArrayEncoder::UCharVec2UInt32tVec(recv1);
				std::vector<uint32_t> z_23= ArrayEncoder::UCharVec2UInt32tVec(recv2);
				std::vector<uint32_t> z_34 = ArrayEncoder::UCharVec2UInt32tVec(recv3);
				std::vector<uint32_t> z_sub_r(count);
				std::vector<uint32_t> z_d(count);
				for(int i= 0;i<count;i++){
					z_sub_r[i]=myBeaverValuesShare2[i]+z_13[i]+z_23[i]+z_34[i];
					z_d[i]=z_sub_r[i]>>PRECISION_BIT_LENGTH;
				}
				
				for(int i=0;i<count;i++){
					myBeaverValuesShare1[i]=rng_134->GetUInt32Array_1();
					myBeaverValuesShare2[i]=0;
					myBeaverValuesShare3[i]=z_d[i]-myBeaverValuesShare1[i];
				}

				std::vector<unsigned char> temp1 = ArrayEncoder::Encode(myBeaverValuesShare3);
				communicator->SendEvaluationPartner1(temp1.data(),temp1.size());
			}
			else if(playerID == BOB){
				std::vector<unsigned char> temp1 = ArrayEncoder::Encode(myBeaverValuesShare1);
				assert(temp1.size() > 0);
				std::vector<unsigned char> temp2 = ArrayEncoder::Encode(myBeaverValuesShare2);
				assert(temp2.size() > 0);
				std::vector<unsigned char> temp3 = ArrayEncoder::Encode(myBeaverValuesShare3);
				assert(temp3.size() > 0);
				std::vector<unsigned char> temp4 = ArrayEncoder::Encode(rd_2);
				assert(temp4.size() > 0);

				communicator->SendEvaluationPartner1(temp1.data(),temp1.size());   //2,3
				communicator->SendEvaluationPartner1(temp2.data(),temp2.size());   //2,4
				communicator->SendEvaluationPartner1(temp4.data(),temp4.size());   //r

				communicator->SendEvaluationPartner2(temp1.data(),temp1.size());   //2,3
				communicator->SendEvaluationPartner2(temp3.data(),temp3.size());   //3,4

				std::vector<unsigned char> recv1(count*sizeof(uint32_t));
				std::vector<uint32_t> zd_3 = ArrayEncoder::UCharVec2UInt32tVec(recv1);
					for(int i=0;i<count;i++){
					myBeaverValuesShare1[i]=0;
					myBeaverValuesShare2[i]=0;
					myBeaverValuesShare3[i]=zd_3[i];
				}

			}else if(playerID == CHARLIE){
				std::vector<unsigned char> temp = ArrayEncoder::Encode(myBeaverValuesShare1);
				assert(temp.size() > 0);
				std::vector<unsigned char> recv1(count*sizeof(uint32_t));
				std::vector<unsigned char> recv2(count*sizeof(uint32_t));
				std::vector<unsigned char> recv3(count*sizeof(uint32_t));
				std::vector<unsigned char> recv4(count*sizeof(uint32_t));
				communicator->AwaitEvaluationPartner1(recv1.data(),recv1.size());  //1,2
				communicator->SendEvaluationPartner1(temp.data(),temp.size());

				communicator->AwaitEvaluationPartner2(recv2.data(),recv2.size());   //2,3
				communicator->AwaitEvaluationPartner2(recv3.data(),recv3.size());   //2,4
				communicator->AwaitEvaluationPartner2(recv4.data(),recv4.size());   //rd_1
				std::vector<uint32_t> z_12 = ArrayEncoder::UCharVec2UInt32tVec(recv1);
				std::vector<uint32_t> z_23= ArrayEncoder::UCharVec2UInt32tVec(recv2);
				std::vector<uint32_t> z_24 = ArrayEncoder::UCharVec2UInt32tVec(recv3);
				rd_1 = ArrayEncoder::UCharVec2UInt32tVec(recv4);
				std::vector<uint32_t> z_sub_r(count);
				std::vector<uint32_t> z_d(count);
				for(int i= 0;i<count;i++){
					z_sub_r[i]=myBeaverValuesShare2[i]+z_12[i]+z_23[i]+z_24[i];
					z_d[i]=z_sub_r[i]>>PRECISION_BIT_LENGTH;
				}
				for(int i=0;i<count;i++){
					myBeaverValuesShare1[i]=0;
					myBeaverValuesShare2[i]=rng_134->GetUInt32Array_1();
					myBeaverValuesShare3[i]=z_d[i]-myBeaverValuesShare2[i];
				}
			}
			else{ //Alice
			for(int i=0;i<count;i++){
					myBeaverValuesShare1[i]=rng_134->GetUInt32Array_1();
					myBeaverValuesShare2[i]=0;
					myBeaverValuesShare3[i]=0;
				}
			}
			
			// std::vector<unsigned char> temp = ArrayEncoder::Encode(myBeaverValuesShare);
			// assert(temp.size() > 0);

			// std::vector<unsigned char> recv(count*sizeof(uint32_t));
			// if (playerID == ALICE || playerID == CHARLIE)
			// {
			// 	communicator->SendEvaluationPartner(temp.data(), temp.size());
			// 	communicator->AwaitEvaluationPartner(recv.data(), recv.size());
			// }
			// else
			// {
			// 	communicator->AwaitEvaluationPartner(recv.data(), recv.size());
			// 	communicator->SendEvaluationPartner(temp.data(), temp.size());
			// }
			
			// std::vector<uint32_t> theirBeaverValues = ArrayEncoder::UCharVec2UInt32tVec(recv);
			// cout<<"***************开始计算结果数据***********"<<endl;
			for(int idx = 0; idx < count; idx++)
			{
				DotProductGate *g = dotGates[idx];
				
				maskedEvaluation1[g->OutputWire].resize(1);
				maskedEvaluation2[g->OutputWire].resize(1);
				maskedEvaluation3[g->OutputWire].resize(1);
				
				// __uint128_t temp = 0;
				maskedEvaluation1[g->OutputWire][0] = myBeaverValuesShare1[idx] + rd_1[idx];
				maskedEvaluation2[g->OutputWire][0] = myBeaverValuesShare2[idx] + rd_2[idx];
				maskedEvaluation3[g->OutputWire][0] = myBeaverValuesShare3[idx] + rd_3[idx];
				// temp += maskedEvaluation[g->OutputWire][0];
				// temp += unTruncatedMasks[(*maskIndex)[g->OutputWire]];
				// temp = (temp >> PRECISION_BIT_LENGTH) - (masks[(*maskIndex)[g->OutputWire]]);
				// maskedEvaluation[g->OutputWire][0] = (uint32_t)temp;
			}
		}
	}
	
	void MaskedEvaluation::EvaluateNDotGate(int layer)
	{
// 		if(playerID == DAVID) std::cout << "MaskedEvaluation::EvaluateNDotGate()" << std::endl;
		auto nDotGates = lc->operator[](layer)->NDotGates;
		
		int count = nDotGates.size();
		cout<<"**********nDot门数量*********"<<endl;
		cout<<count<<endl;
		std::stringstream ss;
		if(count > 0)
		{
			//std::vector<uint32_t> beaverShare = share->GetNextBeaverTriples();	// <lx*ly>
			std::vector<uint32_t> myBeaverValuesShare1(DIM*count,0);			// -<lx*my> - <ly*mx> + <lz> + <lx*ly>
			std::vector<uint32_t> myBeaverValuesShare2(DIM*count,0);	
			std::vector<uint32_t> myBeaverValuesShare3(DIM*count,0);	

			std::vector<uint32_t> rd_1(DIM*count);
			std::vector<uint32_t> rd_2(DIM*count);
			std::vector<uint32_t> rd_3(DIM*count);
			std::vector<uint32_t> r1(DIM*count);
			std::vector<uint32_t> r2(DIM*count);
			std::vector<uint32_t> r(DIM*count);
			std::vector<uint32_t> rd(DIM*count);
			int sum = count*DIM;
			//assert(DIM*count == beaverShare.size());
			//准备用于截断的r数据
			
			if(playerID == ALICE){
					r1 = rng_124->GetUInt32Array(DIM*count);
					for(int i=0; i< sum;i++){
						rd_1[i]=0;
						rd_2[i]=rng_124->GetUInt32Array_1();
						rd_3[i]=0;
					}
				}
				else if(playerID == BOB){
					for(int i = 0;i<DIM*count;i++){
						r1[i] = rng_124->GetUInt32Array_1();
					}
					for(int i = 0;i<DIM*count;i++){
						r2[i] = rng_234->GetUInt32Array_1();
					}
					for(int i=0; i<sum;i++){
						r[i]=r1[i]+r2[i];
						rd[i]=r[i]>> PRECISION_BIT_LENGTH;

						rd_1[i]=rng_124->GetUInt32Array_1();
						rd_2[i]=rd[i]-rd_1[i];
						rd_3[i]=0;
					}
				}
				else if(playerID == CHARLIE)
				{
                    for(int i = 0;i<DIM*count;i++){
						r2[i] = rng_234->GetUInt32Array_1();
					}
					for(int i=0; i<sum;i++){
						rd_2[i]=0;
						rd_3[i]=0;
					}
				}
				else if(playerID == DAVID){
					for(int i = 0;i<DIM*count;i++){
						r1[i] = rng_124->GetUInt32Array_1();
					}
					for(int i = 0;i<DIM*count;i++){
						r2[i] = rng_234->GetUInt32Array_1();
					}
					for(int i=0; i<sum;i++){
						r[i]=r1[i]+r2[i];
						rd[i]=r[i]>> PRECISION_BIT_LENGTH;

						rd_1[i]=0;
						rd_2[i]=rng_124->GetUInt32Array_1();
						rd_3[i]=rd[i]-rd_2[i];
					}
				}
			
			//assert(beaverShare.size() == DIM*count);


			//准备乘法数据
			
			for (int i = 0; i < count; i++)
			{
				NaryDotGate *g = nDotGates[i];
				
				int step = g->InputWires.size()/2;
				for(int idx = 0; idx < step; idx++)
				{
					uint32_t mx1              = maskedEvaluation1[g->InputWires[idx]][0];  // r_idx + lr_idx
					uint32_t mx2              = maskedEvaluation2[g->InputWires[idx]][0];  // r_idx + lr_idx
					uint32_t mx3              = maskedEvaluation3[g->InputWires[idx]][0];  // r_idx + lr_idx
					std::vector<uint32_t> my1 = maskedEvaluation1[g->InputWires[idx + step]]; // y_idx + ly_idx
					std::vector<uint32_t> my2 = maskedEvaluation2[g->InputWires[idx + step]]; // y_idx + ly_idx
					std::vector<uint32_t> my3 = maskedEvaluation3[g->InputWires[idx + step]]; // y_idx + ly_idx
					
					
					int leftWire = g->InputWires[idx];
					int rightWire = g->InputWires[idx + step];

				    if(playerID == ALICE){
						for(int kdx = 0; kdx < DIM; kdx++){
							//myBeaverValuesShare1[DIM*i+kdx]+=mx1*my2[kdx]+mx2*my1[kdx]+mx2*my2[kdx]-r1[DIM*i+kdx];  //1,2
							myBeaverValuesShare1[DIM*i+kdx]+=mx1*my2[kdx]+mx2*my1[kdx]+mx2*my2[kdx];
							//myBeaverValuesShare2[i]+=mx2[kdx]*my3[kdx]+mx3[kdx]*my2[kdx];    //2,4
							//myBeaverValuesShare2[DIM*i+kdx]+=mx1*my1[kdx]+mx2*my2[kdx]+mx3*my3[kdx]+mx1*my2[kdx]+mx2*my1[kdx]+mx1*my3[kdx]+mx3*my1[kdx]+mx2*my3[kdx]+mx3*my2[kdx]-r1[DIM*i+kdx];
							myBeaverValuesShare2[DIM*i+kdx]+=mx1*my1[kdx]+mx2*my2[kdx]+mx3*my3[kdx]+mx1*my2[kdx]+mx2*my1[kdx]+mx1*my3[kdx]+mx3*my1[kdx]+mx2*my3[kdx]+mx3*my2[kdx];
						}
				    }
				//2,3,4
					else if(playerID == BOB){
						for(int kdx = 0; kdx < DIM; kdx++){
							myBeaverValuesShare1[DIM*i+kdx]+=mx1*my2[kdx]+mx2*my1[kdx];  //2,3
							myBeaverValuesShare2[DIM*i+kdx]+=mx1*my3[kdx]+mx3*my1[kdx];  //2,4
							myBeaverValuesShare3[DIM*i+kdx]+=mx2*my3[kdx]+mx3*my2[kdx];    //3,4
						}
					}
				//3,1,4
					else if(playerID == CHARLIE)
					{
						for(int kdx = 0; kdx < DIM; kdx++){
							// myBeaverValuesShare1[DIM*i+kdx]+=mx1*my2[kdx]+mx2*my1[kdx]+mx1*my1[kdx]-r2[DIM*i+kdx];  //1,3
							// myBeaverValuesShare2[DIM*i+kdx]+=mx1*my1[kdx]+mx2*my2[kdx]+mx3*my3[kdx]+mx1*my2[kdx]+mx2*my1[kdx]+mx1*my3[kdx]+mx3*my1[kdx]+mx2*my3[kdx]+mx3*my2[kdx]-r2[DIM*i+kdx];
						    myBeaverValuesShare1[DIM*i+kdx]+=mx1*my2[kdx]+mx2*my1[kdx]+mx1*my1[kdx];  //1,3
							myBeaverValuesShare2[DIM*i+kdx]+=mx1*my1[kdx]+mx2*my2[kdx]+mx3*my3[kdx]+mx1*my2[kdx]+mx2*my1[kdx]+mx1*my3[kdx]+mx3*my1[kdx]+mx2*my3[kdx]+mx3*my2[kdx];
						}
					}
					//1,2,3
					else if(playerID == DAVID){
						for(int kdx = 0; kdx < DIM; kdx++){
						myBeaverValuesShare1[DIM*i+kdx]+=mx1*my2[kdx]+mx2*my1[kdx]+mx2*my2[kdx];  //1,2
						myBeaverValuesShare2[DIM*i+kdx]+=mx2*my3[kdx]+mx3*my2[kdx];    //2,3
						myBeaverValuesShare3[DIM*i+kdx]+=mx1*my3[kdx]+mx3*my1[kdx];  //1,3
						}
					}
					
					//uint32_t lx = share->operator[]((*maskIndex)[leftWire]); 	// lx = <lr>
					
					// for(int kdx = 0; kdx < DIM; kdx++)
					// {
					// 	uint32_t ly = share->operator[]((*maskIndex)[rightWire] + kdx);// ly = <ly[kdx]>
						
					// 	if (playerID == ALICE || playerID == CHARLIE){
					// 		myBeaverValuesShare[DIM*i + kdx] += (-lx*my[kdx] - ly*mx);
					// 	}
					// 	else {
					// 		myBeaverValuesShare[DIM*i + kdx] += (mx*my[kdx] -lx*my[kdx] - ly*mx);
					// 	}
					// }
				}
				if(playerID == ALICE){
					for(int kdx = 0; kdx < DIM; kdx++){
						myBeaverValuesShare1[DIM*i+kdx]-=r1[DIM*i+kdx];
						myBeaverValuesShare2[DIM*i+kdx]-=r1[DIM*i+kdx];
						//myBeaverValuesShare1[i]+=mx1[kdx]*my2[kdx]+mx2[kdx]*my1[kdx]+mx2[kdx]*my2[kdx];  //1,2
					    //myBeaverValuesShare2[i]+=mx2[kdx]*my3[kdx]+mx3[kdx]*my2[kdx];    //2,4
						//myBeaverValuesShare2[i]+=mx1[kdx]*my1[kdx]+mx2[kdx]*my2[kdx]+mx3[kdx]*my3[kdx]+mx1[kdx]*my2[kdx]+mx2[kdx]*my1[kdx]+mx1[kdx]*my3[kdx]+mx3[kdx]*my1[kdx]+mx2[kdx]*my3[kdx]+mx3[kdx]*my2[kdx];
					}
				}
				//3,1,4
				else if(playerID == CHARLIE)
				{
					for(int kdx = 0; kdx < DIM; kdx++){
						myBeaverValuesShare1[DIM*i+kdx]-=r2[DIM*i+kdx];
						myBeaverValuesShare2[DIM*i+kdx]-=r2[DIM*i+kdx];	
					}
				}
				
				// for(int kdx = 0; kdx < DIM; kdx++)
				// {
				// 	myBeaverValuesShare[DIM*i + kdx] += beaverShare[DIM*i + kdx];
				// }
			}

			//
			if(playerID== ALICE){
				std::vector<unsigned char> temp = ArrayEncoder::Encode(myBeaverValuesShare1);
				assert(temp.size() > 0);
				std::vector<unsigned char> recv1(count*sizeof(uint32_t)*DIM);
				std::vector<unsigned char> recv2(count*sizeof(uint32_t)*DIM);
				std::vector<unsigned char> recv3(count*sizeof(uint32_t)*DIM);
				communicator->SendEvaluationPartner2(temp.data(),temp.size());
				communicator->AwaitEvaluationPartner1(recv2.data(),recv2.size());
				communicator->AwaitEvaluationPartner1(recv3.data(),recv3.size());
				communicator->AwaitEvaluationPartner2(recv1.data(),recv1.size());
				std::vector<uint32_t> z_13 = ArrayEncoder::UCharVec2UInt32tVec(recv1);
				std::vector<uint32_t> z_23= ArrayEncoder::UCharVec2UInt32tVec(recv2);
				std::vector<uint32_t> z_34 = ArrayEncoder::UCharVec2UInt32tVec(recv3);
				std::vector<uint32_t> z_sub_r(DIM*count);
				std::vector<uint32_t> z_d(DIM*count);
				for(int i= 0;i<count;i++){
					for(int kdx=0;kdx<DIM;kdx++){
						z_sub_r[DIM*i+kdx]=myBeaverValuesShare2[DIM*i+kdx]+z_13[DIM*i+kdx]+z_23[DIM*i+kdx]+z_34[DIM*i+kdx];
					    z_d[DIM*i+kdx]=z_sub_r[DIM*i+kdx]>>PRECISION_BIT_LENGTH;
					}
				}
				
				for(int i=0;i<count;i++){
					for(int kdx=0;kdx<DIM;kdx++){
						myBeaverValuesShare1[DIM*i+kdx]=rng_134->GetUInt32Array_1();
						myBeaverValuesShare2[DIM*i+kdx]=0;
						myBeaverValuesShare3[DIM*i+kdx]=z_d[DIM*i+kdx]-myBeaverValuesShare1[DIM*i+kdx];
					}
				}
				std::vector<unsigned char> temp1 = ArrayEncoder::Encode(myBeaverValuesShare3);
				cout<<"发送给伙伴1"<<endl;
				communicator->SendEvaluationPartner1(temp1.data(),temp1.size());
			}
			else if(playerID == BOB){
				std::vector<unsigned char> temp1 = ArrayEncoder::Encode(myBeaverValuesShare1);
				assert(temp1.size() > 0);
				std::vector<unsigned char> temp2 = ArrayEncoder::Encode(myBeaverValuesShare2);
				assert(temp2.size() > 0);
				std::vector<unsigned char> temp3 = ArrayEncoder::Encode(myBeaverValuesShare3);
				assert(temp3.size() > 0);
				std::vector<unsigned char> temp4 = ArrayEncoder::Encode(rd_2);
				assert(temp4.size() > 0);

				communicator->SendEvaluationPartner1(temp1.data(),temp1.size());   //2,3
				communicator->SendEvaluationPartner1(temp2.data(),temp2.size());   //2,4
				communicator->SendEvaluationPartner1(temp4.data(),temp4.size());   //r

				communicator->SendEvaluationPartner2(temp1.data(),temp1.size());   //2,3
				communicator->SendEvaluationPartner2(temp3.data(),temp3.size());   //3,4

				std::vector<unsigned char> recv1(count*sizeof(uint32_t)*DIM);
				communicator->AwaitEvaluationPartner2(recv1.data(),recv1.size());
				std::vector<uint32_t> zd_3 = ArrayEncoder::UCharVec2UInt32tVec(recv1);
				for(int i=0;i<count;i++){
					for(int kdx=0;kdx<DIM;kdx++){
						myBeaverValuesShare1[DIM*i+kdx]=0;
						myBeaverValuesShare2[DIM*i+kdx]=0;
						myBeaverValuesShare3[DIM*i+kdx]=zd_3[DIM*i+kdx];
					}
				}

			}else if(playerID == CHARLIE){
				std::vector<unsigned char> temp = ArrayEncoder::Encode(myBeaverValuesShare1);
				assert(temp.size() > 0);
				std::vector<unsigned char> recv1(count*sizeof(uint32_t)*DIM);
				std::vector<unsigned char> recv2(count*sizeof(uint32_t)*DIM);
				std::vector<unsigned char> recv3(count*sizeof(uint32_t)*DIM);
				std::vector<unsigned char> recv4(count*sizeof(uint32_t)*DIM);
				communicator->AwaitEvaluationPartner1(recv1.data(),recv1.size());  //1,2
				communicator->SendEvaluationPartner1(temp.data(),temp.size());

				communicator->AwaitEvaluationPartner2(recv2.data(),recv2.size());   //2,3
				communicator->AwaitEvaluationPartner2(recv3.data(),recv3.size());   //2,4
				communicator->AwaitEvaluationPartner2(recv4.data(),recv4.size());   //rd_1
				std::vector<uint32_t> z_12 = ArrayEncoder::UCharVec2UInt32tVec(recv1);
				std::vector<uint32_t> z_23= ArrayEncoder::UCharVec2UInt32tVec(recv2);
				std::vector<uint32_t> z_24 = ArrayEncoder::UCharVec2UInt32tVec(recv3);
				rd_1 = ArrayEncoder::UCharVec2UInt32tVec(recv4);
				std::vector<uint32_t> z_sub_r(count*DIM);
				std::vector<uint32_t> z_d(count*DIM);
				for(int i= 0;i<count;i++){
					for(int kdx=0;kdx<DIM;kdx++){
						z_sub_r[DIM*i+kdx]=myBeaverValuesShare2[DIM*i+kdx]+z_12[DIM*i+kdx]+z_23[DIM*i+kdx]+z_24[DIM*i+kdx];
						z_d[DIM*i+kdx]=z_sub_r[DIM*i+kdx]>>PRECISION_BIT_LENGTH;
					}
				}
				for(int i=0;i<count;i++){
					for(int kdx=0;kdx<DIM;kdx++){
						myBeaverValuesShare1[DIM*i+kdx]=0;
						myBeaverValuesShare2[DIM*i+kdx]=rng_134->GetUInt32Array_1();
						myBeaverValuesShare3[DIM*i+kdx]=z_d[DIM*i+kdx]-myBeaverValuesShare2[DIM*i+kdx];
					}
				}
			}
			else{ //Alice
			for(int i=0;i<count;i++){
				for(int kdx=0;kdx<DIM;kdx++){
					myBeaverValuesShare1[DIM*i+kdx]=rng_134->GetUInt32Array_1();
					myBeaverValuesShare2[DIM*i+kdx]=0;
					myBeaverValuesShare3[DIM*i+kdx]=0;
				}	
				}
			}

			// // Second, reconstruct mz = xy + lz and compute the truncation
			// std::vector<unsigned char> temp = ArrayEncoder::Encode(myBeaverValuesShare);
			// assert(temp.size() > 0);

			// std::vector<unsigned char> recv(DIM*count*sizeof(uint32_t));
			// if (playerID == ALICE || playerID == CHARLIE)
			// {
			// 	communicator->SendEvaluationPartner(temp.data(), temp.size());
			// 	communicator->AwaitEvaluationPartner(recv.data(), recv.size());
			// }
			// else
			// {
			// 	communicator->AwaitEvaluationPartner(recv.data(), recv.size());
			// 	communicator->SendEvaluationPartner(temp.data(), temp.size());
			// }
			
			// std::vector<uint32_t> theirBeaverValues = ArrayEncoder::UCharVec2UInt32tVec(recv);
			
			for (int idx = 0; idx < count; idx++)
			{
				NaryDotGate *g = nDotGates[idx];
				maskedEvaluation1[g->OutputWire].resize(DIM);
				maskedEvaluation2[g->OutputWire].resize(DIM);
				maskedEvaluation3[g->OutputWire].resize(DIM);
				for(int kdx = 0; kdx < DIM; kdx++)
				{
					//__uint128_t temp = 0;
					maskedEvaluation1[g->OutputWire][kdx] = myBeaverValuesShare1[DIM*idx+kdx] + rd_1[DIM*idx+kdx];
					maskedEvaluation2[g->OutputWire][kdx] = myBeaverValuesShare2[DIM*idx+kdx] + rd_2[DIM*idx+kdx];
					maskedEvaluation3[g->OutputWire][kdx] = myBeaverValuesShare3[DIM*idx+kdx] + rd_3[DIM*idx+kdx];
					// temp += maskedEvaluation[g->OutputWire][kdx];
					// temp += unTruncatedMasks[(*maskIndex)[g->OutputWire] + kdx];
					// temp = (temp >> PRECISION_BIT_LENGTH) - (masks[(*maskIndex)[g->OutputWire] + kdx]);
					// maskedEvaluation[g->OutputWire][kdx] = (uint32_t)temp;
				}
			}
			
			// std::vector<unsigned char> temp = ArrayEncoder::Encode(myBeaverValuesShare);
			// assert(temp.size() > 0);

			// std::vector<unsigned char> recv(DIM*count*sizeof(uint32_t));
			// if (playerID == ALICE || playerID == CHARLIE)
			// {
			// 	communicator->SendEvaluationPartner(temp.data(), temp.size());
			// 	communicator->AwaitEvaluationPartner(recv.data(), recv.size());
			// }
			// else
			// {
			// 	communicator->AwaitEvaluationPartner(recv.data(), recv.size());
			// 	communicator->SendEvaluationPartner(temp.data(), temp.size());
			// }
			
			// std::vector<uint32_t> theirBeaverValues = ArrayEncoder::UCharVec2UInt32tVec(recv);
			
				
			// for(int idx = 0; idx < count; idx++)
			// {
			// 	NaryDotGate *g = nDotGates[idx];
				
			// 	maskedEvaluation[g->OutputWire].resize(DIM);
				
			// 	for(int kdx = 0; kdx < DIM; kdx++)
			// 	{
			// 		maskedEvaluation[g->OutputWire][kdx] = myBeaverValuesShare[DIM*idx + kdx] + theirBeaverValues[DIM*idx + kdx];
			// 	}
			// }
		}
	}
}
