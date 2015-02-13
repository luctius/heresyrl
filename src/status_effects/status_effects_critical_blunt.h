#ifndef STATUS_EFFECTS_CRITICAL_BLUNT_H
#define STATUS_EFFECTS_CRITICAL_BLUNT_H


/* 
   Head 
------------------------------------------------------------------------------
*/


/* 
   Chest 
------------------------------------------------------------------------------
*/


/* 
   Arms 
------------------------------------------------------------------------------
*/
STATUS_EFFECT(SEID_BLUNT_LARM_1, "Bashed Fingers", "Your fingers are numbed and you take a -10% Weapon Skill Penaly for 2 rounds."),
    MESSAGES("Your fingers just took a beating.", "%s fingers just took a beating.", NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags   Strength    param */
        EFFECT(EF_DECREASE_CHAR,      0,    10,         MSR_CHAR_WEAPON_SKILL),
    EFFECTS_END,
    /*Settings      Flags     Minimum  -  Maximum Turns*/
    SETTINGS(       0,        2,          2),
STATUS_EFFECT_END,

STATUS_EFFECT(SEID_BLUNT_LARM_2, "Bashed Elbow", "Your elbow is bashed and you have to make a Hard (-20%) Toughness test or drop what is in your hand."),
    MESSAGES("Your elbow was bashed.", "%s elbow was bashed.", NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags   Strength    param*/
        EFFECT(EF_DISABLED_LARM,      0,     0,          -1),
    EFFECTS_END,
    /*Settings      Flags     Minimum  -  Maximum Turns*/
    SETTINGS(       0,        2,          2),
    CHECK(bf(EF_CHECK_CHARACTERISTIC), MSR_CHAR_TOUGHNESS,   -20),
STATUS_EFFECT_END,

STATUS_EFFECT(SEID_BLUNT_LARM_3, "Smashed Shoulder", "Your shoulder is bashed and receive -20% Weapon Skill and Ballistic Skill for 10 to 20 turns."),
    MESSAGES("Your shoulder was bashed.", "%s shoulder was bashed.", NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags                       Strength    Param*/
        EFFECT(EF_DISABLED_LARM,      0,                         0,         -1),
        EFFECT(EF_DECREASE_CHAR,     MSR_CHAR_WEAPON_SKILL,     20,         -1),
        EFFECT(EF_DECREASE_CHAR,     MSR_CHAR_BALISTIC_SKILL,   20,         -1),
    EFFECTS_END,
    /*Settings      Flags     Minimum  -  Maximum Turns*/
    SETTINGS(       0,        10,          20),
STATUS_EFFECT_END,

STATUS_EFFECT(SEID_BLUNT_LARM_4, "Fractured Collarbone", "You lose the function in that arm until healed by a professional. In addition you are stunned and fall to the ground in excrusating pain."),
    MESSAGES("Your collarbone fractured.", "%s collarbone was fractured.", NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags  Strength    Param   Interval    Max,  Msgs*/
        EFFECT(EF_DISABLED_LARM,    0,     0,          -1),
        EFFECT(EF_PRONE,            0,     0,          -1),
        TICK_EFFECT(EF_STUNNED,     0,     0,          -1,     4,          1,    "You manage to surpress the pain.","%s recovers."),
    EFFECTS_END,
    /*Settings      Flags                   Minimum  -  Maximum Turns*/
    SETTINGS(       bf(SEF_PERMANENT),        1,          1),
    /*Heal  flags   Difficulty      Succesfull heal evolve tid.*/
    HEALING(0,      -20,              SEID_BLUNT_ARM_4_HEALED),
STATUS_EFFECT_END,

STATUS_EFFECT(SEID_BLUNT_LARM_5, "Demolished Ribcage", "You should be dead."),
    MESSAGES("The impact smashed the shoulder to pulp and shatters your ribcage.", "%s's shoulder is smashed to pulp by the impact and his ribcage shatters.", NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags  Strength    Param   Interval    Max,  Msgs*/
        EFFECT(EF_DEAD,    0,     0,          -1),
    EFFECTS_END,
STATUS_EFFECT_END,

STATUS_EFFECT(SEID_BLUNT_RARM_1, "Bashed Fingers", "Your fingers are numbed and you take a -10% Weapon Skill Penaly for 2 rounds."),
    MESSAGES("Your fingers just took a beating.", "%s fingers just took a beating.", NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags   Strength    param */
        EFFECT(EF_DECREASE_CHAR,      0,    10,         MSR_CHAR_WEAPON_SKILL),
    EFFECTS_END,
    /*Settings      Flags     Minimum  -  Maximum Turns*/
    SETTINGS(       0,        2,          2),
STATUS_EFFECT_END,

STATUS_EFFECT(SEID_BLUNT_RARM_2, "Bashed Elbow", "Your elbow is bashed and you have to make a Hard (-20%) Toughness test or drop what is in your hand."),
    MESSAGES("Your elbow was bashed.", "%s elbow was bashed.", NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags   Strength    param*/
        EFFECT(EF_DISABLED_RARM,      0,     0,          -1),
    EFFECTS_END,
    /*Settings      Flags     Minimum  -  Maximum Turns*/
    SETTINGS(       0,        2,          2),
    CHECK(bf(EF_CHECK_CHARACTERISTIC), MSR_CHAR_TOUGHNESS,   -20),
STATUS_EFFECT_END,

STATUS_EFFECT(SEID_BLUNT_RARM_3, "Smashed Shoulder", "Your shoulder is bashed and receive -20% Weapon Skill and Ballistic Skill for 10 to 20 turns."),
    MESSAGES("Your shoulder was bashed.", "%s shoulder was bashed.", NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags                       Strength    Param*/
        EFFECT(EF_DISABLED_RARM,      0,                         0,         -1),
        EFFECT(EF_DECREASE_CHAR,     MSR_CHAR_WEAPON_SKILL,     20,         -1),
        EFFECT(EF_DECREASE_CHAR,     MSR_CHAR_BALISTIC_SKILL,   20,         -1),
    EFFECTS_END,
    /*Settings      Flags     Minimum  -  Maximum Turns*/
    SETTINGS(       0,        10,          20),
STATUS_EFFECT_END,

STATUS_EFFECT(SEID_BLUNT_RARM_4, "Fractured Collarbone", "You lose the function in that arm until healed by a professional. In addition you are stunned and fall to the ground in excrusating pain."),
    MESSAGES("Your collarbone fractured.", "%s collarbone was fractured.", NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags  Strength    Param   Interval    Max,  Msgs*/
        EFFECT(EF_DISABLED_RARM,    0,     0,          -1),
        EFFECT(EF_PRONE,            0,     0,          -1),
        TICK_EFFECT(EF_STUNNED,     0,     0,          -1,     4,          1,    "You manage to surpress the pain.","%s recovers."),
    EFFECTS_END,
    /*Settings      Flags                   Minimum  -  Maximum Turns*/
    SETTINGS(       bf(SEF_PERMANENT),        1,          1),
    /*Heal  flags   Difficulty      Succesfull heal evolve tid.*/
    HEALING(0,      -20,              SEID_BLUNT_ARM_4_HEALED),
STATUS_EFFECT_END,

STATUS_EFFECT(SEID_BLUNT_RARM_5, "Demolished Ribcage", "You should be dead."),
    MESSAGES("The impact smashed the shoulder to pulp and shatters your ribcage.", "%s's shoulder is smashed to pulp by the impact and his ribcage shatters.", NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags  Strength    Param   Interval    Max,  Msgs*/
        EFFECT(EF_DEAD,    0,     0,          -1),
    EFFECTS_END,
STATUS_EFFECT_END,

/* 
   Legs 
------------------------------------------------------------------------------
*/

/* 
   Healed 
------------------------------------------------------------------------------
*/
STATUS_EFFECT(SEID_BLUNT_ARM_3_HEALED, "Healing Smashed Shoulder", "Your shoulder is bashed but healing."),
    MESSAGES(NULL, NULL, NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags                       Strength    Param*/
        EFFECT(EF_DECREASE_CHAR,     MSR_CHAR_WEAPON_SKILL,     20,         -1),
        EFFECT(EF_DECREASE_CHAR,     MSR_CHAR_BALISTIC_SKILL,   20,         -1),
    EFFECTS_END,
    /*Settings      Flags     Minimum  -  Maximum Turns*/
    SETTINGS(       0,        10,          20),
STATUS_EFFECT_END,


STATUS_EFFECT(SEID_BLUNT_ARM_4_HEALED, "Healing Fractured Collarbone", "Your collarbone is fractured but tended to, and your arm has been set in place."),
    MESSAGES(NULL, NULL, NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags  Strength    Param   Interval    Max,  Msgs*/
        EFFECT(EF_DECREASE_CHAR,      0,    20,         MSR_CHAR_WEAPON_SKILL),
        EFFECT(EF_DECREASE_CHAR,      0,    20,         MSR_CHAR_BALISTIC_SKILL),
    EFFECTS_END,
    /*Settings      Flags                   Minimum  -  Maximum Turns*/
    SETTINGS(       0,                      40,          80),
STATUS_EFFECT_END,



#endif /* STATUS_EFFECTS_CRITICAL_BLUNT_H */
