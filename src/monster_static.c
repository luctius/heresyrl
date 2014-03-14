#include "monster_static.h"

#define HUMAN(tid,ic,sdn,ldn,desc,gnr,mwound,ws,bs,st,tg,ag,pr,in,wl,fl,rtrait,comtrait,cartrait,creatrait,sk_basic, sk_adv, sk_exp) \
    [tid]={.uid=0, .template_id=tid, .icon=ic, .icon_attr=COLOR_PAIR(DPL_COLOUR_NORMAL), .sd_name=sdn, .ld_name=ldn, \
    .description=desc, .race=MSR_RACE_HUMAN, .size=MSR_SIZE_AVERAGE, .gender=gnr, .cur_wounds=mwound, .max_wounds=mwound, \
    .fatepoints=0, .race_traits=rtrait, .combat_talents=comtrait, .career_talents=cartrait, .creature_talents=creatrait, \
    .characteristic[MSR_CHAR_WEAPON_SKILL]={ .base_value=ws,}, .characteristic[MSR_CHAR_BALISTIC_SKILL]={ .base_value=bs,}, \
    .characteristic[MSR_CHAR_STRENGTH]={ .base_value=st,}, .characteristic[MSR_CHAR_TOUGHNESS]={ .base_value=tg,}, \
    .characteristic[MSR_CHAR_AGILITY]={ .base_value=ag,}, .characteristic[MSR_CHAR_PERCEPTION]={ .base_value=pr,}, \
    .characteristic[MSR_CHAR_INTELLIGENCE]={ .base_value=in,}, .characteristic[MSR_CHAR_WILLPOWER]={ .base_value=wl,}, \
    .characteristic[MSR_CHAR_FELLOWSHIP]={ .base_value=fl,}, .skills[MSR_SKILL_RATE_BASIC]=sk_basic, \
    .skills[MSR_SKILL_RATE_ADVANCED]=sk_adv, .skills[MSR_SKILL_RATE_EXPERT]=sk_exp }

#define BASIC_FERAL_DESC "description of a basic feral human"

struct msr_monster static_monster_list[] = {
    HUMAN(MSR_ID_BASIC_FERAL,'h',"human","a basic human",BASIC_FERAL_DESC,MSR_GENDER_MALE,14,30,30,30,30,30,30,30,30,30,0,0,0,0,0,0,0),
};
