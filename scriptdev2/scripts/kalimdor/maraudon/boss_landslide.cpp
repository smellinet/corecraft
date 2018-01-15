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
SDName: Boss_Landslide
SD%Complete: 100
SDComment:
SDCategory: Maraudon
EndScriptData */

#include "precompiled.h"

#define SPELL_KNOCKAWAY 18670
#define SPELL_TRAMPLE 5568
#define SPELL_LANDSLIDE 21808

struct MANGOS_DLL_DECL boss_landslideAI : public ScriptedAI
{
    boss_landslideAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 KnockAway_Timer;
    uint32 Trample_Timer;
    uint32 Landslide_Timer;

    void Reset() override
    {
        KnockAway_Timer = 8000;
        Trample_Timer = 2000;
        Landslide_Timer = 0;
    }

    void UpdateAI(const uint32 diff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // KnockAway_Timer
        if (KnockAway_Timer < diff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_KNOCKAWAY);
            KnockAway_Timer = 15000;
        }
        else
            KnockAway_Timer -= diff;

        // Trample_Timer
        if (Trample_Timer < diff)
        {
            DoCastSpellIfCan(m_creature, SPELL_TRAMPLE);
            Trample_Timer = 8000;
        }
        else
            Trample_Timer -= diff;

        // Landslide
        if (m_creature->GetHealthPercent() < 50.0f)
        {
            if (Landslide_Timer < diff)
            {
                m_creature->InterruptNonMeleeSpells(false);
                DoCastSpellIfCan(m_creature, SPELL_LANDSLIDE);
                Landslide_Timer = 60000;
            }
            else
                Landslide_Timer -= diff;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_landslide(Creature* pCreature)
{
    return new boss_landslideAI(pCreature);
}

void AddSC_boss_landslide()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_landslide";
    pNewScript->GetAI = &GetAI_boss_landslide;
    pNewScript->RegisterSelf();
}
