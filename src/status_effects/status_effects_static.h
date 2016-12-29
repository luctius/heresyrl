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

#ifndef STATUS_EFFECTS_STATIC_H
#define STATUS_EFFECTS_STATIC_H

#define SE_NR_CRITICAL_HITS_PER_LOCATION (5)

enum se_ids {
    SEID_NONE,

    /* Draughts */
    SEID_MINOR_HEALING,

    /* Mushrooms */
    SEID_MAD_CAP_AFTER,
    SEID_MAD_CAP,

    /* Weapons */
    SEID_WEAPON_FLAME,

    /*
       Criticals -- it is required that these are in order! ie. HEAD1 , HEAD2, HEAD3 ...
       With exception to the healed ids.
     */
    SEID_BLUNT_LARM_1,
    SEID_BLUNT_LARM_2,
    SEID_BLUNT_LARM_3,
    SEID_BLUNT_LARM_4,
    SEID_BLUNT_LARM_5,
    SEID_BLUNT_RARM_1,
    SEID_BLUNT_RARM_2,
    SEID_BLUNT_RARM_3,
    SEID_BLUNT_RARM_4,
    SEID_BLUNT_RARM_5,
    SEID_BLUNT_ARM_3_HEALED,
    SEID_BLUNT_ARM_4_HEALED,

    /* Environment */
    SEID_SWIMMING,
    SEID_WADE,
    SEID_MUD,
    SEID_MAD_CAP_CLOUD,

    /* System */
    SEID_ENCUMBERED,
    SEID_STEALTH_ATTACKED,

    /*Death*/
    SEID_FATEHEALTH,
    SEID_DEATH_STIMM,

    SEID_MAX,
};

#endif /* STATUS_EFFECTS_STATIC_H */
