#ifndef MANGOS_CLASSIC_CHR_CUSTOMIZATION_CHOICE_H
#define MANGOS_CLASSIC_CHR_CUSTOMIZATION_CHOICE_H

#include "Common.h"

// One (option, choice) pair of a character's look, the client's own encoding. A list of them is
// always sorted by OptionID with each option at most once.
struct ChrCustomizationChoice
{
    uint32 OptionID = 0;
    uint32 ChoiceID = 0;

    bool operator<(ChrCustomizationChoice const& right) const { return OptionID < right.OptionID; }
    bool operator==(ChrCustomizationChoice const& right) const { return OptionID == right.OptionID && ChoiceID == right.ChoiceID; }
    bool operator!=(ChrCustomizationChoice const& right) const { return !(*this == right); }
};

#endif
