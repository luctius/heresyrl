#define CAREER(_cid,_title) [_cid]={.template_id=_cid, .title=_title

#define CAREER_END }

#define CHARACTERISTICS(ws,bs,st,tg,ag,in,wl,per,att,mov,mag)   \
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
    .char_advancements[MSR_SEC_CHAR_MAGIC]=mag      \

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
        CHARACTERISTICS(10,0,5,5,5,0,0,0,1,3,0),
        SKILLS(bf(MSR_SKILLS_DODGE) | bf(MSR_SKILLS_HEAL) | bf(MSR_SKILLS_AWARENESS) ),
        TALENTS(TLT_QUICK_DRAW, TLT_SPEC_WPN_GRP_PARRYING, TLT_STREET_FIGHTING, TLT_STRIKE_TO_STUN, TLT_VERY_STRONG, TLT_VERY_RESILIENT),
        TRAPPINGS(IID_THROWING_KNIFE, IID_THROWING_KNIFE, IID_LEATHER_JACK),
        ALLIES(MID_NONE),
        EXITS(CRID_MERCENARY),
        .weight    = { [MSR_RACE_DWARF]=3,    [MSR_RACE_ELF]=0,    [MSR_RACE_HALFLING]=0,    [MSR_RACE_HUMAN]=200, },
        .available = { [MSR_RACE_DWARF]=true, [MSR_RACE_ELF]=true, [MSR_RACE_HALFLING]=true, [MSR_RACE_HUMAN]=true, },
    CAREER_END,

    CAREER(CRID_MERCENARY, "Mercenary" ),
        DESCR("Description of an Mercenary"),
        CHARACTERISTICS(10,10,5,5,5,0,5,0,1,2,0),
        SKILLS(bf(MSR_SKILLS_DODGE) | bf(MSR_SKILLS_HEAL) | bf(MSR_SKILLS_AWARENESS) ),
        TALENTS(TLT_QUICK_DRAW, TLT_RAPID_RELOAD, TLT_STRIKE_MIGHTY_BLOW, TLT_SHARPSHOOTER, TLT_STRIKE_TO_STUN),
        TRAPPINGS(IID_NONE),
        ALLIES(MID_NONE),
        EXITS(CRID_BODYGUARD),
        .weight    = { [MSR_RACE_DWARF]=3,    [MSR_RACE_ELF]=0,    [MSR_RACE_HALFLING]=0,    [MSR_RACE_HUMAN]=2, },
        .available = { [MSR_RACE_DWARF]=true, [MSR_RACE_ELF]=true, [MSR_RACE_HALFLING]=true, [MSR_RACE_HUMAN]=true, },
    CAREER_END,

    CAREER(CRID_MAX, ""),
    CAREER_END,
};
