// frame.cc

#include "frame.h"

// Constructor which initializes the static fields

frame::frame(team *away, team *home, FILE *fp)
{
    errflag=0;
    error[0] = '\0';

    bat=away;
    pit=home;

    atbat=0;
    inning=1;
    outs=0;
    runs=0;
    pbpfp=fp;
    event[0]='&';

    for ( int i=0; i<4; i++ ) 
	onbase[i]=NULL;
    onbase[0]=ibl[atbat]->up();

    frameput();
    cont=1;
}

// Constructor call in all other cases

frame::frame(char *str)
{
    comment = (char*) calloc(MAX_INPUT, sizeof(char));
    error[0] = '\0';
    char *temp;
    temp = str;
    int i = 0;

    while (*temp == ' ') temp++;
    while (*temp != ' ' && *temp != '\n' && i < CMDLEN )
	event[i++] = *temp++;
    event[i] = '\0';

    i = 0;
    while (*temp == ' ') temp++;
    while (*temp != ' ' && *temp != '\n' && *temp && i < LOCLEN )
	location[i++] = *temp++;
    location[i] = '\0';

    i = 0;
    while (*temp == ' ') temp++;
    while (*temp != ' ' && *temp != '\n' && *temp && i < BRUNLEN )
	baserunning[i++] = *temp++;
    baserunning[i] = '\0';

    cont=1;
    bat=ibl[atbat];
    pit=ibl[(atbat+1)%2];
}

int frame::runadv()
{
    int i,j;
    char *str;
    player *runner;
    player *newbase[4];
    for (i=0; i<4; i++) newbase[i]=onbase[i];

    str=baserunning;

    while (*str) {  
	if (*str=='b') {runner=bat->up(); i=0;}
	else runner=onbase[i=(int) (*str - '0')];
	if (runner) {		// If runner is onbase
	    j=(int) (*(++str)-'0');  
	    switch (*str) {
		case 'o' :			// If runner made an out
		       outs=outs+1;
		       if (newbase[i]==runner && i)
			    newbase[i]=NULL;
		       break;
		case 'h' :			// If runner scored a run
		       if (newbase[i]==runner && i)
			    newbase[i]=NULL;
		       break;
		case '1':			// If runner (batter) went to 1st
		case '2' :			// If runner went to second
		case '3' :			// If runner went to third
		       newbase[j]=runner;
		       if (newbase[i]==runner && i)
			    newbase[i]=NULL;
		       break;
		default :
		    fprintf(stderr,"There was a problem with the input. Please check\n");
		}
	    }
	else  {
	    return 0; 
	    }
	str++;
	}
    for (i=0; i<4; i++) onbase[i]=newbase[i];
    return 1;
}

// runstats(1) takes care of a fielders choice, (it keeps the ownership
// of the baserunner for the previous pitcher for inherited runs to work
// runstats(2) is for any play where it is not obvious what happened
// to the batter, i.e. it outputs in .pbp the result for the batter


    void 
