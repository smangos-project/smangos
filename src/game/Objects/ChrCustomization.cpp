#include "ChrCustomization.h"
#include "Utilities/Random.h"
#include <algorithm>

static uint32 RaceSexKey(uint8 race, uint8 sex)
{
    return (uint32(race) << 8) | sex;
}

void ChrCustomizationStore::AddRaceModel(uint8 race, uint8 sex, uint32 modelId)
{
    m_raceModels[RaceSexKey(race, sex)] = modelId;
}

void ChrCustomizationStore::AddOption(uint32 optionId, uint32 modelId, uint32 reqId)
{
    Option& option = m_options[optionId];
    option.modelId = modelId;
    option.reqId = reqId;
    m_optionsByModel[modelId].push_back(optionId);
}

void ChrCustomizationStore::AddChoice(uint32 choiceId, uint32 optionId, uint32 reqId, uint32 orderIndex)
{
    m_choices[choiceId] = Choice{ choiceId, optionId, reqId, orderIndex };
    m_choicesByOption[optionId].push_back(choiceId);
}

void ChrCustomizationStore::AddReq(uint32 reqId, uint32 reqType, int64 raceMask, int32 classMask, uint32 achievementId, uint32 questId, uint32 itemModifiedAppearanceId)
{
    Req& req = m_reqs[reqId];
    req.type = reqType;
    req.raceMask = raceMask;
    req.classMask = classMask;
    req.unsupported = achievementId || questId || itemModifiedAppearanceId;
}

void ChrCustomizationStore::AddReqChoice(uint32 reqId, uint32 choiceId)
{
    m_pendingReqChoices.emplace_back(reqId, choiceId);
}

std::vector<uint32> ChrCustomizationStore::Finish()
{
    for (auto const& pending : m_pendingReqChoices)
    {
        auto reqItr = m_reqs.find(pending.first);
        auto choiceItr = m_choices.find(pending.second);
        if (reqItr != m_reqs.end() && choiceItr != m_choices.end())
            reqItr->second.requiredChoices[choiceItr->second.optionId].push_back(pending.second);
    }
    m_pendingReqChoices.clear();

    for (auto& entry : m_choicesByOption)
    {
        std::sort(entry.second.begin(), entry.second.end(), [this](uint32 a, uint32 b)
        {
            Choice const& ca = m_choices.at(a);
            Choice const& cb = m_choices.at(b);
            return ca.orderIndex != cb.orderIndex ? ca.orderIndex < cb.orderIndex : ca.id < cb.id;
        });
    }

    for (auto& entry : m_optionsByModel)
    {
        std::vector<uint32>& options = entry.second;
        options.erase(std::remove_if(options.begin(), options.end(), [this](uint32 optionId)
        {
            return m_choicesByOption.find(optionId) == m_choicesByOption.end();
        }), options.end());
        std::sort(options.begin(), options.end());
    }

    std::vector<uint32> unsupported;
    for (auto const& entry : m_reqs)
    {
        if (entry.second.unsupported)
            unsupported.push_back(entry.first);
    }
    std::sort(unsupported.begin(), unsupported.end());
    return unsupported;
}

uint32 ChrCustomizationStore::GetOptionCount() const
{
    uint32 count = 0;
    for (auto const& entry : m_optionsByModel)
        count += uint32(entry.second.size());
    return count;
}

std::vector<uint32> const* ChrCustomizationStore::GetOptions(uint8 race, uint8 sex) const
{
    auto model = m_raceModels.find(RaceSexKey(race, sex));
    if (model == m_raceModels.end())
        return nullptr;
    auto options = m_optionsByModel.find(model->second);
    if (options == m_optionsByModel.end() || options->second.empty())
        return nullptr;
    return &options->second;
}

bool ChrCustomizationStore::MeetsReq(uint32 reqId, uint8 race, uint8 playerClass, ChrCustomizationList const* chosen) const
{
    if (!reqId)
        return true;
    auto itr = m_reqs.find(reqId);
    if (itr == m_reqs.end())
        return true;

    Req const& req = itr->second;
    if (!(req.type & 0x1))
        return true;
    if (req.unsupported)
        return false;
    if (req.raceMask != 0 && req.raceMask != -1 && !(req.raceMask & (int64(1) << (race - 1))))
        return false;
    if (req.classMask != 0 && !(req.classMask & (1 << (playerClass - 1))))
        return false;

    if (chosen)
    {
        for (auto const& required : req.requiredChoices)
        {
            uint32 optionId = required.first;
            auto pair = std::find_if(chosen->begin(), chosen->end(), [optionId](ChrCustomizationChoice const& c) { return c.OptionID == optionId; });
            if (pair == chosen->end() || std::find(required.second.begin(), required.second.end(), pair->ChoiceID) == required.second.end())
                return false;
        }
    }
    return true;
}

