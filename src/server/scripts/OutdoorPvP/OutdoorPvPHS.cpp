/*
 * Copyright (C) 2010 Wowtorn <http://www.wowtorn.net/>
 */

#include "OutdoorPvPHS.h"
#include "OutdoorPvPMgr.h"
#include "OutdoorPvP.h"
#include "WorldPacket.h"
#include "Player.h"
#include "Chat.h"
#include "ObjectMgr.h"
#include "Language.h"
#include "World.h"
#include "ScriptPCH.h"
#include <math.h>


const uint32 HS_LANG_LOSE_A[HS_TOWER_NUM] = {LANG_OPVP_HP_LOSE_BROKENHILL_A, LANG_OPVP_HP_LOSE_OVERLOOK_A, LANG_OPVP_HP_LOSE_STADIUM_A};

const uint32 HS_LANG_LOSE_H[HS_TOWER_NUM] = {LANG_OPVP_HP_LOSE_BROKENHILL_H, LANG_OPVP_HP_LOSE_OVERLOOK_H, LANG_OPVP_HP_LOSE_STADIUM_H};

const uint32 HS_LANG_CAPTURE_A[HS_TOWER_NUM] = {LANG_OPVP_HP_CAPTURE_BROKENHILL_A, LANG_OPVP_HP_CAPTURE_OVERLOOK_A, LANG_OPVP_HP_CAPTURE_STADIUM_A};

const uint32 HS_LANG_CAPTURE_H[HS_TOWER_NUM] = {LANG_OPVP_HP_CAPTURE_BROKENHILL_H, LANG_OPVP_HP_CAPTURE_OVERLOOK_H, LANG_OPVP_HP_CAPTURE_STADIUM_H};

OPvPCapturePointSP::OPvPCapturePointSP(OutdoorPvP* pvp, OutdoorPvPSPTowerType type)
: OPvPCapturePoint(pvp), m_TowerType(type)
{
    SetCapturePointData(HSCapturePoints[type].entry,
        HSCapturePoints[type].map,
        HSCapturePoints[type].x,
        HSCapturePoints[type].y,
        HSCapturePoints[type].z,
        HSCapturePoints[type].o,
        HSCapturePoints[type].rot0,
        HSCapturePoints[type].rot1,
        HSCapturePoints[type].rot2,
        HSCapturePoints[type].rot3);
    AddObject(type,
        HSTowerFlags[type].entry,
        HSTowerFlags[type].map,
        HSTowerFlags[type].x,
        HSTowerFlags[type].y,
        HSTowerFlags[type].z,
        HSTowerFlags[type].o,
        HSTowerFlags[type].rot0,
        HSTowerFlags[type].rot1,
        HSTowerFlags[type].rot2,
        HSTowerFlags[type].rot3);
}

OutdoorPvPHS::OutdoorPvPHS()
{
    m_TypeId = OUTDOOR_PVP_HS;
}

bool OutdoorPvPHS::SetupOutdoorPvP()
{
    m_PlayerCount    = 0;
    m_HordeCount    = 0;
    m_AllianceCount    = 0;

    m_HordeBuff = 0;
    m_AllianceBuff = 0;

    m_ChestTimer = HS_FFA_CHEST_TIMER;
    m_ChestAnnounceTimer = HS_FFA_CHEST_ANNOUNCE_TIMER;
    m_ChestDebugTimer = 60000;
    m_ChestGUID = 0;

    m_TenacityTimer = HS_TENACITY_TIME;
    m_LastResurrectTime = 0;
	
	m_AllianceTowersControlled = 0;
    m_HordeTowersControlled = 0;

    RegisterZone(HS_ZONE);
	    
	AddCapturePoint(new OPvPCapturePointHS(this, HS_TOWER_SOUTH));
    AddCapturePoint(new OPvPCapturePointHS(this, HS_TOWER_DARROW));
    AddCapturePoint(new OPvPCapturePointHS(this, HS_TOWER_NETHANDER));

    sLog->outString("HillsbradMGR : Loaded.");
    return true;
}

