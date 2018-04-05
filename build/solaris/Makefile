##########################################################################
#
#  Usage:    make -f Makefile [switch]
#
#            switch:  build or clean
#
##########################################################################

CC      = /opt/S11/SUNWspro/bin/CC
LDFLAGS = -lrt -ldl
CFLAGS  = -xarch=v9 -KPIC -O -DCE_HHIGH_ORDER_FIRST
ADABAS  = $(ACLDIR)/$(ACLVERS)
IPATH   = $(ADABAS)/inc
LPATH   = $(ADABAS)/lib

cmd = $(CC) $(CFLAGS) -I$(IPATH)  $? \
                -L $(SAG)/common/lib -L $(LPATH)

CPP = ADC.cpp ADCSession.cpp ADCMainTest.cpp
OBJ = ADC.o ADCSession.o ADCMainTest.o

build:          $(CPP)
                $(cmd) -ladalnkx -o ADCMainTest $(LDFLAGS)
                @ rm $(OBJ)
                @ echo "ADCMainTest Created"

clean:
                @ rm -f $(OBJ)
                @ rm -f ADCMainTest
