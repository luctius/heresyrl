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

#define QUEST(_tid, _type)  \
    [_tid] = { \
        .tid  = _tid, \
        .type = _type
#define QUEST_END }

#define DESCRIPTION(des)    .description=des
#define DESCR_ENTRANCE(des) .start=des
#define DESCR_EXIT(des)     .end=des
#define REWARDS(xp, gold) \
    .xp_reward=xp, \
    .gp_reward=gold

#define WEIGHTS(wght,minlvl,maxlvl) \
    .weight=wght, .min_level=minlvl, .max_level=maxlvl

#define DUNGEON(nr, tp, lvls, wgt, i_c, m_c, sx, sy) \
    .dungeon[nr] = { \
        .type = tp, \
        .dungeon_levels = lvls, \
        .weight = wgt, \
        .item_chance = i_c, \
        .monster_chance = m_c, \
        .size = { .x = sx, .y = sy }, \
    }

#define ENEMIES(nr, tp, wgt) \
    .enemies[nr] = { \
        .race = tp, \
        .weight = wgt, \
    }

#define FETCH_PARAMS(start, item, item_cnt) \
    .qst_params[start*2] = item, .qst_params[(start*2)+1] = item_cnt

static struct quest static_quest_list[] = {
    QUEST(QSTID_WISE_WOMAN, QST_TYPE_FETCH),
        DESCRIPTION("fetch mushrooms"),
        DESCR_ENTRANCE(cs_PLAYER "You" cs_CLOSE " arrived at the place where the wise woman asked " cs_PLAYER "you" cs_CLOSE " to search for her mushrooms."),
        DESCR_EXIT("The wise woman was extremely gratefull when " cs_PLAYER "you" cs_CLOSE " returned with the mushrooms."),
        REWARDS(200, 5),
        WEIGHTS(10, 1, 10),
        DUNGEON(0, DUNGEON_TYPE_CAVE,  1, 10, 50, 150, 100, 100),
        DUNGEON(1, DUNGEON_TYPE_PLAIN, 1, 1,  50, 150, 100, 100),
        ENEMIES(0, MSR_RACE_GREENSKIN, 8),
        ENEMIES(1, MSR_RACE_BEAST, 2),
        FETCH_PARAMS(0, IID_MUSHROOM_MAD_CAP, 5),
        FETCH_PARAMS(1, IID_NONE, 0),
    QUEST_END,

    QUEST(QSTID_MAX, QST_TYPE_NONE),
    QUEST_END,
};

const char *quest_description_templates[] = {
    [QST_TYPE_NONE] = "",
    [QST_TYPE_KILL_ALL] = cs_PLAYER "You" cs_CLOSE " must kill all %s's.",
    [QST_TYPE_KILL_LEADER] = cs_PLAYER "You" cs_CLOSE " must kill the leader, %s.",
    [QST_TYPE_FETCH] = cs_PLAYER "You" cs_CLOSE " must retrieve " cs_ATTR "%d" cs_CLOSE " %s.",
};