void OutdoorPvPHS::FillInitialWorldStates(WorldPacket &data)
{
    data << uint32(HS_UI_TOWER_DISPLAY_A) << uint32(1);
    data << uint32(HS_UI_TOWER_DISPLAY_H) << uint32(1);
    data << uint32(HS_UI_TOWER_COUNT_A) << uint32(m_AllianceTowersControlled);
    data << uint32(HS_UI_TOWER_COUNT_H) << uint32(m_HordeTowersControlled);
    data << uint32(HS_UI_TOWER_SLIDER_DISPLAY) << uint32(0);
    data << uint32(HS_UI_TOWER_SLIDER_POS) << uint32(50);
    data << uint32(HS_UI_TOWER_SLIDER_N) << uint32(100);
    for (OPvPCapturePointMap::iterator itr = m_capturePoints.begin(); itr != m_capturePoints.end(); ++itr)
    {
        itr->second->FillInitialWorldStates(data);
    }
}


void OutdoorPvPHS::SendRemoveWorldStates(Player* player)
{
    player->SendUpdateWorldState(HS_UI_TOWER_DISPLAY_A, 0);
    player->SendUpdateWorldState(HS_UI_TOWER_DISPLAY_H, 0);
    player->SendUpdateWorldState(HS_UI_TOWER_COUNT_H, 0);
    player->SendUpdateWorldState(HS_UI_TOWER_COUNT_A, 0);
    player->SendUpdateWorldState(HS_UI_TOWER_SLIDER_N, 0);
    player->SendUpdateWorldState(HS_UI_TOWER_SLIDER_POS, 0);
    player->SendUpdateWorldState(HS_UI_TOWER_SLIDER_DISPLAY, 0);
    for (int i = 0; i < HS_TOWER_NUM; ++i)
    {
        player->SendUpdateWorldState(HS_MAP_N[i], 0);
        player->SendUpdateWorldState(HS_MAP_A[i], 0);
        player->SendUpdateWorldState(HS_MAP_H[i], 0);
    }
}

bool OutdoorPvPHS::Update(uint32 diff)
{
    bool changed = OutdoorPvP::Update(diff);
    if (changed)
    {
        if (m_AllianceTowersControlled == 3)
            TeamApplyBuff(TEAM_ALLIANCE, AllianceBuff, HordeBuff);
        else if (m_HordeTowersControlled == 3)
            TeamApplyBuff(TEAM_HORDE, HordeBuff, AllianceBuff);
        else
        {
            TeamCastSpell(TEAM_ALLIANCE, -AllianceBuff);
            TeamCastSpell(TEAM_HORDE, -HordeBuff);
        }
        SendUpdateWorldState(HS_UI_TOWER_COUNT_A, m_AllianceTowersControlled);
        SendUpdateWorldState(HS_UI_TOWER_COUNT_H, m_HordeTowersControlled);
    }
    // Resurrection System
    m_LastResurrectTime += diff;
    if (m_LastResurrectTime >= HS_RESURRECTION_INTERVAL)
    {
        //sLog->outString("HillsbradMGR : Reviving...");
        if ( GetReviveQueueSize() )
        {
            sLog->outString("HillsbradMGR : Dead players in queue.");
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
                            sh->CastSpell(sh, HS_SPELL_SPIRIT_HEAL, true);
                    }

                    // Resurrection visual
                    plr->CastSpell(plr, HS_SPELL_RESURRECTION_VISUAL, true);
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
                plr->CastSpell(plr, HS_SPELL_SPIRIT_HEAL_MANA, true);
                sObjectAccessor->ConvertCorpseForPlayer(*itr);
            }
            m_ResurrectQueue.clear();
        }
    }

    // Arena Chest System.
    // Update the timer.
	
    //if( m_ChestGUID == 0 )
    //{ 
        if( m_ChestTimer <= diff )
        {
            uint32 ffachest = 0;
            ffachest = urand(0, 9);
            if( uint32 guid = sObjectMgr->AddGOData(HSChestPoints[ffachest].entry, HSChestPoints[ffachest].map, HSChestPoints[ffachest].x, HSChestPoints[ffachest].y, HSChestPoints[ffachest].z, HSChestPoints[ffachest].o, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f) )
            {
                m_ChestGUID = 0;
                SendMessageToAll( "FFA chest has been spawned in the fields. Good luck!" );
				sLog->outString( "Hillsbrad : Spawned Chest(%u) at location %u.", guid,  ffachest);
            }
            m_ChestTimer = HS_FFA_CHEST_TIMER;
        }
        else
        {
            m_ChestTimer -= diff;
            // Announce when?
            if( m_ChestAnnounceTimer < diff )
            {
                uint32 ChestTimerSec = uint32( m_ChestTimer / IN_MILLISECONDS );
                if (
                    (ChestTimerSec < 3         && ChestTimerSec > 0) ||   
                    (ChestTimerSec < 30        && (ChestTimerSec % 5) == 0) ||         // < 30 sec; every 5 sec                      
                    (ChestTimerSec <=5*MINUTE  && (ChestTimerSec % MINUTE) == 0) ||    // < 5 min ; every 1 min
                    (ChestTimerSec <=30*MINUTE && (ChestTimerSec % (5*MINUTE)) == 0) ) // < 30 min; every 5 min
                {
                    std::string str = secsToTimeString( ChestTimerSec );
                    SendMessageToAll( "FFA chest will spawn in %s.", str.c_str() );
                }
                m_ChestAnnounceTimer = HS_FFA_CHEST_ANNOUNCE_TIMER;
            }
            else
                m_ChestAnnounceTimer -= diff;
        }
    //}
    // Chest debug.
    if( m_ChestTimer < diff )
    {
        sLog->outString( "HillsbradMGR : Chest Guid (%u), Timer(%u), Announce Timer (%u).", m_ChestGUID, m_ChestTimer, m_ChestAnnounceTimer );
        m_ChestDebugTimer = 60000;
    }
    else
        m_ChestDebugTimer -= diff;

	ApplyZoneBalanceBuff();
    return changed;
}

