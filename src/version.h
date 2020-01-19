#ifndef VERSION_H
#define VERSION_H



#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_BUILD 1
#define VERSION_SUFFIX "Beta"

#define STRINGIFY_VERSION(A, B, C) CONCAT(A, B, C )
#define CONCAT(A, B, C ) STRINGIFY( A##.##B##.##C )
#define STRINGIFY(A) #A

#define STR_VERSION STRINGIFY_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD) "-" VERSION_SUFFIX

#define VER_FILEVERSION VERSION_MAJOR,VERSION_MINOR,VERSION_BUILD,0
#define STR_FILEVERSION STR_VERSION

// Keep the product version as fixed
#define VER_PRODUCTVERSION VERSION_MAJOR,VERSION_MINOR,VERSION_BUILD,0
#define STR_PRODUCTVERSION STR_VERSION

#define STR_COMPANYNAME "Rainbox Laboratory"
#define STR_FILEDESCRIPTION "DuME"
#define STR_INTERNALNAME "DuME"
#define STR_LEGALCOPYRIGHT "Copyright © 2019-2020 Rainbox Laboratory, Nicolas Dufresne and cotributors"
#define STR_LEGALTRADEMARKS1 "All Rights Reserved"
#define STR_ORIGINALFILENAME "DuME.exe"
#define STR_PRODUCTNAME "DuME - Duduf Media Encoder"

#define STR_COMPANYDOMAIN "rainboxlab.org"

#endif // VERSION_H