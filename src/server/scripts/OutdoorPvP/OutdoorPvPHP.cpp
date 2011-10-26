/*
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "OutdoorPvPHP.h"
#include "OutdoorPvP.h"
#include "OutdoorPvPMgr.h"
#include "Player.h"
#include "WorldPacket.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Language.h"
#include "ScriptPCH.h"

const uint32 HP_LANG_LOSE_A[HP_TOWER_NUM] = {LANG_OPVP_HP_LOSE_BROKENHILL_A, LANG_OPVP_HP_LOSE_OVERLOOK_A, LANG_OPVP_HP_LOSE_STADIUM_A};

const uint32 HP_LANG_LOSE_H[HP_TOWER_NUM] = {LANG_OPVP_HP_LOSE_BROKENHILL_H, LANG_OPVP_HP_LOSE_OVERLOOK_H, LANG_OPVP_HP_LOSE_STADIUM_H};

const uint32 HP_LANG_CAPTURE_A[HP_TOWER_NUM] = {LANG_OPVP_HP_CAPTURE_BROKENHILL_A, LANG_OPVP_HP_CAPTURE_OVERLOOK_A, LANG_OPVP_HP_CAPTURE_STADIUM_A};

const uint32 HP_LANG_CAPTURE_H[HP_TOWER_NUM] = {LANG_OPVP_HP_CAPTURE_BROKENHILL_H, LANG_OPVP_HP_CAPTURE_OVERLOOK_H, LANG_OPVP_HP_CAPTURE_STADIUM_H};

OPvPCapturePointHP::OPvPCapturePointHP(OutdoorPvP* pvp, OutdoorPvPHPTowerType type)
: OPvPCapturePoint(pvp), m_TowerType(type)
{
    SetCapturePointData(HPCapturePoints[type].entry,
        HPCapturePoints[type].map,
        HPCapturePoints[type].x,
        HPCapturePoints[type].y,
        HPCapturePoints[type].z,
        HPCapturePoints[type].o,
        HPCapturePoints[type].rot0,
        HPCapturePoints[type].rot1,
        HPCapturePoints[type].rot2,
        HPCapturePoints[type].rot3);
    AddObject(type,
        HPTowerFlags[type].entry,
        HPTowerFlags[type].map,
        HPTowerFlags[type].x,
        HPTowerFlags[type].y,
        HPTowerFlags[type].z,
        HPTowerFlags[type].o,
        HPTowerFlags[type].rot0,
        HPTowerFlags[type].rot1,
        HPTowerFlags[type].rot2,
        HPTowerFlags[type].rot3);
}

OutdoorPvPHP::OutdoorPvPHP()
{
    m_TypeId = OUTDOOR_PVP_HP;
}

bool OutdoorPvPHP::SetupOutdoorPvP()
{
    m_AllianceTowersControlled = 0;
    m_HordeTowersControlled = 0;
    // add the zones affected by the pvp buff
    for (int i = 0; i < OutdoorPvPHPBuffZonesNum; ++i)
        RegisterZone(OutdoorPvPHPBuffZones[i]);

    AddCapturePoint(new OPvPCapturePointHP(this, HP_TOWER_BROKEN_HILL));

    AddCapturePoint(new OPvPCapturePointHP(this, HP_TOWER_OVERLOOK));

    AddCapturePoint(new OPvPCapturePointHP(this, HP_TOWER_STADIUM));

    return true;
}

void OutdoorPvPHP::HandlePlayerEnterZone(Player* player, uint32 zone)
{
    // add buffs
    if (player->GetTeam() == ALLIANCE)
    {
        if (m_AllianceTowersControlled >=3)
            player->CastSpell(player, AllianceBuff, true);
			player->CastSpell(player, 68652, true); // 50% honor increase
    }
    else
    {
        if (m_HordeTowersControlled >=3)
            player->CastSpell(player, HordeBuff, true);
			player->CastSpell(player, 68652, true); // 50% honor increase
    }
	// Just in case. Make them honorless.
    player->AddAura( HP_SPELL_HONORLESS, player );
    OutdoorPvP::HandlePlayerEnterZone(player, zone);
}

void OutdoorPvPHP::HandlePlayerLeaveZone(Player* player, uint32 zone)
{
    // remove buffs
    if (player->GetTeam() == ALLIANCE)
    {
        player->RemoveAurasDueToSpell(AllianceBuff);
		player->RemoveAurasDueToSpell(68652);
		
    }
    else
    {
        player->RemoveAurasDueToSpell(HordeBuff);
		player->RemoveAurasDueToSpell(68652);
    }
    OutdoorPvP::HandlePlayerLeaveZone(player, zone);
}

bool OutdoorPvPHP::Update(uint32 diff)
{
    bool changed = OutdoorPvP::Update(diff);
    if (changed)
    {
        if (m_AllianceTowersControlled == 3)
		{
            TeamApplyBuff(TEAM_ALLIANCE, AllianceBuff, HordeBuff);
			TeamCastSpell(TEAM_ALLIANCE, 68652);
            TeamCastSpell(TEAM_HORDE, -68652);
		}
        else if (m_HordeTowersControlled == 3)
		{
            TeamApplyBuff(TEAM_HORDE, HordeBuff, AllianceBuff);
			TeamCastSpell(TEAM_HORDE, 68652);
            TeamCastSpell(TEAM_ALLIANCE, -68652);
		}
        else
        {
            TeamCastSpell(TEAM_ALLIANCE, -AllianceBuff);
            TeamCastSpell(TEAM_HORDE, -HordeBuff);
			TeamCastSpell(TEAM_ALLIANCE, -68652);
            TeamCastSpell(TEAM_HORDE, -68652);
        }
        SendUpdateWorldState(HP_UI_TOWER_COUNT_A, m_AllianceTowersControlled);
        SendUpdateWorldState(HP_UI_TOWER_COUNT_H, m_HordeTowersControlled);
    }
	
	// Resurrection System
    m_LastResurrectTime += diff;
    if (m_LastResurrectTime >= HP_RESURRECTION_INTERVAL)
    {
        //sLog->outString("HillsbradMGR : Reviving...");
        if ( GetReviveQueueSize() )
        {
            sLog->outString("HellfireMGR : Dead players in queue.");
            for (std::map<uint64, std::vector<uint64> >::iterator itr = m_ReviveQueue.begin(); itr != m_ReviveQueue.end(); ++itr)
            {
                Creature* sh = NULL;
                for (std::vector<uint64>::const_iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); ++itr2)
                {
                    Player* plr = ObjectAccessor::FindPlayer(*itr2);
                    if (!plr)
                        continue;

                    if (!sh && plr->IsInWorld())
                    {
                        sh = plr->GetMap()->GetCreature(itr->first);
                        // only for visual effect
                        if (sh)
                            // Spirit Heal, effect 117
                            sh->CastSpell(sh, HP_SPELL_SPIRIT_HEAL, true);
                    }

                    // Resurrection visual
                    plr->CastSpell(plr, HP_SPELL_RESURRECTION_VISUAL, true);
                    m_ResurrectQueue.push_back(*itr2);
                }
                (itr->second).clear();
            }
            m_ReviveQueue.clear();
        }
        // Reset last resurrection time
        m_LastResurrectTime = 0;
    }
    else if (m_LastResurrectTime > 500) // Resurrect players only half a second later
    {
        if ( GetResurrectQueueSize() )
        {
            //sLog->outString("HillsbradMGR : Resurrecting...");
            for (std::vector<uint64>::const_iterator itr = m_ResurrectQueue.begin(); itr != m_ResurrectQueue.end(); ++itr)
            {
                Player* plr = ObjectAccessor::FindPlayer(*itr);
                if (!plr)
                    continue;
                plr->ResurrectPlayer(1.0f, false);
                plr->CastSpell(plr, 6962, true);
                plr->CastSpell(plr, HP_SPELL_SPIRIT_HEAL_MANA, true);
                sObjectAccessor->ConvertCorpseForPlayer(*itr);
            }
            m_ResurrectQueue.clear();
        }
    }
	ApplyZoneBalanceBuff();
    return changed;
}

void OutdoorPvPHP::HandlePlayerResurrects(Player* plr, uint32 zone)
{
    plr->AddAura(HP_SPELL_HONORLESS, plr);
}

void OutdoorPvPHP::ApplyZoneBalanceBuff()
{
    for (int i = 0; i <= 1; ++i)
    {
        for (PlayerSet::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
        {
            Player * plr = *itr;
            {	
                if( plr->GetTotalPlayedTime() >= 900 )
                {
                    if ( plr->HasByteFlag( UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_SANCTUARY ) )
                        plr->RemoveByteFlag( UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_SANCTUARY );
                }
                else
                {
                    plr->SetByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_SANCTUARY);
                    plr->CombatStopWithPets();
                }
            }
        }
    }
}

void OutdoorPvPHP::OnCreatureCreate(Creature* creature, bool add)
{
    //OutdoorPvP::OnCreatureCreate(creature, add);
    if ( (creature->GetEntry() == HP_CREATURE_ENTRY_A_SPIRITGUIDE || creature->GetEntry() == HP_CREATURE_ENTRY_H_SPIRITGUIDE) && creature->GetZoneId() == OutdoorPvPHPBuffZones[0] )
    {
        if (add)
        {
            creature->setDeathState(DEAD);
            creature->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, creature->GetGUID());
            creature->SetUInt32Value(UNIT_CHANNEL_SPELL, HP_SPELL_SPIRIT_HEAL_CHANNEL);
            creature->SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);
        }
    }
}

void OutdoorPvPHP::SendAreaSpiritHealerQueryOpcode(Player* pl, const uint64& guid)
{
    WorldPacket data(SMSG_AREA_SPIRIT_HEALER_TIME, 12);
    uint32 time_ = HP_RESURRECTION_INTERVAL - GetLastResurrectTime(); // resurrect every HP_RESURRECTION_INTERVAL / 1000 seconds
    if (time_ == uint32(-1))
        time_ = 0;
    data << guid << time_;
    pl->GetSession()->SendPacket(&data);
}

void OutdoorPvPHP::AddPlayerToResurrectQueue(uint64 npc_guid, uint64 player_guid)
{
    m_ReviveQueue[npc_guid].push_back(player_guid);

    Player* plr = ObjectAccessor::FindPlayer(player_guid);
    if (!plr)
        return;

    plr->CastSpell(plr, HP_SPELL_WAITING_FOR_RESURRECT, true);
}

void OutdoorPvPHP::RemovePlayerFromResurrectQueue(uint64 player_guid)
{
    for (std::map<uint64, std::vector<uint64> >::iterator itr = m_ReviveQueue.begin(); itr != m_ReviveQueue.end(); ++itr)
    {
        for (std::vector<uint64>::iterator itr2 =(itr->second).begin(); itr2 != (itr->second).end(); ++itr2)
        {
            if (*itr2 == player_guid)
            {
                (itr->second).erase(itr2);

                Player* plr = ObjectAccessor::FindPlayer(player_guid);

                if (!plr)
                    return;

                plr->RemoveAurasDueToSpell( HP_SPELL_WAITING_FOR_RESURRECT );

                return;
            }
        }
    }
}

void OutdoorPvPHP::SendRemoveWorldStates(Player* player)
{
    player->SendUpdateWorldState(HP_UI_TOWER_DISPLAY_A, 0);
    player->SendUpdateWorldState(HP_UI_TOWER_DISPLAY_H, 0);
    player->SendUpdateWorldState(HP_UI_TOWER_COUNT_H, 0);
    player->SendUpdateWorldState(HP_UI_TOWER_COUNT_A, 0);
    player->SendUpdateWorldState(HP_UI_TOWER_SLIDER_N, 0);
    player->SendUpdateWorldState(HP_UI_TOWER_SLIDER_POS, 0);
    player->SendUpdateWorldState(HP_UI_TOWER_SLIDER_DISPLAY, 0);
    for (int i = 0; i < HP_TOWER_NUM; ++i)
    {
        player->SendUpdateWorldState(HP_MAP_N[i], 0);
        player->SendUpdateWorldState(HP_MAP_A[i], 0);
        player->SendUpdateWorldState(HP_MAP_H[i], 0);
    }
}

void OutdoorPvPHP::FillInitialWorldStates(WorldPacket &data)
{
    data << uint32(HP_UI_TOWER_DISPLAY_A) << uint32(1);
    data << uint32(HP_UI_TOWER_DISPLAY_H) << uint32(1);
    data << uint32(HP_UI_TOWER_COUNT_A) << uint32(m_AllianceTowersControlled);
    data << uint32(HP_UI_TOWER_COUNT_H) << uint32(m_HordeTowersControlled);
    data << uint32(HP_UI_TOWER_SLIDER_DISPLAY) << uint32(0);
    data << uint32(HP_UI_TOWER_SLIDER_POS) << uint32(50);
    data << uint32(HP_UI_TOWER_SLIDER_N) << uint32(100);
    for (OPvPCapturePointMap::iterator itr = m_capturePoints.begin(); itr != m_capturePoints.end(); ++itr)
    {
        itr->second->FillInitialWorldStates(data);
    }
}

void OPvPCapturePointHP::ChangeState()
{
    uint32 field = 0;
    switch (m_OldState)
    {
    case OBJECTIVESTATE_NEUTRAL:
        field = HP_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_ALLIANCE:
        field = HP_MAP_A[m_TowerType];
        if (uint32 alliance_towers = ((OutdoorPvPHP*)m_PvP)->GetAllianceTowersControlled())
            ((OutdoorPvPHP*)m_PvP)->SetAllianceTowersControlled(--alliance_towers);
        sWorld->SendZoneText(OutdoorPvPHPBuffZones[0], sObjectMgr->GetTrinityStringForDBCLocale(HP_LANG_LOSE_A[m_TowerType]));
        break;
    case OBJECTIVESTATE_HORDE:
        field = HP_MAP_H[m_TowerType];
        if (uint32 horde_towers = ((OutdoorPvPHP*)m_PvP)->GetHordeTowersControlled())
            ((OutdoorPvPHP*)m_PvP)->SetHordeTowersControlled(--horde_towers);
        sWorld->SendZoneText(OutdoorPvPHPBuffZones[0], sObjectMgr->GetTrinityStringForDBCLocale(HP_LANG_LOSE_H[m_TowerType]));
        break;
    case OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE:
        field = HP_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE:
        field = HP_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE:
        field = HP_MAP_A[m_TowerType];
        break;
    case OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE:
        field = HP_MAP_H[m_TowerType];
        break;
    }

    // send world state update
    if (field)
    {
        m_PvP->SendUpdateWorldState(field, 0);
        field = 0;
    }
    uint32 artkit = 21;
    uint32 artkit2 = HP_TowerArtKit_N[m_TowerType];
    switch (m_State)
    {
    case OBJECTIVESTATE_NEUTRAL:
        field = HP_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_ALLIANCE:
    {
        field = HP_MAP_A[m_TowerType];
        artkit = 2;
        artkit2 = HP_TowerArtKit_A[m_TowerType];
        uint32 alliance_towers = ((OutdoorPvPHP*)m_PvP)->GetAllianceTowersControlled();
        if (alliance_towers < 3)
            ((OutdoorPvPHP*)m_PvP)->SetAllianceTowersControlled(++alliance_towers);
        sWorld->SendZoneText(OutdoorPvPHPBuffZones[0], sObjectMgr->GetTrinityStringForDBCLocale(HP_LANG_CAPTURE_A[m_TowerType]));
        break;
    }
    case OBJECTIVESTATE_HORDE:
    {
        field = HP_MAP_H[m_TowerType];
        artkit = 1;
        artkit2 = HP_TowerArtKit_H[m_TowerType];
        uint32 horde_towers = ((OutdoorPvPHP*)m_PvP)->GetHordeTowersControlled();
        if (horde_towers < 3)
            ((OutdoorPvPHP*)m_PvP)->SetHordeTowersControlled(++horde_towers);
        sWorld->SendZoneText(OutdoorPvPHPBuffZones[0], sObjectMgr->GetTrinityStringForDBCLocale(HP_LANG_CAPTURE_H[m_TowerType]));
        break;
    }
    case OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE:
        field = HP_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE:
        field = HP_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE:
        field = HP_MAP_A[m_TowerType];
        artkit = 2;
        artkit2 = HP_TowerArtKit_A[m_TowerType];
        break;
    case OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE:
        field = HP_MAP_H[m_TowerType];
        artkit = 1;
        artkit2 = HP_TowerArtKit_H[m_TowerType];
        break;
    }

    GameObject* flag = HashMapHolder<GameObject>::Find(m_capturePointGUID);
    GameObject* flag2 = HashMapHolder<GameObject>::Find(m_Objects[m_TowerType]);
    if (flag)
    {
        flag->SetGoArtKit(artkit);
    }
    if (flag2)
    {
        flag2->SetGoArtKit(artkit2);
    }

    // send world state update
    if (field)
        m_PvP->SendUpdateWorldState(field, 1);

    // complete quest objective
    if (m_State == OBJECTIVESTATE_ALLIANCE || m_State == OBJECTIVESTATE_HORDE)
        SendObjectiveComplete(HP_CREDITMARKER[m_TowerType], 0);
}

void OPvPCapturePointHP::SendChangePhase()
{
    SendUpdateWorldState(HP_UI_TOWER_SLIDER_N, m_neutralValuePct);
    // send these updates to only the ones in this objective
    uint32 phase = (uint32)ceil((m_value + m_maxValue) / (2 * m_maxValue) * 100.0f);
    SendUpdateWorldState(HP_UI_TOWER_SLIDER_POS, phase);
    // send this too, sometimes the slider disappears, dunno why :(
    SendUpdateWorldState(HP_UI_TOWER_SLIDER_DISPLAY, 1);
}

void OPvPCapturePointHP::FillInitialWorldStates(WorldPacket &data)
{
    switch (m_State)
    {
        case OBJECTIVESTATE_ALLIANCE:
        case OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE:
            data << uint32(HP_MAP_N[m_TowerType]) << uint32(0);
            data << uint32(HP_MAP_A[m_TowerType]) << uint32(1);
            data << uint32(HP_MAP_H[m_TowerType]) << uint32(0);
            break;
        case OBJECTIVESTATE_HORDE:
        case OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE:
            data << uint32(HP_MAP_N[m_TowerType]) << uint32(0);
            data << uint32(HP_MAP_A[m_TowerType]) << uint32(0);
            data << uint32(HP_MAP_H[m_TowerType]) << uint32(1);
            break;
        case OBJECTIVESTATE_NEUTRAL:
        case OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE:
        case OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE:
        default:
            data << uint32(HP_MAP_N[m_TowerType]) << uint32(1);
            data << uint32(HP_MAP_A[m_TowerType]) << uint32(0);
            data << uint32(HP_MAP_H[m_TowerType]) << uint32(0);
            break;
    }
}

bool OPvPCapturePointHP::HandlePlayerEnter(Player* player)
{
    if (OPvPCapturePoint::HandlePlayerEnter(player))
    {
        player->SendUpdateWorldState(HP_UI_TOWER_SLIDER_DISPLAY, 1);
        uint32 phase = (uint32)ceil((m_value + m_maxValue) / (2 * m_maxValue) * 100.0f);
        player->SendUpdateWorldState(HP_UI_TOWER_SLIDER_POS, phase);
        player->SendUpdateWorldState(HP_UI_TOWER_SLIDER_N, m_neutralValuePct);
        return true;
    }
    return false;
}

void OPvPCapturePointHP::HandlePlayerLeave(Player* player)
{
    player->SendUpdateWorldState(HP_UI_TOWER_SLIDER_DISPLAY, 0);
    OPvPCapturePoint::HandlePlayerLeave(player);
}

void OutdoorPvPHP::HandleKillImpl(Player* player, Unit* killed)
{
    if (killed->GetTypeId() != TYPEID_PLAYER)
        return;

    if (player->GetTeam() == ALLIANCE && killed->ToPlayer()->GetTeam() != ALLIANCE && !player->HasAura(2479))
        player->CastSpell(player, AlliancePlayerKillReward, true);
    else if (player->GetTeam() == HORDE && killed->ToPlayer()->GetTeam() != HORDE && !player->HasAura(2479))
        player->CastSpell(player, HordePlayerKillReward, true);
}

uint32 OutdoorPvPHP::GetAllianceTowersControlled() const
{
    return m_AllianceTowersControlled;
}

void OutdoorPvPHP::SetAllianceTowersControlled(uint32 count)
{
    m_AllianceTowersControlled = count;
}

uint32 OutdoorPvPHP::GetHordeTowersControlled() const
{
    return m_HordeTowersControlled;
}

void OutdoorPvPHP::SetHordeTowersControlled(uint32 count)
{
    m_HordeTowersControlled = count;
}

class OutdoorPvP_hellfire_peninsula : public OutdoorPvPScript
{
    public:

        OutdoorPvP_hellfire_peninsula()
            : OutdoorPvPScript("outdoorpvp_hp")
        {
        }

        OutdoorPvP* GetOutdoorPvP() const
        {
            return new OutdoorPvPHP();
        }
};

void AddSC_outdoorpvp_hp()
{
    new OutdoorPvP_hellfire_peninsula();
}