void OutdoorPvPHS::HandlePlayerEnterZone(Player* plr, uint32 zone)
{
    // add buffs
    if (plr->GetTeam() == ALLIANCE)
    {
        if (m_AllianceTowersControlled >=3)
            plr->CastSpell(plr, AllianceBuff, true);
    }
    else
    {
        if (m_HordeTowersControlled >=3)
            plr->CastSpell(plr, HordeBuff, true);
    }
    // Just in case. Make them honorless.
    plr->AddAura( HS_SPELL_HONORLESS, plr );

    OutdoorPvP::HandlePlayerEnterZone(plr,zone);
}

void OutdoorPvPHS::HandlePlayerLeaveZone(Player* plr, uint32 zone)
{
	// remove buffs
    if (plr->GetTeam() == ALLIANCE)
    {
        plr->RemoveAurasDueToSpell(AllianceBuff);
    }
    else
    {
        plr->RemoveAurasDueToSpell(HordeBuff);
    }
    OutdoorPvP::HandlePlayerLeaveZone(plr, zone);
}

void OutdoorPvPHS::HandlePlayerResurrects(Player * plr, uint32 zone)
{
    plr->AddAura(HS_SPELL_HONORLESS, plr);
}

bool OutdoorPvPHS::HandleOpenGo(Player* plr, uint64 guid)
{
    //sLog->outString("HillsbradMGR: Using %u.", guid);
    if( GameObject* obj = plr->GetMap()->GetGameObject( guid ) )
    {
        // Is this the chest?
        if( obj->GetGUIDLow() == m_ChestGUID )
        {
            m_ChestGUID = 0;
            SendMessageToAll( "%s has claimed the chest! The next chest will appear in an hour.", plr->GetName() );
            return false;
        }
		/*
        for (uint32 node = 0; node < HS_CAPTURE_NUM; node+=1)
        {
            if ( obj->GetGUIDLow() == m_TowerPoints[node].gameobject ) // Player just opened a capture point..
            {
                // Player's team == plr->GetTeam()
                switch( m_TowerPoints[node].state )
                {
                    // If neutral.. make it contested..
                    case HS_CAPTURE_NEUTRAL:
                        changeCapturePoint( node, ( plr->GetTeam() == ALLIANCE ) ? HS_CAPTURE_ALLIANCE_CONT : HS_CAPTURE_HORDE_CONT, 60000 );
                        if( plr->GetTeam() == ALLIANCE )
                            SendMessageToAll( "%s has claimed %s tower. If left uncontested Alliance will control it in 60 seconds!", plr->GetName(), m_TowerPoints[node].name.c_str() );
                        else
                            SendMessageToAll( "%s has claimed %s tower. If left uncontested Horde will control it in 60 seconds!", plr->GetName(), m_TowerPoints[node].name.c_str() );
                    break;
                    case HS_CAPTURE_ALLIANCE_CONT:
                    {
                        if( plr->GetTeam() == ALLIANCE )
                        {
                            if ( m_TowerPoints[node].previousState == HS_CAPTURE_NEUTRAL ) 
                                return false;
                            else if ( m_TowerPoints[node].teamIndex == 1 )
                                changeCapturePoint( node, HS_CAPTURE_ALLIANCE, 0 );
                        }
                        else
                            changeCapturePoint( node, HS_CAPTURE_HORDE_CONT, 60000 );
                    }
                    break;
                    case HS_CAPTURE_HORDE_CONT:
                    {
                        if( plr->GetTeam() != ALLIANCE )
                        {
                            
                            if ( m_TowerPoints[node].previousState == HS_CAPTURE_NEUTRAL ) 
                                return false;
                            else if ( m_TowerPoints[node].teamIndex == 2 )
                                changeCapturePoint( node, HS_CAPTURE_HORDE, 0 );
                        }
                        else
                            changeCapturePoint( node, HS_CAPTURE_ALLIANCE_CONT, 60000 );
                    }
                    break;
                    case HS_CAPTURE_ALLIANCE:
                    {
                        if( plr->GetTeam() == ALLIANCE )
                            return false;
                        changeCapturePoint( node, HS_CAPTURE_HORDE_CONT, 60000 );
                        SendMessageToAll( "%s has claimed %s tower. If left uncontested Horde will control it in 60 seconds!", plr->GetName(), m_TowerPoints[node].name.c_str() );
                    }
                    break;
                    case HS_CAPTURE_HORDE:
                    {
                        if( plr->GetTeam() != ALLIANCE )
                            return false;
                        changeCapturePoint( node, HS_CAPTURE_ALLIANCE_CONT, 60000 );
                        SendMessageToAll( "%s has claimed %s tower. If left uncontested Alliance will control it in 60 seconds!", plr->GetName(), m_TowerPoints[node].name.c_str() );
                    }
                    break;
                }
                return false;
            }
        } */
    }
    return false;
}

