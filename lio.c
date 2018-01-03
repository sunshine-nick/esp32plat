//////////////////////////////////////////////
//  RTK101 Hard real time scheduler for wireless networks
//  lio .c
//  low level IO, formatted printing and string routines
//  uses getline in lio.c and runs as an interrupt/event drivern service
//  Copyright Nick Ray, Sunshine labs 2016,2017
//  All Rights Reserved
//////////////////////////////////////////////

#include "./include/platform4esp.h"

// This string contains all the printable/keyable characters used in the kernel       
const char ChkStr[] ={
    "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ !@-+*/%&|^:=()[]$\",.;<>"
};

////////////////////////////////////////////////////////////////////////////////
//                      GLOBAL VARIABLES FOR IO.C                            
////////////////////////////////////////////////////////////////////////////////
char obuf[100];     // Output buffer                                         
char linebuf[100];  // Line buffer used for command line input               
char *cline;		    // Points to current position in line buffer             
char *bufpos;       // Points to the current output buffer position          
char *bufbegin;     // Points to the start of the output buffer              
char *obpos;        // Points to the current output buffer position          
char *obb;          // Output buffer begin                             

void io_init(void){
	obb = obuf;         // Command buffering                                     
	obpos = obuf;       // Command buffer position                               
	cline = linebuf;    // Command line is the same as the line buffer               
}

void printBuff(void){
	long r;
	long size;
	if (obpos == obb)    return;	
	size = (long)(obpos - obb);
	obpos=obb;
	for(r=0;r<size;r++){
		putchar(*obpos++);
	}
	obpos=obb;
}

/*
 * Insert newline into the output buffer
 */
void newline(void){

	if((long)((obpos) -obb)>=(long)MAXBUF){
		printBuff();
	}
	*obpos++ = CR;
	*obpos++ = LF;
	*obpos = 0;
	printbuff();
}


/*
 * Put character in output buffer
 */
void printChar(char c){
  	if ((long)  ((obpos) -obb) >=  (long)MAXBUF){
		printbuff();
	}
	if (c == 10){
	   *obpos++ = LF;
	   printbuff();
	}
	else{
	    *obpos++ = c;
	    *obpos = 0;
	}
}

/*
 * Put string in the output buffer
 */
void printString(char *s){
  	long i=0;

  	while(s[i]){
    	if((long)(obpos - obb) >=  (long)MAXBUF){
    	  	printBuff();
		}
    	if(s[i] == 10){
    	  	*obpos++ = LF;
//  	    *obpos++ = CR;
    	  	i++;
		  	printBuff();
    	}
    	else{
    	  	*obpos++ = s[i++];
		}
  	}
  	*obpos = 0;
}

void printBackspace(void){
	putchar(8);
	putchar(' ');
	putchar(8);
}



void printTab(long n){
	long i=0;

	i= (long)(obpos-obb);
	while (i<=n){
		*obpos++ = ' '; i++; 
	}
	*obpos = 0;
}

////////////////////////////////////////////////////////
// Multifunctional format printing routine
////////////////////////////////////////////////////////
void printNumber(char b,char s,int n,int nolz){
  	int i = 0, tel = 0, x = 0;
  	unsigned char c, ont = 0;
  	long l = 0;
  	char digit[12], temp = 0;
  	const char digits[] = {"0123456789ABCDEF"};

  	if (b == 'b'){
    	if (s == 'w'){
    	  tel = 15;
		}
    	else{
    	  	if(s == 'b'){
				tel = 7;
			}
    	  	else{
    	    	tel = 3;
			}
		}
    	for (i=tel;i>=0;i--){
//			if (tel == 3)     x = 4+i;
			if (i<=7) {
        		x = i;
				c = (unsigned char) n;
			}
			else{
				x = i-8;
				c= (unsigned char)( n>>8);
			}
      		temp=(char)('0'+((c>>x)&1));
     		printc(temp);
    	}
  	}
  	if (b == 'x'){
    	if (s == 'l') tel = 7;
    	if (s == 'w') tel = 3;
      	if (s == 'b') tel = 1;

    	for (i=tel;i>=0;i--){
    	  	l = n;
    	  	c=(unsigned char)((l>>4*i)&0x0f);
    	  	if (nolz == 1){
    	  		if ((c != 0) || (ont == 1) || (i == 0)){
   	     		  		ont = 1;
   	     				printc(digits[c]);
   	     			}
   	   		}
   	   		else{
   	     		printc(digits[c]);
   	   		}
   		}
  	}	
  	if (b == 'd'){
    	l = n;
    	for (i=0;i<=9;i++){
    	  n=l%10;
    	  l=l/10;
    	  digit[i] = (char)(n +'0');
    	}
    	for(i=9;i>=0;i--){
    	  	if ((digit[i] != '0') || (ont == 1) || (i == 0)){
    	    	ont = 1;
    	    	printc(digit[i]);
    	  	}
    	}
  	}
}

