#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<assert.h>
#include<errno.h>
#define COMDLEM 256
#define NAMELEN 64
#define PASSWDF "/etc/passwd"
#define GROUPFL "/etc/group"
#define U   'u'
#define G   'g'
static char * search_name(int id,int m) {

    FILE * fp;
    char buf[256];
    char comd[COMDLEM];
    static char name[NAMELEN];

    switch(m) {
    case 'u':   sprintf(comd,"grep x:%d %s",id,PASSWDF);
                break;
    case 'g':   sprintf(comd,"grep %d %s",id,GROUPFL);
                break;
    default:    printf("[Error]: something wrong.\n");
                exit(-1);
    }

    if((fp = popen(comd,"r")) == NULL) {
        fprintf(stderr,"Con't popen command [%s]: %s",
            comd,strerror(errno));
        exit(-errno);
    }

    if(!fgets(buf,256,fp)) {
        perror("fgets");
        pclose(fp);
        exit(-1);
    }

    for(int i = 0;i < NAMELEN;++i) {
        if(buf[i] == ':') {
        name[i] = 0;
        break;
        }
        name[i] = buf[i];
    }

    name[NAMELEN - 1] = 0;
    
    pclose(fp);

    return name;
}

char * getuname(int id) {
    return search_name(id,U);
}
char * getgname(int id) {
    return search_name(id,G);
}
