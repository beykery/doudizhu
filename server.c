#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <malloc.h>

#define CARD_COUNT 54

#define S_JOKER 18
#define B_JOKER 19
#define CARD_J 11
#define CARD_Q 12
#define CARD_K 13
#define CARD_A 14
#define CARD_2 15

//单排
#define CARD_TYPE_SINGLE 1
//对
#define CARD_TYPE_PAIR 2
//三
#define CARD_TYPE_THREE 3
//顺
#define CARD_TYPE_STRAIGHT 4
//飞机
#define CARD_TYPE_FLY 5
//炸弹
#define CARD_TYPE_BOOM 6

#define NEXT_TURN(i) (++i) % 3
#define CARD_VAL(i) i % 100

typedef struct t_player {
    //玩家的手牌
    int cards[20];
    //玩家姓名
    char* name;
    //地主的标记
    int banker;
}Player;

typedef struct t_card_combine {
    //牌组的类型 只有同类型牌组才可以比大小
    int type;
    //牌组的值 用于比较大小
    int val;
    //牌组所属的玩家
    Player* player;
}CardCombine;

typedef struct t_global_table {
    //牌局中的3名玩家
    Player *players[3];
    //牌局中当前出牌的玩家位置
    int playerTurn;
    //当前人数
    int playerNum;
    //地主
    Player *banker;
    //牌局中的倍数
    int base;
    //牌局中的底牌
    int baseCard[3];
    //牌局结束标志
    int end;
}GlobalTable;

void swap(int *array, int i, int j) {
    int temp = array[i];
    array[i] = array[j];
    array[j] = temp;
}

int shuffle(int *array, int n) {
    srand((unsigned)time(NULL));
    int i, r;
    for (i=0; i<n ;i++) {
        r = rand() % n;
        swap(array, i, r);
    }
}

int bubbleSort(int *array, int n) {
    int i,j;
    for (i=0; i<n; i++) {
        for (j=i; j<n; j++) {
            if (array[i] % 100 < array[j] % 100) {
                swap(array, i, j);
            }
        }
    }
}

void println(int *array, int n) {
    int i=0;
    printf("[");
    for (i=0; i<n; i++) {
        printf("%d,", array[i]);
    }
    printf("]\n");
}

int card[CARD_COUNT] = {
    115,103,104,105,106,107,108,109,110,111,112,113,114,
    215,203,204,205,206,207,208,209,210,211,212,213,214,
    315,303,304,305,306,307,308,309,310,311,312,313,314,
    415,403,404,405,406,407,408,409,410,411,412,413,414,
    518,519
};

int getCardColor(int card, char *ret) {
    switch (card / 100) {
        case 1 : 
            strcpy(ret, "♠");
            break;
        case 2 : 
            strcpy(ret, "♥");
            break;
        case 3 : 
            strcpy(ret, "♣");
            break;
        case 4 : 
            strcpy(ret, "◆");
            break;
        default :
            return 0;
    }
    return 1;
}

int getCardNum(int card, char *ret) {
    int num = card % 100;
    if (num >=3 && num <= 10) {
        sprintf(ret, "%d", num);
    } else if (num == CARD_J) {
        strcpy(ret, "J");
    } else if (num == CARD_Q) {
        strcpy(ret, "Q");
    } else if (num == CARD_K) {
        strcpy(ret, "K");
    } else if (num == CARD_A) {
        strcpy(ret, "A");
    } else if (num == CARD_2) {
        strcpy(ret, "2");
    } else if (num == S_JOKER) {
        strcpy(ret, " W");
    } else if (num == B_JOKER) {
        strcpy(ret, " w");
    } else {
        return 0;
    }
    return 1;
}

int getCardChinese(int card, char* ret) {
    char color[10];
    char chinese[10];
    int ret1 = getCardColor(card, color);
    int ret2 = getCardNum(card, chinese);
    if (ret1 && ret2) {
        sprintf(ret, "%s%s", color, chinese);
        return 1;
    } else if (ret2) {
        sprintf(ret, "%s", chinese);
        return 1;
    }
    return 0;
}

int emitCard(int* table1, int* table2, int* table3, int* base) {
    //洗牌
    shuffle(card, CARD_COUNT);
    //发牌
    int i;
    int emitNum = (CARD_COUNT-3) / 3;
    memcpy(table1, card, emitNum * sizeof(int));
    bubbleSort(table1, 17);
    memcpy(table2, card+17, emitNum * sizeof(int));    
    bubbleSort(table2, 17);
    memcpy(table3, card + emitNum + emitNum, emitNum * sizeof(int));
    bubbleSort(table3, 17);
    memcpy(base, card + emitNum + emitNum + emitNum, 3 * sizeof(int));

    return 1;
}

int same_count(int *array, int n, int *same_element) {
    bubbleSort(array, n);
    int i;
    int same = 1;
    int max_same = 1;
    for (i=0; i<n; i++) {
        if ((array[i]%100 == array[i+1]%100) || 
            (array[i]/100 == 5 && array[i+1]/100 == 5)) {
            same++;
            *same_element = array[i] % 100;
        } else {
            same = 1;
        }
        if (max_same < same) {
            max_same = same;
        }
    }
    return max_same;
}

