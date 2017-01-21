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



const struct ground_effect static_ground_effect_list[] = {
    [GEID_FLAME_AREA] = {
        .uid = 0,
        .tid = GEID_FLAME_AREA,
        .flags = 0,

        .sd_name = L"flames",
        .ld_name = L"flames",
        .description = L"",

        .icon = L'~',
        .icon_attr = TERM_COLOUR_RED,

        .min_energy = TT_ENERGY_TURN * 1,
        .max_energy = TT_ENERGY_TURN * 10,
        .se_id = SEID_WEAPON_FLAME,
    },
    [GEID_MAD_CAP_CLOUD] = {
        .uid = 0,
        .tid = GEID_MAD_CAP_CLOUD,
        .flags = 0,

        .sd_name = L"mad cap poison cloud",
        .ld_name = L"a cloud of mad cap poison",
        .description = L"",

        .icon = L'~',
        .icon_attr = TERM_COLOUR_GREEN,

        .min_energy = TT_ENERGY_TURN * 3,
        .max_energy = TT_ENERGY_TURN * 5,
        .se_id = SEID_MAD_CAP_CLOUD,
    },
};