frame::runstats(int fc)
{
    int i,j;
    char *(b[4]);
    char tempstr[MAX_INPUT];
    char *str;

    pitcher *run_charged_to;
    int somebody_out=0;

    b[1]="to first";
    b[2]="to second";
    b[3]="to third";

    str=baserunning;
    while (*str) {  
	if (*str=='b') {
	    i=0;
	    if ((*(str+1) != 'o') && (fc != 1))
		    runners->add(pit->mound); }
	    
	else i=(int) (*str - '0');
	if (onbase[i]) {		// If runner is onbase
	    str++;  
	    j=(int)*str-'0';
	    switch (*str) {
		case 'o' :			// If runner made an out
		       pit->mound->out++;
		       somebody_out++;
		       if ((i == 0) && (fc > 0)) {
			    sprintf(tempstr,"%s out",onbase[i]->nout());
			    outbuf(pbpfp,tempstr,", ");}
		       else if (i > 0) {
			    sprintf(tempstr,"%s out",onbase[i]->nout());
			    outbuf(pbpfp,tempstr,", ");}
		       break;
		case 'h' :			// If runner scored a run
		       run_charged_to = runners->dequeue();
		       run_charged_to->r++;
		       run_charged_to->er++;
		       onbase[i]->r++;
		       bat->score++;
		       runs++;
		       sprintf(tempstr,"%s scores",onbase[i]->nout());
		       outbuf(pbpfp,tempstr,", ");
		       break;
		case '1':			// If runner (batter) went to 1st
		case '2' :			// If runner went to second
		case '3' :			// If runner went to third
		       if ((i == 0) && (fc > 0)) {
			    sprintf(tempstr,"%s %s",onbase[i]->nout(),b[j]);
			    outbuf(pbpfp,tempstr,", ");}
		       else if (i > 0) {
			    sprintf(tempstr,"%s %s",onbase[i]->nout(),b[j]);
			    outbuf(pbpfp,tempstr,", ");}
		       break;
		default : ;
		}
	    }
	str++;
	}
    outbuf(pbpfp,"",". ");

    // if this is a fc play where nobody is out, we need to queue a pitcher
    if ((fc==1) && !(somebody_out)) runners->add(pit->mound);

    return;
}

    void 
frame::frameput(void)
{
    fprintf( output, "Pit: %-15s ", pit->mound->nout() );
    fprintf( output, "%s: %2d  %s: %2d   ",
    		ibl[0]->nout(), ibl[0]->score,
    		ibl[1]->nout(), ibl[1]->score );
    fprintf( output, "In: %d  Outs: %d\n", inning, outs );
    fprintf( output, "Bat: %-15s ", onbase[0]->nout() );
    for ( int i=1; i<4; i++ ) 
	fprintf( output, "%1d: %-15s ", i, 
		onbase[i] ? onbase[i]->nout() : "    ");
    fprintf( output, "\n" );
}

    void 
frame::rbi()
{
    char *str;

    str=baserunning;
    while (*str) {
        if (*str=='h')
                onbase[0]->rbi++;
        str++;
    }
}

    void 
frame::batterup()
{
    bat->next_up();
    onbase[0]=bat->up();
}

    void 
frame::help(char *str)
{

    if (*error == '\0') {
       fprintf(output,"Invalid string '%s'.\n",str);
    }
    else
	    fprintf(output,"%s\n",error);

    //if (input != stdin) exit(0);

    frameput();	
}

    int 
frame::runchck(char *runstr)
// Now modified to remove double occurrances of baserunning advances.
{
    char *str, *temptr;
    int flag=1, done[4];

    for (int j=0;j<=3;j++)
       done[j]=0;

    char temp[NAMELEN];

    temptr=temp;
    *temptr = '\0';

    // printf("%s\n",runstr);
    str=runstr;

    while (*str) {
	    switch (*str) {
		    case 'b': 
		       if (!done[0]){
			    switch (*(str+1)) {
				    case 'o':
				    case 'h':
				    case '1':
				    case '2':
				    case '3': break;
				    default: flag=0; break; }
			    if (flag) {
				    done[0]=1;
				    strncat(temp,str,2);
				    temptr+=2;
				    *temptr='\0';}}
			    break;
		    case '1':
		       if (!done[1]){
			    switch (*(str+1)) {
				    case 'o':
				    case 'h':
				    case '2':
				    case '3': break;
				    default: flag=0; break;}
			    if (!(onbase[1])) flag=0;
			    if (flag) {
				    done[1]=1;
				    strncat(temp,str,2);
				    temptr+=2;
				    *temptr='\0';}}
			    break;
		    case '2':	
		       if (!done[2]){
			    switch (*(str+1)) {
				    case 'o':
				    case 'h':
				    case '3': break;
				    default: flag=0; break; }
			    if (!(onbase[2])) flag=0;
			    if (flag) {
				    done[2]=1;
				    strncat(temp,str,2);
				    temptr+=2;
				    *temptr='\0';}}
			    break;
		    case '3':
		       if (!done[3]){
			    switch (*(str+1)) {
				    case 'o':
				    case 'h': break;
				    default: flag=0; break; }
			    if (!(onbase[3])) flag=0; 
			    if (flag) {
				    done[3]=1;
				    strncat(temp,str,2);
				    temptr+=2;
				    *temptr='\0';}}
			    break;
		    default: flag=0;
		    }
	    str+=2;
	    }
    if (flag)
       strcpy(runstr,temp);
    else
       strcpy(error,"baserunning is incorrect\n");
    //printf("%s\n",runstr);
    return flag;
}

    void 
