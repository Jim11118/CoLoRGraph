#ifndef SIMPLE_PREPROCESSING_SHARE_H
#define SIMPLE_PREPROCESSING_SHARE_H

#pragma once

#include <stdexcept>

using namespace Utility;

namespace TwoPartyMaskedEvaluation
{
	class NoTripleAtLayerException : public std::exception
	{
	};

	class SimplePreprocessingShare
	{
	private:
		std::vector<uint32_t> MaskShare;
		std::vector<uint32_t> BeaverShare;

	public:
		virtual ~SimplePreprocessingShare()
		{
			delete [] MaskShare;
			delete [] BeaverShare;
		}

		SimplePreprocessingShare(std::vector<uint32_t> masks, std::vector<uint32_t> beaverShare);

		std::vector<uint32_t> GetMaskShare(Range *range);

		uint32_t operator [](int wire);

		uint32_t GetBeaverShare(int wire);
	};
}

#endif
