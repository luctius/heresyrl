#define CONDITION(_id, _name, _desc, _oa_plr, _oa_msr, _oe_plr, _oe_msr, l...) \
                [_id] = { .uid=0, .template_id=_id, .name=_name, .description=_desc, .on_apply_plr=_oa_plr, \
                          .on_apply_msr=_oa_msr, .on_exit_plr=_oe_plr, .on_exit_msr=_oe_msr, .effects = { l }
#define CONDITION_END }

#define SETTINGS(_flags, _diff, _con, _dmin, _dmax) .setting_flags=_flags, .difficulty=_diff, \
                    .continues_to_id=_con, .duration_energy_min=_dmin, .duration_energy_max=_dmax

#define EFFECT(_effect, _flags, _dmg, _diff, _priority) \
                {.effect=_effect, .effect_setting_flags=bf(CDN_ESF_ACTIVE) | _flags, .priority=_priority, .strength=_dmg, \
                .difficulty=_diff, .tick_energy_max=1, .tick_energy=0, }

#define TICK_EFFECT(_effect, _flags, _dmg, _diff, _priority, _tmax) \
                {.effect=_effect, .effect_setting_flags=bf(CDN_ESF_ACTIVE) | bf(CDN_ESF_TICK) | _flags, \
                .priority=_priority, .strength=_dmg, .difficulty=_diff, .tick_energy_max=_tmax, .tick_energy=0, }

struct cdn_condition static_condition_list[] = {
    CONDITION(CID_NONE, "", "", NULL, NULL, NULL, NULL),
            SETTINGS( 0, 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_WITHDRAWL_HEALTH_STIMM, "health stimm withdrawl", "", "are suffering from withdrawl", "starts to shake", "overcome your cravings", "regains his posture", 
            TICK_EFFECT(CDN_EF_MODIFY_WS,   0, CDN_DAMAGE_FIVE, 0, CDN_PRIORITY_VERY_LOW, 4), 
            TICK_EFFECT(CDN_EF_MODIFY_BS,   0, CDN_DAMAGE_FIVE, 0, CDN_PRIORITY_VERY_LOW, 4), 
            TICK_EFFECT(CDN_EF_MODIFY_STR,  0, CDN_DAMAGE_FIVE, 0, CDN_PRIORITY_VERY_LOW, 4), 
            TICK_EFFECT(CDN_EF_MODIFY_AG,   0, CDN_DAMAGE_FIVE, 0, CDN_PRIORITY_VERY_LOW, 4), 
            TICK_EFFECT(CDN_EF_MODIFY_TGH,  0, CDN_DAMAGE_FIVE, 0, CDN_PRIORITY_VERY_LOW, 4), 
            TICK_EFFECT(CDN_EF_MODIFY_PER,  0, CDN_DAMAGE_FIVE, 0, CDN_PRIORITY_VERY_LOW, 4), 
            TICK_EFFECT(CDN_EF_MODIFY_WILL, 0, CDN_DAMAGE_FIVE, 0, CDN_PRIORITY_VERY_LOW, 4), 
            TICK_EFFECT(CDN_EF_MODIFY_INT,  0, CDN_DAMAGE_FIVE, 0, CDN_PRIORITY_VERY_LOW, 4), 
            TICK_EFFECT(CDN_EF_MODIFY_FEL,  0, CDN_DAMAGE_FIVE, 0, CDN_PRIORITY_VERY_LOW, 4), ),
            SETTINGS( bf(CDN_SF_ACTIVE_ALL) | bf(CDN_SF_REQ_CHEM_USE_CHECK) | bf(CDN_SF_DETOXABLE), -20, CID_NONE, 20, 50),
    CONDITION_END,

    /* Stimms*/
    CONDITION(CID_HEALTH_STIMM, "health stimm", "", "begin to feel much better", "'s wounds start to heal", "feel the healing wearing off", NULL, 
            TICK_EFFECT(CDN_EF_HEALTH_TICK, 0, CDN_HEALTH_ONE, 0, CDN_PRIORITY_VERY_LOW, 1), ),
            SETTINGS( bf(CDN_SF_REMOVE_CONTINUE), 0, CID_WITHDRAWL_HEALTH_STIMM, 10, 10),
    CONDITION_END,

    CONDITION(CID_DEATH_STIMM, "death stimm", "", NULL, NULL, NULL, NULL,
            TICK_EFFECT(CDN_EF_DEATH, 0, CDN_DAMAGE_NONE, 0, CDN_PRIORITY_PERMANENT, 0), ),
            SETTINGS(0, 0, CID_NONE, 0, 0),
    CONDITION_END,

    /* Weapon Effects */
    CONDITION(CID_WEAPON_FLAME, "flames", "flames are engulving you", "catch fire", "has catched fire", "manage to stop the flames", "stomps out the flames", 
            EFFECT(CDN_EF_ON_FIRE,          bf(CDN_ESF_INACTIVE_IF_LESS_PRIORITY),   CDN_DAMAGE_NONE,   0, CDN_PRIORITY_VERY_LOW),
            TICK_EFFECT(CDN_EF_DAMAGE_TICK, bf(CDN_ESF_DMG_TYPE_ENERGY),             CDN_DAMAGE_1D10,   0, CDN_PRIORITY_VERY_LOW, 1), ),
            SETTINGS(bf(CDN_SF_REQ_AG_CHECK), 0, CID_NONE, 1, 10),
    CONDITION_END,

    /* Critical hits */
    CONDITION(CID_ENERGY_CRITICAL_HEAD_1, "", "", "", "", "", "", 
            EFFECT(CDN_EF_MODIFY_WS,   0, CDN_DAMAGE_TEN, 0, CDN_PRIORITY_VERY_LOW), 
            EFFECT(CDN_EF_MODIFY_BS,   0, CDN_DAMAGE_TEN, 0, CDN_PRIORITY_VERY_LOW), 
            EFFECT(CDN_EF_MODIFY_STR,  0, CDN_DAMAGE_TEN, 0, CDN_PRIORITY_VERY_LOW), 
            EFFECT(CDN_EF_MODIFY_AG,   0, CDN_DAMAGE_TEN, 0, CDN_PRIORITY_VERY_LOW), 
            EFFECT(CDN_EF_MODIFY_PER,  0, CDN_DAMAGE_TEN, 0, CDN_PRIORITY_VERY_LOW), 
            EFFECT(CDN_EF_MODIFY_WILL, 0, CDN_DAMAGE_TEN, 0, CDN_PRIORITY_VERY_LOW), 
            EFFECT(CDN_EF_MODIFY_INT,  0, CDN_DAMAGE_TEN, 0, CDN_PRIORITY_VERY_LOW), 
            EFFECT(CDN_EF_MODIFY_FEL,  0, CDN_DAMAGE_TEN, 0, CDN_PRIORITY_VERY_LOW), ),
            SETTINGS( bf(CDN_SF_UNIQUE) | bf(CDN_SF_ACTIVE_ALL), 0, CID_NONE, 1, 1),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_HEAD_2, "", "", "", "", "", "", 
            EFFECT(CDN_EF_BLINDNESS,  0, CDN_DAMAGE_NONE, 0, CDN_PRIORITY_LOW), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1, 1),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_HEAD_3, "", "", "", "", "", "", 
            EFFECT(CDN_EF_STUNNED,        0, CDN_DAMAGE_NONE, 0, CDN_PRIORITY_AVERAGE),
            EFFECT(CDN_EF_MODIFY_FATIQUE, 0, CDN_DAMAGE_ONE,  0, CDN_PRIORITY_AVERAGE), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1, 1),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_HEAD_4, "", "", "", "", "", "", 
            EFFECT(CDN_EF_BLINDNESS,      0, CDN_DAMAGE_NONE, 0, CDN_PRIORITY_AVERAGE),
            EFFECT(CDN_EF_MODIFY_FATIQUE, 0, CDN_DAMAGE_TWO,  0, CDN_PRIORITY_HIGH), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1, 5),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_HEAD_5, "", "", "", "", "", "", 
            EFFECT(CDN_EF_BLINDNESS,      0, CDN_DAMAGE_NONE, 0, CDN_PRIORITY_HIGH),
            EFFECT(CDN_EF_MODIFY_FATIQUE, 0, CDN_DAMAGE_THREE,0, CDN_PRIORITY_HIGH), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1, 10),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_HEAD_6, "", "", "", "", "", "", 
            EFFECT(CDN_EF_BLINDNESS,      0, CDN_DAMAGE_NONE, 0, CDN_PRIORITY_VERY_HIGH),
            EFFECT(CDN_EF_MODIFY_FATIQUE, 0, CDN_DAMAGE_1D5,  0, CDN_PRIORITY_VERY_HIGH), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 10, 100),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_HEAD_7, "", "", "", "", "", "", 
            EFFECT(CDN_EF_BLINDNESS,       0, CDN_DAMAGE_NONE, 0, CDN_PRIORITY_PERMANENT),
            EFFECT(CDN_EF_MODIFY_FATIQUE,  0, CDN_DAMAGE_1D10, 0, CDN_PRIORITY_VERY_HIGH),
            EFFECT(CDN_EF_MODIFY_FEL,      bf(CDN_ESF_MODIFY_BASE), CDN_DAMAGE_1D10, 0, CDN_PRIORITY_PERMANENT), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 100, 500),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_HEAD_8, "", "", NULL, "", NULL, "", 
            EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0, CDN_PRIORITY_PERMANENT), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_HEAD_9, "", "", NULL, "", NULL, "", 
            EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0, CDN_PRIORITY_PERMANENT), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,
};

