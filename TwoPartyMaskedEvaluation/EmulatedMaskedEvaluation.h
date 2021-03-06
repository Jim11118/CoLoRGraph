#ifndef EMULATED_MASKED_EVALUATION_H
#define EMULATED_MASKED_EVALUATION_H

#pragma once

#include <stdint.h>
#include <stdexcept>
#include <vector>
#include "PreprocessingShare.h"
#include "../Circuit/LayeredArithmeticCircuit.h"

using namespace Circuit;
using namespace Utility;

namespace TwoPartyMaskedEvaluation
{
	class EmulatedMaskedEvaluation
	{
	public:
		PreprocessingShare *aliceShare;
		PreprocessingShare *bobShare;
		LayeredArithmeticCircuit *lc;
		
		std::vector<std::vector<uint32_t> > maskedEvaluation;
		std::vector<int> *maskIndex;
		
		bool inputAdded = false;
		bool evaluated = false;

		virtual ~EmulatedMaskedEvaluation()
		{
			delete aliceShare;
			delete bobShare;
			delete lc;
// 			delete [] maskedEvaluation;
		}

		EmulatedMaskedEvaluation(PreprocessingShare *aliceShare, PreprocessingShare *bobShare, LayeredArithmeticCircuit *lc);
		EmulatedMaskedEvaluation(PreprocessingShare *aliceShare, PreprocessingShare *bobShare, std::vector<int> *maskIndex, LayeredArithmeticCircuit *lc);

// 		void AddInput(std::vector<uint32_t> input, Range *range);
		void AddInput(std::vector<std::vector<uint32_t> >&& input, Range *range);
		
// 		std::vector<uint32_t> Decrypt(std::vector<uint32_t> mask, Range *range);
		std::vector<std::vector<uint32_t> > Decrypt(std::vector<uint32_t> mask, Range *range);

		void EvaluateCircuit();

	private:
		void EvaluateAddGate(int layer);

		void EvaluateSubGate(int layer);
		
		void EvaluateCAddGate(int layer);
		
		void EvaluateCMulGate(int layer);

		void EvaluateNAddGate(int layer);
		
		void EvaluateMulGate(int layer, PreprocessingShare *aliceBeaver, PreprocessingShare *bobBeaver);
		
		void EvaluateDotGate(int layer, PreprocessingShare *aliceBeaver, PreprocessingShare *bobBeaver);

		uint32_t BeaverEvaluation(MultiplicationGate *g, uint32_t beaver, uint32_t ma, uint32_t mb, PreprocessingShare *share);
	};
}

#endif
