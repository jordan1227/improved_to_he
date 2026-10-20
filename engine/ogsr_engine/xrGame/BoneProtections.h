#pragma once

struct SBoneProtections
{
    struct BoneProtection
    {
        float koeff;
        float armour;
        float blunt{}; // NLC: blunt-impact minimum share of the raw bullet power (third column, 0 = off)
    };
    float m_fHitFrac{};
    shared_str m_section; // NLC: protection section name (script export bone_protection_section)
    typedef xr_map<s16, BoneProtection> storage_type;
    typedef storage_type::iterator storage_it;
    SBoneProtections()
    {
        m_default.koeff = 1.0f;
        m_default.armour = 0;
    }
    BoneProtection m_default;
    storage_type m_bones_koeff;
    void reload(const shared_str& outfit_section, IKinematics* kinematics);
    float getBoneProtection(s16 bone_id);
    float getBoneArmour(s16 bone_id);
    float getBoneBlunt(s16 bone_id); // NLC
};
