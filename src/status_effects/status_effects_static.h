#ifndef STATUS_EFFECTS_STATIC_H
#define STATUS_EFFECTS_STATIC_H

#define SE_NR_CRITICAL_HITS_PER_LOCATION (5)

enum se_ids {
    SEID_NONE,

    /* Withdrawls */
    SEID_WITHDRAWL_HEALTH_STIMM,

    /* Stimms */
    SEID_HEALTH_STIMM,
    SEID_DEATH_STIMM,

    /* Weapons */
    SEID_WEAPON_FLAME,
    SEID_WEAPON_FLAME_AREA,

    /* Criticals -- it is required that these are in order! ie. HEAD1 , HEAD2, HEAD3 ... */
    /* Energy Head */
    SEID_ENERGY_CRITICAL_HEAD_1,
    SEID_ENERGY_CRITICAL_HEAD_2,
    SEID_ENERGY_CRITICAL_HEAD_3,
    SEID_ENERGY_CRITICAL_HEAD_4,
    SEID_ENERGY_CRITICAL_HEAD_5,

    /* Energy Chest */
    SEID_ENERGY_CRITICAL_BODY_1,
    SEID_ENERGY_CRITICAL_BODY_2,
    SEID_ENERGY_CRITICAL_BODY_3,
    SEID_ENERGY_CRITICAL_BODY_4,
    SEID_ENERGY_CRITICAL_BODY_5,

    /* Energy Arms */
    SEID_ENERGY_CRITICAL_RARMS_1,
    SEID_ENERGY_CRITICAL_RARMS_2,
    SEID_ENERGY_CRITICAL_RARMS_3,
    SEID_ENERGY_CRITICAL_RARMS_4,
    SEID_ENERGY_CRITICAL_RARMS_5,

    /* Energy Leg */
    SEID_ENERGY_CRITICAL_RLEGS_1,
    SEID_ENERGY_CRITICAL_RLEGS_2,
    SEID_ENERGY_CRITICAL_RLEGS_3,
    SEID_ENERGY_CRITICAL_RLEGS_4,
    SEID_ENERGY_CRITICAL_RLEGS_5,

    /* Energy Arms */
    SEID_ENERGY_CRITICAL_LARMS_1,
    SEID_ENERGY_CRITICAL_LARMS_2,
    SEID_ENERGY_CRITICAL_LARMS_3,
    SEID_ENERGY_CRITICAL_LARMS_4,
    SEID_ENERGY_CRITICAL_LARMS_5,

    /* Energy Leg */
    SEID_ENERGY_CRITICAL_LLEGS_1,
    SEID_ENERGY_CRITICAL_LLEGS_2,
    SEID_ENERGY_CRITICAL_LLEGS_3,
    SEID_ENERGY_CRITICAL_LLEGS_4,
    SEID_ENERGY_CRITICAL_LLEGS_5,

    /* Impact Head */
    SEID_IMPACT_CRITICAL_HEAD_1,
    SEID_IMPACT_CRITICAL_HEAD_2,
    SEID_IMPACT_CRITICAL_HEAD_3,
    SEID_IMPACT_CRITICAL_HEAD_4,
    SEID_IMPACT_CRITICAL_HEAD_5,

    /* Impact Chest */
    SEID_IMPACT_CRITICAL_BODY_1,
    SEID_IMPACT_CRITICAL_BODY_2,
    SEID_IMPACT_CRITICAL_BODY_3,
    SEID_IMPACT_CRITICAL_BODY_4,
    SEID_IMPACT_CRITICAL_BODY_5,

    /* Impact Arms */
    SEID_IMPACT_CRITICAL_RARMS_1,
    SEID_IMPACT_CRITICAL_RARMS_2,
    SEID_IMPACT_CRITICAL_RARMS_3,
    SEID_IMPACT_CRITICAL_RARMS_4,
    SEID_IMPACT_CRITICAL_RARMS_5,

    /* Impact Leg */
    SEID_IMPACT_CRITICAL_RLEGS_1,
    SEID_IMPACT_CRITICAL_RLEGS_2,
    SEID_IMPACT_CRITICAL_RLEGS_3,
    SEID_IMPACT_CRITICAL_RLEGS_4,
    SEID_IMPACT_CRITICAL_RLEGS_5,

    /* Impact Arms */
    SEID_IMPACT_CRITICAL_LARMS_1,
    SEID_IMPACT_CRITICAL_LARMS_2,
    SEID_IMPACT_CRITICAL_LARMS_3,
    SEID_IMPACT_CRITICAL_LARMS_4,
    SEID_IMPACT_CRITICAL_LARMS_5,

    /* Impact Leg */
    SEID_IMPACT_CRITICAL_LLEGS_1,
    SEID_IMPACT_CRITICAL_LLEGS_2,
    SEID_IMPACT_CRITICAL_LLEGS_3,
    SEID_IMPACT_CRITICAL_LLEGS_4,
    SEID_IMPACT_CRITICAL_LLEGS_5,

