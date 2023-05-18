ctxvS

f 
    The argument following the ’f’ option specifies the name of the archive file to use. 

-c Archive Creation
    - creates a new archive. 
    - If the archive file exists, it is truncated to zero length,
    then all the remaining arguments on the command line are taken as paths to be added to the archive.
    - If a given path is a directory, that directory and all the files and directories below it are added
    to the archive.

-t Archive Listing
    - In list (Table of contents) mode, mytar lists the contents of the given archive file, in order, one per line.
    - If no names are given on the command line, mytar, lists all the files in the archive. 
    - If a name or names are given on the command line, mytar will list the given path and any and all descendents
    of it. 

-x Archive Extraction
    - extracts files from a new archive
    - If no names are given on the command line, mytar, extracts all the files in the archive.
    - If a name or names are given on the command line,
    mytar will extract the given path and any and all descendents of it just like listing.

-S Strict
    - This option forces mytar to be strict in its interpretation of the standard. That is, it requires the
    magic number to be nul-terminated and checks for the version number.
    - Without this option, mytar only checks for the five characters of “ustar” in the magic number
    and ignores the version field. This is required to interoperate with GNU’s tar.

-v 
    -c If the verbose (’v’) option is set, mytar lists files as they are added, one per line.
    -t If the verbose (’v’) option is set, mytar gives expanded information about each file as it lists them
    -x If the verbose (’v’) option is set, mytar lists files as they are extracted, one per line.



