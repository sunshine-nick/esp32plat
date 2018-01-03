//////////////////////////////////////////////
//  RTK101 Hard real time scheduler for wireless networks
//  cli .c
//  Built in command line interprter for testing and configuration
//  Copyright Nick Ray, Sunshine labs 2016,2017
//  All Rights Reserved
//////////////////////////////////////////////

#include "./include/platform4esp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sys/time.h"
#include "esp_deep_sleep.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "string.h"

char sfile[1024];
int gccount = 1024;
char *gcftemp;

typedef struct s_tag{
char ssid[20];
char pword[20];
} ssid_t;

char defsfile[] = {"oclr\notat 4 4 Deep\nfont 1\notat 6 20 Sleep\nwait 5000\ndeep 10000\n"} ;

ctable ctab[C_ENTRIES];
uint8 gdbf=2;
uint32 lseconds =0;                                   
uint8 geflag =0;
uint8 gbreak = 0;

int argc;
char *argv[NARG];   
extern char *cline;
extern char linebuf[];
extern ssid_t ssidtab[];
extern nvs_handle my_handle;

void insert_line(char *s);

/////////////////////////////////////////////////////////////////////////////
// mkparam turns a command string into a set of tokens and initialises arc and argv[]
// which are global
/////////////////////////////////////////////////////////////////////////////
int mkparam(char *line){
	int qflag;
	char *cp;
	//clear buffer before use
	for(argc = 0;argc < NARG;argc++) argv[argc] = 0;
	for(argc = 0;argc < NARG;) 
      {
    	while(*line == ' ' || *line == '\t'){ line++; /* Skip leading white space */  }
    	if(*line == '\0') return 0; /* Empty command line */
    	if(*line == CR) break; /* End of command line */
    	qflag = 0;  /* Quote flag, if a quote is detected this flag is set */     
    	/* Check for quoted token: string detection */
    	if(*line == '"')  
          {
        	line++;   /* Suppress quote */
        	qflag = 1;
          }
    	argv[argc++] = line; /* Beginning of token */
    	/* Find terminating delimiter */
    	if(qflag) 
          {
        	/* Find quote, it must be present */
        	if((cp = xstrchr(line,'"')) == NULC){
            	//Error message: error in command line 
            	return 0 ;             
            }
        	*cp++ = '\0';
        	line = cp;
          }
    	else {
        	/* Find space or tab. If not present then we've already found the last token. */
        	if((cp = xstrchr(line,' ')) == NULC && (cp = xstrchr(line,'\t')) == NULC) { break; }		
        	*cp++ = '\0';
        	line = cp;
           }
      }
return 1;
}

int do_exit(void){
	prints("Bye Bye");
	newline();
	return 0;
}

int do_debug(void){
	uint8 v=0;
    
	if (argc==2) 
    {
		v = xatoi(argv[1]);
    gdbf = v;
    printf("Global debug flag is %d",gdbf);
	}
    else  printf("Global debug flag is %d",gdbf);

return 0;
}
///////////////////////////////////////////////////////////////////
// Add an entry to the command table
// Used by modules
///////////////////////////////////////////////////////////////////
void loadctab(char *com, CMD_PFUNC f,char *s){
	uint8 i;

	for (i=0;i<C_ENTRIES;i++){
    	if (ctab[i].f == 0) {
        	xstrncpy(ctab[i].com,com,4);
        	ctab[i].f = f;
            ctab[i].des =s;
        	break;
        }
    }  
}

int do_help(void){
	uint8 i;
	uint8 j;
  
	prints("\r\nCommands are : \r\n");
	for (i=0;i<C_ENTRIES;i++){
    	if (ctab[i].f != 0) {
        	printd(i);
        	prints("   ");
        	for (j=0;j<4;j++) printc(ctab[i].com[j]);
            prints("   ");
            prints(ctab[i].des);
        	prints("\r\n");
        }
    }  
	return 0;
}


int do_enable(void){
//	uint8 s =0;
//    uint8 o =0;

if (argc == 1) {
    prints("Enable [n] [v] \r\n");
    prints(" n = 0 = 5v  main\r\n");
    prints(" n = 1 = 3v3 sensor \r\n");
    prints(" n = 2 = 5v  lora \r\n");
    prints(" n = 3 = 5v  X relay \r\n");
    prints(" n = 4 = 5v  5v X sensor\r\n");
    prints(" n = 5 = 5v  12v dc/dc \r\n");    
    prints(" v = 1 to turn off, 0 to turn oon\r\n");
    return 0;
    }
 
if (argc >=3) {
//    s = xatol(argv[1]);
//    o = xatol(argv[2]);
    }
else return 0;

return 0;
}