void OutdoorPvPHS::changeCapturePoint( uint32 node, HSCapturePointState newState, uint32 timer )
{
    // Set the states
    m_TowerPoints[node].previousState = m_TowerPoints[node].state;
    m_TowerPoints[node].state = newState;

    sLog->outString("Changing node %u from %u to %u. Timer: %u", node, m_TowerPoints[node].previousState, m_TowerPoints[node].state, timer);
    
    if( timer > 0 )
    {
        m_TowerPoints[node].timer = timer;
        m_TowerPoints[node].timerActive = true;
    }
    else
        m_TowerPoints[node].timerActive = false;
    
    // Now, delete the old gameobject, replace with new...

    if (GameObjectData const* data = objmgr.GetGOData(m_TowerPoints[node].gameobject) )
    {
        objmgr.RemoveGameobjectFromGrid(m_TowerPoints[node].gameobject, data);

        if (GameObject* gob = sObjectAccessor.GetObjectInWorld(MAKE_NEW_GUID(m_TowerPoints[node].gameobject, data->id, HIGHGUID_GAMEOBJECT), (GameObject*)NULL))
        {
            uint32 guid = gob->GetDBTableGUIDLow();
            gob->SetRespawnTime(0);                                 // not save respawn time
            gob->Delete();
            objmgr.DeleteGOData(guid);
        } else
            sLog->outString("CANNOT DELETE %u. NO OBJECT", m_TowerPoints[node].gameobject);
    } else
        sLog->outString("CANNOT DELETE %u. NO DATA", m_TowerPoints[node].gameobject);

    uint32 banner = 0;
    uint32 countForTeam = 0;

    switch(  m_TowerPoints[node].state )
    {
        case HS_CAPTURE_ALLIANCE:
            {
                banner = 850026;

                for (uint32 i = 0; i < HS_CAPTURE_NUM; ++i)
                    if (i == node || m_TowerPoints[node].state == HS_CAPTURE_ALLIANCE )
                        countForTeam++;

                // Award alliance rep and kill credit
                if( OPvPCapturePoint *cp = GetCapturePoint( m_TowerPoints[node].capturepoint ) )
                {
                    cp->RewardReputationToTeam(1050, 5, ALLIANCE);
                    if (countForTeam == HS_CAPTURE_NUM)
                        cp->SendObjectiveCompleteByTeam(OPVP_HS_KCREDIT_3RDTOWER, 0, ALLIANCE );
                    cp->SendObjectiveCompleteByTeam( m_TowerPoints[node].killcredit, 0, ALLIANCE );
                }

                SendMessageToAll( "The Alliance have claimed %s tower!", m_TowerPoints[node].name.c_str() );
                m_TowerPoints[node].teamIndex = 1;
            }
            break;
        case HS_CAPTURE_ALLIANCE_CONT:
            banner = 850027;
            break;
        case HS_CAPTURE_HORDE:
            {
                banner = 850028;

                for (uint32 i = 0; i < HS_CAPTURE_NUM; ++i)
                    if ( i == node || m_TowerPoints[node].state == HS_CAPTURE_HORDE )
                        countForTeam++;

                // Award horde rep and kill credit
                if( OPvPCapturePoint *cp = GetCapturePoint( m_TowerPoints[node].capturepoint ) )
                {
                    cp->RewardReputationToTeam(1085, 5, HORDE);
                    if (countForTeam == HS_CAPTURE_NUM)
                        cp->SendObjectiveCompleteByTeam(OPVP_HS_KCREDIT_3RDTOWER, 0, HORDE);
                    cp->SendObjectiveCompleteByTeam( m_TowerPoints[node].killcredit, 0, HORDE );
                }

                SendMessageToAll( "The Horde have claimed %s tower!", m_TowerPoints[node].name.c_str() );
                m_TowerPoints[node].teamIndex = 2;
            }
            break;
        case HS_CAPTURE_HORDE_CONT:
            banner = 850029;
            break;
    }

    // Spawn the new one..
    m_TowerPoints[node].gameobject = 0;
    if ( uint64 guid = objmgr.AddGOData(banner, HSCapturePoints[node].map, HSCapturePoints[node].x, HSCapturePoints[node].y, HSCapturePoints[node].z - 0.2f, HSCapturePoints[node].o, 99999999999, 0, 0, 0, 0) )
    {
        sLog->outString( "Hillsbrad : Spawned Banner(%u) at node %u.", guid, node);
        m_TowerPoints[node].gameobject = guid;
    }
}