    /* Rending Head */
    SEID_RENDING_CRITICAL_HEAD_1,
    SEID_RENDING_CRITICAL_HEAD_2,
    SEID_RENDING_CRITICAL_HEAD_3,
    SEID_RENDING_CRITICAL_HEAD_4,
    SEID_RENDING_CRITICAL_HEAD_5,

    /* Rending Chest */
    SEID_RENDING_CRITICAL_BODY_1,
    SEID_RENDING_CRITICAL_BODY_2,
    SEID_RENDING_CRITICAL_BODY_3,
    SEID_RENDING_CRITICAL_BODY_4,
    SEID_RENDING_CRITICAL_BODY_5,

    /* Rending Arms */
    SEID_RENDING_CRITICAL_RARMS_1,
    SEID_RENDING_CRITICAL_RARMS_2,
    SEID_RENDING_CRITICAL_RARMS_3,
    SEID_RENDING_CRITICAL_RARMS_4,
    SEID_RENDING_CRITICAL_RARMS_5,

    /* Rending Leg */
    SEID_RENDING_CRITICAL_RLEGS_1,
    SEID_RENDING_CRITICAL_RLEGS_2,
    SEID_RENDING_CRITICAL_RLEGS_3,
    SEID_RENDING_CRITICAL_RLEGS_4,
    SEID_RENDING_CRITICAL_RLEGS_5,

    /* Rending Arms */
    SEID_RENDING_CRITICAL_LARMS_1,
    SEID_RENDING_CRITICAL_LARMS_2,
    SEID_RENDING_CRITICAL_LARMS_3,
    SEID_RENDING_CRITICAL_LARMS_4,
    SEID_RENDING_CRITICAL_LARMS_5,

    /* Rending Leg */
    SEID_RENDING_CRITICAL_LLEGS_1,
    SEID_RENDING_CRITICAL_LLEGS_2,
    SEID_RENDING_CRITICAL_LLEGS_3,
    SEID_RENDING_CRITICAL_LLEGS_4,
    SEID_RENDING_CRITICAL_LLEGS_5,

    /* Explosive Head */
    SEID_EXPLOSIVE_CRITICAL_HEAD_1,
    SEID_EXPLOSIVE_CRITICAL_HEAD_2,
    SEID_EXPLOSIVE_CRITICAL_HEAD_3,
    SEID_EXPLOSIVE_CRITICAL_HEAD_4,
    SEID_EXPLOSIVE_CRITICAL_HEAD_5,

    /* Explosive Chest */
    SEID_EXPLOSIVE_CRITICAL_BODY_1,
    SEID_EXPLOSIVE_CRITICAL_BODY_2,
    SEID_EXPLOSIVE_CRITICAL_BODY_3,
    SEID_EXPLOSIVE_CRITICAL_BODY_4,
    SEID_EXPLOSIVE_CRITICAL_BODY_5,

    /* Explosive Arms */
    SEID_EXPLOSIVE_CRITICAL_RARMS_1,
    SEID_EXPLOSIVE_CRITICAL_RARMS_2,
    SEID_EXPLOSIVE_CRITICAL_RARMS_3,
    SEID_EXPLOSIVE_CRITICAL_RARMS_4,
    SEID_EXPLOSIVE_CRITICAL_RARMS_5,

    /* Explosive Leg */
    SEID_EXPLOSIVE_CRITICAL_RLEGS_1,
    SEID_EXPLOSIVE_CRITICAL_RLEGS_2,
    SEID_EXPLOSIVE_CRITICAL_RLEGS_3,
    SEID_EXPLOSIVE_CRITICAL_RLEGS_4,
    SEID_EXPLOSIVE_CRITICAL_RLEGS_5,

    /* Explosive Arms */
    SEID_EXPLOSIVE_CRITICAL_LARMS_1,
    SEID_EXPLOSIVE_CRITICAL_LARMS_2,
    SEID_EXPLOSIVE_CRITICAL_LARMS_3,
    SEID_EXPLOSIVE_CRITICAL_LARMS_4,
    SEID_EXPLOSIVE_CRITICAL_LARMS_5,

    /* Explosive Leg */
    SEID_EXPLOSIVE_CRITICAL_LLEGS_1,
    SEID_EXPLOSIVE_CRITICAL_LLEGS_2,
    SEID_EXPLOSIVE_CRITICAL_LLEGS_3,
    SEID_EXPLOSIVE_CRITICAL_LLEGS_4,
    SEID_EXPLOSIVE_CRITICAL_LLEGS_5,

    /*Death*/
    SEID_FATEHEALTH,

    SEID_MAX,
};

#endif /* STATUS_EFFECTS_STATIC_H */
