#define MajorVersion  1
#define MinorVersion  0
#define BugFixVersion 2

#define Cat2(x,y) x##y
#define Cat(x,y) Cat2(x,y)
#define MajorVersionBCD Cat(0x,MajorVersion)
#define MinorVersionBCD Cat(0x,MinorVersion)
#define BugFixVersionBCD Cat(0x,BugFixVersion)

#define Quote(x) #x
#define XYZQuoteConCat(x,y,z) Quote(x) "." Quote(y) "." Quote(z)
#define VersionString XYZQuoteConCat(MajorVersion,MinorVersion,BugFixVersion)

#ifdef VERSION_STRING_NOW
main () { printf(VersionString); }
#endif
