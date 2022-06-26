#include <vector>
#include "PreprocessingShare.h"
#include "../Utility/Range.h"

using namespace Utility;

namespace TwoPartyMaskedEvaluation
{

	PreprocessingShare::PreprocessingShare(std::vector<uint32_t>&& masks, std::vector<std::vector<uint32_t> >&& beaverSharesPerLayer) : maskShare(std::move(masks))
	{
		this->beaverShare = std::move(beaverSharesPerLayer);
	}
	
	PreprocessingShare::PreprocessingShare(std::vector<uint32_t>&& masks, std::vector<std::vector<uint32_t> >&& beaverSharesPerLayer, const std::vector<int>& maskIndex) : maskShare(std::move(masks))
	{
		this->beaverShare = std::move(beaverSharesPerLayer);
		this->maskIndex = maskIndex;
	}
	
	PreprocessingShare::PreprocessingShare(std::vector<uint32_t>&& masks, std::vector<std::vector<uint32_t> >&& beaverSharesPerLayer, std::vector<int>&& maskIndex) : maskShare(std::move(masks)), beaverShare(std::move(beaverSharesPerLayer)), maskIndex(std::move(maskIndex))
	{
	}

	uint32_t PreprocessingShare::operator[](int wire)
	{
		return maskShare[wire];
	}
	
	uint32_t PreprocessingShare::getMask(int index)
	{
		return maskShare[index];
	}

	std::vector<uint32_t>  PreprocessingShare::operator[](Range *range)
	{
		std::vector<uint32_t> ret(maskShare.begin() + range->Start, maskShare.begin() + range->Start + range->Length);
		
		return ret;
	}

	std::vector<uint32_t> PreprocessingShare::GetNextBeaverTriples()
	{
		counter++;
		return beaverShare[counter];
	}
}
