# http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/
# https://stackoverflow.com/questions/1814270/gcc-g-option-to-place-all-object-files-into-separate-directory
# https://www.gnu.org/software/make/manual/make.html
# https://www.rapidtables.com/code/linux/gcc/gcc-o.html#optimization
#
# You cannot use comments within variable references or function calls: any instance of # will be treated 
# literally (rather than as the start of a comment) inside a variable reference or function call. 

# Folder in which only header files are located
IDIR = include

# Folder in which object files are going to be located
ODIR := build

# Folder in which libraries are stored
LDIR := lib

.DEFAULT_GOAL := $(ODIR)/um34c

# Compiler to use
CC=gcc
# Compiler flags
CFLAGS=-I$(IDIR) -I$(LDIR) -I./ -O0 -g3
# Linker flags (Search the bluetooth named library when linking)
LDFLAGS = -lbluetooth -lpthread -lncurses

# All .h files located in 'IDIR' and in root of this project are stored in 'DEPS' variable (include/dataTypes.h main.h)
DEPS = $(wildcard $(IDIR)/*.h) $(wildcard *.h)

# All .o files (lib/customPrint.o main.o)
_OBJ := $(patsubst %.c,%.o,$(wildcard $(LDIR)/*.c)) $(patsubst %.c,%.o,$(wildcard *.c))

# All .o files in 'ODIR' (build/lib/customPrint.o build/main.o)
OBJ := $(addprefix $(ODIR)/, $(_OBJ))

# Make .o files from .c files located in 'lib' folder (also create 'ODIR' and 'ODIR'/'LDIR' folder if it does not exists)
$(ODIR)/$(LDIR)/%.o: $(LDIR)/%.c $(DEPS) | $(ODIR) $(ODIR)/$(LDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

# Make .o files from .c files located in this folder (also create 'ODIR' folder if it does not exists)
$(ODIR)/%.o: %.c $(DEPS) | $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

# Create executable
$(ODIR)/um34c: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)



$(ODIR):
	mkdir $(ODIR)

$(ODIR)/$(LDIR):
	mkdir $(ODIR)/$(LDIR)

.PHONY: clean
clean:
	rm -r -f $(ODIR)