// main.cc

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pitcher.h"
#include "player.h"
#include "team.h"
#include "frame.h"
#define DEBUG 1
#define DEBUG2 0

// variables used outside of main()

char VER[8] = "2.7.2";

FILE 	*pbpfp,		// play-by-play output file
	*stsfp,		// stats output file
	*cmdfp,		// commands output file
	*undofp;	// undo output file

FILE *output=stdout;	// where to direct output (default = stdout)
FILE *input=stdin;	// where to read in data
char *ops= "w+";

frame *f0;		// f0 is the current frame to be decoded

char filename[80];	// prefix for all output files

char *buffer;		// output buffer

team *ibl[2];		// pointers to the two teams
queue *runners;		// queue to handle inherited runners
player *onbase[4];	// array of pointers to batter & runners


int errflag =0;         // error flag; true if an error was committed 
                        // in the half inning

int undo     = 0, 	// if in the process of undo, set = 1
    cont     = 1,	// continue reading commands?
    outs     = 0, 	
    atbat    = 0, 
    inning   = 1,
    runs     = 0,
    linesize = 9;	// size of linescore array;

int **linescore;	// linescore array;

int** build_linescore(int);

int openfile(int, char*[]);
void setup();
void setup(int);

void play();
void quit(void);

void outbuf(FILE*,char*,char*);

main(int argc, char *argv[])
{

int flag = 0;		// are there flags? if so set to 1
int input_home = 0,
    input_away = 0;	// set if home/away team's linups are to be
			// read in from a file

int input_file = 0;	// is there an input file? if so set to 1
			// use these flags to determine the position
			// of elements in the argv array, e.g.
			// output file = flag + input_file + ... + 1

if (argc < 2) {fprintf(stderr,
	"Usage: grs [-ahfv] [afile] [hfile] [infile] outfile\n"); exit(0);}

char *flags = argv[1];

if (*flags++ == '-') {
   flag = 1;
   while (*flags)
     switch(*flags++) {
	case 'a':	input_away = 1;
			break;
	case 'h':	input_home = 1;
			break;
	case 'v':	fprintf(stderr,"%s\n",VER);
			exit(0);
	case 'f':	input_file = 1;
			break;
	default	:	fprintf(stderr,
	"Usage: grs [-ahfv] [afile] [hfile] [infile] outfile\n");
			exit(0);
	}
}

if (argc != flag + input_away + input_home + input_file + 2) {
     fprintf(stderr, "Usage: grs [-ahfv] [afile] [hfile] [infile] outfile\n"); 
     exit(0);
     }

if ((input_file) && (input_away || input_home)) {
   fprintf(stderr,"You cannot specify both the -f and -a or -h flags.\n");
   exit(0);
   }


strcpy(filename,argv[flag+input_away+input_home+input_file+1]);

if (openfile(argc,filename)) {
   fprintf(stderr,"Input file %s not found\n",argv[flag+input_away+input_home+input_file+1]);
   exit(0);
   }

if (!(input_file)) {		// no input file case
   if (!(input_away) || 
	(input = fopen(argv[flag+input_away],"r")) == NULL) {
	input = stdin;
	output = stdout;}	// no away team files
   else 
	output = fopen("/dev/null","w");

   setup(0);			// gets lineups for visitors
   if (input != stdin) fclose(input);
	
   if (!(input_home) || 
      (input = fopen(argv[flag+input_away+input_home],"r")) == NULL) {
	   input = stdin;
	   output = stdout;} 	// no home team files
      else
	   output = fopen("/dev/null","w");
   
   setup(1);			// gets lineups for home team
   if (input != stdin) fclose(input);
   
   input = stdin;
   output = stdout;
   setup();			// sets up pbpfp, etc.
   }		// end of no input file "if"
   
else {		// input file exists
   if ((input = fopen(argv[flag+input_away+input_home+input_file],
		   "r")) == NULL) {
	   fprintf(stderr,"Could not open file %s.\n",
		   argv[flag+input_away+input_home+input_file]);
	   exit(0);
           } 
   else
	   output = fopen("/dev/null","w");   // input file ok
   setup(0);			// gets lineups for visitors
   setup(1);			// gets lineups for home team
   setup();			// sets up pbpfp, etc.
   play();
   fclose(input);}
   
input = stdin;
output = stdout;
play();
quit();

fclose(pbpfp);
fclose(stsfp);
fclose(cmdfp);
}

