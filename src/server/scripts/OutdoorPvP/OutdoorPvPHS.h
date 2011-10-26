/*
 * Copyright (C) 2010 Wowtorn <http://www.wowtorn.net/>
 */

#ifndef OUTDOOR_PVP_HS_
#define OUTDOOR_PVP_HS_

#include "OutdoorPvP.h"

#define OutdoorPvPHSBuffZonesNum 1
                                                         //  HP, citadel, ramparts, blood furnace, shattered halls, mag's lair
const uint32 OutdoorPvPHSBuffZones[OutdoorPvPHSBuffZonesNum] = { 267 };

enum HS_Outdoor_Flag_SpellId
{

    HS_SPELL_ADRENALINE        = 68667,

    //HS_SPELL_TENACITY          = 58549,
    HS_SPELL_HONORLESS         = 2479,

    HS_SPELL_WAITING_FOR_RESURRECT     = 2584,                 // Waiting to Resurrect
    HS_SPELL_SPIRIT_HEAL_CHANNEL       = 22011,                // Spirit Heal Channel
    HS_SPELL_SPIRIT_HEAL               = 22012,                // Spirit Heal
    HS_SPELL_RESURRECTION_VISUAL       = 24171,                // Resurrection Impact Visual
    HS_SPELL_SPIRIT_HEAL_MANA          = 44535,                // Spirit Heal
};

enum HS_Timers
{
    HS_TENACITY_TIME                    = 60000,
    HS_RESURRECTION_INTERVAL            = 30000,
    HS_FFA_CHEST_TIMER                  = 3600000,
    HS_FFA_CHEST_ANNOUNCE_TIMER         = 1000,
};

enum HS_Creatures
{
    HS_CREATURE_ENTRY_A_SPIRITGUIDE     = 13116,           // alliance
    HS_CREATURE_ENTRY_H_SPIRITGUIDE     = 13117,           // horde
};

#define HS_ZONE          267

enum OutdoorPvPHSSpells
{
    AlliancePlayerKillReward = 32155,
    HordePlayerKillReward = 32158,
    AllianceBuff = 32071,
    HordeBuff = 32049
};

enum OutdoorPvPHSTowerType
{
    HS_TOWER_SOUTH = 0,
    HS_TOWER_DARROW = 1,
    HS_TOWER_NETHANDER = 2,
    HS_TOWER_NUM = 3
};

const uint32 HS_CREDITMARKER[HS_TOWER_NUM] = {19032, 19028, 19029};

const uint32 HS_CapturePointEvent_Enter[HS_TOWER_NUM] = {11404, 11396, 11388};

const uint32 HS_CapturePointEvent_Leave[HS_TOWER_NUM] = {11403, 11395, 11387};

enum OutdoorPvPHPWorldStates
{
    HS_UI_TOWER_DISPLAY_A = 0x9ba,
    HS_UI_TOWER_DISPLAY_H = 0x9b9,

    HS_UI_TOWER_COUNT_H = 0x9ae,
    HS_UI_TOWER_COUNT_A = 0x9ac,

    HS_UI_TOWER_SLIDER_N = 2475,
    HS_UI_TOWER_SLIDER_POS = 2474,
    HS_UI_TOWER_SLIDER_DISPLAY = 2473
};

const uint32 HS_MAP_N[HS_TOWER_NUM] = {0x9b5, 0x9b2, 0x9a8};

const uint32 HS_MAP_A[HS_TOWER_NUM] = {0x9b3, 0x9b0, 0x9a7};

const uint32 HS_MAP_H[HS_TOWER_NUM] = {0x9b4, 0x9b1, 0x9a6};

const uint32 HS_TowerArtKit_A[HS_TOWER_NUM] = {65, 62, 67};

const uint32 HS_TowerArtKit_H[HS_TOWER_NUM] = {64, 61, 68};

const uint32 HS_TowerArtKit_N[HS_TOWER_NUM] = {66, 63, 69};

const go_type HSCapturePoints[HS_TOWER_NUM] =
{
    {182175, 0, -659.569f, 412.013f, 83.3579f, 3.746807f, 0.0f, 0.0f, 0.087156f, 0.996195f},      // 0 - Southpoint Tower
    {182174, 0, -323.441f, -698.09f, 57.9673f, 1.92390f, 0.0f, 0.0f, 0.008727f, -0.999962f},      // 1 - Darrow Hill Tower
    {182173, 0, -622.047f, -1045.131f, 65.926140f, 3.309312f, 0.0f, 0.0f, 0.017452f, 0.999848f}   // 2 - Nethander Stead Tower
};

