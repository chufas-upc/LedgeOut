#include "Framework/SelectionIndexUtils.h"

int32 WrapSelectionIndex(const int32 CurrentIndex, const int32 Direction, const int32 OptionCount)
{
	const int32 NormalizedIndex = ((CurrentIndex % OptionCount) + OptionCount) % OptionCount;
	return (NormalizedIndex + Direction + OptionCount) % OptionCount;
}