void play()
{

cont = 1;		// obviously we want to execute at least once!

char tempstr[80];
char cmdfile[80];
strcpy(cmdfile,filename);
strcat(cmdfile,".cmd");

while (cont) {
if (DEBUG2) ibl[0]->box_score(stderr); 
if (DEBUG2) ibl[1]->box_score(stderr); 
	fclose(cmdfp);
	fgets(tempstr,100,input);
	cmdfp=fopen(cmdfile,"a");
	
	if (inning > linesize) {
	   int **temp = build_linescore(inning);
	   for (int x = 0; x <= 1; x++)
		for (int y = 0; y < linesize; y++)
			temp[x][y] = linescore[x][y];
	   temp[0][linesize] = 0;
	   temp[1][linesize] = 0;
	   delete(linescore);
	   linesize = inning;
	   linescore = temp;}

//	if (output != stdout) fprintf(stderr,"%s",tempstr);
	f0=new frame(tempstr);

	if (!(feof(input)) && (strlen(tempstr) > 1) && (tempstr[0] != 'u')
		&& (f0->decode()))
	   fprintf(cmdfp,"%s",tempstr);

	if (!(f0->update())) {
		tempstr[strlen(tempstr)-1] = '\0';
		f0->help(tempstr);
		}

	delete(f0);

//	runners->dump();
	}
}

void quit()
{
int x, y;
int val;

y = 16 + (3 * inning);


fprintf(stsfp,"    ");
for (x = 1; x <= inning; x++)
    fprintf(stsfp,"%3d",x);
fprintf(stsfp,"     R  H  E\n");

for (x = 1; x <= y; x++)
    fprintf(stsfp,"-");
fprintf(stsfp,"\n");

fprintf(stsfp,"%s ",ibl[0]);
for (x = 0; x < inning; x++)
    fprintf(stsfp,"%3d",linescore[0][x]);
fprintf(stsfp,"   %3d%3d%3d",ibl[0]->score,ibl[0]->team_hits(),
			     ibl[0]->errors);
fprintf(stsfp,"\n");

fprintf(stsfp,"%s ",ibl[1]);
for (x = 0; x < inning; x++)
    fprintf(stsfp,"%3d",linescore[1][x]);
fprintf(stsfp,"   %3d%3d%3d",ibl[1]->score,ibl[1]->team_hits(),
			     ibl[1]->errors);
fprintf(stsfp,"\n");

for (x = 1; x <= y; x++)
    fprintf(stsfp,"-");
fprintf(stsfp,"\n");
for (val=0; val<2; val++) {
	ibl[val]->box_score(stsfp);
	fprintf(stsfp,"LOB: %d\n",ibl[val]->lob);
	fprintf(stsfp,"E: ");
	ibl[val]->printstat(stsfp,0);
	fprintf(stsfp,"PB: ");
	ibl[val]->printstat(stsfp,1);
	fprintf(stsfp,"GIDP: ");
	ibl[val]->printstat(stsfp,2);
	fprintf(stsfp,"SH: ");
	ibl[val]->printstat(stsfp,3);
	fprintf(stsfp,"SF: ");
	ibl[val]->printstat(stsfp,4);
	fprintf(stsfp,"HBP: ");
	ibl[val]->printstat(stsfp,5);
	fprintf(stsfp,"WP: ");
	ibl[val]->printstat(stsfp,6);
	fprintf(stsfp,"IBB: ");
	ibl[val]->printstat(stsfp,7);
        fprintf(stsfp,"BALK: ");
        ibl[val]->printstat(stsfp,8);
	}
fprintf(stsfp,"\n");
fprintf(pbpfp,"\n");
}

int** build_linescore(int inn)
{
 
int **temp;
 
temp = (int **) malloc (2 * sizeof(int));
temp[0] = (int *) malloc (inn * sizeof(int));
temp[1] = (int *) malloc (inn * sizeof(int));
 
return (temp);
}

