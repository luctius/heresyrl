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
STATUS_EFFECT(SEID_BLUNT_LARM_1, L"Bashed left fingers", L"Your fingers are numbed and you take a -10% Weapon Skill Penaly for 2 rounds.", 0),
    MESSAGES("Your fingers just took a beating.", "%ls fingers just took a beating.", NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags   Strength    param */
        EFFECT(EF_MODIFY_CHAR,      0,    -10,         MSR_CHAR_COMBAT),
    EFFECTS_END,
    /*Settings      Flags     Minimum  -  Maximum Turns*/
    SETTINGS(       0,        2,          2),
STATUS_EFFECT_END,

STATUS_EFFECT(SEID_BLUNT_LARM_2, L"Bashed left elbow", L"Your elbow is bashed and you have to make a Hard (-20%) Toughness test or drop what is in your hand.", 0),
    MESSAGES("Your elbow was bashed.", "%ls elbow was bashed.", NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags   Strength    param*/
        EFFECT(EF_DISABLED_LARM,      0,     0,          -1),
    EFFECTS_END,
    /*Settings      Flags     Minimum  -  Maximum Turns*/
    SETTINGS(       0,        2,          2),
    CHECK(bf(EF_CHECK_CHARACTERISTIC), MSR_CHAR_TOUGHNESS,   -20),
STATUS_EFFECT_END,

STATUS_EFFECT(SEID_BLUNT_LARM_3, L"Smashed left shoulder", L"Your shoulder is bashed and receive -20% Weapon Skill and Ballistic Skill for 10 to 20 turns.", 2),
    MESSAGES("Your shoulder was bashed.", "%ls shoulder was bashed.", NULL, NULL),
    EFFECTS_START
        /* Type  Effect          Flags  Strength    Param*/
        EFFECT(EF_DISABLED_LARM, 0,     0,         -1),
        EFFECT(EF_MODIFY_CHAR,   0,     -10,        MSR_CHAR_COMBAT),
    EFFECTS_END,
    /*Settings      Flags     Minimum  -  Maximum Turns*/
    SETTINGS(       0,        10,          20),
STATUS_EFFECT_END,

STATUS_EFFECT(SEID_BLUNT_LARM_4, L"Fractured left collarbone", L"You lose the function in that arm until healed by a professional. In addition you are stunned and fall to the ground in excrusating pain.", 5),
    MESSAGES("Your collarbone fractured.", "%ls collarbone was fractured.", NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags  Strength    Param   Interval    Max,  Msgs*/
        EFFECT(EF_DISABLED_LARM,    0,     0,          -1),
        EFFECT(EF_PRONE,            0,     0,          -1),
        TICK_EFFECT(EF_STUNNED,     0,     0,          -1,     4,          1,    "You manage to surpress the pain.","%ls recovers."),
    EFFECTS_END,
    /*Settings      Flags                   Minimum  -  Maximum Turns*/
    SETTINGS(       bf(SEF_PERMANENT),        1,          1),
    /*Heal  flags   Difficulty      Succesfull heal evolve tid.*/
    HEALING(0,      -20,              SEID_BLUNT_ARM_4_HEALED),
STATUS_EFFECT_END,

STATUS_EFFECT(SEID_BLUNT_LARM_5, L"Demolished Ribcage", L"You should be dead.", -1),
    MESSAGES("The impact smashed the shoulder to pulp and shatters your ribcage.", "%ls's shoulder is smashed to pulp by the impact and his ribcage shatters.", NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags  Strength    Param   Interval    Max,  Msgs*/
        EFFECT(EF_DEAD,    0,     0,          -1),
    EFFECTS_END,
STATUS_EFFECT_END,

STATUS_EFFECT(SEID_BLUNT_RARM_1, L"Bashed right fingers", L"Your fingers are numbed and you take a -10% Weapon Skill Penaly for 2 rounds.", 0),
    MESSAGES("Your fingers just took a beating.", "%ls fingers just took a beating.", NULL, NULL),
    EFFECTS_START
        /* Type  Effect         Flags   Strength    Param */
        EFFECT(EF_MODIFY_CHAR,  0,      -10,        MSR_CHAR_COMBAT),
    EFFECTS_END,
    /*Settings      Flags     Minimum  -  Maximum Turns*/
    SETTINGS(       0,        2,          2),
STATUS_EFFECT_END,

STATUS_EFFECT(SEID_BLUNT_RARM_2, L"Bashed right elbow", L"Your elbow is bashed and you have to make a Hard (-20%) Toughness test or drop what is in your hand.", 0),
    MESSAGES("Your elbow was bashed.", "%ls elbow was bashed.", NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags   Strength    param*/
        EFFECT(EF_DISABLED_RARM,      0,     0,          -1),
    EFFECTS_END,
    /*Settings      Flags     Minimum  -  Maximum Turns*/
    SETTINGS(       0,        2,          2),
    CHECK(bf(EF_CHECK_CHARACTERISTIC), MSR_CHAR_TOUGHNESS,   -20),
STATUS_EFFECT_END,

STATUS_EFFECT(SEID_BLUNT_RARM_3, L"Smashed right shoulder", L"Your shoulder is bashed and receive -20% Weapon Skill and Ballistic Skill for 10 to 20 turns.", 2),
    MESSAGES("Your shoulder was bashed.", "%ls shoulder was bashed.", NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags   Strength    Param*/
        EFFECT(EF_DISABLED_RARM,      0,    0,          -1),
        EFFECT(EF_MODIFY_CHAR,        0,    -20,        MSR_CHAR_COMBAT),
    EFFECTS_END,
    /*Settings      Flags     Minimum  -  Maximum Turns*/
    SETTINGS(       0,        10,          20),
    /*Heal  flags   Difficulty      Succesfull heal evolve tid.*/
    HEALING(0,      -10,              SEID_BLUNT_ARM_3_HEALED),
STATUS_EFFECT_END,

STATUS_EFFECT(SEID_BLUNT_RARM_4, L"Fractured right collarbone", L"You lose the function in that arm until healed by a professional. In addition you are stunned and fall to the ground in excrusating pain.", 5),
    MESSAGES("Your collarbone fractured.", "%ls collarbone was fractured.", NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags  Strength    Param   Interval    Max,  Msgs*/
        EFFECT(EF_DISABLED_RARM,    0,     0,          -1),
        EFFECT(EF_PRONE,            0,     0,          -1),
        TICK_EFFECT(EF_STUNNED,     0,     0,          -1,     4,          1,    "You manage to surpress the pain.","%ls recovers."),
    EFFECTS_END,
    /*Settings      Flags                   Minimum  -  Maximum Turns*/
    SETTINGS(       bf(SEF_PERMANENT),        1,          1),
    /*Heal  flags   Difficulty      Succesfull heal evolve tid.*/
    HEALING(0,      -20,              SEID_BLUNT_ARM_4_HEALED),
STATUS_EFFECT_END,

STATUS_EFFECT(SEID_BLUNT_RARM_5, L"Demolished Ribcage", L"You should be dead.", -1),
    MESSAGES("The impact smashed the shoulder to pulp and shatters your ribcage.", "%ls's shoulder is smashed to pulp by the impact and his ribcage shatters.", NULL, NULL),
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
STATUS_EFFECT(SEID_BLUNT_ARM_3_HEALED, L"Healing Shoulder", L"Your shoulder is bashed but healing.", 0),
    MESSAGES(NULL, NULL, NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags                       Strength    Param*/
        EFFECT(EF_MODIFY_CHAR,     MSR_CHAR_COMBAT,              20,         -1),
    EFFECTS_END,
    /*Settings      Flags     Minimum  -  Maximum Turns*/
    SETTINGS(       0,        10,          20),
STATUS_EFFECT_END,


STATUS_EFFECT(SEID_BLUNT_ARM_4_HEALED, L"Healing Collarbone", L"Your collarbone is fractured but tended to, and your arm has been set in place.", 0),
    MESSAGES(NULL, NULL, NULL, NULL),
    EFFECTS_START
        /* Type  Effect             Flags  Strength    Param   Interval    Max,  Msgs*/
        EFFECT(EF_MODIFY_CHAR,      0,    20,         MSR_CHAR_COMBAT),
    EFFECTS_END,
    /*Settings      Flags                   Minimum  -  Maximum Turns*/
    SETTINGS(       0,                      40,          80),
STATUS_EFFECT_END,



#endif /* STATUS_EFFECTS_CRITICAL_BLUNT_H */
