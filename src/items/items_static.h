#ifndef ITEMS_STATIC_H
#define ITEMS_STATIC_H

enum item_ids {
    IID_NONE,

    /* Wearables*/
    IID_LEATHER_SKULLCAP,
    IID_LEATHER_JERKIN,
    IID_LEATHER_JACK,
    IID_LEATHER_LEGGINS,
    IID_LEATHER_FULL,

    IID_STDD_LEATHER_SKULLCAP,
    IID_STDD_LEATHER_JERKIN,
    IID_STDD_LEATHER_JACK,
    IID_STDD_LEATHER_LEGGINS,
    IID_STDD_LEATHER_FULL,

    IID_MAIL_COIF,
    IID_MAIL_SHIRT,
    IID_MAIL_SLEEVED_SHIRT,
    IID_MAIL_COAT,
    IID_MAIL_SLEEVED_COAT,
    IID_MAIL_LEGGINS,
    IID_MAIL_FULL,

    /* Tools */
    /* Fixed Items */
    IID_FIXED_LIGHT,

    /* Tools */
    IID_TORCH,

    /* Weapons */
    /* Creature Attacks */
    IID_HUMAN_UNARMED,
    IID_CREATURE_BITE_UNTRAINED,
    IID_CREATURE_BITE_TRAINED,

    /* Melee */
    IID_KNIFE,
    IID_HAND_WEAPON,
    IID_AXE_GOOD,
    IID_PICK_GOOD,
    IID_HAMMER_GOOD,
    IID_SWORD_GOOD,

    /* Ranged */
    IID_SHORT_BOW,
    IID_LONG_BOW,

    /* Thrown */
    IID_THROWING_KNIFE,

    /* Grenades */
    IID_FIRE_BOMB,

    /* Ammo */
    IID_ARROW,

    /* Food */
    /* Stimms */
    IID_STIMM_HEALTH,
    IID_STIMM_DEATH,

    /* status effect items */
    IID_BODYPART_GRENADE,

    /* Last Entry */
    IID_MAX,
};

#endif /* ITEMS_STATIC_H */
