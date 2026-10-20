#include "stdafx.h"
#include "BoneProtections.h"
#include "..\Include/xrRender/Kinematics.h"
#include "..\Include/xrRender/KinematicsAnimated.h"

float SBoneProtections::getBoneProtection(s16 bone_id)
{
    storage_it it = m_bones_koeff.find(bone_id);
    if (it != m_bones_koeff.end())
        return it->second.koeff;
    else
        return m_default.koeff;
}

float SBoneProtections::getBoneArmour(s16 bone_id)
{
    storage_it it = m_bones_koeff.find(bone_id);
    if (it != m_bones_koeff.end())
        return it->second.armour;
    else
        return m_default.armour;
}

float SBoneProtections::getBoneBlunt(s16 bone_id)
{
    storage_it it = m_bones_koeff.find(bone_id);
    return it != m_bones_koeff.end() ? it->second.blunt : m_default.blunt;
}

void SBoneProtections::reload(const shared_str& bone_sect, IKinematics* kinematics)
{
    VERIFY(kinematics);
    m_bones_koeff.clear();
    m_section = bone_sect;

    m_fHitFrac = READ_IF_EXISTS(pSettings, r_float, bone_sect, "hit_fraction", 0.1f);

    m_default.koeff = 1.0f;
    m_default.armour = 0.0f;
    m_default.blunt = 0.0f;

    CInifile::Sect& protections = pSettings->r_section(bone_sect);
    for (const auto& i : protections.Data)
    {
        string256 buffer;
        float Koeff = (float)atof(_GetItem(i.second.c_str(), 0, buffer));
        float Armour = (float)atof(_GetItem(i.second.c_str(), 1, buffer));

        BoneProtection BP;
        BP.koeff = Koeff;
        BP.armour = Armour;
        // NLC: optional third column, blunt-impact minimum (docs/damage_model/DESIGN_npc_damage_model.md 21)
        BP.blunt = _GetItemCount(i.second.c_str()) > 2 ? (float)atof(_GetItem(i.second.c_str(), 2, buffer)) : 0.f;

        if (!xr_strcmp(i.first.c_str(), "default"))
        {
            m_default = BP;
        }
        else
        {
            if (!xr_strcmp(i.first.c_str(), "hit_fraction"))
                continue;

            s16 bone_id = kinematics->LL_BoneID(i.first);
            R_ASSERT2(BI_NONE != bone_id, i.first.c_str());
            m_bones_koeff.insert(mk_pair(bone_id, BP));
        }
    }
}
