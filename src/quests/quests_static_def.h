
static struct quest static_quest_list[] = {
    [QSTID_WISE_WOMAN] = {
        .tid  = QSTID_WISE_WOMAN,
        .type = QST_TYPE_FETCH,

        .description = "fetch mushrooms",
        .start       = NULL,
        .end         = "she is gratefull",
        .xp_reward   = 300,
        .gp_reward   = 10,
    },
};

