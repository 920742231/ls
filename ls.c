#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include<string.h>
#include<time.h>
#include<sys/time.h>
#include<assert.h>
#include<errno.h>
#include<getopt.h>
#include<dirent.h>
#include<sys/types.h>

#ifndef MALLOC
#define MALLOC(value,type,size) {\
    value = (type)malloc(size);\
    assert(value);}
#endif

#define LONG_MOD(mode) (mode |= 0x02)
#define RECU_MOD(mode) (mode |= 0x04)

#ifndef PUSAGE
#define PUSAGE printf(\
    "Usage: ls [-l]/[-R]/[-lR]/ <file list>\n")
#endif

void do_ls(char * fname,int options);

int main(int argc,char ** argv) {

    int mode;
    char opt;
    char ** args;
    
    /*
     *  Set mode 0001 as not options,print file 
     *  state on simple mode(just file name).
     */
    mode = 1;
    /*  -------------------------------------------
     *  Explain commamd options by user parameters.
     *  And add options to value 'mode' to record.
     *  
     *  LONG_MODE----------------------------------
     *  '-l' in options,means print file message on
     *  long mode(use file states in struct stat).
     *  Set mode |= 0010 as long mode.
     *  
     *  RECU_MODE----------------------------------
     *  '-R' in options,means print file message on
     *  recu mode(Recuersivly prirnt the direntory 
     *  tree).
     *  Set mode |= 0100 as recu mode.
     *  
     *  PUSAGE-------------------------------------
     *  Wrong optons,show tips to user.
     * 
     *  And option '-l' and '-R' are legal to use 
     *  together.
     */
    while((opt = getopt(argc,argv,"lR")) \
        != -1){
        switch(opt) {
        case 'l':   LONG_MOD(mode);
                    break;
        case 'R':   RECU_MOD(mode);
                    break;  
        default:    PUSAGE;
                    exit(0);
        }
    }

    /*  Create files list.  */
    if(optind == argc) {
    /*  No other args as file name,use default file "." */
        MALLOC(args,char **,sizeof(char **) * 2);
        MALLOC(args[0],char *,sizeof(char) * 2);
        strcpy(args[0],".");
        args[1] = NULL;
    }
    else {
    /*  Files' names are indicated by user parameters. */
        MALLOC(args,char **,
            sizeof(char *) * (argc - optind + 1));
        for(int i=optind,j = 0;i < argc;++i,++j)
            args[j] = argv[i];
        args[argc - optind] = NULL;
    }

    while(*args) do_ls(*(args++),mode);

    return 0;
}

/*
Options fo ls.
LONGMODE : ls -l
NORMMODE : ls
RECUMODE : ls -R
LONGRECU : ls -lR / ls -l -R */
#define NORMMODE    1
#define LONGMODE    3
#define RECUMODE    5
#define LONGRECU    7

#define FNMELEN     512

static inline void err_exit(char *,int);

void show_st(char *,struct stat *);

/*
 * Show file state on long mode or normal mode.
 */
static void __norm_long(struct stat * st,
    char * fname,int mode) {
    
    int fd;
    DIR * dir;
    struct stat _st;
    struct dirent * dt;
    char name[FNMELEN];

    /*
     *  Not a directory,show its message.
     */
    if(!S_ISDIR(st->st_mode)) {
        if(mode == NORMMODE) printf("%s\n",fname);
        else if(mode == LONGMODE) show_st(fname,st);
        return;
    }
    /*
     *  Else,open this driectory and show its child
     *  files's message.
     */
    if(!(dir = opendir(fname))) 
        err_exit("opendir",-errno);

    printf("%s:\n",fname);

    while((dt = readdir(dir))) {
    if(mode == NORMMODE) 
        printf("%s  ",dt->d_name);
    else if(mode == LONGMODE) {
        sprintf(name,"%s/%s",fname,dt->d_name);
        
        if((fd = open(name,O_RDONLY)) < 0)
            err_exit("open",-errno);
        if(fstat(fd,&_st) < 0) 
            err_exit("fstat",-errno);
        
        show_st(dt->d_name,&_st);
        
        close(fd);
        }
    }

    if(mode == NORMMODE) printf("\n");

    closedir(dir);  
}