bool ChrCustomizationStore::OptionMeetsReq(uint32 optionId, uint8 race, uint8 playerClass) const
{
    auto itr = m_options.find(optionId);
    return itr != m_options.end() && MeetsReq(itr->second.reqId, race, playerClass, nullptr);
}

bool ChrCustomizationStore::PairValid(uint8 race, uint8 playerClass, uint32 optionId, uint32 choiceId, ChrCustomizationList const* chosen) const
{
    if (!OptionMeetsReq(optionId, race, playerClass))
        return false;
    auto choice = m_choices.find(choiceId);
    if (choice == m_choices.end() || choice->second.optionId != optionId)
        return false;
    return MeetsReq(choice->second.reqId, race, playerClass, chosen);
}

bool ChrCustomizationStore::IsNpcOnly(uint32 choiceId) const
{
    uint32 reqId = m_choices.at(choiceId).reqId;
    if (!reqId)
        return false;
    auto itr = m_reqs.find(reqId);
    return itr != m_reqs.end() && !(itr->second.type & 0x1);
}

bool ChrCustomizationStore::Validate(uint8 race, uint8 playerClass, uint8 sex, ChrCustomizationList const& list) const
{
    std::vector<uint32> const* options = GetOptions(race, sex);
    if (!options)
        return false;

    for (size_t i = 0; i < list.size(); ++i)
    {
        if (i > 0 && list[i - 1].OptionID >= list[i].OptionID)
            return false;
        if (!std::binary_search(options->begin(), options->end(), list[i].OptionID))
            return false;
        if (!PairValid(race, playerClass, list[i].OptionID, list[i].ChoiceID, &list))
            return false;
    }
    return true;
}

bool ChrCustomizationStore::Complete(uint8 race, uint8 playerClass, uint8 sex, ChrCustomizationList& list) const
{
    std::vector<uint32> const* options = GetOptions(race, sex);
    ChrCustomizationList original = list;
    ChrCustomizationList result;

    if (options)
    {
        ChrCustomizationList sorted = original;
        std::stable_sort(sorted.begin(), sorted.end());
        for (ChrCustomizationChoice const& pair : sorted)
        {
            if (!result.empty() && result.back().OptionID == pair.OptionID)
                continue;
            if (std::binary_search(options->begin(), options->end(), pair.OptionID)
                && PairValid(race, playerClass, pair.OptionID, pair.ChoiceID, &original))
                result.push_back(pair);
        }

        for (uint32 optionId : *options)
        {
            if (std::any_of(result.begin(), result.end(), [optionId](ChrCustomizationChoice const& c) { return c.OptionID == optionId; }))
                continue;
            if (!OptionMeetsReq(optionId, race, playerClass))
                continue;
            for (uint32 choiceId : m_choicesByOption.at(optionId))
            {
                if (!IsNpcOnly(choiceId) && PairValid(race, playerClass, optionId, choiceId, &result))
                {
                    result.push_back({ optionId, choiceId });
                    break;
                }
            }
        }
        std::sort(result.begin(), result.end());
    }

    bool changed = result != original;
    list = std::move(result);
    return changed;
}

void ChrCustomizationStore::SelectRandom(uint8 race, uint8 playerClass, uint8 sex, ChrCustomizationList& list) const
{
    list.clear();
    std::vector<uint32> const* options = GetOptions(race, sex);
    if (!options)
        return;

    for (uint32 optionId : *options)
    {
        if (!OptionMeetsReq(optionId, race, playerClass))
            continue;
        std::vector<uint32> candidates;
        for (uint32 choiceId : m_choicesByOption.at(optionId))
        {
            if (!IsNpcOnly(choiceId) && MeetsReq(m_choices.at(choiceId).reqId, race, playerClass, &list))
                candidates.push_back(choiceId);
        }
        if (!candidates.empty())
            list.push_back({ optionId, candidates[urand(0, uint32(candidates.size() - 1))] });
    }
}
