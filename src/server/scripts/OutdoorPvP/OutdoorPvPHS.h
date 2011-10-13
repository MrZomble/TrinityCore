/*
 * Copyright (C) 2010 Wowtorn <http://www.wowtorn.net/>
 */

#ifndef OUTDOOR_PVP_HS_
#define OUTDOOR_PVP_HS_

//#include "OutdoorPvPImpl.h"

enum HS_Outdoor_Flag_SpellId
{
    HS_SPELL_VISUAL_REWARD_1   = 35151,
    HS_SPELL_VISUAL_REWARD_2   = 50757,
    HS_SPELL_VISUAL_REWARD_3   = 45311,

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

enum OutdoorPvPHSTowerType{
    HS_TOWER_LOWER = 0,
    HS_TOWER_LOWER_EAST = 1,
    HS_TOWER_MAIN = 2,
    HS_TOWER_NUM = 3
};

/* const go_type HSCapturePoints[HS_TOWER_NUM] =
{
    {850000,0,-474.745361f,-1373.009399f,53.323555f,0.174533f,0.0f,0.0f,0.087156f,0.996195f},     // 0 - Lower
    {850001,0,-569.107239f,-1510.894653f,52.848061f,0.174533f,0.0f,0.0f,0.087156f,0.996195f},     // 1 - Lower East
    {850002,0,-450.357513f,-1480.748779f,92.521019f,0.174533f,0.0f,0.0f,0.087156f,0.996195f}     // 2 - Main
}; */

/* const go_type HSChestPoints[10] = {
    {850015,0,-594.745361,-51.738495,45.944431,0.174533,0,0,0,0},      // 0 - Young Field
    {850015,0,-466.411224,-42.806705,54.477070,0.000000,0,0,0,0},      // 1 - Middle Field
    {850015,0,-422.661194,106.235573,54.188080,5.424229,0,0,0,0},      // 2 - Upper Field
    {850015,0,-565.477844,53.209713,48.447845,0.216379,0,0,0,0},       // 3 - Blacksmith
    {850015,0,-350.670868,-11.088767,55.021141,1.549651,0,0,0,0},      // 4 - Flowers
    {850015,0,-396.697510,-56.452023,54.452023,3.887430,0,0,0,0},      // 5 - Barn
    {850015,0,-548.236938,-105.017731,53.077545,1.641531,0,0,0,0},     // 6 - House Roof
    {850015,0,-457.335114,92.229897,58.199024,4.366654,0,0,0,0},       // 7 - Cart
    {850015,0,-486.734344,119.628014,60.418327,3.231307,0,0,0,0},      // 8 - Inside Town Hall
    {850015,0,-337.863281,38.192230,55.173203,2.969530,0,0,0,0}       // 9 - Water Towers
}; */

/* class OPvPCapturePointHS : public OPvPCapturePoint
{
    public:
        OPvPCapturePointHS(OutdoorPvP * pvp, OutdoorPvPHSTowerType type);
        bool Update(uint32 diff);
        void ChangeState();
        void SendChangePhase();
        void FillInitialWorldStates(WorldPacket & data);
        // used when player is activated/inactivated in the area
        bool HandlePlayerEnter(Player * plr);
        void HandlePlayerLeave(Player * plr);
        
        void RespawnVisualTower(uint32 entry);

        void DeleteSpawns();
    private:
        OutdoorPvPHSTowerType m_TowerType;
        bool m_locked;
}; */

class OutdoorPvPHS : public OutdoorPvP
{

    friend class OPvPCapturePointHS;

    public:
        OutdoorPvPHS();
        bool SetupOutdoorPvP();

        bool Update(uint32 diff);

        void FillInitialWorldStates(WorldPacket &data);
        void SendRemoveWorldStates(Player * plr);

        void HandlePlayerEnterZone(Player *plr, uint32 zone);
        void HandlePlayerLeaveZone(Player *plr, uint32 zone);
        void HandlePlayerResurrects(Player * plr, uint32 zone);
    //  void HandleKill(Player * plr, Unit * killed);
        bool HandleOpenGo(Player *plr, uint64 guid);
    //  bool HandleDropFlag(Player * plr, uint32 spellId);

        void ApplyZoneBalanceBuff();

        void OnGameObjectCreate(GameObject *obj, bool add);
        void OnCreatureCreate(Creature *creature, bool add);

    //  void RewardItem(Player *plr, uint32 item_id, uint32 count);

    //  void UpdateFlagPosition(float x, float y, bool carrier);
    //  void UpdateFlagPoI(Player *plr);
    //  void PlaySoundToAll(uint32 SoundID, bool carrier );
    //  void SendPacketToAll(WorldPacket *packet, bool carrier );
        void SendMessageToAll( const char *format, ... );
    //  void SendMessageToCarrier( const char *format, ... );

    //  uint64& GetFlagHolderGUID() { return m_FlagHolderGUID; }

        // Resurrection System
        void SendAreaSpiritHealerQueryOpcode(Player *pl, const uint64& guid);
        void AddPlayerToResurrectQueue(uint64 npc_guid, uint64 player_guid);
        void RemovePlayerFromResurrectQueue(uint64 player_guid);

        void CheckSetDurnholdeMain();
        void SetJustCaptured(uint32 team);

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

        // Timers.
        uint32 m_TenacityTimer;
        // Count.
        uint32 m_PlayerCount;
        uint32 m_HordeCount;
        uint32 m_AllianceCount;

        uint32 m_HordeBuff;
        uint32 m_AllianceBuff;

        // Durnholde
        uint32 m_DurnholdeOwner;
        uint32 m_DurnholdeLockout;

        // Durnholde Towers.
        uint16 m_TowerStateMain, m_TowerStateEast, m_TowerStateWest;
};

#endif