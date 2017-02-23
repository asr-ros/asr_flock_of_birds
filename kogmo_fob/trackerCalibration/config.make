# config.make (this is -*- Makefile -*-)
# 
# \author:  <dobre@ira.uka.de>
# 
# This is a skeleton for a configuration makefile. This file will be
# included if it exists and can be used to set some flags for the
# project.
#
# There needs to be only one toplevel config.make for each project,
# but you can provide one for each subproject as well; the makefile
# rules will read $(TOPDIR)/config.make first and then the local
# config.make.
# 
# $Author: knoop $
# $Locker:  $
# $Revision: 1.1.1.1 $
# $Date: 2005/03/01 15:39:25 $

#
# Flags for compiler and linker:
#         "How exactly do I want the program to be built?
#

# MY_CCFLAGS: Additional arguments for the compiler.
MY_CCFLAGS=

# MY_LDFLAGS: Additional arguments for the linker.
MY_LDFLAGS=

#
# Include / Library specifications:
#         "What things other people have written go into the program?"
#

# MY_INCPATHS: Contains paths to additional include files.
MY_INCPATHS=../gloveServer /usr/include/qt3\
		$(KOGMO_RTDB_HOME)/include \
		$(KOGMO_OBJECTS)


# MY_LIBPATHS: Contains paths to additional libraries.
MY_LIBPATHS= $(KOGMO_RTDB_HOME)/lib

# MY_IDLPATHS: Contains paths to additional IDL files.
MY_IDLPATHS=

# MY_LIBS: Contains additional libraries the program is to be linked with.
MY_LIBS= kogmo_rtdb


# MY_TESTINCPATHS: Additional include paths for single-module tests.
MY_TESTINCPATHS=

# MY_TESTLIBPATHS: Additional library paths for single-module tests.
MY_TESTLIBPATHS=

# MY_TESTINCPATHS: Additional libraries for single-module tests.
MY_TESTLIBS=

# VERBOSE: Set this to "yes" to get more information about the build
# process (compiler and linker arguments, etc.)
VERBOSE=yes

# DEBUG: Set this to "no" to prevent generation of debugging
# info. Saves a bit in disk space.
DEBUG=yes

# AUTODOC: Set this to "no" to prevent automatical documentation
# generation. Saves a bit in compile time.
AUTODOC=yes

# ESSENTIAL: Set this to "no" to continue compiling the parent project
# even if something in this subproject generated an error.
ESSENTIAL=yes

# USECORBA: Set this to "yes" if you don't have any IDL files but
# want to use CORBA anyway. If you do have IDL files, don't worry
# about this; having IDL files overrides this setting.
USECORBA=no

# USEQT: Set this to "yes" if you don't have any .moc or .ui files but
# want to use Qt anyway. If you do have .moc or .ui files, don't worry
# about this; having such files overrides this setting.
USEQT=yes

