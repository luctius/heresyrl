/*
    This file is part of heresyRL.

    heresyRL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    heresyRL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with heresyRL.  If not, see <http://www.gnu.org/licenses/>.
*/

#define CAREER(_cid,_title) [_cid]={.template_id=_cid, .title=_title

#define CAREER_END }

#define CHARACTERISTICS(ws,bs,st,tg,ag,in,wl,per,att,mov,mag,wnds)   \
    .char_advancements[MSR_CHAR_WEAPON_SKILL]=ws,   \
    .char_advancements[MSR_CHAR_BALISTIC_SKILL]=bs, \
    .char_advancements[MSR_CHAR_STRENGTH]=st,       \
    .char_advancements[MSR_CHAR_TOUGHNESS]=tg,      \
    .char_advancements[MSR_CHAR_AGILITY]=ag,        \
    .char_advancements[MSR_CHAR_INTELLIGENCE]=in,   \
    .char_advancements[MSR_CHAR_WILLPOWER]=wl,      \
    .char_advancements[MSR_CHAR_PERCEPTION]=per,    \
    .char_advancements[MSR_SEC_CHAR_ATTACKS]=att,   \
    .char_advancements[MSR_SEC_CHAR_MOVEMENT]=mov,  \
    .char_advancements[MSR_SEC_CHAR_MAGIC]=mag,     \
    .wounds = wnds

#define SKILLS(_skills) .skills=_skills

#define TALENTS(t...) .talents = { t, TLT_NONE, } 
#define TRAPPINGS(t...) .trappings = { t, IID_NONE, } 
#define EXITS(e...)   .exit_template_ids = { e, CRID_NONE, } 
#define DESCR(d)      .description=d
#define ALLIES(ally_id) .allies_ids={ ally_id, MID_NONE, }

static struct cr_career static_career_list[] = {
    CAREER(CRID_NONE, " "),
    CAREER_END,

    CAREER(CRID_BODYGUARD, "Bodyguard" ),
        DESCR("Description of an Bodyguard"),
        CHARACTERISTICS(10,0,5,5,5,0,0,0,1,0,0,3),
        SKILLS(bf(MSR_SKILLS_DODGE) | bf(MSR_SKILLS_HEAL) | bf(MSR_SKILLS_AWARENESS) ),
        TALENTS(TLT_QUICK_DRAW, TLT_SPEC_WPN_GRP_PARRYING, TLT_STREET_FIGHTING, TLT_STRIKE_TO_STUN, TLT_VERY_STRONG, TLT_VERY_RESILIENT),
        TRAPPINGS(IID_THROWING_KNIFE, IID_THROWING_KNIFE, IID_LEATHER_JACK),
        ALLIES(MID_NONE),
        EXITS(CRID_MERCENARY),
        .weight    = { [MSR_RACE_DWARF]=3,    [MSR_RACE_ELF]=0,    [MSR_RACE_HALFLING]=0,    [MSR_RACE_HUMAN]=2, },
        .available = { [MSR_RACE_DWARF]=true, [MSR_RACE_ELF]=true, [MSR_RACE_HALFLING]=true, [MSR_RACE_HUMAN]=true, },
    CAREER_END,

    CAREER(CRID_MERCENARY, "Mercenary" ),
        DESCR("Description of an Mercenary"),
        CHARACTERISTICS(10,10,5,5,5,0,5,0,1,0,0,3),
        SKILLS(bf(MSR_SKILLS_DODGE) | bf(MSR_SKILLS_HEAL) | bf(MSR_SKILLS_AWARENESS) ),
        TALENTS(TLT_QUICK_DRAW, TLT_RAPID_RELOAD, TLT_STRIKE_MIGHTY_BLOW, TLT_SHARPSHOOTER, TLT_STRIKE_TO_STUN),
        TRAPPINGS(IID_SHORT_BOW, IID_ARROW, IID_NONE),
        ALLIES(MID_NONE),
        EXITS(CRID_BODYGUARD),
        .weight    = { [MSR_RACE_DWARF]=3,    [MSR_RACE_ELF]=0,    [MSR_RACE_HALFLING]=0,    [MSR_RACE_HUMAN]=2, },
        .available = { [MSR_RACE_DWARF]=true, [MSR_RACE_ELF]=true, [MSR_RACE_HALFLING]=true, [MSR_RACE_HUMAN]=true, },
    CAREER_END,

    CAREER(CRID_FIELDWARDEN, "Fieldwarden" ),
        DESCR("Description of an Fieldwarden"),
        CHARACTERISTICS(5,10,0,5,10,0,10,0,0,0,0,2),
        SKILLS(bf(MSR_SKILLS_STEALTH) | bf(MSR_SKILLS_FOLLOW_TRAIL) | bf(MSR_SKILLS_AWARENESS) | bf(MSR_SKILLS_SURVIVAL) ),
        TALENTS(TLT_FLEET_FOOTED, TLT_SAVVY, TLT_MIGHTY_SHOT, TLT_RAPID_RELOAD, TLT_ROVER, TLT_SPEC_WPN_GRP_THROWING, TLT_QUICK_DRAW),
        TRAPPINGS(IID_SHORT_BOW, IID_ARROW, IID_NONE),
        ALLIES(MID_NONE),
        EXITS(CRID_MERCENARY),
        .weight    = { [MSR_RACE_DWARF]=0,     [MSR_RACE_ELF]=0,     [MSR_RACE_HALFLING]=5,    [MSR_RACE_HUMAN]=0, },
        .available = { [MSR_RACE_DWARF]=false, [MSR_RACE_ELF]=false, [MSR_RACE_HALFLING]=true, [MSR_RACE_HUMAN]=false, },
    CAREER_END,

    CAREER(CRID_MAX, ""),
    CAREER_END,
};
