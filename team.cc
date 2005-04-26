// team.cc

#include "team.h"

// Constructor functions

team::team()
{
char str[80];
int flag=1;
int i; 

   while (flag) {
	fprintf(output,"\nEnter the three letter team name:");
	fgets(str,100,input);
	if (strlen(str) == 3) {
 	    flag=0; 
	    for (i=0; i< 3; i++) {
		if (str[i] > 'Z' || str[i] < 'a') {
		    fprintf(stderr,"\nUse all caps only");
		    if (input == stdin) flag=1; else exit(0);}
		else if (str[i] <= 'z') str[i]=(char)(32+(int)str[i]);
		else if (str[i] < 'A') {
		    fprintf(stderr,"\nUse all caps only");
		    if (input == stdin) flag=1; else exit(0);}
	    }
	}
	else if (input != stdin) {
		fprintf(stderr,"Team name must be 3 letters!"); exit(0);}
    }
    strcpy(ibl,str);
    lineup= (struct pl_list *) malloc(sizeof (struct pl_list));
    lineup->head=NULL;
    lineup->next=NULL;
    lineup->ord=0;
    current=NULL;
    pitchers= (struct pit_list *) malloc(sizeof (struct pit_list));
    printf("\n");
    score=0;
    errors=0;
    lob=0;

    for (i = 0;i <= 8; i++){
	extra_stats[i].ord = 0;
	extra_stats[i].name[0] = '\n';
	extra_stats[i].next = NULL;}
}

// Constructor function getting team name; Assumes name is correct

team::team(char *str)
{
int i = 0; 

    while (i < 3) ibl[i++] = *str++;
    ibl[i] = '\0';
    lineup=(pl_list *)malloc(sizeof (struct pl_list));
    lineup->head=NULL;
    lineup->ord=0;
    lineup->next=NULL;
    pitchers= (struct pit_list *) malloc(sizeof (struct pit_list));
    current=NULL;
    score=0;
    errors=0;
    lob=0;

    for (i = 0;i <= 8; i++){
	extra_stats[i].ord = 0;
	extra_stats[i].name[0] = '\n';
	extra_stats[i].next = NULL;}
}

char* team::pos_change(int i)
{
int j = 0;

pl_list *oldpl = lineup;
char pos[3];
char comment[80];

char tempstr[512];
char *ptr = tempstr;

while (oldpl->next && oldpl->next->ord <= i) oldpl = oldpl->next;

fprintf(output,"Enter new position for %d - ",i);
fgets(tempstr,100,input);
while (*ptr != '\n') pos[j++]=*ptr++; 
pos[j]='\0';
fprintf(cmdfp,"%d\n",i);
fprintf(cmdfp,"%s",tempstr);
fprintf(output,"\n");

oldpl->head->new_pos(pos);
sprintf(comment,"%s moves to %s.",oldpl->head->nout(),pos);

return comment;
}

// Adds a new player at position i in the batting order
char* team::insert(int i, char *def, char *tempstr)
{
    struct pl_list *newpl,*oldpl;
    int flag,j;
    char name[20];
    char mlb[4];
    char pos[3];
    char comment[80];
    char str[512];
    
    newpl=(struct pl_list *)malloc(sizeof (struct pl_list));
    newpl->ord=i;
    newpl->next=NULL;
    flag=1;
    
    if (!(*tempstr)) {
	  tempstr=str;
        if (!(tempstr)) {fprintf(stderr,"Malloc error\n");exit(0);}
    	while (flag) {
		fprintf(output,"Player, MLB team & Position for %d - ",i);
		flag=0;
		fgets(str,100,input);
		if (strcmp(def,"ph"))	
		   fprintf(cmdfp,"%d\n",i);
		fprintf(cmdfp,"%s",str);
		fprintf(output,"\n");
		j=0;
		while ((name[j]=*tempstr++) != ' ' && j++ < 20) ;
		if (j==20) {
		   fprintf(stderr,"\nName too long\n");
		   if (input == stdin) flag=1; else exit(0);}
		else name[j]='\0';
		j=0;
		while ((mlb[j]=*tempstr++) != ' ' && j++ < 4) ;
		if (j==4) {
                   fprintf(stderr,"\nMLB too long\n");
                   if (input == stdin) flag=1; else exit(0);}
		else mlb[j]='\0'; 
		j=0;
		while (*tempstr != '\n') pos[j++]=*tempstr++;
		pos[j]='\0';}
    }
    else {    
	j=0;
	while ((name[j]=*tempstr++) != ' ' && j++ < 20) ;
	name[j]='\0';
	j=0;
	while ((mlb[j]=*tempstr++) != ' ' && j++ < 4) ;
	mlb[j]='\0'; 
	j=0;
	while (*tempstr != '\0') pos[j++]=*tempstr++;
	pos[j]='\0';}

    oldpl=lineup;
    while (oldpl->next && oldpl->next->ord <= i) oldpl=oldpl->next;

    if (!(strcmp(pos,def))) {
	newpl->head= new player(name,ibl,mlb,def);
	sprintf(comment,"%s %s for %s, batting %d.",
		name,def,oldpl->head->nout(),i);}
    else if (pos[0] == 'p') {
	newpl->head= new player(name,ibl,mlb,pos);
	sprintf(comment,"%s now pitching for %s, batting %d.",
		name,ibl,i);}
    else if (def[0] == '\0') {
	newpl->head= new player(name,ibl,mlb,pos);
	sprintf(comment,"%s now %s for %s, batting %d.",
		name,pos,ibl,i);}
    else {
	newpl->head= new player(name,ibl,mlb,def);
	newpl->head->new_pos(pos); 
        sprintf(comment,"%s %s for %s, batting %d.",
                name,def,oldpl->head->nout(),i);}

    newpl->next=oldpl->next;
    oldpl->next=newpl;
    if (up()==oldpl->head) current=newpl;

    return comment;
}

