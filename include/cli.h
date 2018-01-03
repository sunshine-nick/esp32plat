/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#ifndef __CLI_H__
#define __CLI_H__
typedef int (*CMD_PFUNC)(void);

typedef struct ct{
	char com[4];
	CMD_PFUNC f;     //pointer to function for this item
    char *des;
}ctable;

#define C_ENTRIES 40		//Command entries
#define NARG 6

extern ctable ctab[];
extern int argc;
extern char *argv[]; 
extern char *cline;		// Points to current position in line buffer 

void loadctab(char *command, CMD_PFUNC function,char *description);
void initctab();
void do_command(uint8 c);
int exec(char *s);
int do_rreg(void);
int do_after(void);
int do_fsmPrintInfo(void);
int do_onevent(void);
void make_time(void);
void cli_init(void);
void ts(void);
    
extern int year;
extern int month;
extern int day;
extern int hour;
extern int min;
extern int sec;

#endif /*__CLI_H__*/
/* [] END OF FILE */