void OutdoorPvPHS::ApplyZoneBalanceBuff()
{
    //sLog->outString("HillsbradMGR : Applying Tenacity. %u (%u) Horde. %u (%u) Alliance.", m_HordeBuff, m_HordeCount, m_AllianceBuff, m_AllianceCount);
    for (int i = 0; i <= 1; ++i)
    {
        for (PlayerSet::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
        {
            Player * plr = *itr;
            {	/*
                //plr->RemoveAurasDueToSpell( HS_SPELL_TENACITY );
                if( plr->GetTeam() == ALLIANCE )
                {
                    if( m_AllianceBuff > 0 )
                        plr->SetAuraStack( HS_SPELL_TENACITY, plr, m_AllianceBuff );
                }
                else
                {
                    if( m_HordeBuff > 0 )
                        plr->SetAuraStack( HS_SPELL_TENACITY, plr, m_HordeBuff );
                } */
                // New player protection, fuck a new timer... WILL ADD WHEN REQUIRED.
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

// On creature spawn

void OutdoorPvPHS::OnCreatureCreate(Creature* creature, bool add)
{
    //OutdoorPvP::OnCreatureCreate(creature, add);
    if ( (creature->GetEntry() == HS_CREATURE_ENTRY_A_SPIRITGUIDE || creature->GetEntry() == HS_CREATURE_ENTRY_H_SPIRITGUIDE) && creature->GetZoneId() == HS_ZONE )
    {
        if (add)
        {
            creature->setDeathState(DEAD);
            creature->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, creature->GetGUID());
            creature->SetUInt32Value(UNIT_CHANNEL_SPELL, HS_SPELL_SPIRIT_HEAL_CHANNEL);
            creature->SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);
        }
    }
}

void OutdoorPvPHS::OnGameObjectCreate(GameObject* obj, bool add)
{
    if( !m_map )
        m_map = obj->GetMap();
    OutdoorPvP::OnGameObjectCreate(obj, add);
}

// Resurrection System

void OutdoorPvPHS::SendAreaSpiritHealerQueryOpcode(Player* pl, const uint64& guid)
{
    WorldPacket data(SMSG_AREA_SPIRIT_HEALER_TIME, 12);
    uint32 time_ = HS_RESURRECTION_INTERVAL - GetLastResurrectTime(); // resurrect every HS_RESURRECTION_INTERVAL / 1000 seconds
    if (time_ == uint32(-1))
        time_ = 0;
    data << guid << time_;
    pl->GetSession()->SendPacket(&data);
}

void OutdoorPvPHS::AddPlayerToResurrectQueue(uint64 npc_guid, uint64 player_guid)
{
    m_ReviveQueue[npc_guid].push_back(player_guid);

    Player* plr = ObjectAccessor::FindPlayer(player_guid);
    if (!plr)
        return;

    plr->CastSpell(plr, HS_SPELL_WAITING_FOR_RESURRECT, true);
}

void OutdoorPvPHS::RemovePlayerFromResurrectQueue(uint64 player_guid)
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

                plr->RemoveAurasDueToSpell( HS_SPELL_WAITING_FOR_RESURRECT );

                return;
            }
        }
    }
}

