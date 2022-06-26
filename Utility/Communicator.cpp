#include <cassert>
#include "Communicator.h"
#define MAX_PACKAGE_SIZE 1073741824

namespace Utility
{
	Communicator::Communicator(bool isAlice, std::vector<Network *> &inputChannels, std::vector<Network *> &outputChannels) : isAlice(isAlice), inputChannels(inputChannels), outputChannels(outputChannels){
		sendingSize = 0;
		receivingSize = 0;
	}
//消息发送函数，第一个参数表示在输入通道的第几个人
	void Communicator::SendMessage(uint64_t player, unsigned char *msg, uint64_t size)
	{
		sendingSize += size;
		
		if(size > 1024*1024) std::cout << machineId << ": sending " << size << " bytes" << std::endl;
		
		uint64_t count = 0;
		uint64_t bufferSize;
		
		while(count < size)
		{
			if (count + MAX_PACKAGE_SIZE < size)
			{
				bufferSize = MAX_PACKAGE_SIZE;
			}
			else
			{
				bufferSize = size - count;
			}
			
			uint64_t bytes_sent = 0;

			while(bytes_sent < bufferSize)
			{
				uint64_t bs = send(inputChannels[player]->sock_fd, msg + count, bufferSize - bytes_sent, 0);

				if(bs < 0)
				{
					perror("error: send");
					exit(1);
				}

				bytes_sent += bs;
				count += bs;
			}
		}
		cout<<count<<"**************"<<size<<endl;
		assert(count == size);
	}
//消息获得函数，第一个参数表示在输出通道的第几个人
	uint64_t Communicator::AwaitMessage(uint64_t player, unsigned char *msg, uint64_t size)
	{
		receivingSize += size;
		if(size > 1024*1024) std::cout << machineId << ": awaiting " << size << " bytes" << std::endl;
		
		
		uint64_t sd = outputChannels[player]->sock_fd;
		fd_set reading_set;
		
		while (true)
		{
			FD_ZERO(&reading_set); //clear the socket set 
			FD_SET(sd, &reading_set); //add master socket to set
			uint64_t activity = select(sd+1, &reading_set , NULL , NULL , NULL);   //&timeout
			if ((activity < 0) && (errno!=EINTR))  { perror("select failed"); exit(1); }
			if (activity > 0)
			{
				if (FD_ISSET(sd, &reading_set)){
					uint64_t count = 0;
					uint64_t bufferSize;
					uint64_t bytes_received = 0;
					
					while(count < size)
					{
						if (count + MAX_PACKAGE_SIZE < size)
						{
							bufferSize = MAX_PACKAGE_SIZE;
						}
						else
						{
							bufferSize = size - count;
						}
						
						uint64_t arrived = 0;

						while (arrived < bufferSize){
							uint64_t br = recv(sd, msg + count, bufferSize - arrived, 0);
							arrived += br;
							count += br;
						}

						bytes_received += arrived;

						// count += MAX_PACKAGE_SIZE;
					}
					cout<<bytes_received<<"**************"<<size<<endl;
					assert(bytes_received == size);

					return bytes_received;
				}
				
			}
			else if (activity == 0){
				return 0;
			}
		}
	}
	void Communicator::SendEvaluationHelper(unsigned char *msg, uint64_t size,bool direction)
	{
		SendMessage(ALICE, msg, size);
	}
	
	void Communicator::SendEvaluationPartner1(unsigned char *msg, uint64_t size)
	{
		SendMessage(PARTNER1, msg, size);
	}
	
	void Communicator::AwaitEvaluationPartner1(unsigned char *msg, uint64_t size)
	{
		AwaitMessage(PARTNER1, msg, size);
	}
	void Communicator::SendEvaluationPartner2(unsigned char *msg, uint64_t size)
	{
		SendMessage(PARTNER2, msg, size);
	}
	
	void Communicator::AwaitEvaluationPartner2(unsigned char *msg, uint64_t size)
	{
		AwaitMessage(PARTNER2, msg, size);
	}
	void Communicator::SendEvaluationAlice( unsigned char *msg, uint64_t size){
		SendMessage(ALICE, msg, size);

	}
	void Communicator::AwaitEvaluationAlice(unsigned char *msg, uint64_t size)
	{
		cout<<"**************等待数据***************"<<endl;
		AwaitMessage(ALICE, msg, size);
	}

