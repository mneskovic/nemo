/* file_size, file_lines:
 *
 *      FILE_SIZE:   get size (in bytes) of a file,
 *                   return -1 if file not found or some other error
 *                   find out by looking at 'errno'
 *
 *      xx-xxx-xx   Original version        PJT
 *      25-nov-90   doc written - nemo_main PJT
 *	25-feb-92   happy gcc 2.0	    PJT
 *	 5-nov-93   added second arg to file_lines!!!	PJT
 *	26-feb-94   ansi and better TESTBED defaults	pjt
 *      20-jun-01   prototypes 
 *	12-sep-01   nemo_
 *      21-sep-03   changed meaning of 'deflen' in nemo_file_lines    PJT
 */

#include <stdinc.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

int nemo_file_size(char *name)
{
  struct stat buf;
  
  if (stat(name,&buf) == 0)
    return buf.st_size;
  else {
    dprintf(1,"nemo_file_size: stat returned errno=%d\n",errno);
    return -1;
  }
}

int nemo_file_time(char *name)
{
  struct stat buf;
  
  if (stat(name,&buf) == 0)
    return (int)buf.st_mtime;
  else {
    dprintf(1,"nemo_file_time: stat returned errno=%d\n",errno);
    return -1;
  }
}

/*
 *      FILE_LINES:   get size (in lines) of a file,
 *                    return -1 if file not found or some other error
 *                    find out by looking at 'errno'
 */

#define BUFSIZE  8192

int nemo_file_lines(char *name, int deflen)
{
    int len, n, cnt=0;
    char *cp, *buf;
    stream str;

    len = nemo_file_size(name);
    if (len<0) {       /* probably a pipe or special NEMO thing, so we need hints */
      if (deflen == 0) return MAXLINES;
      return ABS(deflen);
    }
    if (len==0) return 0;    /* file has 0 length, so no lines */
    if (deflen > 0) return deflen;
    
#if 0
    if (isatty(fileno(str))) {
        warning("nemo_file_lines: %s is not a true file",name);
        return(-1);
    }
#endif

    buf = (char *) allocate(BUFSIZE);
    str = stropen(name,"r");

    /* this is a very expensive operation, preferred it to pass
     * an estimated length to the program 
     */

    while ( (n=fread(buf,1,BUFSIZE,str)) > 0) {
        cp = buf;
        while (n--)
            if (*cp++ == '\n')
                cnt++;
    }
    free(buf);
    return cnt;
}


#if defined(TESTBED)

#include <getparam.h>

string defv[] = {
    "in=file_size.c\n       File to test file_size/lines on",
    "nmax=10000\n           Default size for files on pipes",
    "VERSION=1.4\n          12-sep-01 PJT",
    NULL,
};
string usage="file_size.c testbed";

nemo_main()
{
    string fname;
    int deflen;
    char cmd[128];
    extern int errno;

    fname = getparam("in");
    deflen = getiparam("nmax");
    printf("%s file_size() = %d %d\n",
        fname, nemo_file_size(fname), nemo_file_time(fname));
    if (errno != 0)
        perror("perror::Error in file_size");

    printf("%s file_lines() = %d\nwc: ",
        fname, nemo_file_lines(fname,deflen));
    sprintf(cmd,"wc %s",fname);
    system(cmd);
        
}
#endif
