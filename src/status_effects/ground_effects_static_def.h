

const struct ground_effect static_ground_effect_list[] = {
    [GEID_FLAME_AREA] = {
        .uid = 0,
        .tid = GEID_FLAME_AREA,
        .flags = 0,

        .sd_name = "flames",
        .ld_name = "flames",
        .description = "",

        .icon = '~',
        .icon_attr = TERM_COLOUR_RED,

        .min_energy = TT_ENERGY_TURN * 1,
        .max_energy = TT_ENERGY_TURN * 10,
        .se_id = SEID_WEAPON_FLAME,
    },
    [GEID_MAD_CAP_CLOUD] = {
        .uid = 0,
        .tid = GEID_MAD_CAP_CLOUD,
        .flags = 0,

        .sd_name = "mad cap poison cloud",
        .ld_name = "a cloud of mad cap poison",
        .description = "",

        .icon = '~',
        .icon_attr = TERM_COLOUR_GREEN,

        .min_energy = TT_ENERGY_TURN * 3,
        .max_energy = TT_ENERGY_TURN * 5,
        .se_id = SEID_MAD_CAP_CLOUD,
    },
};

