#define MajorVersion 1
#define MinorVersion 0
#define DevelVersion 0

#define Cat2(x,y) x##y
#define Cat(x,y) Cat2(x,y)
#define MajorVersionBCD Cat(0x,MajorVersion)
#define MinorVersionBCD Cat(0x,MinorVersion)
#define DevelVersionBCD Cat(0x,DevelVersion)

#define Quote(x) #x
#define XYZQuoteConCat(x,y,z) Quote(x) "." Quote(y) "." Quote(z)
#define VersionString XYZQuoteConCat(MajorVersion,MinorVersion,DevelVersion)

#ifdef VERSION_STRING_NOW
main () { printf(VersionString); }
#endif