void setup()
{

char tempstr[80];
	
linescore = build_linescore(9);
for (int x = 0; x <= 1; x++)
   for (int y = 0; y <= 8; y++)
        linescore[x][y] = 0;
linesize = 9;

ibl[0]->make_lineups_pit();
ibl[1]->make_lineups_pit();
	
runners = new queue;

fprintf(output,"\nEnter one line description of game conditions.\n");
fgets(tempstr,100,input);
fprintf(cmdfp,"%s",tempstr);
fprintf(pbpfp,"grs version %s\n",VER);
fprintf(pbpfp,"%s at %s \n",ibl[0]->nout(),ibl[1]->nout());
fprintf(pbpfp,"%s\n",tempstr);
fprintf(pbpfp,"Starting pitchers - %s for %s, and %s for %s\n",
    ibl[0]->mound->nout(),ibl[0]->nout(),ibl[1]->mound->nout(),ibl[1]->nout());

f0=new frame(ibl[0],ibl[1],pbpfp);
buffer=(char*)malloc(160);
*buffer='\0';
sprintf(tempstr,"\n%s %d: ",ibl[atbat]->nout(),inning);
outbuf(pbpfp,tempstr);
}
 
void setup(int tm)
{

char tempstr[80];
	
int i;
int flag = 1;
	
char nm[2][6];
strcpy(nm[0],"away\0");
strcpy(nm[1],"home\0");
 
tempstr[0]='\0';

   while (flag) {
	fprintf(output,"Enter a 3 letter name for %s team:\n",nm[tm]);
	fgets(tempstr,100,input);
	if (strlen(tempstr) == 4) {
	    flag=0;
	    i = 0;
	    while (!(flag) && (i < 3)) {
		 if (!(tempstr[i] >= 'A' && tempstr[i] <= 'Z')) {
		    fprintf(stderr,"Use all caps only\n");
		    if (input == stdin) flag=1; else exit(0);}
		i++;
	    }
	}
	else {
		fprintf(stderr,"Team name must be 3 letters!\n");
		if (input == stdin) flag=1; else exit(0);}
    }
fprintf(cmdfp,"%s",tempstr);

//if (ibl[tm]) delete(ibl[tm]);
ibl[tm] =new team(tempstr);
ibl[tm]->make_lineups();

}

int openfile(int argc, char *argv)
{
int result;
char pbpfile[80];

	result=argc;
        result=0;
        // if (DEBUG) cmdfp=pbpfp=stsfp=stdout;
        // else {
        strcpy(pbpfile,argv);
        strcat(pbpfile,".pbp");
        if ((pbpfp=fopen(pbpfile,ops)) == NULL)
                {
                fprintf(stderr,"can't open play-by-play file\n");
                result=1;
                }
        strcpy(pbpfile,argv);
        strcat(pbpfile,".sts");
        if ((stsfp=fopen(pbpfile,ops)) == NULL)
                {
                fprintf(stderr,"can't open stats fIle\n");
                result=1;
                }
        strcpy(pbpfile,argv);
        strcat(pbpfile,".cmd");
        if ((cmdfp=fopen(pbpfile,ops)) == NULL)
                {
                fprintf(stderr,"can't open commands file\n");
                result=1;
                }
        // } /* end of debug else claue */
return(result);
}

void outbuf(FILE *fp,char *str,char *punc)
{

char tempstr[160];
char *ptr = tempstr;
int count=0;

if (*punc == '\n') {
        strcat(buffer, punc);
        fputs(buffer,fp);
	fputs(str,fp);
        *buffer=(char)0;
	}
else if (strlen(str)+strlen(buffer) > 75) {
	strcpy(tempstr, buffer);
	strcat(tempstr, punc);
	strcat(tempstr, str);

	strncpy(buffer, tempstr, 65);

	while (count != 65) { count++; ptr++; }
	while (!(isspace(*ptr)))
	   buffer[count++] = *(ptr++);

	buffer[count]='\0';
	ptr++;

	buffer=strcat(buffer,"\n");
	fputs(buffer,fp);

	strcpy(buffer, ptr);
	}
else {
	strcat(buffer,punc);
	strcat(buffer,str);
	}

}

