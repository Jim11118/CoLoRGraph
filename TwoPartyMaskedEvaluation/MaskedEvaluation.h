#ifndef MASKED_EVALUATION_H
#define MASKED_EVALUATION_H

#pragma once

#include <vector>
#include <stdint.h>
#include <stdexcept>
#include "MaskedEvaluation.h"
#include "PreprocessingShare.h"
#include "MaskedEvaluationException.h"

#include "../Circuit/LayeredArithmeticCircuit.h"
#include "../Circuit/ArithmeticGate.h"
#include "../Utility/Communicator.h"
#include "../Utility/ISecureRNG.h"

using namespace Circuit;
using namespace Utility;

#define ALICE   0
#define BOB     1
#define CHARLIE 2
#define DAVID   3

namespace TwoPartyMaskedEvaluation
{
	class MaskedEvaluation
	{
	public:
		Communicator *communicator;
		LayeredArithmeticCircuit *lc;
		AESRNG *rng_124;
		AESRNG *rng_234;
		AESRNG *rng_134;
		
		// Which player is running the evaluation
		//哪一个参与者在运行计算
		int playerID;
		
		// share holds <lx> <ly> <lx*ly>
		PreprocessingShare *share;
		
		// masks hold lx'
		std::vector<uint32_t> masks; 
		std::vector<uint32_t> unTruncatedMasks;
		
		std::vector<std::vector<uint32_t>> maskedEvaluation1;
		std::vector<std::vector<uint32_t>> maskedEvaluation2;
		std::vector<std::vector<uint32_t>> maskedEvaluation3;
		
		std::vector<int> *maskIndex;
		
		bool inputAdded = false;
		bool evaluated = false;


		virtual ~MaskedEvaluation()
		{
			delete communicator;
			delete lc;
			delete share;
// 			delete maskedEvaluation;
		}
		MaskedEvaluation(LayeredArithmeticCircuit *lc, Communicator *communicator,AESRNG *rng_124,AESRNG *rng_234,AESRNG *rng_134);
        MaskedEvaluation(LayeredArithmeticCircuit *lc, PreprocessingShare *share, Communicator *communicator,AESRNG *rng_124,AESRNG *rng_234,AESRNG *rng_134);
		MaskedEvaluation(LayeredArithmeticCircuit *lc, PreprocessingShare *share, Communicator *communicator);
		MaskedEvaluation(LayeredArithmeticCircuit *lc, PreprocessingShare *share, std::vector<int> *maskIndex, Communicator *communicator);

		void AddInput(const std::vector<std::vector<uint32_t>>& input1,const std::vector<std::vector<uint32_t> >& input2,const std::vector<std::vector<uint32_t> >& input3);
		std::vector<std::vector<uint32_t>> getout1();
		std::vector<std::vector<uint32_t>> getout2();
		std::vector<std::vector<uint32_t>> getout3();

		std::vector<std::vector<uint32_t>> Decrypt(std::vector<uint32_t> mask, Range *range);

		/// <summary>
		/// Computes the masked evaluation of a circuit on a layer per layer basis. 
		/// </summary>
		void EvaluateCircuit();

	private:
		void EvaluateAddGate(int layer);

		void EvaluateSubGate(int layer);
		
		void EvaluateCAddGate(int layer);
		
		void EvaluateCMulGate(int layer);
		
		void EvaluateNAddGate(int layer);

		void EvaluateMulGate(int layer);
		
		void EvaluateDotGate(int layer);
		
		void EvaluateNDotGate(int layer);

	private:
// 		uint32_t BeaverEvaluation(MultiplicationGate *ag, uint32_t beaver, PreprocessingShare *share);

	};
}

#endif