// Creates the lineups for a team
int team::make_lineups()
{
    struct pl_list *newpl;
    int i,j,flag;

    char str[512];
    char *tempstr = str;
    char name[20];
    char mlb[4];
    char pos[3];

    current=newpl=lineup;
    fprintf(output,"Enter lineup for %s \n",ibl);
    for (i=1;i<10;i++)
	{
	if (i==1) newpl=lineup;
	else {
	    newpl->next=(struct pl_list *)malloc(sizeof (struct pl_list));
	    newpl=newpl->next;
	    newpl->next=NULL;
	    }
	newpl->ord=i;
  	flag=1;
	while (flag) {
	    fprintf(output,"Player, MLB team & Position for %d - ",i);
	    flag=0;
	    fgets(str,100,input);
	    fprintf(cmdfp,"%s",str);
	    tempstr=str;
	    j=0;
	    while ((name[j]=*tempstr++) != ' ' && j++ < 20) ;
	    if (j==20) {
                   fprintf(stderr,"\nName too long\n");
                   if (input == stdin) flag=1; else exit(0);}
	    else name[j]='\0';
	    j=0;
	    while ((mlb[j]=*tempstr++) != ' ' && j++ < 4) ;
	    if (j==4) {
                   fprintf(stderr,"\nMLB too long\n");
                   if (input == stdin) flag=1; else exit(0);}
	    else mlb[j]='\0'; 
	    j=0;
            while (*tempstr != '\n') pos[j++]=*tempstr++;
            pos[j]='\0';
	    }
	newpl->head= new player(name,ibl,mlb,pos);
	}
    fprintf(output,"\n");
    return 1;
}

// Do the pitchers
int team::make_lineups_pit()
{
    int j,flag;

    char str[512];
    char *tempstr = str;
    char name[20];
    char mlb[4];
    char throws;

    flag=1;
    while (flag) {
	fprintf(output,"Starting pitcher for %s, MLB team & Throws - ",ibl);
	flag=0;
	fgets(str,100,input);
	fprintf(cmdfp,"%s",str);
	tempstr=str;
	j=0;
	while ((name[j]=*tempstr++) != ' ' && j++ < 20) ;
	if (j==20) {
                   fprintf(stderr,"\nName too long\n");
                   if (input == stdin) flag=1; else exit(0);}
	else name[j]='\0';
	j=0;
	while ((mlb[j]=*tempstr++) != ' ' && j++ < 4) ;
	if (j==4) {
                   fprintf(stderr,"\nMLB too long\n");
                   if (input == stdin) flag=1; else exit(0);}
	else mlb[j]='\0'; 
	throws=*tempstr;
	}
    pitchers->head= new pitcher(name,ibl,mlb,throws);
    pitchers->next=0;
    mound=pitchers->head;
//fprintf(stderr,"%d:%s->%d\n",pitchers,pitchers->head->nout(),pitchers->next);
    fprintf(output,"\n");
    return 1;
}


