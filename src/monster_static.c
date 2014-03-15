#include "monster_static.h"

#define MONSTER(tid,_icon,_sd_name,_ld_name,desc,_gender,maxwounds) \
    [tid]={.uid=0, .template_id=tid, .icon=_icon, .sd_name=_sd_name, .ld_name=_ld_name, .description=desc,\
        .gender=_gender, .cur_wounds=maxwounds, .max_wounds=maxwounds, .fatepoints=0,
#define MONSTER_END }

#define CHARACTERISTICS(ws,bs,st,tg,ag,pr,in,wl,fl) \
    .characteristic[MSR_CHAR_WEAPON_SKILL]={ .base_value=ws,}, .characteristic[MSR_CHAR_BALISTIC_SKILL]={ .base_value=bs,}, \
    .characteristic[MSR_CHAR_STRENGTH]={ .base_value=st,}, .characteristic[MSR_CHAR_TOUGHNESS]={ .base_value=tg,}, \
    .characteristic[MSR_CHAR_AGILITY]={ .base_value=ag,}, .characteristic[MSR_CHAR_PERCEPTION]={ .base_value=pr,}, \
    .characteristic[MSR_CHAR_INTELLIGENCE]={ .base_value=in,}, .characteristic[MSR_CHAR_WILLPOWER]={ .base_value=wl,}, \
    .characteristic[MSR_CHAR_FELLOWSHIP]={ .base_value=fl,} 

#define SKILLS(basic_skills, advanced_skills, expert_skills) .skills[MSR_SKILL_RATE_BASIC]=basic_skills, \
    .skills[MSR_SKILL_RATE_ADVANCED]=advanced_skills, .skills[MSR_SKILL_RATE_EXPERT]=expert_skills

#define HUMAN(tal0,tal1,tal2) .race=MSR_RACE_HUMAN, .size=MSR_SIZE_AVERAGE, \
        .talents[0]= TALENTS0_CREATURE_TALENT_HANDS | tal0, .talents[1]=tal1, .talents[2]=tal2

#define BASIC_FERAL_DESC "description of a basic feral human"

struct msr_monster static_monster_list[] = {
    MONSTER(MSR_ID_BASIC_FERAL,'h',"human","a feral human",BASIC_FERAL_DESC,MSR_GENDER_MALE,14)
        HUMAN(TALENTS0_BASIC_WEAPON_TRAINING_SOLID_PROJECTILE,TALENTS1_TALENT_DODGE,0),
        CHARACTERISTICS(30,30,30,30,30,30,30,30,30),
        SKILLS(0,0,0),
    MONSTER_END,

};