char buf[20];

char *numToString(int n){
	int i = 0;
	unsigned char ont = 0;
	long l = 0;
	char digit[12];
	//const char digits[] = {"0123456789ABCDEF"};
	char *b;

	b = buf;
	  
	l = n;
	for (i=0;i<=9;i++){
	  	n=l%10;
	  	l=l/10;
	  	digit[i] = (char)(n +'0');
	}
	for(i=9;i>=0;i--){
		if ((digit[i] != '0') || (ont == 1) || (i == 0)){
	    	ont = 1;
	    	*b++ = digit[i];
	    }
	}
	*b++ = '\0';
	return buf;
}

///////////////////////////////////////////////////////////////
// Conversion routine: convert ASCII string to integer value
///////////////////////////////////////////////////////////////
/*
 * Test for digit('0'-'9')
 */
int is_digit(char c)
{
if (c=='$') return 1;
if((c>='0')&&(c<='9'))  return 1;
return 0;
}  

int xatoi(char *s)
{
    register long result = 0;
    register long ch ;
    unsigned char neg = 0;

    do{
	    ch = *s++;
    } while( ((ch >= 9) && (ch <= 13)) || (ch == 32) );     /* white space */
    if( ch == '-' ){                 /* check for '-' or '+' */
        neg = 1;
	}
    else if( ch != '+' ){
        s--;
	}
    for ( ;; ){
        ch = *( s++ );
        if ( ch<'0' || ch>'9' ){
            break;  /* end of string reached */
		}
        result = 10 * result + (long)(ch - '0');
    }
    return( neg ? -result : result );
}

/*
 * Conversion routine: convert ASCII string to long value
 */
int xatol(char *s)
{
    register long result = 0;
    register unsigned char ch ;
    unsigned char neg = 0;

        do
        {
                ch = *s++;
        } while( ((ch >= 9) && (ch <= 13)) || (ch == 32) );     /* white space */

        if( ch == '-' )                 /* check for '-' or '+' */
                neg = 1;
        else if( ch != '+' )
                s--;

        for ( ;; )
        {
                ch = *( s++ );

                if ( ch<'0' || ch>'9' )
                        break;  /* end of string reached */

                result = 10 * result + ch - '0';
        }

        return( neg ? -result : result );
}

/*
 * Conversion routine: convert Hex string to 32 bit value
 */
int xhtol(char *s)
{
  long ret;
  char c;

  ret = 0;
  while((c = *s++) != '\0')
  {
    c &= 0x7f;
    if (c >= '0' && c <= '9')
      ret = ret*16 + (c- '0');
    else
      if (c >= 'a' && c <= 'f')
        ret = ret*16 + (10 + c - 'a');
      else
        if (c >= 'A' && c <= 'F')
          ret = ret*16 + (10 + c - 'A');
        else
          break;
  }
  return ret;
}

/*
 * Conversion routine: convert ASCII string to 32 bit value
 */
long xxatol(char *s)
{
  long rslt=0;

  if (s[0] == '$')
    return(xhtol(&s[1]));
  if (s[0] == '0' && s[1] == 'x')
    return(xhtol(&s[2]));
  if (is_digit(s[0]))
    return(xatol(&s[0]));
  if ((s[0]=='-') && is_digit(s[1]))
  {
    rslt=xatol(&s[1]);
    return -1*rslt;
  }

  return(-1);
}

/////////////////////////////////////////////////////////////////////
// Copy string from source to destination: replaces strncpy function.
//////////////////////////////////////////////////////////////////////
void xstrncpy(char *t, char *s, int maxlen){
   int teller = 0;

   while (teller <= (maxlen-1)){
        t[teller] = s[teller];
        teller++;
   }
}