int do_pinhi(void){
uint8 p;
if (argc >=2) p = xatol(argv[1]);
else p = 25;
printf("GPIO %d high",p);
gpio_set_direction(p, GPIO_MODE_OUTPUT);
gpio_set_level(p, 1);
newline();
return 0;
}

int do_pinlo(void){
uint8 p;
if (argc >=2) p = xatol(argv[1]);
else p = 25;
printf("GPIO %d low",p);
gpio_set_direction(p, GPIO_MODE_OUTPUT);
gpio_set_level(p, 0);
newline();
return 0;
}


int do_deep(void){
int ms;
uint64_t us;

if (argc >=2) { 
  ms = xatol(argv[1]);
  us = ms*1000;
  esp_deep_sleep(us);
  }
newline();
return 0;
}

int do_wait(void){
int ms;

if (argc >=2) { 
  ms = xatol(argv[1]);
  vTaskDelay(ms / portTICK_PERIOD_MS);
  }
newline();
return 0;
}

////////////////////////////////////////////////////////////////////////
// line editing command which sets up the command file buffer for editing
// and extracts the line to be edited
// gccount and gcftemp and used later by the insert line function
// splits the buffer at the isertion point by inserting a 0
// gcftemp points to the begining of the split buffer
///////////////////////////////////////////////////////////////////////

int do_ledit(void){
int l;
int i=0;
int n=0;
int b=0;
char *lbegin;

if (argc >=2)   l = xatol(argv[1]) ;
else l = 1;

//printf("\nargc = %d l=  %d %s  %s \n",argc, l, argv[0], argv[1]);
lbegin = &sfile[0];
b=0;

// find line l
while (i<gccount)     // dont overrun the EOBUF
    {
    lbegin = &sfile[i];  // remember begining
    b = i;
    while (sfile[i]!='\n') i++;
    i++;
    if (n==l) break;
    n++;
    }
n=0;    
i=b;
// find end of line l and count chars
while (sfile[i]!='\n') {i++; n++;}
i++;    
// load line l into commnd line i now points to next line entry;
strncpy(linebuf,lbegin,n);
//set the command line pointer
cline = &linebuf[n];
// make a split by making the start of this line 0
*lbegin = 0;
// remember the start of the split off buffer
gcftemp=&sfile[i];
// goto edit mode
if (argc == 3) { strcat(linebuf,"\nedit\n!"); insert_line(linebuf) ; }
else geflag=1; 
newline();
return 0;
}

int do_ladd(void){
uint16 l;

if (argc >=2) //insert a line 
  {
  
  }
  else       // add line at end
  {
  l = strlen(sfile);
  sfile[l]   = 'e';
  sfile[l+1] = 'd';
  sfile[l+2] = 'i';
  sfile[l+3] = 't';
  sfile[l+4] = '\n';
  sfile[l+5] = 0;
  }    

return 0;
}

uint8 fmode = 0;

int do_exec_buf(void)
{
char *bb;
char *be;
uint ci = 0;
uint8 n;

bb = sfile;

while (ci<128)
  {
  if (gbreak ==1) return 0;
  if (sfile[ci] == '\n')
    {
     be = &sfile[ci];
     n = (uint8)(be-bb); 
     memset (linebuf,0,100);
     strncpy(linebuf,bb,n);
     linebuf[n+1] = 0;
     bb = be+1;
     printf("Exec buf %d: %s \n",ci,linebuf);
     exec(linebuf);
    }
  ci++; 
  }
printf(">EOF\n");
return 0;
}

////////////////////////////////////////////////////////////////////
// allows any string to be executed as a single command
// limitation is all commands have to be exactly 4 char long or more
////////////////////////////////////////////////////////////////////

int exec(char *s){
	FUNC0 f;
	int rc=0;
	uint8 i;
  rc = mkparam(s);
	if (rc == 0) return rc; 
  //s now points to first param (the command) which is 0 terminated
	for (i=0;i<C_ENTRIES;i++){
		if (xstrncmp(argv[0],ctab[i].com, 4)==0)  { rc = ctab[i].f(); break;}   
	}
	return rc; //success  or 0;
}

 
////////////////////////////////////////////////////////////////////
// start up command file  end line editing
////////////////////////////////////////////////////////////////////
// the old cftab gets split by the edit command
// the insertion point is a null terminated end to the first part of the table
// and gcftemp points to the start of the split part which is null terminated
// 

