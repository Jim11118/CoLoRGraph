#ifndef COMMUNICATOR_BUILDER_H__
#define COMMUNICATOR_BUILDER_H__

#pragma once

#include "../Network.hpp"
#include "Communicator.h"

namespace Utility
{
	class CommunicatorBuilder
	{
	public:
		static void BuildCommunicator(int party, Machine *machine, Communicator * &c)
		{
			//主要修改部分,需要和五个人构建网络连接
			Network *cPartner1,*cPartner2, *cAlice, *cBob, *cCharlie;
			
			//这里 设置是否是Alice，在本框架中可能没用
			bool isAlice = true;
			//这里根据每人的通信id，存储通信信道。
			
			if(Alice == party)
			{
				cPartner1= machine->partiesDown[Bob - Alice - 1];   //BOb  0
				cPartner2 = machine->partiesDown[Charlie-Alice-1];   //Charlie   1
				cAlice   = machine->partiesDown[ 2];    //   David  2
				cBob     = machine->partiesDown[3];   //   Eric   3
				cCharlie = machine->partiesDown[4];   //  Frank   4

			  
				std::vector<Network *> input = {cPartner1,cPartner2, cAlice, cBob,cCharlie};
				std::vector<Network *> output =  {cPartner1,cPartner2, cAlice, cBob,cCharlie};
				
				c = new Communicator(isAlice, input, output);
			}
			else if(Bob == party)
			{
				cPartner1 = machine->partiesDown[0];                         //Charlie
				cPartner2 = machine->partiesUp[0];          //Alice
				cAlice   = machine->partiesDown[ 1];    //   David 
				cBob     = machine->partiesDown[ 2];   //   Eric
				cCharlie = machine->partiesDown[3];   //  Frank
			  
				std::vector<Network *> input = {cPartner1,cPartner2, cAlice, cBob,cCharlie};
				std::vector<Network *> output =  {cPartner1,cPartner2, cAlice, cBob,cCharlie};
				
				
				c = new Communicator(!isAlice, input, output);
			}
			else if(Charlie == party)
			{
				cPartner1 = machine->partiesUp[0];             //Alice          
				cPartner2 = machine->partiesUp[1];          //Bob
				cAlice   = machine->partiesDown[ 0];    //   David 
				cBob     = machine->partiesDown[ 1];   //   Eric
				cCharlie = machine->partiesDown[2];   //  Frank
			  
				std::vector<Network *> input = {cPartner1,cPartner2, cAlice, cBob,cCharlie};
				std::vector<Network *> output =  {cPartner1,cPartner2, cAlice, cBob,cCharlie};
				
				
				c = new Communicator(isAlice, input, output);
			}
			else if(David == party)
			{
				cPartner1 = machine->partiesDown[0];        //   Eric            
				cPartner2 = machine->partiesDown[1];      //  Frank    
				cAlice   = machine->partiesUp[ 0];  //Alice      
				cBob     = machine->partiesUp[ 1];  //Bob
				cCharlie = machine->partiesUp[2];   //Charlie
			  
				std::vector<Network *> input = {cPartner1,cPartner2, cAlice, cBob,cCharlie};
				std::vector<Network *> output =  {cPartner1,cPartner2, cAlice, cBob,cCharlie};
				
				
				c = new Communicator(!isAlice, input, output);
			}
			else if(Eric == party)
			{
				cPartner1 = machine->partiesDown[0];        //   Frank    
				cPartner2 = machine->partiesUp[3];      //  David
				cAlice   = machine->partiesUp[ 0];  //Alice      
				cBob     = machine->partiesUp[ 1];  //Bob
				cCharlie = machine->partiesUp[2];   //Charlie
			  
				std::vector<Network *> input = {cPartner1,cPartner2, cAlice, cBob,cCharlie};
				std::vector<Network *> output =  {cPartner1,cPartner2, cAlice, cBob,cCharlie};
				
				
				c = new Communicator(!isAlice, input, output);
				

			}
			else if(Frank == party)
			{
				cPartner1 = machine->partiesUp[3];        //   Eric            
				cPartner2 = machine->partiesUp[4];      //  Frank    
				cAlice   = machine->partiesUp[ 0];  //Alice      
				cBob     = machine->partiesUp[ 1];  //Bob
				cCharlie = machine->partiesUp[2];   //Charlie
			  
				std::vector<Network *> input = {cPartner1,cPartner2, cAlice, cBob,cCharlie};
				std::vector<Network *> output =  {cPartner1,cPartner2, cAlice, cBob,cCharlie};
				c = new Communicator(!isAlice, input, output);

			}
		}
	};
}

#endif