int straight_count(int *array, int n, int *straight_sum) {
    if (n<5) {
        return 0;
    }
    bubbleSort(array, n);
    println(array, n);
    int i = 0;
    *straight_sum = CARD_VAL(array[0]);
    for (i=1; i<n; i++) {
        if (CARD_VAL(array[i]) == CARD_VAL(array[i-1]) - 1) {
            *straight_sum += CARD_VAL(array[i]);
        } else {
            return 0;
        }
    }
    return 1;
}

/**
 * 单牌 双牌 三张 顺子 炸弹
 * @param  cards [description]
 * @param  n     [description]
 * @return       [description]
 */
CardCombine *getCardCombine(int *cards, int n) {
    CardCombine *cc = malloc(sizeof(CardCombine));
    cc->type = -1;
    int straight_sum = 0;
    //判断顺子
    if (straight_count(cards, n, &straight_sum) == 1) {
        cc->type = CARD_TYPE_STRAIGHT;
        cc->val = straight_sum;
        return cc;
    }
    int same_element = 0;
    int same = same_count(cards, n, &same_element);
    if (same == 1 && n == 1) {
        //单牌
        cc->type = CARD_TYPE_SINGLE;
        cc->val = same_element;
    } else if (same == 2) {
        if (cards[0]%100 == 18) {
            //王炸
            cc->type = CARD_TYPE_BOOM;
            cc->val = same_element * 10;
        } else {
            //对儿
            cc->type = CARD_TYPE_PAIR;
            cc->val = same_element;
        }
    } else if (same == 3) {
        //三带N
        cc->type = CARD_TYPE_THREE;
        cc->val = same_element;
    } else if (same == 4) {
        //炸弹
        cc->type = CARD_TYPE_BOOM;
        cc->val = same_element * 10;
    }
    return cc;
}

Player *create_player(char* name) {
    Player *p = malloc(sizeof(Player));
    p->name = name;
    p->banker = 0;
    return p;
}

GlobalTable *create_table() {
    GlobalTable *t = malloc(sizeof(GlobalTable));
    t->base = 1;
    t->playerNum = 0;
    t->end = 0;
    t->banker = NULL;
    srand((unsigned)time(NULL));
    t->playerTurn = rand() % 3;
    return t;
}

void table_join(GlobalTable *t, Player *p) {
    t->playerNum++;
    t->players[t->playerNum - 1] = p;
}

void dumpPlayer(Player *p) {
    printf("user:%s\n", p->name);
    printf("hold cards: ");
    char temp[20];
    int i;
    for (i=0; i<20; i++) {
        if (getCardChinese(p->cards[i], temp)) {
            printf("%s,", temp);
        }
    }
    printf("\n");
}


void call_banker(Player *p, int* base) {
    p->cards[17] = base[0];
    p->cards[18] = base[1];
    p->cards[19] = base[2];
    bubbleSort(p->cards, 20);
    p->banker = 1;
}

int main() {
    int i;
    char temp[25];
    int base[3];
    // //创建一个牌桌
    // GlobalTable *table = create_table();
    // //创建三个玩家
    // Player *player1 = create_player("user1");
    // Player *player2 = create_player("user2");
    // Player *player3 = create_player("user3");
    // //三个玩家加入牌桌
    // table_join(table, player1);
    // table_join(table, player2);
    // table_join(table, player3);
    // //给三个人发牌
    // emitCard(player1->cards, player2->cards, player3->cards, base);
    // //有一个人叫地主,拿走三张牌
    // call_banker(player1, base);

    // dumpPlayer(player1);
    // dumpPlayer(player2);
    // dumpPlayer(player3);
    // int cards[5] = {303,304,405,106,107};
    // CardCombine *cc = getCardCombine(cards, 5);
    // printf("card type %d value %d\n", cc->type, cc->val);

    char command[20];
    char arg[20];
    int user_online = 0;
    Player *wait[10];
    GlobalTable *table;
    while (1) {
        scanf("%s %s", command, arg);
        if (strcmp(command, "join")==0) {
            Player *p = create_player(arg);
            printf("welcome %s join\n", arg);
            wait[user_online] = p; 
            user_online ++;
            if (user_online == 3) {
                printf("人数达到游戏数量\n");
                table = create_table();
                table_join(table, wait[user_online]);
                user_online--;
                table_join(table, wait[user_online]);
                user_online--;
                table_join(table, wait[user_online]);
                user_online--;
                printf("发牌完成\n");
                emitCard(table->players[0]->cards, table->players[1]->cards, table->players[2]->cards, base);
                while (table->end == 0) {
                    if (table->banker == null) {
                        printf("是否叫地主?");
                        scanf("%s", command);
                        if (strcmp(command, "yes")==0) {
                            table->banker = table->players[table->playerTurn];
                            table->base *= 2; 
                        }
                    }
                    printf("welcome %s join\n", arg);
                }
            }
        }
    }
    
    return 0;
}