void xstrcpy(char *t, char *s)
{
while(*s) *t++ = *s++;
}

void mccpy(char *t, char *s, int maxlen){
   int teller = 0;

   while (teller <= (maxlen-1)){
        t[teller] = s[teller];
        teller++;
   }
}

///////////////////////////////////////////////////////////////
// Compare maxlen characters in source and destination strings.
// Replaces strncmp() result < 0 if s<t, 0 if s==t, >0 if s>t.
/////////////////////////////////////////////////////////////////
long xstrncmp(char *s, char *t, long  maxlen){
  long i;

  	for (i=0; s[i] == t[i]; i++){
        /* Note: This is tricky and easily overlooked. If s is shorter
           than t the '\0' will be compared with the next character of t
           and they will not match and an error code will be returned
           So use maxlen-1 instead */
        if (i == maxlen-1){
			return 0;
		}
  	}
  return (long)(s[i] - t[i]);
}


/////////////////////////////////////////////////////////////////////
// Return pointer to specified character c: replaces strchr function.
/////////////////////////////////////////////////////////////////////

char *xstrchr(char *s, char c){
   long i;

   for ( i=0;s[i]!='\0';i++){
        if(s[i]==c){
			return s+i;
		}
   }
   return 0;
}
/////////////////////////////////////////////////////////////////////
// Return pointer to specified character C, within n characters
/////////////////////////////////////////////////////////////////////

char *xstrnchr(char *s,char c,int n){
	int x=0;
	while (x<n){
		if (*s == c){
			return s;
		}
		s++;
		x++;
	}
	return 0;
}

void printIntJustified(int n){
	if (n < 10)         printString("     ");
	else if (n < 100)   printString("    ");
	else if (n <1000)   printString("   ");
	else if (n <10000)  printString("  ");
	else if (n <100000) printString(" ");
	else printString(" ");
	printld(n);
}

void printIntLeftJustified(int n){
	printld(n);
	if (n < 10) 		printString("    ");
	else if (n < 100) 	printString("   ");
	else 				printString("  ");
}
///////////////////////////////////////////////////////////////////////
// Get a line from input device
///////////////////////////////////////////////////////////////////////

extern uint8 do_single(char c);
extern uint8 geflag;
extern uint8 gbreak;

int getLine(void){
char c;

c = getchar() ; 
//if (c !=255) {printd(c); NL; }
 
switch(c)
      {
    	case 0x0  :   
    	case 0xFF : return(-1);            /* Nothing entered */
    	case 27   : gbreak = 1;return(-2); /* Escape key      */
    	case 1    : return(-3);            /* <Ctrl A> key    */
    	case 8    :                        /* <Backspace>     */
    	    *cline-- = (char)0;
    	    *cline = (char)0;
    	    printBackspace();
    	    return(0);
    	case 10:
      case 13:                           /* <Enter>: line ready to be processed */
          if (geflag ==1)*cline++ = (char)c; 
    	    *cline = NULC;
				  cline = linebuf;
          return(1);
          
      default : // If valid character, print and put character in line buffer
          if(xstrchr((char *)ChkStr,(char)c)!=NULC)
          {
          if (do_single(c) == 0) //process as normall otherwise drop char
            {
 	          *cline++ = (char)c;
	          *cline = NULC;
 	          putchar(c);
            }

 	        return(0);
          }
          else
          {
          return (-1);
		      }
      }
}

char liobuf[20];

char *ntos(int n)
{
int i = 0;
unsigned char ont = 0;
long l = 0;
char digit[12];
//const char digits[] = {"0123456789ABCDEF"};
char *b;
//char t[50];
b = liobuf;
int num = n;    
if (num <0)   
{
    n = num * -1;
    *b++ ='-';  
}
l = n;
for (i=0;i<=9;i++)
  {
  n=l%10;
  l=l/10;
  digit[i] = (char)(n +'0');
  }
for(i=9;i>=0;i--)
  {
  if ((digit[i] != '0') || (ont == 1) || (i == 0))
    {
    ont = 1;
    *b++ = digit[i];
    }
  }
*b++ = '\0';

return liobuf;
}
/* [] END OF FILE */
