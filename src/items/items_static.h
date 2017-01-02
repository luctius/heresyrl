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

#ifndef ITEMS_STATIC_H
#define ITEMS_STATIC_H

enum item_ids {
    IID_NONE,

    IID_MONEY,

    /* Wearables*/
    IID_FLAK_HELMET,
    IID_FLAK_GAUNTLETS,
    IID_FLAK_LIGHT_COAT,
    IID_FLAK_VEST,
    IID_FLAK_JACKET,
    IID_FLAK_CLOAK,
    IID_FLAK_GUARD_ARMOUR,

    /* Tools */
    /* Fixed Items */
    IID_FIXED_LIGHT,

    /* Tools */
    IID_GLOW_GLOBE,

    /* Weapons */

    /* Melee */
    IID_KNIFE,
    IID_SWORD,

    /* 2h Melee */
    IID_2H_SWORD,

    /* Ranged */
    /* Primitive */
    IID_SHORT_BOW,
    IID_LONG_BOW,

    /* Pistols */
    IID_STUB_AUTOMATIC,
    IID_STUB_REVOLVER,
    IID_LAS_PISTOL,

    /* Rifles */
    IID_LAS_GUN,
    IID_AUTO_GUN,

    /* Thrown */
    IID_THROWING_KNIFE,

    /* Grenades */
    IID_FIRE_BOMB,
    IID_FRAG_GRENADE,

    /* Ammo */
    IID_ARROW,
    IID_PISTOL_AMMO_SP,
    IID_PISTOL_AMMO_LAS,
    IID_BASIC_AMMO_SP,
    IID_BASIC_AMMO_LAS,
    
    /* Food */
    /* Potions */
     IID_STIMM_HEALTH_MINOR,

    /* Mushrooms */
    IID_MUSHROOM_MAD_CAP,

    /* Creature Attacks */
    IID_HUMAN_UNARMED,
    IID_CREATURE_BITE_UNTRAINED,
    IID_CREATURE_BITE_TRAINED,

    /* status effect items */
    IID_BODYPART_GRENADE,

    /* Debug */
    IID_STIMM_DEATH,

    /* Last Entry */
    IID_MAX,
};

#endif /* ITEMS_STATIC_H */
