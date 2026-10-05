"""Names for M2 key bones, attachments, event 4CCs and texture types."""

KEY_BONES = ("ArmL", "ArmR", "ShoulderL", "ShoulderR", "SpineLow", "Waist", "Head", "Jaw", "IndexFingerR", "MiddleFingerR",
             "PinkyFingerR", "RingFingerR", "ThumbR", "IndexFingerL", "MiddleFingerL", "PinkyFingerL", "RingFingerL",
             "ThumbL", "$BTH", "$CSR", "$CSL", "_Breath", "_Name", "_NameMount", "$CHD", "$CCH", "Root", "Wheel1",
             "Wheel2", "Wheel3", "Wheel4", "Wheel5", "Wheel6", "Wheel7", "Wheel8")

ATTACHMENTS = ("Shield", "HandRight", "HandLeft", "ElbowRight", "ElbowLeft", "ShoulderRight", "ShoulderLeft", "KneeRight",
               "KneeLeft", "HipRight", "HipLeft", "Helm", "Back", "ShoulderFlapRight", "ShoulderFlapLeft",
               "ChestBloodFront", "ChestBloodBack", "Breath", "PlayerName", "Base", "Head", "SpellLeftHand",
               "SpellRightHand", "Special1", "Special2", "Special3", "SheathMainHand", "SheathOffHand", "SheathShield",
               "PlayerNameMounted", "LargeWeaponLeft", "LargeWeaponRight", "HipWeaponLeft", "HipWeaponRight", "Chest",
               "HandArrow", "Bullet", "SpellHandOmni", "SpellHandDirected", "VehicleSeat1")

TEXTURE_TYPES = {0: "hardcoded", 1: "body skin", 2: "object skin", 3: "weapon blade", 4: "weapon handle", 5: "environment",
                 6: "hair", 7: "facial hair", 8: "skin extra", 9: "ui skin", 10: "tauren mane", 11: "creature skin 1",
                 12: "creature skin 2", 13: "creature skin 3", 14: "item icon"}

BLENDING = ("opaque", "alpha key", "alpha", "no-alpha add", "add", "mod", "mod2x", "blend add")


def key_bone(key_bone_id):
    return KEY_BONES[key_bone_id] if 0 <= key_bone_id < len(KEY_BONES) else ""


def bone_name(index, key_bone_id):
    return f"{key_bone(key_bone_id) or 'bone'}_{index}"


def attachment(att_id):
    return ATTACHMENTS[att_id] if 0 <= att_id < len(ATTACHMENTS) else f"Attachment{att_id}"


def event(identifier):
    """Four-character code of an event identifier stored as a little-endian u32."""
    return int(identifier).to_bytes(4, "little").decode("latin-1")
