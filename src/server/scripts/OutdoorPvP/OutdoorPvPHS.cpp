/*
 * Copyright (C) 2010 Wowtorn <http://www.wowtorn.net/>
 */

#include "OutdoorPvPHS.h"
#include "OutdoorPvPMgr.h"
#include "OutdoorPvP.h"
#include "WorldPacket.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "Language.h"
#include "World.h"


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

    m_DurnholdeOwner = 0;
    m_DurnholdeLockout = 0;

    m_TowerStateEast = 0;
    m_TowerStateWest = 0;
    m_TowerStateMain = 0;

    RegisterZone(HS_ZONE);
    
    AddCapturePoint(new OPvPCapturePointHS(this,HS_TOWER_LOWER));

    AddCapturePoint(new OPvPCapturePointHS(this,HS_TOWER_LOWER_EAST));

    AddCapturePoint(new OPvPCapturePointHS(this,HS_TOWER_MAIN));

    sLog->outString("HillsbradMGR : Loaded.");

    return true;
}

void OutdoorPvPHS::FillInitialWorldStates(WorldPacket &data)
{
    sLog->outString("Send initial world states...");
    data << uint32(2473) << uint32(0);
    data << uint32(2474) << uint32(50);
    data << uint32(2475) << uint32(100);

    data << 0xa3e << 2;
    data << 0xa3d << 1;
    data << 0xa3c << uint32(0);

    for (OPvPCapturePointMap::iterator itr = m_capturePoints.begin(); itr != m_capturePoints.end(); ++itr)
    {
        itr->second->FillInitialWorldStates(data);
    }
}

void OutdoorPvPHS::SendRemoveWorldStates(Player * plr)
{
    return;
}

