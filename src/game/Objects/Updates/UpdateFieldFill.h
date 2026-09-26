#ifndef UPDATEFIELDFILL_H
#define UPDATEFIELDFILL_H

#include "UpdateField.h"
#include <utility>

class Object;
class Unit;
class Player;
class Item;
class Bag;
class GameObject;
class Corpse;
class DynamicObject;

// Copies one leaf field to `value`, touching the change mask (via the ModifyValue chain `getRef`
// lazily builds) only when the value actually differs from `current`'s -- ModifyValue marks its
// bit unconditionally, so the compare has to happen first, against a plain read that doesn't call
// it. `getRef` is called at most once, and only on a real change.
template<typename StructT, typename GetRef, typename FieldT, int32 FBlockBit, uint32 FBit>
inline void FillField(StructT const& current, GetRef&& getRef, UpdateField<FieldT, FBlockBit, FBit> StructT::* leafField, FieldT value)
{
    if (FieldT(current.*leafField) != value)
        SetUpdateFieldValue(getRef().ModifyValue(leafField), std::move(value));
}

// Same, for a plain (non-update-field) member of a structure that itself carries no change mask
// (e.g. UFUnitChannel): the containing UpdateField's own bit is what tracks this leaf.
template<typename StructT, typename GetRef, typename FieldT>
inline void FillField(StructT const& current, GetRef&& getRef, FieldT StructT::* leafField, FieldT value)
{
    if (current.*leafField != value)
        SetUpdateFieldValue(getRef().ModifyValue(leafField), std::move(value));
}

// Same, for an UpdateFieldArray leaf slot.
template<typename StructT, typename GetRef, typename FieldT, std::size_t Size, uint32 FBit, int32 FirstElementBit>
inline void FillField(StructT const& current, GetRef&& getRef, UpdateFieldArray<FieldT, Size, FBit, FirstElementBit> StructT::* leafField, uint32 index, FieldT value)
{
    if (FieldT((current.*leafField)[index]) != value)
        SetUpdateFieldValue(getRef().ModifyValue(leafField, index), std::move(value));
}

// Convenience form for the common case: the leaf lives directly on the struct reached from
// `owner`'s own top-level UpdateField member (Object::m_objectData, Unit::m_unitData, ...).
template<typename Owner, typename StructT, int32 SBlockBit, uint32 SBit, typename FieldT, int32 FBlockBit, uint32 FBit>
inline void FillField(Owner& owner, UpdateField<StructT, SBlockBit, SBit> Owner::* structField,
    UpdateField<FieldT, FBlockBit, FBit> StructT::* leafField, FieldT value)
{
    FillField(*(owner.*structField), [&]() { return owner.m_values.ModifyValue(structField); }, leafField, std::move(value));
}

template<typename Owner, typename StructT, int32 SBlockBit, uint32 SBit, typename FieldT, std::size_t Size, uint32 FBit, int32 FirstElementBit>
inline void FillField(Owner& owner, UpdateField<StructT, SBlockBit, SBit> Owner::* structField,
    UpdateFieldArray<FieldT, Size, FBit, FirstElementBit> StructT::* leafField, uint32 index, FieldT value)
{
    FillField(*(owner.*structField), [&]() { return owner.m_values.ModifyValue(structField); }, leafField, index, std::move(value));
}

void FillObjectData(Object& object, uint32 dynamicFlags);
void FillUnitData(Unit& unit);
void FillPlayerData(Player& player);
void FillActivePlayerData(Player& player);
void FillItemData(Item& item);
void FillContainerData(Bag& bag);
void FillGameObjectData(GameObject& gameObject);
void FillCorpseData(Corpse& corpse);
void FillDynamicObjectData(DynamicObject& dynamicObject);

// Fills whichever of the above apply to `object`'s concrete type. Called once per dirty object per
// update, before its create/values blocks are built.
void FillUpdateFieldData(Object& object);

#endif
