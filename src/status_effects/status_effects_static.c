#define STATUS_EFFECT(_id, _name, _desc, _of_plr, _of_msr, _oe_plr, _oe_msr, l...) \
                [_id] = { .uid=0, .template_id=_id, .name=_name, .description=_desc, .on_first_plr=_of_plr, \
                          .on_first_msr=_of_msr, .on_exit_plr=_oe_plr, .on_exit_msr=_oe_msr, .effects = { l }
#define STATUS_EFFECT_END }

#define GROUND(_icon, _iconattr, _dmin, _dmax, _desc) \
                .permissible_on_ground=true, .icon=_icon, .icon_attr=_iconattr, .see_description=_desc, \
                .grnd_duration_energy_min=(_dmin*TT_ENERGY_TURN), .grnd_duration_energy_max=(_dmax*TT_ENERGY_TURN)

#define SETTINGS(_flags, _diff, _con, _dmin, _dmax) .setting_flags=_flags, .difficulty=_diff, \
                    .continues_to_id=_con, .duration_energy_min=(_dmin*TT_ENERGY_TURN), .duration_energy_max=(_dmax*TT_ENERGY_TURN)

#define EFFECT(_effect, _flags, _dmg, _diff) \
                {.effect=_effect, .effect_setting_flags=bf(SESF_ACTIVE) | _flags, .strength=_dmg, \
                .difficulty=_diff, .tick_energy_max=1, .tick_energy=0, }

#define TICK_EFFECT(_effect, _flags, _dmg, _diff, _tmax) \
                {.effect=_effect, .effect_setting_flags=bf(SESF_ACTIVE) | bf(SESF_TICK) | _flags, \
                .strength=_dmg, .difficulty=_diff, .tick_energy_max=(_tmax*TT_ENERGY_TURN), .tick_energy=0, }

#define TALENT_EFFECT(_effect, _flags, _opt) \
                {.effect=_effect, .effect_setting_flags=bf(SESF_ACTIVE) | _flags, .optional=_opt, \
                .tick_energy_max=1, .tick_energy=0, }

static struct status_effect static_status_effect_list[] = {
    STATUS_EFFECT(SEID_NONE, "", "", NULL, NULL, NULL, NULL),
            SETTINGS( 0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_WITHDRAWL_HEALTH_STIMM, "health stimm withdrawl", "", "You start to crave for more health stimms.", "%s starts to shake.", "You overcome your cravings.", "%s regains his posture.", 
            /* Type         Effect        Flags     Damage     Difficulty   Interval   */
            TICK_EFFECT(SETF_DECREASE_WS,   0, SE_STRENGTH_ONE, 0,          5), 
            TICK_EFFECT(SETF_DECREASE_BS,   0, SE_STRENGTH_ONE, 0,          5), 
            TICK_EFFECT(SETF_DECREASE_STR,  0, SE_STRENGTH_ONE, 0,          5), 
            TICK_EFFECT(SETF_DECREASE_AG,   0, SE_STRENGTH_ONE, 0,          5), 
            TICK_EFFECT(SETF_DECREASE_TGH,  0, SE_STRENGTH_ONE, 0,          5), 
            TICK_EFFECT(SETF_DECREASE_PER,  0, SE_STRENGTH_ONE, 0,          5), 
            TICK_EFFECT(SETF_DECREASE_WILL, 0, SE_STRENGTH_ONE, 0,          5), 
            TICK_EFFECT(SETF_DECREASE_INT,  0, SE_STRENGTH_ONE, 0,          5), 
            TICK_EFFECT(SETF_DECREASE_FEL,  0, SE_STRENGTH_ONE, 0,          5), ),
            /*Settings      Flags                                Difficulty    Next     Minimum  -  Maximum Turns*/
            SETTINGS(bf(SEF_REQ_CHEM_USE_CHECK) | bf(SEF_DETOXABLE), -20,   SEID_NONE,   20,          50),
    STATUS_EFFECT_END,

    /* Stimms*/
    STATUS_EFFECT(SEID_HEALTH_STIMM, "health stimm", "", "Your wounds begin to heal.", "%s's wounds begin to heal.", "You feel the healing wearing off.", NULL, 
            /* Type         Effect        Flags   Strength     Difficulty   Interval   */
            TICK_EFFECT(SETF_HEALTH_TICK, 0,   SE_STRENGTH_ONE, 0,           1), ),
            /*Settings      Flags         Difficulty    Next                Minimum  -  Maximum Turns*/
            SETTINGS( bf(SEF_REMOVE_CONTINUE), 0, SEID_WITHDRAWL_HEALTH_STIMM, 10,        10),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_FATEHEALTH, "fatepoint_health", "", NULL, NULL, NULL, NULL, 
            /* Type         Effect      Flags     Strength     Difficulty  Interval */
            TICK_EFFECT(SETF_HEALTH_TICK, 0, SE_STRENGTH_4D10,    0,         1), ),
            /*Settings      Flags     Difficulty    Next       Minimum  -  Maximum Turns*/
            SETTINGS(       0,        0,          SEID_NONE,      2,         2),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_DEATH_STIMM, "death stimm", "", NULL, NULL, NULL, NULL,
            /* Type         Effect         Flags     Strength     Difficulty   Interval   */
            TICK_EFFECT(SETF_INSTANT_DEATH,  0,  SE_STRENGTH_NONE, 0,            0), ),
            /*Settings   Flags     Difficulty       Next    Minimum  -  Maximum Turns*/
            SETTINGS(       0,        0,          SEID_NONE,      0,          0),
    STATUS_EFFECT_END,

    /* Weapon Effects */
    STATUS_EFFECT(SEID_WEAPON_FLAME, "flames", "flames are engulving you", "You catch fire.", "%s has catched fire.", "You manage to put out the flames.", "%s stomps out the flames.", 
            /* Type         Effect           Flags                        Strength          Difficulty   Interval   */
            EFFECT(     SETF_ON_FIRE,          0,                       SE_STRENGTH_NONE,     0),
            TICK_EFFECT(SETF_DAMAGE_TICK, bf(SESF_DMG_TYPE_ENERGY),     SE_STRENGTH_1D10,     0,           1), ),
            /*Settings      Flags                       Difficulty    Next          Minimum  -  Maximum Turns*/
            SETTINGS(bf(SEF_REQ_AG_CHECK),   0,    SEID_NONE,         1,             10),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_WEAPON_FLAME_AREA, "flames", "flames are engulving you", "You catch fire.", "%s has catched fire.", "You manage to put out the flames.", "%s stomps out the flames.", 
            /* Type         Effect           Flags                        Strength          Difficulty   Interval   */
            EFFECT(     SETF_ON_FIRE,          0,                       SE_STRENGTH_NONE,     0),
            TICK_EFFECT(SETF_DAMAGE_TICK, bf(SESF_DMG_TYPE_ENERGY),     SE_STRENGTH_1D10,     0,           1), ),
            /*Settings      Flags                       Difficulty    Next          Minimum  -  Maximum Turns*/
            SETTINGS(bf(SEF_REQ_AG_CHECK),   0,    SEID_NONE,         1,             10),
            /*Ground,  Icon, Colour,  Minimum - Maximum Turns,  Description */
            GROUND(    '~',  0,        2,        4,           "Flames linger on the ground."),
    STATUS_EFFECT_END,







    /* Critical hits */

    /* Energy critical hits */

    /*Head*/
    /* -10 penalty on all characteristics for 1 round */
    STATUS_EFFECT(SEID_ENERGY_CRITICAL_HEAD_1, "energy critical head 1", "", "", "", "", "", 
            EFFECT(SETF_DECREASE_WS,   0, SE_STRENGTH_TEN, 0), 
            EFFECT(SETF_DECREASE_BS,   0, SE_STRENGTH_TEN, 0), 
            EFFECT(SETF_DECREASE_STR,  0, SE_STRENGTH_TEN, 0), 
            EFFECT(SETF_DECREASE_AG,   0, SE_STRENGTH_TEN, 0), 
            EFFECT(SETF_DECREASE_PER,  0, SE_STRENGTH_TEN, 0), 
            EFFECT(SETF_DECREASE_WILL, 0, SE_STRENGTH_TEN, 0), 
            EFFECT(SETF_DECREASE_INT,  0, SE_STRENGTH_TEN, 0), 
            EFFECT(SETF_DECREASE_FEL,  0, SE_STRENGTH_TEN, 0), ),
            SETTINGS(0, 0, SEID_NONE, 1, 1),
    STATUS_EFFECT_END,

    /* blinded for 1 round */
    STATUS_EFFECT(SEID_ENERGY_CRITICAL_HEAD_2, "energy critical head 2", "", "", "", "", "", 
            EFFECT(SETF_BLINDNESS,  0, SE_STRENGTH_NONE, 0), ),
            SETTINGS(0, 0, SEID_NONE, 1, 1),
    STATUS_EFFECT_END,

    /* stunned for 1 round, 1 lvl of fatique */
    STATUS_EFFECT(SEID_ENERGY_CRITICAL_HEAD_3, "energy critical head 3", "", "", "", "", "", 
            EFFECT(SETF_STUNNED,        0, SE_STRENGTH_NONE, 0),
            EFFECT(SETF_DECREASE_FATIQUE, 0, SE_STRENGTH_ONE,  0), ),
            SETTINGS(0, 0, SEID_NONE, 1, 1),
    STATUS_EFFECT_END,

    /* blindess for 1d5 rounds, fatique 2 lvls */
    STATUS_EFFECT(SEID_ENERGY_CRITICAL_HEAD_4, "energy critical head 4", "", "", "", "", "", 
            EFFECT(SETF_BLINDNESS,      0, SE_STRENGTH_NONE, 0),
            EFFECT(SETF_DECREASE_FATIQUE, 0, SE_STRENGTH_TWO,  0), ),
            SETTINGS(0, 0, SEID_NONE, 1, 5),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_HEAD_5, "energy critical head 5", "", NULL, "", NULL, "", 
            EFFECT(SETF_INSTANT_DEATH,  0, SE_STRENGTH_NONE, 0), ),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    /* Chest */
    STATUS_EFFECT(SEID_ENERGY_CRITICAL_BODY_1, "energy critical body 1", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_BODY_2, "energy critical body 2", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_BODY_3, "energy critical body 3", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_BODY_4, "energy critical body 4", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_BODY_5, "energy critical body 5", "", NULL, NULL, NULL, NULL,
            EFFECT(SETF_INSTANT_DEATH,  0, SE_STRENGTH_NONE, 0), ),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    /* Arms */
    STATUS_EFFECT(SEID_ENERGY_CRITICAL_RARMS_1, "energy critical arms 1", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_RARMS_2, "energy critical arms 2", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_RARMS_3, "energy critical arms 3", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_RARMS_4, "energy critical arms 4", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_RARMS_5, "energy critical arms 5", "", NULL, NULL, NULL, NULL,
            EFFECT(SETF_INSTANT_DEATH,  0, SE_STRENGTH_NONE, 0), ),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    /* Legs */
    STATUS_EFFECT(SEID_ENERGY_CRITICAL_RLEGS_1, "energy critical legs 1", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_RLEGS_2, "energy critical legs 2", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_RLEGS_3, "energy critical legs 3", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_RLEGS_4, "energy critical legs 4", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_RLEGS_5, "energy critical legs 5", "", NULL, NULL, NULL, NULL,
            EFFECT(SETF_INSTANT_DEATH,  0, SE_STRENGTH_NONE, 0), ),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    /* Arms */
    STATUS_EFFECT(SEID_ENERGY_CRITICAL_LARMS_1, "energy critical arms 1", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_LARMS_2, "energy critical arms 2", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_LARMS_3, "energy critical arms 3", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_LARMS_4, "energy critical arms 4", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_LARMS_5, "energy critical arms 5", "", NULL, NULL, NULL, NULL,
            EFFECT(SETF_INSTANT_DEATH,  0, SE_STRENGTH_NONE, 0), ),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    /* Legs */
    STATUS_EFFECT(SEID_ENERGY_CRITICAL_LLEGS_1, "energy critical legs 1", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_LLEGS_2, "energy critical legs 2", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_LLEGS_3, "energy critical legs 3", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_LLEGS_4, "energy critical legs 4", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_ENERGY_CRITICAL_LLEGS_5, "energy critical legs 5", "", NULL, NULL, NULL, NULL,
            EFFECT(SETF_INSTANT_DEATH,  0, SE_STRENGTH_NONE, 0), ),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

                /* Impact critical hits */

                /*Head*/
                STATUS_EFFECT(SEID_IMPACT_CRITICAL_HEAD_1, 
                            "impact critical head 1",
                            "",
                            "The impact fills your head with a ringing noise.",
                            "Reaching for his ears, %s shakes his head.",
                            NULL,
                            NULL, 
                        EFFECT(SETF_DECREASE_FATIQUE,  bf(SESF_REQ_TGH_CHECK) , SE_STRENGTH_ONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 1, 1),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_HEAD_2,
                            "impact critical head 2",
                            "",
                            "You are momentarily blinded by the blood spraying from your nose.",
                            "As the blood is spraying from %s's nose, it is momentarily blinded.",
                            NULL,
                            NULL, 
                        EFFECT(SETF_BLINDNESS,                         0, SE_STRENGTH_NONE, 0),
                        EFFECT(SETF_STUNNED,   bf(SESF_REQ_TGH_CHECK), SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 1, 1),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_HEAD_3,
                            "impact critical head 3",
                            "",
                            "As the blow impacts your head, you stumble and are momentarily stunned.",
                            "Taking a blow to the head, %s stumbles and is momentarily stunned.",
                            NULL,
                            NULL, 
                        EFFECT(SETF_STUNNED,        0,                     SE_STRENGTH_NONE, 0),
                        EFFECT(SETF_DECREASE_FATIQUE, 0,                     SE_STRENGTH_ONE,  0),
                        EFFECT(SETF_DECREASE_INT,     bf(SESF_PERMANENT), SE_STRENGTH_ONE,  0), ),
                        SETTINGS(0,  0, SEID_NONE, 1, 1),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_HEAD_4, 
                            "impact critical head 4",
                            "",
                            NULL,
                            NULL,
                            "Blood pours from your eyes, ears and nose as the attack pulverizes your brain.",
                            "Blood pours from %s eyes, ears and nose as the attack pulverizes the brain.", 
                        EFFECT(SETF_INSTANT_DEATH,         0, SE_STRENGTH_NONE,  0), ),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_HEAD_5,
                            "impact critical head 5",
                            "",
                            NULL,
                            NULL,
                            "Your head bursts open, spraying brain matter in all directions",
                            "%s head bursts open, spraying brain matter in all directions", 
                        EFFECT(SETF_EXPLODE,       0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                /* Chest */
                STATUS_EFFECT(SEID_IMPACT_CRITICAL_BODY_1,
                            "impact critical body 1",
                            "",
                            "The blow steals the air out of your lungs.",
                            "The blow steals the air from %s's lungs.",
                            NULL,
                            NULL,
                        EFFECT(SETF_STUNNED,       0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 0.5, 0.5),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_BODY_2, 
                            "impact critical body 2",
                            "",
                            "The attack breaks a rib.",
                            "The attack breaks %s's rib.",
                            NULL,
                            NULL, 
                        EFFECT(SETF_STUNNED, 0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 1.5, 1.5),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_BODY_3,
                            "impact critical body 3",
                            "",
                            "You receive a solid blow to the chest and you double over for a moment, clutching yourself in agony.",
                            "A solid blow to %s's chest causes him to double over in pain and agony.",
                            NULL,
                            NULL,
                        EFFECT(SETF_STUNNED,        0,                         SE_STRENGTH_NONE,  0),
                        EFFECT(SETF_DECREASE_FATIQUE, bf(SESF_REQ_TGH_CHECK), SE_STRENGTH_1D5,   0), ),
                        SETTINGS(0,  0, SEID_NONE, 2, 2),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_BODY_4,
                            "impact critical body 4",
                            "",
                            "The force of the attack ruptures several of your organs and knocks you down gasping in wretched pain.",
                            "The force of the attack ruptures several organs, knocking %s down, gasping in wretched pain.",
                            NULL,
                            NULL,
                        EFFECT(SETF_BLOODLOSS,      0, SE_STRENGTH_NONE,   0),
                        EFFECT(SETF_DECREASE_FATIQUE, 0, SE_STRENGTH_1D10,   0), ),
                        SETTINGS(0,  0, SEID_NONE, 1, 10),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_BODY_5, 
                            "impact critical body 5",
                            "",
                            NULL,
                            NULL,
                            "You jerk back from the force of the attack, throwing back your head and spewing out a yet of blood.",
                            "Jerking back from the force of the attack, %s spews out a yet of blood.", 
                        EFFECT(SETF_INSTANT_DEATH,         0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                /* Right Arms */
                STATUS_EFFECT(SEID_IMPACT_CRITICAL_RARMS_1,
                            "impact critical arms 1",
                            "",
                            "The attack numbs your right arm.",
                            "The attack numbs %s's arm.",
                            NULL,
                            NULL,
                        EFFECT(SETF_DISABLE_RARM,  0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 1, 1),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_RARMS_2,
                            "impact critical arms 2",
                            "",
                            "The impact inflicts crushing pain upon your right arm.",
                            "The impact inflicts crushing pain upon %s's right arm.",
                            NULL,
                            NULL,
                        EFFECT(SETF_STUNNED,       0, SE_STRENGTH_NONE, 0),
                        EFFECT(SETF_DISABLE_RARM,  0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 1, 1),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_RARMS_3,
                            "impact critical arms 3",
                            "",
                            "Muscle and bone take a pounding as the attack rips through your right arm.",
                            "Muscle and bone take a pounding as the attack rips through %s right arm.", 
                            NULL, 
                            NULL,
                        EFFECT(SETF_DISABLE_RARM,  0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 50, 200),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_RARMS_4,
                            "impact critical arms 4",
                            "",
                            "With a loud snap, the right arm bone is shattered and left hanging limply at your side, dribbling blood onto the ground.",
                            "With a loud snap, the right arm bone is shattered and left hanging limply at %s's side, dribbling blood onto the ground.",
                            NULL,
                            NULL,
                        EFFECT(SETF_DISABLE_RARM,  0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, bf(SEF_PERMANENT), SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_RARMS_5,
                            "impact critical arms 5",
                            "",
                            NULL,
                            NULL,
                            "In a rain of blood, gore and meat, the your right arm is removed from your body. Screaming incoherently, you twists about in agony for a few seconds before collapsing to the ground.",
                            "In a rain of blood, gore and meat, the right arm is removed from %s's body. Screaming incoherently, it twists about in agony for a few seconds before collapsing to the ground.",
                        EFFECT(SETF_INSTANT_DEATH,         0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                /* Right Legs */
                STATUS_EFFECT(SEID_IMPACT_CRITICAL_RLEGS_1,
                            "impact critical legs 1",
                            "",
                            "A light blow to the right leg leaves you gasping for air.",
                            "A light blow to the right leg leave %s gasping for air.",
                            NULL,
                            NULL,
                        EFFECT(SETF_DECREASE_FATIQUE, bf(SESF_REQ_TGH_CHECK), SE_STRENGTH_ONE, 0), ),
                        SETTINGS(0,  0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_RLEGS_2,
                            "impact critical legs 2",
                            "",
                            "A solid blow to the right leg sends lightning agony coursing through your body.",
                            "A solid blow to the right leg sends lightning agony coursing through a %s's body.",
                            NULL,
                            NULL,
                        EFFECT(SETF_DISABLE_RLEG,   0, SE_STRENGTH_NONE, 0),
                        EFFECT(SETF_DECREASE_FATIQUE, 0, SE_STRENGTH_1D5,  0), ),
                        SETTINGS(0,  0, SEID_NONE, 1, 5),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_RLEGS_3,
                            "impact critical legs 3",
                            "",
                            "A powerful impact to the right leg causes micro fractures in your bones, inflicting considerable agony.",
                            "A powerful impact to the right leg causes micro fractures in %s's bones, inflicting considerable agony.",
                            NULL,
                            NULL,
                        EFFECT(SETF_DECREASE_AG,      0, SE_STRENGTH_1D5, 0),
                        EFFECT(SETF_DECREASE_FATIQUE, 0, SE_STRENGTH_1D5, 0), ),
                        SETTINGS(0,  0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_RLEGS_4,
                            "impact critical legs 4",
                            "",
                            "With a nasty crunch, the right leg is broken and you are knocked down mewling in pain.",
                            "With a nasty crunch, the right leg is broken and %s is knocked down mewling in pain.",
                            NULL,
                            NULL,
                        EFFECT(SETF_DISABLE_RLEG,   0, SE_STRENGTH_1D5, 0),
                        EFFECT(SETF_DECREASE_FATIQUE, 0, SE_STRENGTH_TWO, 0), ),
                        SETTINGS(0 | bf(SEF_PERMANENT), 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_RLEGS_5,
                            "impact critical legs 5",
                            "",
                            NULL,
                            NULL,
                            "The hit rips apart the flesh of the right leg, causing blood to spray out in all directions. Even as you try futilely to stop the sudden flood of vital fluid, you fall to ground and die in a spreading pool of gore.",
                            "The hit rips apart the flesh of the right leg, causing blood to spray out in all directions. Trying futilely to stop the sudden flood of vital fluid, %s falls to ground and dies in a spreading pool of gore.",
                        EFFECT(SETF_INSTANT_DEATH,         0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                /* Left Arms */
                STATUS_EFFECT(SEID_IMPACT_CRITICAL_LARMS_1,
                            "impact critical arms 1",
                            "",
                            "The attack numbs your left arm.",
                            "The attack numbs %s's arm.",
                            NULL,
                            NULL,
                        EFFECT(SETF_DISABLE_LARM,  0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 1, 1),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_LARMS_2,
                            "impact critical arms 2",
                            "",
                            "The impact inflicts crushing pain upon your left arm.",
                            "The impact inflicts crushing pain upon %s's left arm.",
                            NULL,
                            NULL,
                        EFFECT(SETF_STUNNED,      0, SE_STRENGTH_NONE, 0),
                        EFFECT(SETF_DISABLE_LARM, 0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 1, 1),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_LARMS_3,
                            "impact critical arms 3",
                            "",
                            "Muscle and bone take a pounding as the attack rips through your left arm.",
                            "Muscle and bone take a pounding as the attack rips through %s left arm.",
                            NULL,
                            NULL,
                        EFFECT(SETF_DISABLE_RARM,  0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 50, 200),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_LARMS_4,
                            "impact critical arms 4",
                            "",
                            "With a loud snap, the left arm bone is shattered and left hanging limply at your side, dribbling blood onto the ground.",
                            "With a loud snap, the left arm bone is shattered and left hanging limply at %s's side, dribbling blood onto the ground.",
                            NULL,
                            NULL,
                        EFFECT(SETF_DISABLE_LARM,  0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, bf(SEF_PERMANENT), SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_LARMS_5,
                            "impact critical arms 5",
                            "",
                            NULL,
                            NULL,
                            "In a rain of blood, gore and meat, the your left arm is removed from your body. Screaming incoherently, you twists about in agony for a few seconds before collapsing to the ground.",
                            "In a rain of blood, gore and meat, the left arm is removed from %s's body. Screaming incoherently, it twists about in agony for a few seconds before collapsing to the ground.",
                        EFFECT(SETF_INSTANT_DEATH,         0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                /* Left Legs */
                STATUS_EFFECT(SEID_IMPACT_CRITICAL_LLEGS_1,
                            "impact critical legs 1",
                            "",
                            "A light blow to the left leg leaves you gasping for air.",
                            "A light blow to the left leg leave %s gasping for air.",
                            NULL,
                            NULL,
                        EFFECT(SETF_DECREASE_FATIQUE, bf(SESF_REQ_TGH_CHECK), SE_STRENGTH_ONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_LLEGS_2,
                            "impact critical legs 2",
                            "",
                            "A solid blow to the left leg sends lightning agony coursing through your body.",
                            "A solid blow to the left leg sends lightning agony coursing through a %s's body.",
                            NULL,
                            NULL,
                        EFFECT(SETF_DISABLE_LLEG,   0, SE_STRENGTH_NONE, 0),
                        EFFECT(SETF_DECREASE_FATIQUE, 0, SE_STRENGTH_1D5,  0), ),
                        SETTINGS(0,  0, SEID_NONE, 1, 5),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_LLEGS_3,
                                "impact critical legs 3",
                                "",
                                "A powerful impact to the right leg causes micro fractures in your bones, inflicting considerable agony.",
                                "A powerful impact to the right leg causes micro fractures in your bones, inflicting considerable agony.",
                                NULL,
                                NULL,
                        EFFECT(SETF_DECREASE_AG,      0, SE_STRENGTH_1D5, 0),
                        EFFECT(SETF_DECREASE_FATIQUE, 0, SE_STRENGTH_1D5, 0), ),
                        SETTINGS(0,  0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_LLEGS_4,
                            "impact critical legs 4",
                            "",
                            "With a nasty crunch, the right leg is broken and you are knocked down mewling in pain.",
                            "With a nasty crunch, the right leg is broken and %s is knocked down mewling in pain.",
                            NULL,
                            NULL,
                        EFFECT(SETF_DISABLE_LLEG,   0, SE_STRENGTH_1D5, 0),
                        EFFECT(SETF_DECREASE_FATIQUE, 0, SE_STRENGTH_TWO, 0), ),
                        SETTINGS(0 | bf(SEF_PERMANENT), 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_IMPACT_CRITICAL_LLEGS_5,
                            "impact critical legs 5",
                            "",
                            NULL,
                            NULL,
                            "The hit rips apart the flesh of the right leg, causing blood to spray out in all directions. Even as you try futilely to stop the sudden flood of vital fluid, you fall to ground and die in a spreading pool of gore.",
                            "The hit rips apart the flesh of the right leg, causing blood to spray out in all directions. Trying futilely to stop the sudden flood of vital fluid, %s falls to ground and dies in a spreading pool of gore.",
                        EFFECT(SETF_INSTANT_DEATH,         0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

    /* Rending critical hits */

    /*Head*/
    STATUS_EFFECT(SEID_RENDING_CRITICAL_HEAD_1, "rending critical head 1", "", "", "", "", "", 
            EFFECT(SETF_DECREASE_FEL,  0, SE_STRENGTH_TEN, 0), ),
            SETTINGS(0, 0, SEID_NONE, 1, 1),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_HEAD_2, "rending critical head 2", "", "", "", "", "", 
            EFFECT(SETF_BLINDNESS,  0, SE_STRENGTH_NONE, 0), ),
            SETTINGS(0, 0, SEID_NONE, 1, 1),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_HEAD_3, "rending critical head 3", "", "", "", "", "", 
            EFFECT(SETF_DECREASE_FATIQUE, 0, SE_STRENGTH_ONE,  0), ),
            SETTINGS(0, 0, SEID_NONE, 1, 1),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_HEAD_4, "rending critical head 4", "", "", "", "", "", 
            EFFECT(SETF_DECREASE_FATIQUE, 0, SE_STRENGTH_TWO,  0), ),
            SETTINGS(0, 0, SEID_NONE, 1, 5),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_HEAD_5, "rending critical head 5", "", NULL, "", NULL, "", 
            EFFECT(SETF_INSTANT_DEATH,  0, SE_STRENGTH_NONE, 0), ),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    /* Chest */
    STATUS_EFFECT(SEID_RENDING_CRITICAL_BODY_1, "rending critical body 1", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_BODY_2, "rending critical body 2", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_BODY_3, "rending critical body 3", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_BODY_4, "rending critical body 4", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_BODY_5, "rending critical body 5", "", NULL, NULL, NULL, NULL,
            EFFECT(SETF_INSTANT_DEATH,  0, SE_STRENGTH_NONE, 0), ),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    /* Arms */
    STATUS_EFFECT(SEID_RENDING_CRITICAL_RARMS_1, "rending critical arms 1", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_RARMS_2, "rending critical arms 2", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_RARMS_3, "rending critical arms 3", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_RARMS_4, "rending critical arms 4", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_RARMS_5, "rending critical arms 5", "", NULL, NULL, NULL, NULL,
            EFFECT(SETF_INSTANT_DEATH,  0, SE_STRENGTH_NONE, 0), ),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    /* Legs */
    STATUS_EFFECT(SEID_RENDING_CRITICAL_RLEGS_1, "rending critical legs 1", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_RLEGS_2, "rending critical legs 2", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_RLEGS_3, "rending critical legs 3", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_RLEGS_4, "rending critical legs 4", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_RLEGS_5, "rending critical legs 5", "", NULL, NULL, NULL, NULL,
            EFFECT(SETF_INSTANT_DEATH,  0, SE_STRENGTH_NONE, 0), ),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    /* Arms */
    STATUS_EFFECT(SEID_RENDING_CRITICAL_LARMS_1, "rending critical arms 1", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_LARMS_2, "rending critical arms 2", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_LARMS_3, "rending critical arms 3", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_LARMS_4, "rending critical arms 4", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_LARMS_5, "rending critical arms 5", "", NULL, NULL, NULL, NULL,
            EFFECT(SETF_INSTANT_DEATH,  0, SE_STRENGTH_NONE, 0), ),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    /* Legs */
    STATUS_EFFECT(SEID_RENDING_CRITICAL_LLEGS_1, "rending critical legs 1", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_LLEGS_2, "rending critical legs 2", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_LLEGS_3, "rending critical legs 3", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_LLEGS_4, "rending critical legs 4", "", NULL, NULL, NULL, NULL),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

    STATUS_EFFECT(SEID_RENDING_CRITICAL_LLEGS_5, "rending critical legs 5", "", NULL, NULL, NULL, NULL,
            EFFECT(SETF_INSTANT_DEATH,  0, SE_STRENGTH_NONE, 0), ),
            SETTINGS(0, 0, SEID_NONE, 0, 0),
    STATUS_EFFECT_END,

                /* Explosive critical hits */

                /*Head*/
                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_HEAD_1, "explosive critical head 1", "", "", "", "", "", 
                        EFFECT(SETF_DECREASE_FEL,  0, SE_STRENGTH_TEN, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 1, 1),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_HEAD_2, "explosive critical head 2", "", "", "", "", "", 
                        EFFECT(SETF_BLINDNESS,  0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 1, 1),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_HEAD_3, "explosive critical head 3", "", "", "", "", "", 
                        EFFECT(SETF_DECREASE_FATIQUE, 0, SE_STRENGTH_ONE,  0), ),
                        SETTINGS(0, 0, SEID_NONE, 1, 1),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_HEAD_4, "explosive critical head 4", "", "", "", "", "", 
                        EFFECT(SETF_DECREASE_FATIQUE, 0, SE_STRENGTH_TWO,  0), ),
                        SETTINGS(0, 0, SEID_NONE, 1, 5),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_HEAD_5, "explosive critical head 5", "", NULL, "", NULL, "", 
                        EFFECT(SETF_INSTANT_DEATH,  0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                /* Chest */
                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_BODY_1, "explosive critical body 1", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_BODY_2, "explosive critical body 2", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_BODY_3, "explosive critical body 3", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_BODY_4, "explosive critical body 4", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_BODY_5, "explosive critical body 5", "", NULL, NULL, NULL, NULL,
                        EFFECT(SETF_INSTANT_DEATH,  0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                /* Arms */
                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_RARMS_1, "explosive critical arms 1", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_RARMS_2, "explosive critical arms 2", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_RARMS_3, "explosive critical arms 3", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_RARMS_4, "explosive critical arms 4", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_RARMS_5, "explosive critical arms 5", "", NULL, NULL, NULL, NULL,
                        EFFECT(SETF_INSTANT_DEATH,  0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                /* Legs */
                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_RLEGS_1, "explosive critical legs 1", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_RLEGS_2, "explosive critical legs 2", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_RLEGS_3, "explosive critical legs 3", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_RLEGS_4, "explosive critical legs 4", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_RLEGS_5, "explosive critical legs 5", "", NULL, NULL, NULL, NULL,
                        EFFECT(SETF_INSTANT_DEATH,  0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                /* Arms */
                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_LARMS_1, "explosive critical arms 1", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_LARMS_2, "explosive critical arms 2", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_LARMS_3, "explosive critical arms 3", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_LARMS_4, "explosive critical arms 4", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_LARMS_5, "explosive critical arms 5", "", NULL, NULL, NULL, NULL,
                        EFFECT(SETF_INSTANT_DEATH,  0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                /* Legs */
                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_LLEGS_1, "explosive critical legs 1", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_LLEGS_2, "explosive critical legs 2", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_LLEGS_3, "explosive critical legs 3", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_LLEGS_4, "explosive critical legs 4", "", NULL, NULL, NULL, NULL),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,

                STATUS_EFFECT(SEID_EXPLOSIVE_CRITICAL_LLEGS_5, "explosive critical legs 5", "", NULL, NULL, NULL, NULL,
                        EFFECT(SETF_INSTANT_DEATH,  0, SE_STRENGTH_NONE, 0), ),
                        SETTINGS(0, 0, SEID_NONE, 0, 0),
                STATUS_EFFECT_END,
};

