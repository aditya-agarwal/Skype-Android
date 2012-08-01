# Directories.
SRC= .

# Tools.
CC= g++
LD= g++
AR= ar
RM= rm -f

# Flags.
CCFLAGS= -Os -Wall -g-

# Code files.
BMCRYPTO_LIB_OBJS=  sha1.o AttributeContainer.o signature-padding.o Crc.o tc_crypto.o


bmcrypto.a: $(BMCRYPTO_LIB_OBJS)
	$(AR) rc $@ $(BMCRYPTO_LIB_OBJS:%.o=$(SRC)/%.o)

%.o : %.cpp $(INCLUDES)
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c $< -o $@

clean: 
	$(RM) *.o
	$(RM) *~
	
bare: clean
	$(RM) bmcrypto.a

