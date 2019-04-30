

### Defining tools and stuff ######
CXX		=	g++
CC		=	gcc
LINKER	=	$(CXX)

### Defining flags and stuff ######
CXXFLAGS	=	-pthread -m64 -W -Wall -fPIC -std=c++11 -g -O0#-O3 -mtune=native
CCFLAGS		=	-pthread -m64 -W -Wall -fPIC -g -O0#-O3 -mtune=native
LIBS		=	-L../lib -lsirius

### Includes ####
INCLUDE		=	-I./ -I../../include

TARGET = XIAsort

# Sources
CXX_SRC =	main.cpp \
			Event_builder.cpp \
			Unpacker.cpp \
			Sort_Funct.cpp \
			spectrum_rw.cpp

C_SRC	=	experimentsetup.c \
			XIA_CFD.c

C_OBJ = $(C_SRC:%.c=%.o)
CXX_OBJ = $(CXX_SRC:%.cpp=%.o)
OBJS = $(CXX_OBJ) $(C_OBJ)

### Rules ####

.c.o:
	$(CC) $(INCLUDE) $(CCFLAGS) -c $*.c


.cpp.o:
	$(CXX) $(INCLUDE) $(CXXFLAGS) -c $*.cpp


all: $(TARGET)


$(TARGET): $(OBJS)
	$(LINKER) -o $@ $^ $(LIBS)


FORCE: 

clean:
	rm -f *.o
	rm -f $(TARGET)