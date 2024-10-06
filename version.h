#define MajorVersion  1
#define MinorVersion  1
#define BugFixVersion 0

#define Cat2(x,y) x##y
#define Cat(x,y) Cat2(x,y)
#define MajorVersionBCD Cat(0x,MajorVersion)
#define MinorVersionBCD Cat(0x,MinorVersion)
#define BugFixVersionBCD Cat(0x,BugFixVersion)

#define Quote(x) #x
#define XYZQuoteConCat(x,y,z) Quote(x) "." Quote(y) "." Quote(z)
#define VersionString XYZQuoteConCat(MajorVersion,MinorVersion,BugFixVersion)

#ifdef VERSION_STRING_NOW
#include <stdio.h>
int main () { printf(VersionString); return 0; }
#endif
