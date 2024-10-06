// $Header$
// -------=====================<<<< COPYRIGHT >>>>========================-------
//          Copyright (c) 1995-2001 David Caldwell,  All Rights Reserved.
//  See full text of copyright notice and limitations of use in file COPYRIGHT.h
// -------================================================================-------

#include "COPYRIGHT.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include "version.h"

#define CR '\r'
#define LF '\n'
#define qCR "\r"
#define qLF "\n"

struct {
    char *operation;
    char *name;
    char *eol;
} gEOL[] = {
    { "tomac",  "Macintosh (CR)", qCR     },
    { "tounix", "Unix (LF)",      qLF     },
    { "todos",  "DOS (CRLF)",     qCR qLF },
    { "towin",  "DOS (CRLF)",     qCR qLF },
    { NULL }
};

int Eol(char *inFileName,char *outFileName,char *eol);
int ConvertEOFs(char *in, char *out,int length, char *eol);
int move(char *oldpath, char *newpath,char *eol);

void Usage(char *program)
{
    printf("%s version %s\n",program,VersionString);
    printf("usage:\n"
           "   %s [-c <conversion>] [infile [-o outfile] ...]\n"
           "      For each infile, if '-o' is specified, 'infile' will be copy converted to 'outfile'.\n"
           "      Otherwise, 'infile' will be converted in-line.\n"
           "   %s -h   # Print help\n"
           "   %s -v   # Print version information\n"
           "\n"
           " <conversion> can be one of:\n",program,program,program);
    for (int i=0;gEOL[i].operation;i++)
        printf("    %-6s  -- convert End-Of-Lines to %s style.\n",gEOL[i].operation,gEOL[i].name);
    exit(1);
}

int streq(char *s,char *s1)
{
    return strcmp(s,s1) == 0;
}

FILE *OpenAndErr(char *name, char *mode)
{
    //special case for "-" filename.
    if (streq(name,"-")) {
        if (*mode == 'r')
            return stdin;
        if (*mode == 'w')
            return stdout;
    }

    FILE *file = fopen(name,mode);
    if (!file) {
        fprintf(stderr,"couldn't open '%s'.\n",name);
        exit(1);
    }
    return file;
}
int CheckErrors(FILE *f, char *name)
{
    if (ferror(f)) {
        fprintf(stderr,"Error using file '%s'.\n",name);
        return 1;
    }
    return 0;
}

int main (int c, char **v)
{
    char *program,*operation,*base;
    program = v[0];   // this doesn't change -- its what they said to run

    if((base=strrchr(v[0],'/')) || (base=strrchr(v[0],'\\')))
        program = base + 1;

#if defined(__WIN32__) || defined(_WIN32)
    for(char *cc=program; *cc; cc++)
        *cc = tolower(*cc);
    char *dot = strstr(program,".exe");
    if(dot)
        *dot = '\0';
#endif

    operation = program;

    int ch;
    while ((ch = getopt(c, v, "c:vh?")) != -1) {
        switch(ch) {
            case 'c': operation = optarg; break;
            case 'h':
            case '?': Usage(program);
            case 'v': printf("%s %s\n",program,VersionString); exit(0);

        }
    }
    c -= optind;
    v += optind;

    if (c==0) {
        static char *stdinout_args[] = { "-", "-o", "-" };
        v = stdinout_args;
        c = 3;
    }

    int style;
    for (style=0;gEOL[style].operation;style++)
        if (strcmp(gEOL[style].operation,operation) == 0)
            goto goodOperation;
    fprintf(stderr,"%s: Bad operation %s\n", program, operation);
    Usage(program);

  goodOperation:
    while(c) {
        char *inFileName = *v;
        v++;c--;
        char *outFileName = NULL;
        if (c) {
            if (strcmp(*v,"-o")==0) {
                v++;c--;
                if (!c) {
                    fprintf(stderr,"%s: -o requires a filename\n",program);
                    Usage(program);
                }
                outFileName = *v;
                v++;c--;
            }
        }

        Eol(inFileName,outFileName,gEOL[style].eol);
    }
}

int Eol(char *inFileName,char *outFileName,char *eol)
{
    FILE *in = OpenAndErr(inFileName,"rb");

    char tmpName[L_tmpnam];
    int inPlace = 0;
    if (!outFileName) {
        outFileName = tmpnam(tmpName);
        inPlace = 1;
    }

    FILE *out = OpenAndErr(outFileName,"wb");

#define BUFFER_SIZE 16384
    do {
        char inBuffer[BUFFER_SIZE];
        char outBuffer[BUFFER_SIZE*2]; // worst case unix->dos, with whole file of LFs.
        int read  = fread(inBuffer,1,BUFFER_SIZE,in);
        int write = ConvertEOFs(inBuffer,outBuffer,read,eol);
        fwrite(outBuffer,1,write,out);
        if (CheckErrors(in,inFileName) || CheckErrors(out,outFileName))
            exit(1);
    } while (!feof(in));
    fclose(out);
    fclose(in);

    if (inPlace) {
        if (!remove(inFileName)) {
            if (move(outFileName,inFileName,eol)) {
                //fprintf(stderr,"Couldn't move '%s' to '%s'\n",outFileName, inFileName);
                return -1;
            }
        } else {
            fprintf(stderr,"Couldn't delete file '%s'.\n",inFileName);
            return -1;
        }
    }
    return 0;
}

int ConvertEOFs(char *in, char *out,int length, char *eol)
{
    static char last = 0;
    int outLength=0;
    int eolLength = strlen(eol);

    while (length--) {
        char current = *in++;
        if ((last == CR || last == LF) && (current == CR || current == LF) && last != current)
            current = 0; // dont count this char in next round--throw away CRLF pairs (or LFCR)
        else if (current == CR || current == LF) {
            strncpy(out,eol,eolLength);
            out += eolLength;
            outLength += eolLength;
        } else {
            *out++ = current;
            outLength++;
        }
        last = current;
    }
    return outLength;
}

#include <errno.h>

int move(char *oldpath, char *newpath,char *eol)
{
    int err=0;
    if ((err = rename(oldpath,newpath)) && errno == EXDEV) {
        // Cant move accross filesystems, so do it manually
        if (Eol(oldpath,newpath,eol)) {
            fprintf(stderr,"Couldn't copy file '%s' to '%s'\n",oldpath,newpath);
            return -1;
        }
        if (remove(oldpath)) {
            fprintf(stderr,"Couldn't delete '%s'.\n",oldpath);
            return -1;
        }
    } else if (err) {
        fprintf(stderr,"Couldn't rename '%s' to '%s'.\n",oldpath,newpath);
        return -1;
    }
    return 0;
}
