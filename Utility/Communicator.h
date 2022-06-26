#ifndef COMMUNICATOR_H__
#define COMMUNICATOR_H__

#pragma once

#include "../Network.hpp"
#include <vector>

namespace Utility
{
	class Communicator
	{
	public:
		uint64_t sendingSize;
		uint64_t receivingSize;
		//用处还不清楚
		const bool isAlice;
		uint64_t machineId = -1;
        //这里用数字表示通信
		static constexpr uint64_t PARTNER1= 0;
		static constexpr uint64_t PARTNER2 = 1;
		static constexpr uint64_t ALICE = 2;
		static constexpr uint64_t BOB = 3;
		static constexpr uint64_t CHARLIE = 4;
		
		std::vector<Network *> const inputChannels;
		std::vector<Network *> const outputChannels;

		/// <summary>
		/// Order of input channels is Partner1，Partner2, Alice, Bob，Charli.
		/// </summary>
		/// <param name="isAlice"></param>
		/// <param name="inputChannels"></param>
		/// <param name="outputChannels"></param>
		
		Communicator(bool isAlice, std::vector<Network *> &inputChannels, std::vector<Network *> &outputChannels);

		void SendMessage(uint64_t player, unsigned char *msg, uint64_t size);

		uint64_t AwaitMessage(uint64_t player, unsigned char *msg, uint64_t bufferSize);
        void SendEvaluationHelper(unsigned char *msg, uint64_t size,bool direction);

		void SendEvaluationPartner1(unsigned char *msg, uint64_t size);
		
		void AwaitEvaluationPartner1(unsigned char *msg, uint64_t size);

		void SendEvaluationPartner2(unsigned char *msg, uint64_t size);
	
	    void AwaitEvaluationPartner2(unsigned char *msg, uint64_t size);

		void SendEvaluationPartner(unsigned char *msg, uint64_t size);
		
		void AwaitEvaluationPartner(unsigned char *msg, uint64_t size);

		void SendAlice(unsigned char *msg, uint64_t size);
		
		void Transfer(unsigned char *msg, uint64_t size);

		uint64_t AwaitAlice(unsigned char *msg, uint64_t size);

		void SendBob(unsigned char *msg, uint64_t size);
		
		uint64_t AwaitBob(unsigned char *msg, uint64_t size);
		uint64_t AwaitCharlie(unsigned char *msg, uint64_t size);

		void SendVerificationPartner(unsigned char *msg, uint64_t size);

		void SendOtherGroupPartner(int party,unsigned char *msg,uint64_t size);
		void  AwaitOtherGroupPartner(int party,unsigned char *msg,uint64_t size);
	    
		uint64_t AwaitVerificationPartner(unsigned char *msg, uint64_t size);

		void SendNonPartner(unsigned char *msg, uint64_t size);
		
		uint64_t AwaitNonPartner(unsigned char *msg, uint64_t size);
		

		void SendToAll(unsigned char *msg, uint64_t size);
		//与自己相对的Alice=2通信
		void SendEvaluationAlice( unsigned char *msg, uint64_t size);
		void AwaitEvaluationAlice(unsigned char *msg, uint64_t size);
		//与自己相对的Bob=3通信
		void SendEvaluationBob( unsigned char *msg, uint64_t size);
		void AwaitEvaluationBob(unsigned char *msg, uint64_t size);
		//与自己相对的Bob=4通信
		void SendEvaluationCharlie( unsigned char *msg, uint64_t size);
		void AwaitEvaluationCharlie(unsigned char *msg, uint64_t size);
		
		
		void SendToPlayers(unsigned char *m1, unsigned char *m2, unsigned char *m3, uint64_t size)
		{
			SendVerificationPartner(m1, size);
			SendEvaluationPartner(m3, size);
			SendNonPartner(m2, size);
		}
		
		void AwaitFromPlayers(unsigned char *m1, unsigned char *m2, unsigned char *m3, uint64_t size)
		{
			AwaitNonPartner(m1, size);
			AwaitVerificationPartner(m2, size);
			AwaitEvaluationPartner(m3, size);
		}
	};
}

#endif