// Shit

void OutdoorPvPHS::SendMessageToAll(const char *format, ... )
{
    if(format)
    {
        va_list ap;
        char message [1024];
        message[0] = '\0';
        va_start(ap, format);
        vsnprintf( message, 1024, format, ap );
        va_end(ap);
        for (int i = 0; i <= 1; ++i)
        {
            for (PlayerSet::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
            {
                Player * plr = *itr;
                {
                    ChatHandler(plr).PSendSysMessage( message );
                }
            }
        }
    }
}


void OPvPCapturePointHS::ChangeState()
{
    uint32 field = 0;
    switch (m_OldState)
    {
    case OBJECTIVESTATE_NEUTRAL:
        field = HS_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_ALLIANCE:
        field = HS_MAP_A[m_TowerType];
        if (uint32 alliance_towers = ((OutdoorPvPHS*)m_PvP)->GetAllianceTowersControlled())
            ((OutdoorPvPHS*)m_PvP)->SetAllianceTowersControlled(--alliance_towers);
        sWorld->SendZoneText(OutdoorPvPHSBuffZones[0], sObjectMgr->GetTrinityStringForDBCLocale(HS_LANG_LOSE_A[m_TowerType]));
        break;
    case OBJECTIVESTATE_HORDE:
        field = HS_MAP_H[m_TowerType];
        if (uint32 horde_towers = ((OutdoorPvPHS*)m_PvP)->GetHordeTowersControlled())
            ((OutdoorPvPHS*)m_PvP)->SetHordeTowersControlled(--horde_towers);
        sWorld->SendZoneText(OutdoorPvPHSBuffZones[0], sObjectMgr->GetTrinityStringForDBCLocale(HS_LANG_LOSE_H[m_TowerType]));
        break;
    case OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE:
        field = HS_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE:
        field = HS_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE:
        field = HS_MAP_A[m_TowerType];
        break;
    case OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE:
        field = HS_MAP_H[m_TowerType];
        break;
    }

    // send world state update
    if (field)
    {
        m_PvP->SendUpdateWorldState(field, 0);
        field = 0;
    }
    uint32 artkit = 21;
    uint32 artkit2 = HS_TowerArtKit_N[m_TowerType];
    switch (m_State)
    {
    case OBJECTIVESTATE_NEUTRAL:
        field = HS_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_ALLIANCE:
    {
        field = HS_MAP_A[m_TowerType];
        artkit = 2;
        artkit2 = HS_TowerArtKit_A[m_TowerType];
        uint32 alliance_towers = ((OutdoorPvPHS*)m_PvP)->GetAllianceTowersControlled();
        if (alliance_towers < 3)
            ((OutdoorPvPHS*)m_PvP)->SetAllianceTowersControlled(++alliance_towers);
        sWorld->SendZoneText(OutdoorPvPHSBuffZones[0], sObjectMgr->GetTrinityStringForDBCLocale(HS_LANG_CAPTURE_A[m_TowerType]));
        break;
    }
    case OBJECTIVESTATE_HORDE:
    {
        field = HS_MAP_H[m_TowerType];
        artkit = 1;
        artkit2 = HS_TowerArtKit_H[m_TowerType];
        uint32 horde_towers = ((OutdoorPvPHS*)m_PvP)->GetHordeTowersControlled();
        if (horde_towers < 3)
            ((OutdoorPvPHS*)m_PvP)->SetHordeTowersControlled(++horde_towers);
        sWorld->SendZoneText(OutdoorPvPHSBuffZones[0], sObjectMgr->GetTrinityStringForDBCLocale(HS_LANG_CAPTURE_H[m_TowerType]));
        break;
    }
    case OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE:
        field = HS_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE:
        field = HS_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE:
        field = HS_MAP_A[m_TowerType];
        artkit = 2;
        artkit2 = HS_TowerArtKit_A[m_TowerType];
        break;
    case OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE:
        field = HS_MAP_H[m_TowerType];
        artkit = 1;
        artkit2 = HS_TowerArtKit_H[m_TowerType];
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
        SendObjectiveComplete(HS_CREDITMARKER[m_TowerType], 0);
}
void OPvPCapturePointHS::SendChangePhase()
{
    SendUpdateWorldState(HS_UI_TOWER_SLIDER_N, m_neutralValuePct);
    // send these updates to only the ones in this objective
    uint32 phase = (uint32)ceil((m_value + m_maxValue) / (2 * m_maxValue) * 100.0f);
    SendUpdateWorldState(HS_UI_TOWER_SLIDER_POS, phase);
    // send this too, sometimes the slider disappears, dunno why :(
    SendUpdateWorldState(HS_UI_TOWER_SLIDER_DISPLAY, 1);
}

void OPvPCapturePointHS::FillInitialWorldStates(WorldPacket &data)
{
	switch (m_State)
    {
        case OBJECTIVESTATE_ALLIANCE:
        case OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE:
            data << uint32(HS_MAP_N[m_TowerType]) << uint32(0);
            data << uint32(HS_MAP_A[m_TowerType]) << uint32(1);
            data << uint32(HS_MAP_H[m_TowerType]) << uint32(0);
            break;
        case OBJECTIVESTATE_HORDE:
        case OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE:
            data << uint32(HS_MAP_N[m_TowerType]) << uint32(0);
            data << uint32(HS_MAP_A[m_TowerType]) << uint32(0);
            data << uint32(HS_MAP_H[m_TowerType]) << uint32(1);
            break;
        case OBJECTIVESTATE_NEUTRAL:
        case OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE:
        case OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE:
        default:
            data << uint32(HS_MAP_N[m_TowerType]) << uint32(1);
            data << uint32(HS_MAP_A[m_TowerType]) << uint32(0);
            data << uint32(HS_MAP_H[m_TowerType]) << uint32(0);
            break;
    }
}

bool OPvPCapturePointHS::HandlePlayerEnter(Player* plr)
{
    if (OPvPCapturePoint::HandlePlayerEnter(plr))
    {
        plr->SendUpdateWorldState(HS_UI_TOWER_SLIDER_DISPLAY, 1);
        uint32 phase = (uint32)ceil((m_value + m_maxValue) / (2 * m_maxValue) * 100.0f);
        plr->SendUpdateWorldState(HS_UI_TOWER_SLIDER_POS, phase);
        plr->SendUpdateWorldState(HS_UI_TOWER_SLIDER_N, m_neutralValuePct);
        return true;
    }
    return false;
}

void OPvPCapturePointHS::HandlePlayerLeave(Player* plr)
{
    plr->SendUpdateWorldState(HS_UI_TOWER_SLIDER_DISPLAY, 0);
    OPvPCapturePoint::HandlePlayerLeave(plr);
}

void OutdoorPvPHS::HandleKillImpl(Player* player, Unit* killed)
{
    if (killed->GetTypeId() != TYPEID_PLAYER)
        return;

    if (player->GetTeam() == ALLIANCE && killed->ToPlayer()->GetTeam() != ALLIANCE)
        player->CastSpell(player, AlliancePlayerKillReward, true);
    else if (player->GetTeam() == HORDE && killed->ToPlayer()->GetTeam() != HORDE)
        player->CastSpell(player, HordePlayerKillReward, true);
}

uint32 OutdoorPvPHS::GetAllianceTowersControlled() const
{
    return m_AllianceTowersControlled;
}

void OutdoorPvPHS::SetAllianceTowersControlled(uint32 count)
{
    m_AllianceTowersControlled = count;
}

uint32 OutdoorPvPHS::GetHordeTowersControlled() const
{
    return m_HordeTowersControlled;
}

void OutdoorPvPHS::SetHordeTowersControlled(uint32 count)
{
    m_HordeTowersControlled = count;
}

class OutdoorPvP_hellscream : public OutdoorPvPScript
{
    public:

        OutdoorPvP_hellscream()
            : OutdoorPvPScript("outdoorpvp_hs")
        {
        }

        OutdoorPvP* GetOutdoorPvP() const
        {
            return new OutdoorPvPHS();
        }
};

void AddSC_outdoorpvp_hs()
{
    new OutdoorPvP_hellscream();
}