frame::runcat(int adv)
/*
    runcat(-4) lead runner advances 1 base
    runcat(-3) lead runner out
    runcat(-2) everyone but the batter advances 1 base (wp, pb, etc.)
    runcat(-1) advance 1 base if forced only (bb, hb, etc.)
    runcat(0) batter is out
    runcat(x) everyone advances x base(s)
*/
{
    int test = 0;
    int i;
    char temp[NAMELEN];
    char *temptr;

    temptr=temp;
    *temptr='\0';


    switch (adv) {
    case 0 : 					// batter out
	    strcat(baserunning,"bo");
	    break;
	    
    case -1 :					// advance if forced
	    if (!onbase[1])
	       strcat(baserunning,"b1");
	    else if (!onbase[2])
	       strcat(baserunning,"b112");
	    else if (!onbase[3])
	       strcat(baserunning,"b11223");
	    else
	       strcat(baserunning,"b112233h");
	    break;
    //else {

    case -2 :					// runners advance 1 base
	    for (i=1;i<=3;i++)
	       if (onbase[i]) {
		    sprintf(temptr++,"%1d",i);
		    test=i+1;
		    if (test >= 4)
		       *temptr++='h';
		    else
		       sprintf(temptr++,"%1d",test);}
	    break;

    case -3 :
    //	strcat(baserunning,"1o");	
	    for (i=1;i<=3;i++)
	       if (onbase[i]) test=i;
	    sprintf(temptr++,"%1d",test);
	    sprintf(temptr++,"o");
	    break;

    case -4 :
	    for (i=1;i<=3;i++)
	       if (onbase[i]) test=i;
	    sprintf(temptr++,"%1d",test);
	    if (++test == 4)
	       sprintf(temptr++,"h");
	    else if (test > 0)
	       sprintf(temptr++,"%1d",test);
	    break;

    default :
	    *temptr++='b';
	    if (adv == 4)
	       *temptr++='h';
	    else
	       sprintf(temptr++,"%1d",adv);

	    for (i=1;i<=3;i++)
	       if (onbase[i]) {
		    sprintf(temptr++,"%1d",i);
		    test=i+adv;
		    if (test >= 4)
		       *temptr++='h';
		    else
		       sprintf(temptr++,"%1d",test);}
    } // switch

    *temptr='\0';
    //printf("%s\n",temp);
    strcat(baserunning,temp);
    //printf("%s\n",baserunning);
}

    int 
frame::three()
{
    if (baserunning[0] == '\0') {
	sprintf(error,"%s must have baserunning data.",event);
	return 0;
    }
    else
	return 1;
}

    void 
frame::who_stat(int stat)
{
   int who = location[0] - '0';
// printf("who - %d",who);
   switch (who) {
        case 0 : pit->newstat("\0",stat); break;
        case 1 : pit->newstat(pit->mound->nout(),stat); break;
        default: pit->newstat(pit->posout(who),stat);}
}

    void 
frame::cleanup()
{
	delete(runners);

	delete(ibl[0]);
	ibl[0] = 0;
	delete(ibl[1]);
	ibl[1] = 0;

	free(linescore[0]);
	free(linescore[1]);
	free(linescore);
	free(buffer);
}

    int 