static void __recu_ls(struct stat * st,
    char * fname,int mode) {

    int fd;
    DIR * dir;
    struct stat _st;
    struct dirent * dt;
    char name[FNMELEN];

    /*
     *  Show current file's message.
     */
    __norm_long(st,fname,mode - 4);
    
    if(!S_ISDIR(st->st_mode)) return;
    
    /*
     *  Show child directories message recursivly.
     */
    if(!(dir = opendir(fname)))
        err_exit("opendir",-errno);

    while((dt = readdir(dir))) {
        if(strcmp(".",dt->d_name) == 0 \
        || strcmp("..",dt->d_name) == 0)
            continue;
        
        sprintf(name,"%s/%s",fname,dt->d_name);
        
        if((fd = open(name,O_RDONLY)) < 0)
            err_exit("open",-errno);
        if(fstat(fd,&_st) < 0) 
            err_exit("fstat",-errno);
        /*
         *  Not a directory,don't need showing states.
         */
        if(!S_ISDIR(_st.st_mode)) goto cont;

        /*
         *  Else.
         */
        __recu_ls(&_st,name,mode);
cont:
        close(fd);
    }

    closedir(dir);
}

#define __long_ls(st,name) __norm_long(st,name,LONGMODE)
#define __norm_ls(st,name) __norm_long(st,name,NORMMODE)
#define __long_recu(st,name) __recu_ls(st,name,LONGRECU)
#define __norm_recu(st,name) __recu_ls(st,name,RECUMODE)

void do_ls(char * name,int options) {

    int fd;
    struct stat st;

    if((fd = open(name,O_RDONLY)) < 0) 
        err_exit("open",-errno);
    if(fstat(fd,&st) < 0)
        err_exit("fstat",-errno);

    /*
     *  Show different kinds of file message by options.
     */
    switch(options) {
        case NORMMODE:  __norm_ls(&st,name);
                        break;
        case LONGMODE:  __long_ls(&st,name);
                        break;
        case RECUMODE:  __norm_recu(&st,name);
                        break;
        case LONGRECU:  __long_recu(&st,name);
                        break;
        default:        printf("[Error] something wrong!\n");
                        exit(-1);
    }

    close(fd);
}

/*
File modes:
EXEC - "--x"(001)
WRIT - "-w-"(010)
WADX - "-wx"(011)
READ - "r--"(100)
RADX - "r-x"(101)
RADW - "rw-"(110)
RWAX - "rwx"(111)
*/
#define EXEC    1
#define WRIT    2
#define WADX    3
#define READ    4
#define RADX    5
#define RADW    6
#define RWEX    7

#define FMOD_TRANS(mode) (mode & 0x07)

static void __explain_mod(int mode,char * str) {

    switch(mode) {
    case READ:  snprintf(str,4,"%s","r--");
                break;
    case WRIT:  snprintf(str,4,"%s","-w-");
                break;
    case RADW:  snprintf(str,4,"%s","rw-");
                break;
    case EXEC:  snprintf(str,4,"%s","--x");
                break;
    case RADX:  snprintf(str,4,"%s","r-x");
                break;
    case WADX:  snprintf(str,4,"%s","-wx");
                break;
    case RWEX:  snprintf(str,4,"%s","rwx");
    default:    break;
    }
}

static char * __get_mode_str(int mode) {
    int pos;
    char * mode_str;
    
    MALLOC(mode_str,char *,12);

    /*
     *  Get file type.
     */
    struct stat st;
    if(S_ISDIR(mode)) mode_str[0] = 'd';
    else if(S_ISCHR(mode)) mode_str[0] = 'c';
    else if(S_ISBLK(mode)) mode_str[0] = 'b';
    else mode_str[0] = '-';

    pos = 1;
    
    /*
     *  Get file mode.
     */
    __explain_mod(FMOD_TRANS(mode >> 6),mode_str + pos);
    pos += 3;
    __explain_mod(FMOD_TRANS(mode >> 3),mode_str + pos);
    pos += 3;
    __explain_mod(FMOD_TRANS(mode),mode_str + pos);
    pos += 3;

    mode_str[pos] = 0;

    return mode_str;
}

extern char * getuname(int);
extern char * getgname(int);

/*
 *  Print file states.
 */
#define __SHOW_ST(mode,nlink,u,g,len,mtim,name) \
    printf("%s\t%ld\t%s\t%s\t%ld\t%s\t%s\n",\
    mode,nlink,u,g,len,mtim,name)

static void __show_st(char * fname,struct stat * st) {
    
    char * mode_str;
    char * time_str;
    char * usr_name;
    char * grp_name;

    mode_str = __get_mode_str(st->st_mode); //mode string

    time_str = ctime(&st->st_mtim.tv_sec);  //time string
    time_str[strlen(time_str) - 1] = 0;     //delete '/n'

    usr_name = getuname(st->st_uid);        //user name
    grp_name = getgname(st->st_gid);        //group name

    __SHOW_ST(mode_str,st->st_nlink,usr_name,grp_name,
        st->st_size,time_str,fname);
    
    free(mode_str);
    free(usr_name);
    free(grp_name);
}

void show_st(char * fname,struct stat * st) {
    __show_st(fname,st);
}

static inline void err_exit(char * msg,int no) {
    fprintf(stderr,"[Error] %s: %s\n",msg,strerror(errno));
    exit(no);
}
