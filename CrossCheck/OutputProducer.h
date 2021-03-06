#pragma once
#include <stdint.h>
#include <vector>
#include "../Utility/Communicator.h"
#include "../Utility/Range.h"

using namespace Utility;

namespace CrossCheck
{
	class OutputProducer
	{
	public:
		std::vector<std::vector<uint32_t> > maskedEvaluation;
		std::vector<uint32_t> mask;
		Communicator *communicator;

		virtual ~OutputProducer()
		{
			delete communicator;
		}

		OutputProducer(Communicator *communicator, std::vector<std::vector<uint32_t> >& maskedEvaluation, std::vector<uint32_t> &mask);

		std::vector<std::vector<uint32_t> >  ComputeOutput(Range *range);
	};
}
