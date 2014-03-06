/*
   This file is included in items.c.
   This is purely my sanity that it is seperate.
   TODO create a nice solution.
 */

#define AVERAGE_TORCH_DESC "This a generic torch."
#define AVERAGE_STUB_AUTOMATIC_DESC "This a generic stub automatic."

#define CLOTHING(item_id,item_sd_name,item_ld_name,item_desc,cloth_typ,dr,avail,item_quality,item_weight,item_cost,delay) \
    [item_id] = { .save_id=0, .list_id=item_id, .item_type=ITEM_TYPE_WEARABLE, .availability=avail, .quality=item_quality, \
    .attributes=ITEM_ATTRIBUTE_NONE, .age=0, .weight=item_weight, .cost=item_cost, .sd_name=item_sd_name, \
    .ld_name=item_ld_name, .description=item_desc, .icon=']', .icon_attr=COLOR_PAIR(DPL_COLOUR_NORMAL), .use_delay=delay, \
    .stacked_quantity=0, .max_quantity=1, .dropable=true, .specific.wearable = { .wearable_type=cloth_typ, .damage_reduction=dr, }, }

#define MELEE(item_id,item_sd_name,item_ld_name,item_desc,wpn_cat,dmg_die,dmg_add,dmg_tp,pen,avail,item_quality,item_weight,item_cost,delay,special) \
    [item_id] = { .save_id=0, .list_id=item_id, .item_type=ITEM_TYPE_WEAPON, .availability=avail, .quality=item_quality, \
    .attributes=ITEM_ATTRIBUTE_NONE, .age=0, .weight=item_weight, .cost=item_cost, .sd_name=item_sd_name, \
    .ld_name=item_ld_name, .description=item_desc, .icon='|', .icon_attr=COLOR_PAIR(DPL_COLOUR_NORMAL), .use_delay=delay, \
    .stacked_quantity=0, .max_quantity=1, .dropable=true, .specific.weapon = { .weapon_type=WEAPON_TYPE_MELEE, .weapon_category=wpn_cat, \
    .dmg_type=dmg_tp, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=0, .penetration=pen, .special_quality=special, .jammed=false, }, }

#define RANGED(item_id,item_sd_name,item_ld_name,item_desc,wpn_cat,dmg_die,dmg_add,dmg_tp,pen,ran,rofsi,rofse,rofau,magsz,avail,item_quality,item_weight,item_cost,delay,special) \
    [item_id] = { .save_id=0, .list_id=item_id, .item_type=ITEM_TYPE_WEAPON, .availability=avail, .quality=item_quality, \
    .attributes=ITEM_ATTRIBUTE_NONE, .age=0, .weight=item_weight, .cost=item_cost, .sd_name=item_sd_name, \
    .ld_name=item_ld_name, .description=item_desc, .icon='|', .icon_attr=COLOR_PAIR(DPL_COLOUR_NORMAL), .use_delay=delay, \
    .stacked_quantity=0, .max_quantity=1, .dropable=true, .specific.weapon = { .weapon_type=WEAPON_TYPE_RANGED, .weapon_category=wpn_cat, \
    .dmg_type=dmg_tp, .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=ran, .rof = { [WEAPON_ROF_SETTING_SINGLE]=rofsi, \
    [WEAPON_ROF_SETTING_SEMI]=rofse, [WEAPON_ROF_SETTING_AUTO]=rofau, }, .magazine_sz=magsz, .magazine_left=magsz, \
    .penetration=pen, .special_quality=special, .jammed=false, }, }

#define LIGHT(item_id,item_sd_name,item_ld_name,item_desc,lumin,dur,avail,item_quality,item_weight,item_cost,delay) \
    [item_id] = { .save_id=0, .list_id=item_id, .item_type=ITEM_TYPE_TOOL, .availability=avail, .quality=item_quality, \
    .attributes=ITEM_ATTRIBUTE_NONE, .age=0, .weight=item_weight, .cost=item_cost, .sd_name=item_sd_name, \
    .ld_name=item_ld_name, .description=item_desc, .icon='(', .icon_attr=COLOR_PAIR(DPL_COLOUR_NORMAL), .use_delay=delay, \
    .stacked_quantity=1, .max_quantity=100, .dropable=true, .specific.tool = { .tool_type=TOOL_TYPE_LIGHT, .energy=dur, \
    .energy_left=dur, .light_luminem=lumin, .lit=false, }, }

#define AMMO(item_id,item_sd_name,item_ld_name,item_desc,ammo_typ,energ,avail,item_quality,item_weight,item_cost,delay) \
    [item_id] = { .save_id=0, .list_id=item_id, .item_type=ITEM_TYPE_AMMO, .availability=avail, .quality=item_quality, \
    .attributes=ITEM_ATTRIBUTE_NONE, .age=0, .weight=item_weight, .cost=item_cost, .sd_name=item_sd_name, \
    .ld_name=item_ld_name, .description=item_desc, .icon='\'', .icon_attr=COLOR_PAIR(DPL_COLOUR_NORMAL), .use_delay=delay, \
    .stacked_quantity=1, .max_quantity=100, .dropable=true, .specific.ammo = { .ammo_type=ammo_typ, .energy=energ, energy_left=energ, }, }

struct itm_item static_item_list[] = {
    LIGHT(ITEM_ID_AVERAGE_TORCH,"torch","a torch",AVERAGE_TORCH_DESC,3,100,ITEM_AVAILABILITY_PLENTIFUL,ITEM_QUALITY_AVERAGE,1,1,0),
    RANGED(ITEM_ID_AVERAGE_STUB_AUTOMATIC,"stub automatic","a stub automatic","",WEAPON_CATEGORY_PISTOL,1,3,
                WEAPON_DMG_TYPE_IMPACT,0,30,1,3,0,9,ITEM_AVAILABILITY_PLENTIFUL,ITEM_QUALITY_AVERAGE,15,50,1,0),
    RANGED(ITEM_ID_AVERAGE_STUB_REVOLVER,"stub revolver","a stub revolver","",WEAPON_CATEGORY_PISTOL,1,3,
                WEAPON_DMG_TYPE_IMPACT,0,6,1,0,0,9,ITEM_AVAILABILITY_PLENTIFUL,ITEM_QUALITY_AVERAGE,15,50,1,WEAPON_SPEC_QUALITY_RELIABLE),

};