// Outputs the box score
void team::box_score(FILE *fp)
{
struct pl_list *newpl;
struct pit_list *newpit;

int ab, h, r, rbi, b2, b3, hr, bb, k, sb, cs, pal, par, out, er;

ab=h=r=rbi=b2=b3=hr=bb=k=sb=cs=pal=par=out=er=0;

fprintf(fp,"\n\nBATTERS  %-29s AB  R  H RI 2B 3B HR SB CS BB  K PL PR\n", ibl);
	newpl=lineup;
	while (newpl) {
		ab=ab+newpl->head->ab;
        	h=h+newpl->head->h;
        	r=r+newpl->head->r;
        	rbi=rbi+newpl->head->rbi;
        	b2=b2+newpl->head->b2;
        	b3=b3+newpl->head->b3;
        	hr=hr+newpl->head->hr;
        	bb=bb+newpl->head->bb;
        	k=k+newpl->head->k;
        	sb=sb+newpl->head->sb;
        	cs=cs+newpl->head->cs;
        	pal=pal+newpl->head->pal;
        	par=par+newpl->head->par;
		fprintf(fp,"%i ",newpl->ord);
		newpl->head->sout(fp);
		fprintf(fp,"\n");
		newpl=newpl->next;
	}
fprintf(fp,"\n");
fprintf(fp,"TOTALS for %-26s %3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d\n",
			ibl,ab,r,h,rbi,b2,b3,hr,sb,cs,bb,k,pal,par);

ab=h=r=rbi=b2=b3=hr=bb=k=sb=cs=pal=par=out=er=0;

fprintf(fp,"\nPITCHERS  %-22s   IP  H  R ER BB  K HR \n", ibl);
	newpit=pitchers;
	while (newpit) {
		out=out+newpit->head->out;
		h=h+newpit->head->h;
		r=r+newpit->head->r;
		er=er+newpit->head->er;
		bb=bb+newpit->head->bb;
		k=k+newpit->head->k;
		hr=hr+newpit->head->hr;
		newpit->head->sout(fp);
		fprintf(fp,"\n");
		newpit=newpit->next;
	}
fprintf(fp,"\n");
fprintf(fp,"TOTALS for %-20s %3d.%1d%3d%3d%3d%3d%3d%3d\n\n",
                ibl,out/3,out % 3,h,r,er,bb,k,hr);
}

// Returns the next batter in the order
player* team::next_up()
{

if (current && current->next) current=current->next;
else current=lineup;

while (current->next && current->ord==current->next->ord) 
		current=current->next;

return current->head;

}

player* team::back_up()
{

if(current->ord != 1)
	current=findord_pl(current->ord-1);
else
	current=findord_pl(9);

return current->head;

}
// Returns the current batter
player* team::up()
{
return current->head;
}

// Inputs a new pitcher
int team::new_pit()
{
    struct pit_list *newpit,*oldpit;
    int flag,j;
    char str[512];
    char *tempstr = str;
    char name[20];
    char mlb[4];
    char pos[3];
    char bats = '0';
    
    newpit=(struct pit_list *)malloc(sizeof (struct pit_list));
    newpit->next=NULL;
    flag=1;
    while (flag) {
	fprintf(output,"Relief Pitcher, MLB team & Throws - ");
	flag=0;
	fgets(str,100,input);
	fprintf(cmdfp,"%s",str);
	j=0;
	while ((name[j]=*tempstr++) != ' ' && j++ < 20) ;
	if (j==20) {
                   fprintf(stderr,"\nName too long\n");
                   if (input == stdin) flag=1; else exit(0);}
	else name[j]='\0';
	j=0;
	while ((mlb[j]=*tempstr++) != ' ' && j++ < 4) ;
	if (j==4) {
                   fprintf(stderr,"\nMLB too long\n");
                   if (input == stdin) flag=1; else exit(0);}
	else mlb[j]='\0'; 
	pos[0]=*tempstr;
	while (*++tempstr != '\n') bats = *tempstr;
    }
//here
    newpit->head= new pitcher(name,ibl,mlb,pos[0]);
    newpit->next=0;
    oldpit=pitchers;
//fprintf(stderr,"\n%d:%s->%d ",oldpit,oldpit->head->nout(),oldpit->next);
    while (oldpit->next) {
//fprintf(stderr,"%d:%s->%d ",oldpit,oldpit->head->nout(),oldpit->next);
	oldpit=oldpit->next;}
    oldpit->next=newpit;
    mound=newpit->head;
    return (bats - '0');
}

char* team::nout()
{
return(ibl);
}

player* team::findord(int i)
{
struct pl_list *oldpl;

    oldpl=lineup;
    while (oldpl->next && oldpl->next->ord <= i) oldpl=oldpl->next;
    return(oldpl->head);
}

struct pl_list* team::findord_pl(int i)
{
struct pl_list *oldpl;

    oldpl=lineup;
    while (oldpl->next && oldpl->next->ord <= i) oldpl=oldpl->next;
    return(oldpl);
}
void team::print_lineup(void)
{
struct pl_list *newpl;

fprintf(output,"\n Lineup for %s\n",nout());
newpl=lineup;
while (newpl) {
	while (newpl->next && newpl->ord==newpl->next->ord) newpl=newpl->next;
	fprintf(output,"%d. %s, %s\n",newpl->ord,newpl->head->nout(),newpl->head->pout());
	newpl=newpl->next;
	}
}

int team::team_hits ()
{

int total = 0;
struct pl_list *newpl;

newpl=lineup;

while(newpl) {
   total = total + newpl->head->h;
   newpl = newpl->next;}

return (total);
}

