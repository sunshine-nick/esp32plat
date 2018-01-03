#define MAXTRAN 16

#define CYCLIC  0
#define SINGLE  1
#define LAZY    2

#define CHANGESTATE(x)   EV_SET(ttab[x].ev)
#define STARTTRANS(x)    EV_SET(ttab[x].ev)

typedef struct tran_st {
    char name[8];
    char fname[8];
    char tname[8];
    FUNC6 *from;
    FUNC6 *to;
    FUNC6 *trans;
    FUNC6 *oexit;
    FUNC6 *tentry;
    uint8 ev;
    uint8 sidx;
    uint8 stage;
    uint8 mode;
    uint16 delay;
} transition;

void do_transition(uint8 t);
void fsm_init(void);
void fsm_app_init(void);
void set_tmode(uint8 idx, uint8 mode, uint16 delay);
