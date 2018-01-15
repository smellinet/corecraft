/* Copyright (C) 2006 - 2012 ScriptDev2 <http://www.scriptdev2.com/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Undercity
SD%Complete: 95
SDComment: Quest support: 9180(post-event).
SDCategory: Undercity
EndScriptData */

/* ContentData
npc_lady_sylvanas_windrunner
npc_highborne_lamenter
EndContentData */

#include "precompiled.h"

/*######
## npc_lady_sylvanas_windrunner
######*/

#define SAY_LAMENT_END -1000196
#define EMOTE_LAMENT_END -1000197

#define SOUND_CREDIT 10896
#define ENTRY_HIGHBORNE_LAMENTER 21628
#define ENTRY_HIGHBORNE_BUNNY 21641

#define SPELL_HIGHBORNE_AURA 37090
#define SPELL_SYLVANAS_CAST 36568
#define SPELL_RIBBON_OF_SOULS 34432 // the real one to use might be 37099

float HighborneLoc[4][3] = {
    {1285.41f, 312.47f, 0.51f}, {1286.96f, 310.40f, 1.00f},
    {1289.66f, 309.66f, 1.52f}, {1292.51f, 310.50f, 1.99f},
};
#define HIGHBORNE_LOC_Y -61.00f
#define HIGHBORNE_LOC_Y_NEW -55.50f

struct MANGOS_DLL_DECL npc_lady_sylvanas_windrunnerAI : public ScriptedAI
{
    npc_lady_sylvanas_windrunnerAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    uint32 LamentEvent_Timer;
    bool LamentEvent;
    ObjectGuid m_targetGuid;

    float myX;
    float myY;
    float myZ;

    void Reset() override
    {
        myX = m_creature->GetX();
        myY = m_creature->GetY();
        myZ = m_creature->GetZ();

        LamentEvent_Timer = 5000;
        LamentEvent = false;
        m_targetGuid.Clear();
    }

    void JustSummoned(Creature* summoned) override
    {
        if (summoned->GetEntry() == ENTRY_HIGHBORNE_BUNNY)
        {
            if (Creature* pBunny =
                    m_creature->GetMap()->GetCreature(m_targetGuid))
            {
                pBunny->NearTeleportTo(
                    pBunny->GetX(), pBunny->GetY(), myZ + 15.0f, 0.0f);
                summoned->CastSpell(pBunny, SPELL_RIBBON_OF_SOULS, false);
            }

            m_targetGuid = summoned->GetObjectGuid();
        }
    }

    void UpdateAI(const uint32 diff) override
    {
        if (LamentEvent)
        {
            if (LamentEvent_Timer < diff)
            {
                auto pos = m_creature->GetPoint(
                    2 * M_PI_F * rand_norm_f(), 20.0f * rand_norm_f());
                m_creature->SummonCreature(ENTRY_HIGHBORNE_BUNNY, pos.x, pos.y,
                    pos.z, 0, TEMPSUMMON_TIMED_DESPAWN, 3000);

                LamentEvent_Timer = 2000;
                if (!m_creature->has_aura(SPELL_SYLVANAS_CAST))
                {
                    DoScriptText(SAY_LAMENT_END, m_creature);
                    DoScriptText(EMOTE_LAMENT_END, m_creature);
                    LamentEvent = false;
                }
            }
            else
                LamentEvent_Timer -= diff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_lady_sylvanas_windrunner(Creature* pCreature)
{
    return new npc_lady_sylvanas_windrunnerAI(pCreature);
}

bool QuestRewarded_npc_lady_sylvanas_windrunner(
    Player* /*pPlayer*/, Creature* pCreature, Quest const* pQuest)
{
    if (pQuest->GetQuestId() == 9180)
    {
        if (npc_lady_sylvanas_windrunnerAI* pSylvanAI =
                dynamic_cast<npc_lady_sylvanas_windrunnerAI*>(pCreature->AI()))
        {
            pSylvanAI->LamentEvent = true;
            pSylvanAI->DoPlaySoundToSet(pCreature, SOUND_CREDIT);
        }

        pCreature->CastSpell(pCreature, SPELL_SYLVANAS_CAST, false);

        for (auto& elem : HighborneLoc)
            pCreature->SummonCreature(ENTRY_HIGHBORNE_LAMENTER, elem[0],
                elem[1], HIGHBORNE_LOC_Y, elem[2], TEMPSUMMON_TIMED_DESPAWN,
                160000);
    }

    return true;
}

/*######
## npc_highborne_lamenter
######*/

struct MANGOS_DLL_DECL npc_highborne_lamenterAI : public ScriptedAI
{
    npc_highborne_lamenterAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    uint32 EventMove_Timer;
    uint32 EventCast_Timer;
    bool EventMove;
    bool EventCast;

    void Reset() override
    {
        EventMove_Timer = 10000;
        EventCast_Timer = 17500;
        EventMove = true;
        EventCast = true;
    }

    void UpdateAI(const uint32 diff) override
    {
        if (EventMove)
        {
            if (EventMove_Timer < diff)
            {
                m_creature->SetLevitate(true);
                m_creature->movement_gens.push(
                    new movement::PointMovementGenerator(0, m_creature->GetX(),
                        m_creature->GetY(), HIGHBORNE_LOC_Y_NEW, false, false));
                EventMove = false;
            }
            else
                EventMove_Timer -= diff;
        }
        if (EventCast)
        {
            if (EventCast_Timer < diff)
            {
                DoCastSpellIfCan(m_creature, SPELL_HIGHBORNE_AURA);
                EventCast = false;
            }
            else
                EventCast_Timer -= diff;
        }
    }
};

CreatureAI* GetAI_npc_highborne_lamenter(Creature* pCreature)
{
    return new npc_highborne_lamenterAI(pCreature);
}

void AddSC_undercity()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_lady_sylvanas_windrunner";
    pNewScript->GetAI = &GetAI_npc_lady_sylvanas_windrunner;
    pNewScript->pQuestRewardedNPC = &QuestRewarded_npc_lady_sylvanas_windrunner;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_highborne_lamenter";
    pNewScript->GetAI = &GetAI_npc_highborne_lamenter;
    pNewScript->RegisterSelf();
}
