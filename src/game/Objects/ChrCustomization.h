#ifndef MANGOS_CHR_CUSTOMIZATION_H
#define MANGOS_CHR_CUSTOMIZATION_H

#include "Common.h"
#include "Server/Packets/Classic/ChrCustomizationChoice.h"
#include <unordered_map>
#include <vector>

typedef std::vector<ChrCustomizationChoice> ChrCustomizationList;

// Character look data and rules. Filled by the loader (or a test fixture), then queried; holds no
// database or Player dependencies.
class ChrCustomizationStore
{
    public:
        void AddRaceModel(uint8 race, uint8 sex, uint32 modelId);
        void AddOption(uint32 optionId, uint32 modelId, uint32 reqId);
        void AddChoice(uint32 choiceId, uint32 optionId, uint32 reqId, uint32 orderIndex);
        void AddReq(uint32 reqId, uint32 reqType, int64 raceMask, int32 classMask, uint32 achievementId, uint32 questId, uint32 itemModifiedAppearanceId);
        void AddReqChoice(uint32 reqId, uint32 choiceId);

        // Sorts the data and drops options without choices. Returns the ids of requirements the
        // server cannot evaluate (achievement, quest or item); they always read as unmet.
        std::vector<uint32> Finish();

        uint32 GetOptionCount() const;

        bool Validate(uint8 race, uint8 playerClass, uint8 sex, ChrCustomizationList const& list) const;
        // Drops invalid pairs and fills missing options with their first valid choice. Returns whether the list changed.
        bool Complete(uint8 race, uint8 playerClass, uint8 sex, ChrCustomizationList& list) const;
        void SelectRandom(uint8 race, uint8 playerClass, uint8 sex, ChrCustomizationList& list) const;
        bool MeetsReq(uint32 reqId, uint8 race, uint8 playerClass, ChrCustomizationList const* chosen) const;

    private:
        struct Option
        {
            uint32 modelId = 0;
            uint32 reqId = 0;
        };
        struct Choice
        {
            uint32 id = 0;
            uint32 optionId = 0;
            uint32 reqId = 0;
            uint32 orderIndex = 0;
        };
        struct Req
        {
            uint32 type = 0;
            int64 raceMask = 0;
            int32 classMask = 0;
            bool unsupported = false;
            std::unordered_map<uint32, std::vector<uint32>> requiredChoices; // option id -> choice ids
        };

        std::vector<uint32> const* GetOptions(uint8 race, uint8 sex) const;
        bool PairValid(uint8 race, uint8 playerClass, uint32 optionId, uint32 choiceId, ChrCustomizationList const* chosen) const;
        bool OptionMeetsReq(uint32 optionId, uint8 race, uint8 playerClass) const;

        std::unordered_map<uint32, uint32> m_raceModels;                       // (race << 8 | sex) -> model
        std::unordered_map<uint32, Option> m_options;
        std::unordered_map<uint32, Choice> m_choices;
        std::unordered_map<uint32, std::vector<uint32>> m_optionsByModel;      // sorted by option id
        std::unordered_map<uint32, std::vector<uint32>> m_choicesByOption;     // sorted by (order, id)
        std::unordered_map<uint32, Req> m_reqs;
        std::vector<std::pair<uint32, uint32>> m_pendingReqChoices;
};

#endif