frame::decode()
{
    if (event[0] == '\0') {
	    return 1;} 			// No event to decode, ignore.

    if (runchck(location)) {		// Baserunning in location field?
	    strcpy(baserunning,location);
	    location[0] = '\0';}
	    
    if (!(strcmp(event,"ph")))
	    return 1;  
    else if (!(strcmp(event,"pr"))) 
	    return 1;
    else if (!(strcmp(event,"np"))) 
	    return 1; 
    else if (!(strcmp(event,"dr")) || !(strcmp(event,"dc"))) 
	    return 1; 
    else if (!(strcmp(event,"la")))
	    return 1;
    else if (!(strcmp(event,"lh")))
	    return 1;
    else if (!(strcmp(event,"un"))) 
	    return 1;
    else if (!(strcmp(event,"en"))) 
	    return 1; 
    else if (!(strcmp(event,"eg"))) 
	    return 1; 
    else if (!(strcmp(event,"cm"))) 
	    return 1; 
    else if (!(strcmp(event,"so"))) 
	    return 1; 
    else if (!(strcmp(event,"kd"))) 
	    return 1;
    else if (!(strcmp(event,"bb"))) 
	    return 1;
    else if (!(strcmp(event,"iw"))) 
	    return 1;
    else if (!(strcmp(event,"ci"))) 
	    return 1;
    else if (!(strcmp(event,"hp")) || !(strcmp(event,"hb"))) 
	    return 1;
    else if (!(strcmp(event,"wp"))) 
	    return 1;
    else if (!(strcmp(event,"pb")))
	    return 1;
    else if (!(strcmp(event,"bk"))) 
	    return 1;
    else if (!(strcmp(event,"sb"))) 
	    return 1;
    else if (!(strcmp(event,"th"))) 
	    return 1;
    else if (!(strcmp(event,"cs"))) 
	    return 1;
    else if (!(strcmp(event,"kc")))
	    return 1;
    else if (!(strcmp(event,"ks")))
	    return 1;
    else if (!(strcmp(event,"pk"))) 
	    return 1;
    else if (!(strcmp(event,"oa"))) 
	    return 1;
    else if (!(strcmp(event,"1b"))) 
	    return 1;
    else if (!(strcmp(event,"2b"))) 
	    return 1;
    else if (!(strcmp(event,"3b"))) 
	    return 1;
    else if (!(strcmp(event,"hr"))) 
	    return 1;
    else if (!(strcmp(event,"gd")) || !(strcmp(event,"dp"))) 
	    return 1;
    else if (!(strcmp(event,"fd"))) 
	    return 1;
    else if (!(strcmp(event,"ld"))) 
	    return 1;
    else if (!(strcmp(event,"lo"))) 
	    return 1;
    else if (!(strcmp(event,"fc"))) 
	    return 1;
    else if (!(strcmp(event,"hg"))) 
	    return 1;
    else if (!(strcmp(event,"rg"))) 
	    return 1;
    else if (!(strcmp(event,"go")))
	    return 1;
    else if (!(strcmp(event,"sg"))) 
	    return 1;
    else if (!(strcmp(event,"hf")))
	    return 1;
    else if (!(strcmp(event,"po")))
	    return 1;
    else if (!(strcmp(event,"fp")) || !(strcmp(event,"pf")))
	    return 1;
    else if (!(strcmp(event,"sf"))) 
	    return 1;
    else if (!(strcmp(event,"sh"))) 
	    return 1;
    else if (!(strcmp(event,"lf"))) 
	    return 1;
    else if (!(strcmp(event,"df")) || !(strcmp(event,"wt")))
	    return 1;
    else if (!(strcmp(event,"fo")))
        return 1;
    else if (!(strcmp(event,"er"))) 
	    return 1;
    else if (!(strcmp(event,"ea"))) 
	    return 1;
    else if (!(strcmp(event,"nj")))
	    return 1;
    else if (!(strcmp(event,"fa")))
	    return 1;
    else return 0;
}