const go_type HSTowerFlags[HS_TOWER_NUM] =
{
    {183515, 0, -677.897f, -426.711f, 25.766f, 0.10695f, 0.0f, 0.0f, 1.0f, 0.0f},              // 0 - Southpoint Tower
    {183515, 0, -339.712f, -708.957f, 54.033f, 4.45808f, 0.0f, 0.0f, 0.999962f, 0.008727f},    // 1 - Darrow Hill Tower
    {183515, 0, -606.495f, -1064.064f, 56.5104f, 5.86003f, 0.0f, 0.0f, 0.999962f, -0.008727f}  // 2 - Nethander Stead Tower
};

// FFA Chest Spawns.

const go_type HSChestPoints[10] = {
    {850015,0,-594.745361f,-51.738495f,45.944431f,0.174533f,0.0f,0.0f,0.0f,0.0f},      // 0 - Young Field
    {850015,0,-466.411224f,-42.806705f,54.477070f,0.000000f,0.0f,0.0f,0.0f,0.0f},      // 1 - Middle Field
    {850015,0,-422.661194f,106.235573f,54.188080f,5.424229f,0.0f,0.0f,0.0f,0.0f},      // 2 - Upper Field
    {850015,0,-565.477844f,53.209713f,48.447845f,0.216379f,0.0f,0.0f,0.0f,0.0f},       // 3 - Blacksmith
    {850015,0,-350.670868f,-11.088767f,55.021141f,1.549651f,0.0f,0.0f,0.0f,0.0f},      // 4 - Flowers
    {850015,0,-396.697510f,-56.452023f,54.452023f,3.887430f,0.0f,0.0f,0.0f,0.0f},      // 5 - Barn
    {850015,0,-548.236938f,-105.017731f,53.077545f,1.641531f,0.0f,0.0f,0.0f,0.0f},     // 6 - House Roof
    {850015,0,-457.335114f,92.229897f,58.199024f,4.366654f,0.0f,0.0f,0.0f,0.0f},       // 7 - Cart
    {850015,0,-486.734344f,119.628014f,60.418327f,3.231307f,0.0f,0.0f,0.0f,0.0f},      // 8 - Inside Town Hall
    {850015,0,-337.863281f,38.192230f,55.173203f,2.969530f,0.0f,0.0f,0.0f,0.0f},       // 9 - Water Towers
};

class OPvPCapturePointHS : public OPvPCapturePoint
{
    public:
        OPvPCapturePointHS(OutdoorPvP* pvp, OutdoorPvPHSTowerType type);
        bool Update(uint32 diff);
        void ChangeState();
        void SendChangePhase();
        void FillInitialWorldStates(WorldPacket & data);
        
		// used when player is activated/inactivated in the area
        bool HandlePlayerEnter(Player * plr);
        void HandlePlayerLeave(Player * plr);

    private:
        OutdoorPvPHSTowerType m_TowerType;
};




class OutdoorPvPHS : public OutdoorPvP
{

    friend class OPvPCapturePointHS;

    public:
        OutdoorPvPHS();
        bool SetupOutdoorPvP();

        bool Update(uint32 diff);

        void FillInitialWorldStates(WorldPacket &data);
        void SendRemoveWorldStates(Player* player);

        void HandlePlayerEnterZone(Player* plr, uint32 zone);
        void HandlePlayerLeaveZone(Player* plr, uint32 zone);
        void HandlePlayerResurrects(Player * plr, uint32 zone);
        bool HandleOpenGo(Player* plr, uint64 guid);

        void ApplyZoneBalanceBuff();

        //void OnGameObjectCreate(GameObject* obj, bool add);
        void OnCreatureCreate(Creature* creature, bool add);
        void SendMessageToAll( const char *format, ... );

        // Resurrection System
        void SendAreaSpiritHealerQueryOpcode(Player* pl, const uint64& guid);
        void AddPlayerToResurrectQueue(uint64 npc_guid, uint64 player_guid);
        void RemovePlayerFromResurrectQueue(uint64 player_guid);
		
		// tower cap system
        void HandleKillImpl(Player* player, Unit* killed);

        uint32 GetAllianceTowersControlled() const;
        void SetAllianceTowersControlled(uint32 count);

        uint32 GetHordeTowersControlled() const;
        void SetHordeTowersControlled(uint32 count);

    private:

        // Resurrection System
        std::vector<uint64> m_ResurrectQueue;
        uint32 m_LastResurrectTime;
        std::map<uint64, std::vector<uint64> >  m_ReviveQueue;
        uint32 GetLastResurrectTime() const { return m_LastResurrectTime; }
        uint32 GetReviveQueueSize() const { return m_ReviveQueue.size(); }
        uint32 GetResurrectQueueSize() const { return m_ResurrectQueue.size(); }

        // FFA Chest System.
        uint64 m_ChestGUID;

        uint32 m_ChestTimer;
        uint32 m_ChestAnnounceTimer;
        uint32 m_ChestDebugTimer;
		
		// how many towers are controlled
        uint32 m_AllianceTowersControlled;
        uint32 m_HordeTowersControlled;


        Map * m_map;
};

#endif