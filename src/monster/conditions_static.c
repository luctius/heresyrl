#define CONDITION(_id, _name, _desc, _oa_plr, _oa_msr, _oe_plr, _oe_msr, l...) \
                [_id] = { .uid=0, .template_id=_id, .name=_name, .description=_desc, .on_apply_plr=_oa_plr, \
                          .on_apply_msr=_oa_msr, .on_exit_plr=_oe_plr, .on_exit_msr=_oe_msr, .effects = { l }
#define CONDITION_END }

#define SETTINGS(_flags, _diff, _con, _dmin, _dmax) .setting_flags=_flags, .difficulty=_diff, \
                    .continues_to_id=_con, .duration_energy_min=(_dmin*TT_ENERGY_TURN), .duration_energy_max=(_dmax*TT_ENERGY_TURN)

#define EFFECT(_effect, _flags, _dmg, _diff) \
                {.effect=_effect, .effect_setting_flags=bf(CDN_ESF_ACTIVE) | _flags, .strength=_dmg, \
                .difficulty=_diff, .tick_energy_max=1, .tick_energy=0, }

#define TICK_EFFECT(_effect, _flags, _dmg, _diff, _tmax) \
                {.effect=_effect, .effect_setting_flags=bf(CDN_ESF_ACTIVE) | bf(CDN_ESF_TICK) | _flags, \
                .strength=_dmg, .difficulty=_diff, .tick_energy_max=(_tmax*TT_ENERGY_TURN), .tick_energy=0, }

static struct cdn_condition static_condition_list[] = {
    CONDITION(CID_NONE, "", "", NULL, NULL, NULL, NULL),
            SETTINGS( 0, 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_WITHDRAWL_HEALTH_STIMM, "health stimm withdrawl", "", "You start to crave for more health stimms.", "%s starts to shake.", "You overcome your cravings.", "%s regains his posture.", 
            TICK_EFFECT(CDN_EF_MODIFY_WS,   0, CDN_DAMAGE_FIVE, 0, 4), 
            TICK_EFFECT(CDN_EF_MODIFY_BS,   0, CDN_DAMAGE_FIVE, 0, 4), 
            TICK_EFFECT(CDN_EF_MODIFY_STR,  0, CDN_DAMAGE_FIVE, 0, 4), 
            TICK_EFFECT(CDN_EF_MODIFY_AG,   0, CDN_DAMAGE_FIVE, 0, 4), 
            TICK_EFFECT(CDN_EF_MODIFY_TGH,  0, CDN_DAMAGE_FIVE, 0, 4), 
            TICK_EFFECT(CDN_EF_MODIFY_PER,  0, CDN_DAMAGE_FIVE, 0, 4), 
            TICK_EFFECT(CDN_EF_MODIFY_WILL, 0, CDN_DAMAGE_FIVE, 0, 4), 
            TICK_EFFECT(CDN_EF_MODIFY_INT,  0, CDN_DAMAGE_FIVE, 0, 4), 
            TICK_EFFECT(CDN_EF_MODIFY_FEL,  0, CDN_DAMAGE_FIVE, 0, 4), ),
            SETTINGS( bf(CDN_SF_ACTIVE_ALL) | bf(CDN_SF_REQ_CHEM_USE_CHECK) | bf(CDN_SF_DETOXABLE), -20, CID_NONE, 20, 50),
    CONDITION_END,

    /* Stimms*/
    CONDITION(CID_HEALTH_STIMM, "health stimm", "", "Your wounds begin to heal.", "%s's wounds begin to heal.", "You feel the healing wearing off.", NULL, 
            TICK_EFFECT(CDN_EF_HEALTH_TICK, 0, CDN_HEALTH_ONE, 0, 1), ),
            SETTINGS( bf(CDN_SF_REMOVE_CONTINUE), 0, CID_WITHDRAWL_HEALTH_STIMM, 10, 10),
    CONDITION_END,

    CONDITION(CID_FATEHEALTH, "fatepoint_health", "", NULL, NULL, NULL, NULL, 
            TICK_EFFECT(CDN_EF_HEALTH_TICK, 0, CDN_HEALTH_FIVE, 0, 1), ),
            SETTINGS(0, 0, CID_NONE, 20, 20),
    CONDITION_END,

    CONDITION(CID_DEATH_STIMM, "death stimm", "", NULL, NULL, NULL, NULL,
            TICK_EFFECT(CDN_EF_DEATH, 0, CDN_DAMAGE_NONE, 0, 0), ),
            SETTINGS(0, 0, CID_NONE, 0, 0),
    CONDITION_END,

    /* Weapon Effects */
    CONDITION(CID_WEAPON_FLAME, "flames", "flames are engulving you", "You catch fire.", "%s has catched fire.", "You manage to put out the flames.", "%s stomps out the flames.", 
            EFFECT(CDN_EF_ON_FIRE,          0,                              CDN_DAMAGE_NONE,   0),
            TICK_EFFECT(CDN_EF_DAMAGE_TICK, bf(CDN_ESF_DMG_TYPE_ENERGY),    CDN_DAMAGE_1D10,   0, 1), ),
            SETTINGS(bf(CDN_SF_REQ_AG_CHECK), 0, CID_NONE, 1, 10),
    CONDITION_END,










    /* Critical hits */

    /* Energy critical hits */

    /*Head*/
    /* -10 penalty on all characteristics for 1 round */
    CONDITION(CID_ENERGY_CRITICAL_HEAD_1, "energy critical head 1", "", "", "", "", "", 
            EFFECT(CDN_EF_MODIFY_WS,   0, CDN_DAMAGE_TEN, 0), 
            EFFECT(CDN_EF_MODIFY_BS,   0, CDN_DAMAGE_TEN, 0), 
            EFFECT(CDN_EF_MODIFY_STR,  0, CDN_DAMAGE_TEN, 0), 
            EFFECT(CDN_EF_MODIFY_AG,   0, CDN_DAMAGE_TEN, 0), 
            EFFECT(CDN_EF_MODIFY_PER,  0, CDN_DAMAGE_TEN, 0), 
            EFFECT(CDN_EF_MODIFY_WILL, 0, CDN_DAMAGE_TEN, 0), 
            EFFECT(CDN_EF_MODIFY_INT,  0, CDN_DAMAGE_TEN, 0), 
            EFFECT(CDN_EF_MODIFY_FEL,  0, CDN_DAMAGE_TEN, 0), ),
            SETTINGS( bf(CDN_SF_UNIQUE) | bf(CDN_SF_ACTIVE_ALL), 0, CID_NONE, 1, 1),
    CONDITION_END,

    /* blinded for 1 round */
    CONDITION(CID_ENERGY_CRITICAL_HEAD_2, "energy critical head 2", "", "", "", "", "", 
            EFFECT(CDN_EF_BLINDNESS,  0, CDN_DAMAGE_NONE, 0), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1, 1),
    CONDITION_END,

    /* stunned for 1 round, 1 lvl of fatique */
    CONDITION(CID_ENERGY_CRITICAL_HEAD_3, "energy critical head 3", "", "", "", "", "", 
            EFFECT(CDN_EF_STUNNED,        0, CDN_DAMAGE_NONE, 0),
            EFFECT(CDN_EF_MODIFY_FATIQUE, 0, CDN_DAMAGE_ONE,  0), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1, 1),
    CONDITION_END,

    /* blindess for 1d5 rounds, fatique 2 lvls */
    CONDITION(CID_ENERGY_CRITICAL_HEAD_4, "energy critical head 4", "", "", "", "", "", 
            EFFECT(CDN_EF_BLINDNESS,      0, CDN_DAMAGE_NONE, 0),
            EFFECT(CDN_EF_MODIFY_FATIQUE, 0, CDN_DAMAGE_TWO,  0), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1, 5),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_HEAD_5, "energy critical head 5", "", NULL, "", NULL, "", 
            EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    /* Chest */
    CONDITION(CID_ENERGY_CRITICAL_BODY_1, "energy critical body 1", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_BODY_2, "energy critical body 2", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_BODY_3, "energy critical body 3", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_BODY_4, "energy critical body 4", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_BODY_5, "energy critical body 5", "", NULL, NULL, NULL, NULL,
            EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    /* Arms */
    CONDITION(CID_ENERGY_CRITICAL_RARMS_1, "energy critical arms 1", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_RARMS_2, "energy critical arms 2", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_RARMS_3, "energy critical arms 3", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_RARMS_4, "energy critical arms 4", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_RARMS_5, "energy critical arms 5", "", NULL, NULL, NULL, NULL,
            EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    /* Legs */
    CONDITION(CID_ENERGY_CRITICAL_RLEGS_1, "energy critical legs 1", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_RLEGS_2, "energy critical legs 2", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_RLEGS_3, "energy critical legs 3", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_RLEGS_4, "energy critical legs 4", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_RLEGS_5, "energy critical legs 5", "", NULL, NULL, NULL, NULL,
            EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    /* Arms */
    CONDITION(CID_ENERGY_CRITICAL_LARMS_1, "energy critical arms 1", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_LARMS_2, "energy critical arms 2", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_LARMS_3, "energy critical arms 3", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_LARMS_4, "energy critical arms 4", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_LARMS_5, "energy critical arms 5", "", NULL, NULL, NULL, NULL,
            EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    /* Legs */
    CONDITION(CID_ENERGY_CRITICAL_LLEGS_1, "energy critical legs 1", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_LLEGS_2, "energy critical legs 2", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_LLEGS_3, "energy critical legs 3", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_LLEGS_4, "energy critical legs 4", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_ENERGY_CRITICAL_LLEGS_5, "energy critical legs 5", "", NULL, NULL, NULL, NULL,
            EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

                /* Impact critical hits */

                /*Head*/
                CONDITION(CID_IMPACT_CRITICAL_HEAD_1, "impact critical head 1", "", "The impact fills your head with a ringing noise.", "", "", "", 
                        EFFECT(CDN_EF_MODIFY_FATIQUE,  bf(CDN_ESF_REQ_TGH_CHECK) , CDN_DAMAGE_ONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1, 1),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_HEAD_2, "impact critical head 2", "", "You are momentarily blinded by the blood spraying from your nose.", "", "", "", 
                        EFFECT(CDN_EF_BLINDNESS,                         0, CDN_DAMAGE_NONE, 0),
                        EFFECT(CDN_EF_STUNNED,   bf(CDN_ESF_REQ_TGH_CHECK), CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1, 1),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_HEAD_3, "impact critical head 3", "", "", "", "", "", 
                        EFFECT(CDN_EF_STUNNED,        0,                     CDN_DAMAGE_NONE,  0),
                        EFFECT(CDN_EF_MODIFY_FATIQUE, 0,                     CDN_DAMAGE_ONE,   0),
                        EFFECT(CDN_EF_MODIFY_INT,     bf(CDN_ESF_PERMANENT), CDN_DAMAGE_ONE,   0),
                        EFFECT(CDN_EF_STUMBLE,        0,                     CDN_DAMAGE_NONE,  0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE),  0, CID_NONE, 1, 1),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_HEAD_4, "impact critical head 4", "", NULL, NULL, "Blood pours from your eyes, ears and nose as the attack pulverizes your brain.", "Blood pours from %s eyes, ears and nose as the attack pulverizes %s brain.", 
                        EFFECT(CDN_EF_DEATH,         0, CDN_DAMAGE_NONE,  0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_HEAD_5, "impact critical head 5", "", NULL, NULL, "Your head bursts open, spraying brain matter in all directions", "%s head bursts open, spraying brain matter in all directions", 
                        EFFECT(CDN_EF_EXPLODE,       0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                /* Chest */
                CONDITION(CID_IMPACT_CRITICAL_BODY_1, "impact critical body 1", "", "The blow steals the air out of your lungs.", "The blow steals the air from %s's lungs.", NULL,  NULL,
                        EFFECT(CDN_EF_STUNNED, 0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0.5, 0.5),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_BODY_2, "impact critical body 2", "", "The attack breaks a rib.", "The attack breaks %s's rib.", NULL, NULL, 
                        EFFECT(CDN_EF_STUNNED, 0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1.5, 1.5),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_BODY_3, "impact critical body 3", "", "You receive a solid blow to the chest and you double over for a moment, clutching yourself in agony.", "A solid blow to %s's chest causes him to double over in pain and agony.", NULL, NULL,
                        EFFECT(CDN_EF_STUNNED,        0,                         CDN_DAMAGE_NONE,  0),
                        EFFECT(CDN_EF_MODIFY_FATIQUE, bf(CDN_ESF_REQ_TGH_CHECK), CDN_DAMAGE_1D5,   0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE),  0, CID_NONE, 2, 2),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_BODY_4, "impact critical body 4", "", "The force of the attack ruptures several of your organs and knocks you down gasping in wretched pain.", "The force of the attack ruptures several organs, knocking %s down, gasping in wretched pain.", NULL, NULL,
                        EFFECT(CDN_EF_BLOODLOSS,      0, CDN_DAMAGE_NONE,   0),
                        EFFECT(CDN_EF_MODIFY_FATIQUE, 0, CDN_DAMAGE_1D10,   0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE),  0, CID_NONE, 1, 10),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_BODY_5, "impact critical body 5", "", NULL, NULL, "You jerk back from the force of the attack, throwing back your head and spewing out a yet of blood.", "Jerking back from the force of the attack, %s spews out a yet of blood.", 
                        EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                /* Right Arms */
                CONDITION(CID_IMPACT_CRITICAL_RARMS_1, "impact critical arms 1", "", "The attack numbs your right arm.", "The attack numbs %s's arm.", NULL, NULL,
                        EFFECT(CDN_EF_DISABLE_RARM, 0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1, 1),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_RARMS_2, "impact critical arms 2", "", "The impact inflicts crushing pain upon your right arm.", "The impact inflicts crushing pain upon %s's right arm.", NULL, NULL,
                        EFFECT(CDN_EF_STUNNED,      0, CDN_DAMAGE_NONE, 0),
                        EFFECT(CDN_EF_DISABLE_RARM, 0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1, 1),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_RARMS_3, "impact critical arms 3", "", "Muscle and bone take a pounding as the attack rips through your right arm.", "Muscle and bone take a pounding as the attack rips through %s right arm.", NULL, NULL,
                        EFFECT(CDN_EF_DISABLE_RARM, 0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 50, 200),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_RARMS_4, "impact critical arms 4", "", "With a loud snap, the right arm bone is shattered and left hanging limply at your side, dribbling blood onto the ground.", "With a loud snap, the right arm bone is shattered and left hanging limply at %s's side, dribbling blood onto the ground.", NULL, NULL,
                        EFFECT(CDN_EF_DISABLE_RARM, 0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), bf(CDN_SF_PERMANENT), CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_RARMS_5, "impact critical arms 5", "", NULL, NULL, "In a rain of blood, gore and meat, the your right arm is removed from your body. Screaming incoherently, you twists about in agony for a few seconds before collapsing to the ground.", "In a rain of blood, gore and meat, the right arm is removed from %s's body. Screaming incoherently, %s twists about in agony for a few seconds before collapsing to the ground.",
                        EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                /* Right Legs */
                CONDITION(CID_IMPACT_CRITICAL_RLEGS_1, "impact critical legs 1", "", "A light blow to the right leg leaves you gasping for air.", "A light blow to the right leg leave %s gasping for air.", NULL, NULL,
                        EFFECT(CDN_EF_MODIFY_FATIQUE, bf(CDN_ESF_REQ_TGH_CHECK), CDN_DAMAGE_ONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_RLEGS_2, "impact critical legs 2", "", "A solid blow to the right leg sends lightning agony coursing through your body.", "A solid blow to the right leg sends lightning agony coursing through a %s's body.", NULL, NULL,
                        EFFECT(CDN_EF_DISABLE_RLEG,   0, CDN_DAMAGE_NONE, 0),
                        EFFECT(CDN_EF_MODIFY_FATIQUE, 0, CDN_DAMAGE_1D5,  0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE),  0, CID_NONE, 1, 5),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_RLEGS_3, "impact critical legs 3", "", "A powerful impact to the right leg causes micro fractures in your bones, inflicting considerable agony.", "A powerful impact to the right leg causes micro fractures in your bones, inflicting considerable agony.", NULL, NULL,
                        EFFECT(CDN_EF_MODIFY_AG,      0, CDN_DAMAGE_1D5, 0),
                        EFFECT(CDN_EF_MODIFY_FATIQUE, 0, CDN_DAMAGE_1D5, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE),  0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_RLEGS_4, "impact critical legs 4", "", "With a nasty crunch, the right leg is broken and you are knocked down mewling in pain.", "With a nasty crunch, the right leg is broken and %s is knocked down mewling in pain.", NULL, NULL,
                        EFFECT(CDN_EF_DISABLE_RLEG,   0, CDN_DAMAGE_1D5, 0),
                        EFFECT(CDN_EF_MODIFY_FATIQUE, 0, CDN_DAMAGE_TWO, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE) | bf(CDN_SF_PERMANENT), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_RLEGS_5, "impact critical legs 5", "", NULL, NULL, "The hit rips apart the flesh of the right leg, causing blood to spray out in all directions. Even as you try futilely to stop the sudden flood of vital fluid, you fall to ground and die in a spreading pool of gore.", "The hit rips apart the flesh of the right leg, causing blood to spray out in all directions. Trying futilely to stop the sudden flood of vital fluid, %s falls to ground and dies in a spreading pool of gore.",
                        EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                /* Left Arms */
                CONDITION(CID_IMPACT_CRITICAL_LARMS_1, "impact critical arms 1", "", "The attack numbs your left arm.", "The attack numbs %s's arm.", NULL, NULL,
                        EFFECT(CDN_EF_DISABLE_LARM, 0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1, 1),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_LARMS_2, "impact critical arms 2", "", "The impact inflicts crushing pain upon your left arm.", "The impact inflicts crushing pain upon %s's left arm.", NULL, NULL,
                        EFFECT(CDN_EF_STUNNED,      0, CDN_DAMAGE_NONE, 0),
                        EFFECT(CDN_EF_DISABLE_LARM, 0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1, 1),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_LARMS_3, "impact critical arms 3", "", "Muscle and bone take a pounding as the attack rips through your left arm.", "Muscle and bone take a pounding as the attack rips through %s left arm.", NULL, NULL,
                        EFFECT(CDN_EF_DISABLE_RARM, 0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 50, 200),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_LARMS_4, "impact critical arms 4", "", "With a loud snap, the left arm bone is shattered and left hanging limply at your side, dribbling blood onto the ground.", "With a loud snap, the left arm bone is shattered and left hanging limply at %s's side, dribbling blood onto the ground.", NULL, NULL,
                        EFFECT(CDN_EF_DISABLE_LARM, 0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), bf(CDN_SF_PERMANENT), CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_LARMS_5, "impact critical arms 5", "", NULL, NULL, "In a rain of blood, gore and meat, the your left arm is removed from your body. Screaming incoherently, you twists about in agony for a few seconds before collapsing to the ground.", "In a rain of blood, gore and meat, the left arm is removed from %s's body. Screaming incoherently, %s twists about in agony for a few seconds before collapsing to the ground.",
                        EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                /* Left Legs */
                CONDITION(CID_IMPACT_CRITICAL_LLEGS_1, "impact critical legs 1", "", "A light blow to the left leg leaves you gasping for air.", "A light blow to the left leg leave %s gasping for air.", NULL, NULL,
                        EFFECT(CDN_EF_MODIFY_FATIQUE, bf(CDN_ESF_REQ_TGH_CHECK), CDN_DAMAGE_ONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_LLEGS_2, "impact critical legs 2", "", "A solid blow to the left leg sends lightning agony coursing through your body.", "A solid blow to the left leg sends lightning agony coursing through a %s's body.", NULL, NULL,
                        EFFECT(CDN_EF_DISABLE_LLEG,   0, CDN_DAMAGE_NONE, 0),
                        EFFECT(CDN_EF_MODIFY_FATIQUE, 0, CDN_DAMAGE_1D5,  0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE),  0, CID_NONE, 1, 5),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_LLEGS_3, "impact critical legs 3", "", "A powerful impact to the right leg causes micro fractures in your bones, inflicting considerable agony.", "A powerful impact to the right leg causes micro fractures in your bones, inflicting considerable agony.", NULL, NULL,
                        EFFECT(CDN_EF_MODIFY_AG,      0, CDN_DAMAGE_1D5, 0),
                        EFFECT(CDN_EF_MODIFY_FATIQUE, 0, CDN_DAMAGE_1D5, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE),  0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_LLEGS_4, "impact critical legs 4", "", "With a nasty crunch, the right leg is broken and you are knocked down mewling in pain.", "With a nasty crunch, the right leg is broken and %s is knocked down mewling in pain.", NULL, NULL,
                        EFFECT(CDN_EF_DISABLE_LLEG,   0, CDN_DAMAGE_1D5, 0),
                        EFFECT(CDN_EF_MODIFY_FATIQUE, 0, CDN_DAMAGE_TWO, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE) | bf(CDN_SF_PERMANENT), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_IMPACT_CRITICAL_LLEGS_5, "impact critical legs 5", "", NULL, NULL, "The hit rips apart the flesh of the right leg, causing blood to spray out in all directions. Even as you try futilely to stop the sudden flood of vital fluid, you fall to ground and die in a spreading pool of gore.", "The hit rips apart the flesh of the right leg, causing blood to spray out in all directions. Trying futilely to stop the sudden flood of vital fluid, %s falls to ground and dies in a spreading pool of gore.",
                        EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

    /* Rending critical hits */

    /*Head*/
    CONDITION(CID_RENDING_CRITICAL_HEAD_1, "rending critical head 1", "", "", "", "", "", 
            EFFECT(CDN_EF_MODIFY_FEL,  0, CDN_DAMAGE_TEN, 0), ),
            SETTINGS( bf(CDN_SF_UNIQUE) | bf(CDN_SF_ACTIVE_ALL), 0, CID_NONE, 1, 1),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_HEAD_2, "rending critical head 2", "", "", "", "", "", 
            EFFECT(CDN_EF_BLINDNESS,  0, CDN_DAMAGE_NONE, 0), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1, 1),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_HEAD_3, "rending critical head 3", "", "", "", "", "", 
            EFFECT(CDN_EF_MODIFY_FATIQUE, 0, CDN_DAMAGE_ONE,  0), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1, 1),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_HEAD_4, "rending critical head 4", "", "", "", "", "", 
            EFFECT(CDN_EF_MODIFY_FATIQUE, 0, CDN_DAMAGE_TWO,  0), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1, 5),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_HEAD_5, "rending critical head 5", "", NULL, "", NULL, "", 
            EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    /* Chest */
    CONDITION(CID_RENDING_CRITICAL_BODY_1, "rending critical body 1", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_BODY_2, "rending critical body 2", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_BODY_3, "rending critical body 3", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_BODY_4, "rending critical body 4", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_BODY_5, "rending critical body 5", "", NULL, NULL, NULL, NULL,
            EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    /* Arms */
    CONDITION(CID_RENDING_CRITICAL_RARMS_1, "rending critical arms 1", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_RARMS_2, "rending critical arms 2", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_RARMS_3, "rending critical arms 3", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_RARMS_4, "rending critical arms 4", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_RARMS_5, "rending critical arms 5", "", NULL, NULL, NULL, NULL,
            EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    /* Legs */
    CONDITION(CID_RENDING_CRITICAL_RLEGS_1, "rending critical legs 1", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_RLEGS_2, "rending critical legs 2", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_RLEGS_3, "rending critical legs 3", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_RLEGS_4, "rending critical legs 4", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_RLEGS_5, "rending critical legs 5", "", NULL, NULL, NULL, NULL,
            EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    /* Arms */
    CONDITION(CID_RENDING_CRITICAL_LARMS_1, "rending critical arms 1", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_LARMS_2, "rending critical arms 2", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_LARMS_3, "rending critical arms 3", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_LARMS_4, "rending critical arms 4", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_LARMS_5, "rending critical arms 5", "", NULL, NULL, NULL, NULL,
            EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    /* Legs */
    CONDITION(CID_RENDING_CRITICAL_LLEGS_1, "rending critical legs 1", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_LLEGS_2, "rending critical legs 2", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_LLEGS_3, "rending critical legs 3", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_LLEGS_4, "rending critical legs 4", "", NULL, NULL, NULL, NULL),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

    CONDITION(CID_RENDING_CRITICAL_LLEGS_5, "rending critical legs 5", "", NULL, NULL, NULL, NULL,
            EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
            SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
    CONDITION_END,

                /* Explosive critical hits */

                /*Head*/
                CONDITION(CID_EXPLOSIVE_CRITICAL_HEAD_1, "explosive critical head 1", "", "", "", "", "", 
                        EFFECT(CDN_EF_MODIFY_FEL,  0, CDN_DAMAGE_TEN, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE) | bf(CDN_SF_ACTIVE_ALL), 0, CID_NONE, 1, 1),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_HEAD_2, "explosive critical head 2", "", "", "", "", "", 
                        EFFECT(CDN_EF_BLINDNESS,  0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1, 1),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_HEAD_3, "explosive critical head 3", "", "", "", "", "", 
                        EFFECT(CDN_EF_MODIFY_FATIQUE, 0, CDN_DAMAGE_ONE,  0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1, 1),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_HEAD_4, "explosive critical head 4", "", "", "", "", "", 
                        EFFECT(CDN_EF_MODIFY_FATIQUE, 0, CDN_DAMAGE_TWO,  0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 1, 5),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_HEAD_5, "explosive critical head 5", "", NULL, "", NULL, "", 
                        EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                /* Chest */
                CONDITION(CID_EXPLOSIVE_CRITICAL_BODY_1, "explosive critical body 1", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_BODY_2, "explosive critical body 2", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_BODY_3, "explosive critical body 3", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_BODY_4, "explosive critical body 4", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_BODY_5, "explosive critical body 5", "", NULL, NULL, NULL, NULL,
                        EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                /* Arms */
                CONDITION(CID_EXPLOSIVE_CRITICAL_RARMS_1, "explosive critical arms 1", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_RARMS_2, "explosive critical arms 2", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_RARMS_3, "explosive critical arms 3", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_RARMS_4, "explosive critical arms 4", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_RARMS_5, "explosive critical arms 5", "", NULL, NULL, NULL, NULL,
                        EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                /* Legs */
                CONDITION(CID_EXPLOSIVE_CRITICAL_RLEGS_1, "explosive critical legs 1", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_RLEGS_2, "explosive critical legs 2", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_RLEGS_3, "explosive critical legs 3", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_RLEGS_4, "explosive critical legs 4", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_RLEGS_5, "explosive critical legs 5", "", NULL, NULL, NULL, NULL,
                        EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                /* Arms */
                CONDITION(CID_EXPLOSIVE_CRITICAL_LARMS_1, "explosive critical arms 1", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_LARMS_2, "explosive critical arms 2", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_LARMS_3, "explosive critical arms 3", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_LARMS_4, "explosive critical arms 4", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_LARMS_5, "explosive critical arms 5", "", NULL, NULL, NULL, NULL,
                        EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                /* Legs */
                CONDITION(CID_EXPLOSIVE_CRITICAL_LLEGS_1, "explosive critical legs 1", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_LLEGS_2, "explosive critical legs 2", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_LLEGS_3, "explosive critical legs 3", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_LLEGS_4, "explosive critical legs 4", "", NULL, NULL, NULL, NULL),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,

                CONDITION(CID_EXPLOSIVE_CRITICAL_LLEGS_5, "explosive critical legs 5", "", NULL, NULL, NULL, NULL,
                        EFFECT(CDN_EF_DEATH,  0, CDN_DAMAGE_NONE, 0), ),
                        SETTINGS( bf(CDN_SF_UNIQUE), 0, CID_NONE, 0, 0),
                CONDITION_END,
};

