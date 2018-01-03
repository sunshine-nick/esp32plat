//////////////////////////////////////////////////////////////////////////
// (c) 2008 Microboxsys  Tangray                                          
//
// Konvoistraat 18                                                 
// 8923 CW LeeuwardenThe Netherlands                
// tel: (+31) (0)58 2132063                                      
// fax: (+31) (0)511 462698                                       
// E-mail: wsales@tangray.nl                                       
// URL: http://www.tangray.nl                                     
//
// Initial author            : Nick ray                               
// Initial creation date : 12 July   2008                      
// Version number       : 0.1                                     
/////////////////////////////////////////////////////////////////////////

#define TPS      1000    // kernel timer interrupt rate ('Ticks Per Second') 
#define TPS5	 5000   // 5 secs in TICS
#define TPS2	  500   // 0.5 seconds
#define TPSFACTOR 1		// 1 slider unit (0-100) in ms

#define MAX_TIM  16    // Max defined soft timers
#define MAX_ACT 16

#define CB_KB      1
#define CB_SSOCK   2
#define CB_CSOCK   4
#define CB_SOFT    8
#define CB_IO      16
#define CB_ETIMER  32
#define CB_ATIMER 64
#define CB_RES     128



/*****************************************************************************/
/*                      I/O RELATED      DEFINES                             */
/*****************************************************************************/

#define MAXBUF 81
#define LF 10
#define CR 13
#define BUFFERED 1
#define NON_BUFFERED 0
#define NULCP (char *)(0)
#define NULC  (char)(0)

/*****************************************************************************/
/*                      I/O RELATED        MACROS                            */
/*****************************************************************************/

#define printwb(y)		printNumber('b','w',y,1); 	//print word in binary
#define printd(y)   	printNumber('d','w',y,1); 	//print decimal

#define printbx(y)  	printNumber('x','b',y,1);	//print byte hex
#define printwx(y)  	printNumber('x','w',y,1);	//print word hex
#define printlx(y)  	printNumber('x','l',y,1);	//print long hex
#define printx0(y)  	printNumber('x','b',y,0); 	//print hex with leading zeros
#define printwx0(y) 	printNumber('x','w',y,0);	//print word in hex with leading zeros
#define printlx0(y) 	printNumber('x','l',y,0); 	//print long in hex with leading zeros

#define printbx0(y) 	printNumber('x','b',y,0);	//print byte in hex with leading zeros
#define printb(y)   	printNumber('b','b',y,1);	//print binary
#define printsb(y)  	printNumber('b','w',y,1);	//print short binary
#define printld(y)  	printNumber('d','l',y,1);	//print long decimal
#define printc(y)		  printChar(y);				//print character
#define printbuff(y)	printBuff(y);				//print buffer
#define printstr(y)		printString(y);				//print string
#define prints(y)		  printString(y);				//print string
#define NL	newline();

/*****************************************************************************/
/*             FUNCTION DECLARATION IO.C INTERFACE FUNCTIONS                 */
/*****************************************************************************/
void io_init(void);
void printBuff(void);
void newline(void);
void printChar(char c);
void printString(char *s);
void printBackspace(void);
void printTab(long n);
void printNumber(char b,char s,int n,int nolz);
char *numToString(int n);	
void xstrcpy(char *t, char *s);
void xstrncpy(char *t, char *s, int maxlen);
void mccpy(char *t, char *s, int maxlen);
long xstrncmp(char *s, char *t, long  maxlen);
char *xstrchr(char *s, char c);
char *xstrnchr(char *s,char c,int n);
void printIntJustified(int n);
void printIntLeftJustified(int n);
int getLine(void);
char *ntos(int n);
//---------------------------------------------------------------------------------------
void  rip(char *s);
void  printbuff(void);
void  resetbuf(char *p);
void  restorebuf(void);
void  prints(char *s);
void  backspace(void);
void  outchar(char c);
void  tab(long n);
void  dots(long n);
void  space(long n);
void printn(char b,char s,int n,int nolz);
int xatoi(char *s);
int xatol(char *s);
int xhtol(char *s);
long xxatol(char *s);
long xstrlen(char *s);
long xstrcmp(char *s, char *t);
long xstrncmp(char *s, char *t, long  maxlen);
char  *xstrupr(char *str);
void  xstrcpy(char *t, char *s);
void  xstrcat(char *s1, char *s2);
void  xstrncpy(char *t, char *s, int maxlen);
void  xmemset(char *s, char c, long n);
char *xstrchr(char *s, char c);
char *xstrnchr(char *s,char c,int n);
char *xselstr(char *s, char *b, char *e);
void init_io(void);
int  getline(void);
int is_alnum(char c);
int is_alpha(char c);
int is_digit(char c);
char to_upper(char c);
char *xstrip(char *s);
extern void OpeningScreen(void);
char *ntos(int n);
void page_mode(char *pagebuf,int m);
void mail_mode(char *pagebuf,int m);
char * selstr(char *s,char *b,char *e, char *t);
void xurlcpy(char *t, char *s);
void pr_s_just(char *n,int w);
void pr_n_just(int n);
void pr_n_just2(int n);
void pr_n_ljust(int n);
////////////////////////////////////////////////////////////////
// external routines for char IO
////////////////////////////////////////////////////////////////
int read_char(void);
char  *get_s(void);
void   ser_init(void);

   


/////////////////////////////////////////////////////////////////
// version declarations. will be set in the C module files
////////////////////////////////////////////////////////////////
//extern int p_ver;
//extern int s_ver;
//extern int ma_ver;
//extern char linebuf[]; 
 
