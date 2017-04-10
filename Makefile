# compiler to use
CC = c++
CFLAGS=-O3 -std=c++14
OFLAGS=-lpthread -fopenmp

LOCALPATH=/broad/IDP-Dx_work/nirmalya/local
INC = -I${LOCALPATH}/include

BOOSTLIBS=${LOCALPATH}/lib/libboost_serialization.a ${LOCALPATH}/lib/libboost_regex.a -lboost_iostreams -lz
PROG_OPT_LIB=${LOCALPATH}/lib/libboost_program_options.a
STXXLLIBS=${LOCALPATH}/lib/libstxxl.a

all: tools
    
tools:
	$(CC) $(CFLAGS) $(INC) concensus_mono.cpp -o concensus $(BOOSTLIBS) $(PROG_OPT_LIB)
	#$(CC) $(CFLAGS) $(INC) dict_builder.cpp -o dict_builder $(BOOSTLIBS) $(PROG_OPT_LIB)
	#$(CC) $(CFLAGS) $(INC) index_splitter.cpp -o index_splitter $(BOOSTLIBS) $(PROG_OPT_LIB)
	#$(CC) $(CFLAGS) $(INC) dict_builder_test.cpp -o dict_builder_test $(BOOSTLIBS) $(PROG_OPT_LIB)
	#$(CC) $(CFLAGS) $(INC) barcode_splitter.cpp -o bc_splitter $(BOOSTLIBS) $(PROG_OPT_LIB)
	#$(CC) $(CFLAGS) $(INC) barcode_splitter_rts.cpp -o bc_splitter_rts $(BOOSTLIBS) $(PROG_OPT_LIB)
	#$(CC) $(CFLAGS) $(INC) ConfigLoader.cpp -o ConfigLoader $(BOOSTLIBS) $(PROG_OPT_LIB)
	#$(CC) $(CFLAGS) $(INC) barcode_loader.cpp -o barcode_loader $(BOOSTLIBS) $(PROG_OPT_LIB)
