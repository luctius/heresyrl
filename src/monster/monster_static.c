#include "monster_static.h"

#include "monster_descriptions.h"

#define MONSTER(tid,_icon,_sd_name,_ld_name,desc,_gender,maxwounds) \
    [tid]={.uid=0, .template_id=tid, .icon=_icon, .sd_name=_sd_name, .ld_name=_ld_name, .description=desc,\
        .gender=_gender, .cur_wounds=maxwounds, .max_wounds=maxwounds, .fate_points=0,
#define MONSTER_END }

#define CHARACTERISTICS(ws,bs,st,tg,ag,in,pr,wl,fl) \
    .characteristic[MSR_CHAR_WEAPON_SKILL]={ .base_value=ws,}, .characteristic[MSR_CHAR_BALISTIC_SKILL]={ .base_value=bs,}, \
    .characteristic[MSR_CHAR_STRENGTH]={ .base_value=st,}, .characteristic[MSR_CHAR_TOUGHNESS]={ .base_value=tg,}, \
    .characteristic[MSR_CHAR_AGILITY]={ .base_value=ag,}, .characteristic[MSR_CHAR_PERCEPTION]={ .base_value=pr,}, \
    .characteristic[MSR_CHAR_INTELLIGENCE]={ .base_value=in,}, .characteristic[MSR_CHAR_WILLPOWER]={ .base_value=wl,}, \
    .characteristic[MSR_CHAR_FELLOWSHIP]={ .base_value=fl,} 

#define SKILLS(basic_skills, advanced_skills, expert_skills) .skills[MSR_SKILL_RATE_BASIC]=basic_skills, \
    .skills[MSR_SKILL_RATE_ADVANCED]=advanced_skills, .skills[MSR_SKILL_RATE_EXPERT]=expert_skills

#define HUMAN(tal0,tal1,tal2) .race=MSR_RACE_HUMAN, .size=MSR_SIZE_AVERAGE, \
        .talents[0]= T0_CREATURE_WEAPON_TALENT | tal0, .talents[1]=tal1, .talents[2]=tal2, .creature_traits = CREATURE_TRAITS_NONE

#define BEAST(tal0,tal1,tal2) .race=MSR_RACE_BEAST, .size=MSR_SIZE_AVERAGE, \
        .talents[0]= T0_CREATURE_WEAPON_TALENT | tal0, .talents[1]=tal1, .talents[2]=tal2, .creature_traits = CREATURE_TRAITS_BESTIAL | CREATURE_TRAITS_QUADRUPED

struct msr_monster static_monster_list[] = {
    MONSTER(MSR_ID_BASIC_FERAL,'h',"human","a feral human",MSR_DESC_FERAL_HUMAN,MSR_GENDER_MALE,14)
        HUMAN(T0_BASIC_WEAPON_TRAINING_LAS | T0_PISTOL_WEAPON_TRAINING_SP,T1_TALENT_DODGE,0),
        CHARACTERISTICS(30,30,30,30,30,30,30,30,30),
        SKILLS(0,0,0), MONSTER_END,

    MONSTER(MSR_ID_VICIOUS_DOG,'d',"dog","a vicious dog",MSR_DESC_VISIOUS_DOG,MSR_GENDER_MALE,12)
        BEAST(0,0,T2_SPRINT),
        CHARACTERISTICS(30,0,30,30,30,15,38,40,30),
        SKILLS(SKILLS_AWARENESS|SKILLS_SILENT_MOVE|SKILLS_TRACKING, SKILLS_AWARENESS|SKILLS_SILENT_MOVE|SKILLS_TRACKING ,0), MONSTER_END,
};