	void Communicator::SendEvaluationBob( unsigned char *msg, uint64_t size){
		SendMessage(BOB, msg, size);

	}
	void Communicator::AwaitEvaluationBob(unsigned char *msg, uint64_t size)
	{
		AwaitMessage(BOB, msg, size);
	}

	void Communicator::SendEvaluationCharlie( unsigned char *msg, uint64_t size){
		SendMessage(CHARLIE, msg, size);

	}
	void Communicator::AwaitEvaluationCharlie(unsigned char *msg, uint64_t size)
	{
		AwaitMessage(CHARLIE, msg, size);
	}


	// void Communicator::SendEvaluationPartner(unsigned char *msg, uint64_t size)
	// {
	// 	SendMessage(PARTNER, msg, size);
	// }
	
	// void Communicator::AwaitEvaluationPartner(unsigned char *msg, uint64_t size)
	// {
	// 	AwaitMessage(PARTNER, msg, size);
	// }

	void Communicator::SendAlice(unsigned char *msg, uint64_t size)
	{
		SendMessage(ALICE, msg, size);
	}
	
	// void Communicator::Transfer(unsigned char *msg, uint64_t size)
	// {
	// 	SendAlice(msg, size);
	// 	SendBob(msg, size);
	// }

	uint64_t Communicator::AwaitAlice(unsigned char *msg, uint64_t size)
	{
		return AwaitMessage(ALICE, msg, size);
	}

	void Communicator::SendBob(unsigned char *msg, uint64_t size)
	{
		SendMessage(BOB, msg, size);
	}
	
	uint64_t Communicator::AwaitBob(unsigned char *msg, uint64_t size)
	{
		return AwaitMessage(BOB, msg, size);
	}

	uint64_t Communicator::AwaitCharlie(unsigned char *msg, uint64_t size)
	{
		return AwaitMessage(CHARLIE, msg, size);
	}

	// void Communicator::SendVerificationPartner(unsigned char *msg, uint64_t size)
	// {
	// 	if (isAlice)
	// 	{
	// 		SendAlice(msg, size);
	// 	}
	// 	else
	// 	{
	// 		SendBob(msg, size);
	// 	}
	// }
	void Communicator::SendOtherGroupPartner(int party,unsigned char *msg,uint64_t size)
	{
		int group_party = party%3;  
		//
		if (group_party==0)
		{
			SendEvaluationAlice(msg, size);
		}
		else if(group_party==1)
		{
			SendEvaluationBob(msg, size);
		}
		else{
			SendEvaluationCharlie(msg,size);
		}
	}

	void  Communicator::AwaitOtherGroupPartner(int party,unsigned char *msg,uint64_t size)
	{
		int group_party = party%3;  
		//
		if (group_party==0)
		{
			AwaitEvaluationAlice(msg, size);
		}
		else if(group_party==1)
		{
			AwaitEvaluationBob(msg, size);
		}
		else{
			AwaitEvaluationCharlie(msg,size);
		}
	}
     
	// uint64_t Communicator::AwaitVerificationPartner(int party, unsigned char *msg, uint64_t size)
	// {
	// 	if (isAlice)
	// 	{
	// 		return AwaitAlice(msg, size);
	// 	}
	// 	else
	// 	{
	// 		return AwaitBob(msg, size);
	// 	}
	//}

	// void Communicator::SendNonPartner(unsigned char *msg, uint64_t size)
	// {
	// 	if (isAlice)
	// 	{
	// 		SendBob(msg, size);
	// 	}
	// 	else
	// 	{
	// 		SendAlice(msg, size);
	// 	}
	// }
	
	// uint64_t Communicator::AwaitNonPartner(unsigned char *msg, uint64_t size)
	// {
	// 	if (isAlice)
	// 	{
	// 		return AwaitBob(msg, size);
	// 	}
	// 	else
	// 	{
	// 		return AwaitAlice(msg, size);
	// 	}
	// }

	// void Communicator::SendToAll(unsigned char *msg, uint64_t size)
	// {
	// 	SendEvaluationPartner(msg, size);
	// 	SendAlice(msg, size);
	// 	SendBob(msg, size);
	// }
}
