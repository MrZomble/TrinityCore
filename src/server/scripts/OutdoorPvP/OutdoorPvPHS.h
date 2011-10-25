/*
 * Copyright (C) 2010 Wowtorn <http://www.wowtorn.net/>
 */

#ifndef OUTDOOR_PVP_HS_
#define OUTDOOR_PVP_HS_

#include "OutdoorPvP.h"

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
    // Capture points for ease of use...
    HS_CAPTURE_SOUTH    = 3,
    HS_CAPTURE_DARROW    = 4,
    HS_CAPTURE_NETHANDER   = 5,
    HS_TOWER_NUM = 6
};

const go_type HSConquestPoints[HS_TOWER_NUM] = 
{
    {850000,0,-474.745361,-1373.009399,53.323555,0.174533,0,0,0.087156,0.996195},     // 0 - Lower
    {850001,0,-569.107239,-1510.894653,52.848061,0.174533,0,0,0.087156,0.996195},     // 1 - Lower East
    {850002,0,-450.357513,-1480.748779,92.521019,0.174533,0,0,0.087156,0.996195},     // 2 - Main
    // AB capture system.
    {850030,0,-659.569298,412.013489,83.357964,3.746807,0,0,0,0},                     // 0 - Southpoint Tower
    {850031,0,-323.441711,-698.090149,57.967308,1.92390,0,0,0,0},                     // 1 - Darrow Hill Tower
    {850032,0,-622.047485,-1045.131592,65.926140,3.309312,0,0,0}                     // 2 - Nethander Stead Tower
};

// Tower Capture.

enum OutdoorPvPHSCaptureType{
    HS_CAPTURE_NUM = 3
};

const go_type HSCapturePoints[HS_TOWER_NUM] =
{
    {850000,0,-474.745361f,-1373.009399f,53.323555f,0.174533f,0.0f,0.0f,0.087156f,0.996195f},     // 0 - Lower
    {850001,0,-569.107239f,-1510.894653f,52.848061f,0.174533f,0.0f,0.0f,0.087156f,0.996195f},     // 1 - Lower East
    {850002,0,-450.357513f,-1480.748779f,92.521019f,0.174533f,0.0f,0.0f,0.087156f,0.996195f}     // 2 - Main
};

enum HSCaptureObjectId
{
    HS_CAPTURE_BANNER_0    = 850023,       // Southpoint banner
    HS_CAPTURE_BANNER_1    = 850024,       // Darrow Hill banner
    HS_CAPTURE_BANNER_2    = 850025,       // Nethander Stead banner
};

enum HSCapturePointState
{
    HS_CAPTURE_NEUTRAL       = 0,
    HS_CAPTURE_ALLIANCE      = 1,
    HS_CAPTURE_ALLIANCE_CONT = 2,
    HS_CAPTURE_HORDE         = 3,
    HS_CAPTURE_HORDE_CONT    = 4
};

struct HSCapturePoint
{
    uint64      gameobject;
    uint32      timer;
    bool        timerActive;
    bool        locked;
    HSCapturePointState state;
    HSCapturePointState previousState;
    uint8       teamIndex;
    uint32      capturepoint;
    std::string name;
    uint32      killcredit;
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
};

class OutdoorPvPHS : public OutdoorPvP
{

    friend class OPvPCapturePointHS;

    public:
        OutdoorPvPHS();
        bool SetupOutdoorPvP();

        bool Update(uint32 diff);

        void FillInitialWorldStates(WorldPacket &data);
        void SendRemoveWorldStates(Player * plr);

        void HandlePlayerEnterZone(Player* plr, uint32 zone);
        void HandlePlayerLeaveZone(Player* plr, uint32 zone);
        void HandlePlayerResurrects(Player * plr, uint32 zone);
    //  void HandleKill(Player * plr, Unit * killed);
        bool HandleOpenGo(Player* plr, uint64 guid);
    //  bool HandleDropFlag(Player * plr, uint32 spellId);

        void changeCapturePoint( uint32 node, HSCapturePointState newState, uint32 timer );

        void ApplyZoneBalanceBuff();

        void OnGameObjectCreate(GameObject* obj, bool add);
        void OnCreatureCreate(Creature* creature, bool add);

    //  void RewardItem(Player *plr, uint32 item_id, uint32 count);

    //  void UpdateFlagPosition(float x, float y, bool carrier);
    //  void UpdateFlagPoI(Player *plr);
    //  void PlaySoundToAll(uint32 SoundID, bool carrier );
    //  void SendPacketToAll(WorldPacket *packet, bool carrier );
        void SendMessageToAll( const char *format, ... );
    //  void SendMessageToCarrier( const char *format, ... );

    //  uint64& GetFlagHolderGUID() { return m_FlagHolderGUID; }

        // Resurrection System
        void SendAreaSpiritHealerQueryOpcode(Player* pl, const uint64& guid);
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

        // Tower Capture System
        HSCapturePoint   m_TowerPoints[HS_CAPTURE_NUM];

        // Timers.
        uint32 m_TenacityTimer;
        uint32 m_TowerAnnounceTimer;
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

        Map * m_map;
};

#endif