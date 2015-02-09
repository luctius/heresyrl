#ifndef STATUS_EFFECTS_STATIC_H
#define STATUS_EFFECTS_STATIC_H

#define SE_NR_CRITICAL_HITS_PER_LOCATION (5)

enum se_ids {
    SEID_NONE,

    /* Draughts */
    SEID_MINOR_HEALING,

    /* Weapons */
    SEID_WEAPON_FLAME,

    /* Criticals -- it is required that these are in order! ie. HEAD1 , HEAD2, HEAD3 ... */
    SEID_BLUNT_RARM_1,
    SEID_BLUNT_RARM_2,
    SEID_BLUNT_RARM_3,
    SEID_BLUNT_RARM_4,
    SEID_BLUNT_RARM_5,

    /*Death*/
    SEID_FATEHEALTH,
    SEID_DEATH_STIMM,

    SEID_MAX,
};

#endif /* STATUS_EFFECTS_STATIC_H */