bool OutdoorPvPHS::Update(uint32 diff)
{
    bool changed = OutdoorPvP::Update(diff);
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
	
    if( m_ChestGUID == 0 )
    {
        if( m_ChestTimer < diff )
        {
            uint32 ffachest = 0;
            ffachest = urand(0, 9);
            if( uint32 guid = sObjectMgr->AddGOData(HSChestPoints[ffachest].entry, HSChestPoints[ffachest].map, HSChestPoints[ffachest].x, HSChestPoints[ffachest].y, HSChestPoints[ffachest].z, HSChestPoints[ffachest].o, 99999999999, 0, 0, 0, 0) )
			{
                //sLog->outString( "Hillsbrad : Spawned Chest(%u) at location %u.", guid,  ffachest);
                m_ChestGUID = guid;
                SendMessageToAll( "FFA chest has been spawned in the fields. Good luck!" );
				sLog->outString( "Hillsbrad : Spawned Chest(%u) at location %u.", m_ChestGUID,  ffachest);
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
    } 
    // Chest debug. 
    if( m_ChestTimer < diff )
    {
        sLog->outString( "HillsbradMGR : Chest Guid (%u), Timer (%u), Announce Timer (%u).", m_ChestGUID, m_ChestTimer, m_ChestAnnounceTimer );
        m_ChestDebugTimer = 60000;
    }
    else
        m_ChestDebugTimer -= diff;
	/*
    // Player scale checker.
    if( m_TenacityTimer <= diff )
    {
        float ratio = 0;
        uint32 stack = 0;
        
        // Reset the stack count
        m_HordeBuff = 0;
        m_AllianceBuff = 0;
        
        if( m_HordeCount > 0 && m_AllianceCount > 0 )
        {
            if( m_AllianceCount > m_HordeCount )
                ratio = m_AllianceCount / m_HordeCount;
            else
                ratio = m_HordeCount / m_AllianceCount;
        }

        if( ratio >= 1.25f )
        { // Buff for lower team
            stack = ratio * ratio;
            if( stack > 20 )
                stack = 20;
            if( m_HordeCount < m_AllianceCount )
                m_HordeBuff = stack;
            else
                m_AllianceBuff = stack;
        }
        // Apply the buff to players.
        ApplyZoneBalanceBuff();
        m_TenacityTimer = HS_TENACITY_TIME;
    } else
        m_TenacityTimer -= diff; 
	*/
	ApplyZoneBalanceBuff();
    return changed;
}

void OutdoorPvPHS::HandlePlayerEnterZone(Player * plr, uint32 zone)
{
    // Remove Tenacity
    //plr->RemoveAurasDueToSpell( HS_SPELL_TENACITY );

    // Update the player count.
    m_PlayerCount++;
    // Update team count.
    if( plr->GetTeam() == ALLIANCE )
        m_AllianceCount++;
    else
        m_HordeCount++;

    // Just in case. Make them honorless.
    plr->AddAura( HS_SPELL_HONORLESS, plr );

    OutdoorPvP::HandlePlayerEnterZone(plr,zone);
}

void OutdoorPvPHS::HandlePlayerLeaveZone(Player * plr, uint32 zone)
{
    // Remove Tenacity
    //plr->RemoveAurasDueToSpell( HS_SPELL_TENACITY );

    // Update the player count.
    if( m_PlayerCount > 0 )
        m_PlayerCount--;
    // Update team count.
    if( plr->GetTeam() == ALLIANCE )
        m_AllianceCount--;
    else
        m_HordeCount--;

    OutdoorPvP::HandlePlayerLeaveZone(plr, zone);
}

void OutdoorPvPHS::HandlePlayerResurrects(Player * plr, uint32 zone)
{
    plr->AddAura( HS_SPELL_HONORLESS, plr );
}

bool OutdoorPvPHS::HandleOpenGo(Player* plr, uint64 guid)
{
    sLog->outString("HillsbradMGR: Using %u.", guid);
    if(GameObject* obj = plr->GetMap()->GetGameObject(guid))
    {
        // Is this the chest?
        if(obj->GetGUIDLow() == m_ChestGUID)
        {
            m_ChestGUID = 0;
            SendMessageToAll("%s has claimed the chest! The next chest will appear in an hour.", plr->GetName());
            return false;
        }

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
        }
    }
    return false;
}

 void OutdoorPvPHS::ApplyZoneBalanceBuff()
{
    //sLog->outString("HillsbradMGR : Applying Tenacity. %u (%u) Horde. %u (%u) Alliance.", m_HordeBuff, m_HordeCount, m_AllianceBuff, m_AllianceCount);
    for (int i = 0; i <= 1; ++i)
    {
        for (PlayerSet::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
        {
            Player * plr = *itr;
            {
                //plr->RemoveAurasDueToSpell( HS_SPELL_TENACITY );
                // if( plr->GetTeam() == ALLIANCE )
                // {
                    // if( m_AllianceBuff > 0 )
                        // plr->SetAuraStack( HS_SPELL_TENACITY, plr, m_AllianceBuff );
                // }
                // else
                // {
                    // if( m_HordeBuff > 0 )
                        // plr->SetAuraStack( HS_SPELL_TENACITY, plr, m_HordeBuff );
                // }
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

void OutdoorPvPHS::OnCreatureCreate(Creature *creature, bool add)
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

void OutdoorPvPHS::OnGameObjectCreate(GameObject* go, bool add)
{

    sLog->outString("yeap, %u added.", go->GetEntry());
    //OutdoorPvP::OnGameObjectCreate(go, add);
}

// Resurrection System

void OutdoorPvPHS::SendAreaSpiritHealerQueryOpcode(Player *pl, const uint64& guid)
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
        for (std::vector<uint64>::iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); ++itr2)
        {
            if (*itr2 == player_guid)
            {
                (itr->second).erase(itr2);
                if (Player* plr = ObjectAccessor::FindPlayer(player_guid))
                    plr->RemoveAurasDueToSpell(HS_SPELL_WAITING_FOR_RESURRECT);
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

void OutdoorPvPHS::SetJustCaptured(uint32 team)
{
    // Change any points over to team.
    for (OPvPCapturePointMap::iterator itr = m_capturePoints.begin(); itr != m_capturePoints.end(); ++itr)
    {
        if ( !itr->second )
            continue;
        // Okay.. -.-
        switch( team )
        {
            case ALLIANCE:
                if( itr->second->m_State != OBJECTIVESTATE_ALLIANCE )
                {
                    itr->second->m_value = itr->second->m_maxValue;
                    itr->second->m_OldState = itr->second->m_State;
                    itr->second->m_State = OBJECTIVESTATE_ALLIANCE;
                    itr->second->m_team  = TEAM_ALLIANCE;
                    itr->second->ChangeTeam(itr->second->m_team);
                    itr->second->ChangeState();
                }
                break;
            case HORDE:
                if( itr->second->m_State != OBJECTIVESTATE_HORDE )
                {
                    itr->second->m_value = 0 - itr->second->m_maxValue;
                    itr->second->m_OldState = itr->second->m_State;
                    itr->second->m_State = OBJECTIVESTATE_HORDE;
                    itr->second->m_team = TEAM_HORDE;
                    itr->second->ChangeTeam(itr->second->m_team);
                    itr->second->ChangeState();
                }
                break;
        }
    }
    // Okay, now the lock out timer... 

    // Spawns.

    // ETC.
}

void OutdoorPvPHS::CheckSetDurnholdeMain()
{
    // Different rules for neutral... Open to anyone...
    if( m_DurnholdeOwner == 0 )
    {
        m_TowerStateMain = 0;
        return;
    }
    // Same teams?
    if( m_TowerStateWest == m_TowerStateMain )
    {
        // Who is the owner of this place?
        if( m_DurnholdeOwner == m_TowerStateWest )
        {
            // Lock it.
            m_TowerStateMain = 1;
            return;
        }
    }
    // Unlock it be default. 
    if( m_TowerStateMain > 0 )
        m_TowerStateMain = 0;
}

// Capture Points

OPvPCapturePointHS::OPvPCapturePointHS(OutdoorPvP *pvp, OutdoorPvPHSTowerType type)
: OPvPCapturePoint(pvp), m_TowerType(type)
{
    SetCapturePointData(
        HSCapturePoints[type].entry,
        HSCapturePoints[type].map,
        HSCapturePoints[type].x,
        HSCapturePoints[type].y,
        HSCapturePoints[type].z,
        HSCapturePoints[type].o,
        HSCapturePoints[type].rot0,
        HSCapturePoints[type].rot1,
        HSCapturePoints[type].rot2,
        HSCapturePoints[type].rot3
    );

    RespawnVisualTower( 184382 );
}

bool OPvPCapturePointHS::Update(uint32 diff)
{
    // can update even in locked state if gathers the controlling faction
   // bool canupdate = ((((OutdoorPvPTF*)m_PvP)->m_AllianceTowersControlled > 0) && m_activePlayers[0].size() > m_activePlayers[1].size()) ||
      //      ((((OutdoorPvPTF*)m_PvP)->m_HordeTowersControlled > 0) && m_activePlayers[0].size() < m_activePlayers[1].size());
    // if gathers the other faction, then only update if the pvp is unlocked
    //canupdate = canupdate || !((OutdoorPvPTF*)m_PvP)->m_IsLocked;

    // Capture Point?
    if( !m_capturePoint )
        return false;

    // Can main tower change states?
    if( m_capturePoint->GetEntry() == 850002 && ((OutdoorPvPHS*)m_PvP)->m_TowerStateMain > 0 )
        return false;

    return /* canupdate &&  */ OPvPCapturePoint::Update(diff);
}


void OPvPCapturePointHS::ChangeState()
{

    sLog->outString("%u changed states from %u to %u.", m_capturePoint->GetEntry(), m_OldState, m_State);
    
    switch( m_capturePoint->GetEntry() )
    {
        case 850000: // Durnholde - Lower West Tower
            switch(m_State)
            {
                case OBJECTIVESTATE_HORDE:
                    ((OutdoorPvPHS*)m_PvP)->SendMessageToAll("Horde have taken the lower west of Durnholde Keep!");
                    RespawnVisualTower( 184380 );
                    ((OutdoorPvPHS*)m_PvP)->m_TowerStateWest = HORDE;
                break;
                case OBJECTIVESTATE_ALLIANCE:
                    ((OutdoorPvPHS*)m_PvP)->SendMessageToAll("Alliance have taken the lower west of Durnholde Keep!");
                    RespawnVisualTower( 184381 );
                    ((OutdoorPvPHS*)m_PvP)->m_TowerStateWest = ALLIANCE;
                break;
                case OBJECTIVESTATE_NEUTRAL:
                case OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE:
                case OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE:
                    RespawnVisualTower( 184382 );
                break;
            }
            ((OutdoorPvPHS*)m_PvP)->CheckSetDurnholdeMain();
        break;
        case 850001: // Durnholde - Lower East Tower
            switch(m_State)
            {
                case OBJECTIVESTATE_HORDE:
                    ((OutdoorPvPHS*)m_PvP)->SendMessageToAll("Horde have taken the lower east of Durnholde Keep!");
                    RespawnVisualTower( 184380 );
                    ((OutdoorPvPHS*)m_PvP)->m_TowerStateEast = HORDE;
                break;
                case OBJECTIVESTATE_ALLIANCE:
                    ((OutdoorPvPHS*)m_PvP)->SendMessageToAll("Alliance have taken the lower east of Durnholde Keep!");
                    RespawnVisualTower( 184381 );
                    ((OutdoorPvPHS*)m_PvP)->m_TowerStateEast = ALLIANCE;
                break;
                case OBJECTIVESTATE_NEUTRAL:
                case OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE:
                case OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE:
                    RespawnVisualTower( 184382 );
                break;
            }
            ((OutdoorPvPHS*)m_PvP)->CheckSetDurnholdeMain();
        break;
        case 850002: // Durnholde - Main Tower
            switch(m_State)
            {
                case OBJECTIVESTATE_HORDE:
                    RespawnVisualTower( 184380 );
                    // Need to check if was previous owner etc.
                    if( ((OutdoorPvPHS*)m_PvP)->m_DurnholdeOwner != HORDE )
                    {
                        ((OutdoorPvPHS*)m_PvP)->SendMessageToAll("Durnholde Keep has been captured by the Horde!");
                        ((OutdoorPvPHS*)m_PvP)->m_DurnholdeOwner = HORDE;
                        ((OutdoorPvPHS*)m_PvP)->SetJustCaptured( HORDE );
                    }
                    else
                        ((OutdoorPvPHS*)m_PvP)->SendMessageToAll("Durnholde Keep has been resecured by the Horde!");
                break;
                case OBJECTIVESTATE_ALLIANCE:
                    RespawnVisualTower( 184381 );
                    if( ((OutdoorPvPHS*)m_PvP)->m_DurnholdeOwner != ALLIANCE )
                    {
                        ((OutdoorPvPHS*)m_PvP)->SendMessageToAll("Durnholde Keep has been captured by the Alliance!");
                        ((OutdoorPvPHS*)m_PvP)->m_DurnholdeOwner = ALLIANCE;
                        ((OutdoorPvPHS*)m_PvP)->SetJustCaptured( ALLIANCE );
                    }
                    else
                        ((OutdoorPvPHS*)m_PvP)->SendMessageToAll("Durnholde Keep has been resecured by the Alliance!");
                break;
                case OBJECTIVESTATE_NEUTRAL:
                case OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE:
                case OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE:
                    RespawnVisualTower( 184382 );
                break;
            }
        break;
    }
/*
    switch(m_OldState)
    {
    case OBJECTIVESTATE_NEUTRAL:
        //field = HP_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_ALLIANCE:
       // field = HP_MAP_A[m_TowerType];
       // if (((OutdoorPvPHP*)m_PvP)->m_AllianceTowersControlled)
       //     ((OutdoorPvPHP*)m_PvP)->m_AllianceTowersControlled--;
       // sWorld.SendZoneText(267,objmgr.GetTrinityStringForDBCLocale(HP_LANG_LOOSE_A[m_TowerType]));
        sLog->outString("ALLIANCE!");
        break;
    case OBJECTIVESTATE_HORDE:
       // field = HP_MAP_H[m_TowerType];
       // if (((OutdoorPvPHP*)m_PvP)->m_HordeTowersControlled)
       //     ((OutdoorPvPHP*)m_PvP)->m_HordeTowersControlled--;
        //sWorld.SendZoneText(OutdoorPvPHPBuffZones[0],objmgr.GetTrinityStringForDBCLocale(HP_LANG_LOOSE_H[m_TowerType]));
        //sLog->outString("HORDE!");
        break;
    case OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE:
       // field = HP_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE:
        //field = HP_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE:
       // field = HP_MAP_A[m_TowerType];
        break;
    case OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE:
      //  field = HP_MAP_H[m_TowerType];
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
    switch(m_State)
    {
    case OBJECTIVESTATE_NEUTRAL:
        field = HP_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_ALLIANCE:
        field = HP_MAP_A[m_TowerType];
        artkit = 2;
        artkit2 = HP_TowerArtKit_A[m_TowerType];
        if (((OutdoorPvPHP*)m_PvP)->m_AllianceTowersControlled<3)
            ((OutdoorPvPHP*)m_PvP)->m_AllianceTowersControlled++;
        sWorld.SendZoneText(OutdoorPvPHPBuffZones[0],objmgr.GetTrinityStringForDBCLocale(HP_LANG_CAPTURE_A[m_TowerType]));
        break;
    case OBJECTIVESTATE_HORDE:
        field = HP_MAP_H[m_TowerType];
        artkit = 1;
        artkit2 = HP_TowerArtKit_H[m_TowerType];
        if (((OutdoorPvPHP*)m_PvP)->m_HordeTowersControlled<3)
            ((OutdoorPvPHP*)m_PvP)->m_HordeTowersControlled++;
        sWorld.SendZoneText(OutdoorPvPHPBuffZones[0],objmgr.GetTrinityStringForDBCLocale(HP_LANG_CAPTURE_H[m_TowerType]));
        break;
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
        SendObjectiveComplete(HP_CREDITMARKER[m_TowerType], 0); */
}

void OPvPCapturePointHS::SendChangePhase()
{
   // SendUpdateWorldState(2475, m_neutralValuePct);
    // send these updates to only the ones in this objective
    //uint32 phase = (uint32)ceil((m_value + m_maxValue) / (2 * m_maxValue) * 100.0f);
   // SendUpdateWorldState(2474, phase);
    // send this too, sometimes the slider disappears, dunno why :(
   // SendUpdateWorldState(2473, 1);
    OPvPCapturePoint::SendChangePhase();
}

void OPvPCapturePointHS::FillInitialWorldStates(WorldPacket &data)
{
}

bool OPvPCapturePointHS::HandlePlayerEnter(Player *plr)
{
    sLog->outString("Hillsbrad Point : Player %u entered %u radius.", plr->GetGUIDLow(), m_capturePoint->GetEntry() );
    return OPvPCapturePoint::HandlePlayerEnter(plr);
   // {
    //    plr->SendUpdateWorldState(2473, 1);
    //    uint32 phase = (uint32)ceil((m_value + m_maxValue) / (2 * m_maxValue) * 100.0f);
    //    plr->SendUpdateWorldState(2474, phase);
   //     plr->SendUpdateWorldState(2475, m_neutralValuePct);
   //     return true;
   // }
   // return false;
}

void OPvPCapturePointHS::HandlePlayerLeave(Player *plr)
{
    plr->SendUpdateWorldState(2473, 0);
    OPvPCapturePoint::HandlePlayerLeave(plr);
}

void OPvPCapturePointHS::RespawnVisualTower(uint32 entry)
{
    DeleteSpawns();
    AddObject(m_TowerType, entry,
    HSCapturePoints[m_TowerType].map,
    HSCapturePoints[m_TowerType].x,
    HSCapturePoints[m_TowerType].y,
    HSCapturePoints[m_TowerType].z,
    HSCapturePoints[m_TowerType].o,
    HSCapturePoints[m_TowerType].rot0,
    HSCapturePoints[m_TowerType].rot1,
    HSCapturePoints[m_TowerType].rot2,
    HSCapturePoints[m_TowerType].rot3);
}

void OPvPCapturePointHS::DeleteSpawns()
{
    for (std::map<uint32,uint64>::iterator i = m_Objects.begin(); i != m_Objects.end(); ++i)
        DelObject(i->first);
    for (std::map<uint32,uint64>::iterator i = m_Creatures.begin(); i != m_Creatures.end(); ++i)
        DelCreature(i->first);
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