void team::newstat(char *pl_name, int stat)
{
struct stat_list *curr, *prev;

extra_stats[stat].ord++;
prev = curr = extra_stats[stat].next;

if (*pl_name) {
//printf("p:%d c:%d\n",prev,curr);
   if (prev) {
	while ((curr) && (strcmp(prev->name,pl_name))){
//printf("p:%d c:%d\n",prev,curr);
		prev = curr;
		curr = curr->next;}
	if (!(strcmp(prev->name,pl_name))){
		prev->ord++;}
	else {
//printf("p:%d c:%d\n",prev,curr);
		curr = (stat_list*) malloc(sizeof(stat_list));
		curr->ord = 1;
		strcpy(curr->name,pl_name);	
		curr->next = NULL;
		prev->next = curr;}
//printf("p:%d c:%d\n",prev,curr);
   }	
   else {
//printf("p:%d c:%d\n",prev,curr);
	prev = (stat_list*) malloc(sizeof(stat_list));
	prev->ord = 1;
	strcpy(prev->name,pl_name);
	prev->next = NULL;
	extra_stats[stat].next = prev;
//printf("p:%d c:%d\n",prev,curr);
   }

} // end if (name)

}

void team::printstat(FILE *fp,int stat)
{

struct stat_list *curr;
//printf("es:%d\n",extra_stats[stat].next);

curr = extra_stats[stat].next;

if (extra_stats[stat].ord){
	fprintf(fp,"%d - ",extra_stats[stat].ord);
	while(curr->next) {
//printf("c:%d cn:%d\n",curr,curr->next);
	   if (curr->ord > 1) {
		fprintf(fp,"%s %d, ",curr->name,curr->ord);
		curr = curr->next;}
	   else {
                fprintf(fp,"%s, ",curr->name);
                curr = curr->next;}
	}
        if (curr->ord > 1)
	   fprintf(fp,"%s %d.\n",curr->name,curr->ord);
	else
           fprintf(fp,"%s.\n",curr->name);
}

else
	fprintf(fp,"none.\n");
}

char* team::posout(int position)
{

struct pl_list *curr = lineup;

while (curr->next) {
	if (curr->ord == curr->next->ord)
		curr = curr->next;
	else if (curr->head->posn == position)
		return (curr->head->nout());
	else
		curr = curr->next;}

if (curr->head->posn == position)
        return (curr->head->nout());
else
	return ("\0");
}

int team::what_ord(player *up)
{

   struct pl_list *curr = lineup;

   while (curr->head != up)
	curr = curr->next;

   return (curr->ord);
}

void team::decisions()
{

struct pit_list *curr = pitchers;
char wls[100];

fprintf(output,"\nEnter W/L/S for appropriate %s pitcher.  <CR> if none.\n",ibl);
while (curr) {
	fprintf(output,"%s :",curr->head->nout());
	fgets(wls,80,input);
	fprintf(cmdfp,"%s",wls);
	switch (*wls) {
		case 'w' :
		case 'W' : curr->head->dec = 'W';
			   break;
		case 'l' :
		case 'L' : curr->head->dec = 'L';
			   break;
		case 's' :
		case 'S' : curr->head->dec = 'S';
			   break;}
	curr = curr->next;
	}
}
 
void team::unearned(int inning)
{
int numout=0;
int ur=0;
char urstr[100];

struct pit_list *curr = pitchers;

while (curr) 
      {
      if (curr->head->out + numout >= (inning-1)*3)
              {
              fprintf(output,"Enter unearned runs for %s ",curr->head->nout());
              fgets(urstr,80,input);
	      fprintf(cmdfp,"%s",urstr);
              ur=atoi(urstr);
	      while (ur < 0 || ur > curr->head->er) {
                fprintf(stderr,"Invalid unearned runs total.\n");
              	fprintf(output,"Enter unearned runs for %s ",curr->head->nout());
              	fgets(urstr,80,input);
	      	fprintf(cmdfp,"%s",urstr);
              	ur=atoi(urstr);
	      }
              numout+=curr->head->out;
	      curr->head->er=curr->head->er-ur;
              }
      else
              numout+=curr->head->out;
      curr=curr->next;
      }
}

team::~team()
{
   pl_list *killme;

   while (lineup) {
	killme = lineup;
	delete (killme->head);
	lineup =  lineup->next;
	free (killme);}

   pit_list *killmetoo;

   while (pitchers) {
	killmetoo = pitchers;
	delete (killmetoo->head);
	pitchers = pitchers->next;
	free (killmetoo);}

   stat_list *prev, *curr;

   for (int i=0; i<9; i++) {
	prev = curr = extra_stats[i].next;
	while (curr) {
	   prev = curr;
	   curr = curr->next;
	   free (prev);}
	}
//fprintf(stderr,"deleted %s\n",this->nout());
}