void insert_line(char *s)
{
char newtab[1024];

//copy first part of cftab to the newtab
strcpy(newtab,sfile);
//add in the new line
strcat(newtab,s);
// copy the last part of the old table
strcat(newtab,gcftemp);
//copy new table babck to old table 
strcpy(sfile,newtab);
//update gccount
gccount= strlen(sfile);
//printf("\nLine inserted. New size : %d\n",gccount);
}

int do_printsfile(void)
{
int i=0;
int n=0;
int j = 0;
char s[40];

memset(s,0,40);

printf("\nCommand File\n=============\n");
while (i<gccount)     // dont overrun the EOBUF
    {
    //find next EO line
    j=i;
    while (i<gccount) {if (sfile[i]!='\n') i++; else break;}
    if (i==gccount) break;
    i++;
    //copy line to temp buffer and print it.
    memset(s,0,40);
    strncpy(s,&sfile[j],i-j);
    printf("%d: %s",n,s);
    // i points to begiining next line 
    // increment line number
    n++;
    }
return 0;
}

int do_loadsfile(void)
{
memset(sfile,0,1024);
strcpy(sfile,defsfile);
gccount= strlen(sfile);
printf("Loaded start file %d\n",gccount) ;
return 0;
}

esp_err_t err ;

//esp_err_t nvs_set_blob(nvs_handle handle, const char* key, const void* value, size_t length
int do_savesfile(void)
{
err = nvs_set_str(my_handle,"sfile",sfile);
if (err == ESP_OK) 
  { 
  nvs_commit(my_handle);
  printf("Saved start file \n") ;
  }
else printf("NVS Error \n");
return 0; 
}

int do_restoresfile(void)
{                    
size_t string_size;
err = nvs_get_str(my_handle, "sfile", NULL, &string_size);
if (err == ESP_OK) 
  {
  nvs_get_str(my_handle, "sfile", sfile, &string_size);
  printf("Restored start file \n") ;
  }
else printf("NVS Error \n");  
return 0; 
}

//////////////////////////////////////////////////////////////////////
// initialise the BASIC command set
// Other commands will be added by each module using loadctab()
//////////////////////////////////////////////////////////////////////
void cli_init(){
	loadctab("help",do_help,"This command");
	loadctab("exit",do_exit,"Does nothing");
  loadctab("debu",do_debug,"Set debug level");
  loadctab("enab",do_enable,"enable power");
  loadctab("splo",do_pinlo,"Set GPIO lo"); 
  loadctab("sphi",do_pinhi,"Set GPIO lhi"); 
  loadctab("deep",do_deep,"Deep sleep n?");  
  loadctab("frun",do_exec_buf,"Exec string"); 
  loadctab("wait",do_wait,"delay script"); 
  loadctab("fedi",do_ledit,"Edit start script"); 
  loadctab("flad",do_ladd,"Add line to start scipt"); 
  loadctab("fres",do_restoresfile,"Restore sfile from flash"); 
  loadctab("fsav",do_savesfile,"Save sfile to flash"); 
  loadctab("flod",do_loadsfile,"Load default sfile");  
  loadctab("fpri",do_printsfile,"Display  sfile");             
         
  // this is the command line handler for the console uart
	//on_event(V_CRDY,"CLI     ", do_command,ClearAfterUse);
}

////////////////////////////////////////////////////////////////////////////
// handle single char commands
////////////////////////////////////////////////////////////////////////////
uint8 do_single(char c)
{
switch  (c)
  {
//  case '>' : geflag = 1 ; return 1;
  case '<' : geflag = 0 ; return 1;
  
  }                                       
  
return 0;
} 
////////////////////////////////////////////////////////////////////
// called when a line is ready for processing
////////////////////////////////////////////////////////////////////
void do_command(uint8 c){
  if (geflag ==1) {
      insert_line(linebuf);
      geflag =0;
      if (geflag ==0) printf("\nSSMON<%d> ",c);
      return;
      }
	if (geflag ==0) {
      exec(linebuf);
      if (geflag ==0) printf("\nSSMON<%d> ",c);
      }
  if (geflag ==1) {
  	  printf("\nEDIT<%d> ",gccount);
      printf("%s",linebuf);
      }
}
                            

/* [] END OF FILE */
