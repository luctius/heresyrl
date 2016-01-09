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
        .type = _type,
#define QUEST_END }

#define DESCRIPTION(des)    .description=des
#define DESCR_ENTRANCE(des) .start=des
#define DESCR_EXIT(des)     .end=des
#define REWARDS(xp, gold) \
    .xp_reward=xp, \
    .gp_reward=gold

#define DUNGEON(nr, tp, wgt) \
    .dungeon[nr] = { \
        .type = tp, \
        .weight = wgt, \
    }

#define ENEMIES(nr, tp, wgt) \
    .enemies[nr] = { \
        .race = tp, \
        .weight = wgt, \
    }

static struct quest static_quest_list[] = {

    QUEST(QSTID_WISE_WOMAN, QST_TYPE_FETCH)
        DESCRIPTION("fetch mushrooms"),
        DESCR_ENTRANCE(NULL),
        DESCR_EXIT("gratefull"),
        REWARDS(300, 100),
        DUNGEON(0, DUNGEON_TYPE_CAVE, 1),
        ENEMIES(0, MSR_RACE_GREENSKIN, 8),
        ENEMIES(1, MSR_RACE_BEAST, 2),
    QUEST_END,
};

