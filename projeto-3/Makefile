# ...existing code...
# all:
# 	gcc -o cuda src/cuda.c src/utils.c -lm -lpthread -fopenmp -lcudart -L/usr/local/cuda/lib64 -I/usr/local/cuda/include

CUDA_PATH ?= /usr/local/cuda
NVCC := /usr/bin/nvcc
CC := gcc

NVCCFLAGS := -O3 -Xcompiler -fopenmp
CFLAGS := -O3 -fopenmp
INCLUDES := -I$(CUDA_PATH)/include
LDFLAGS := -L$(CUDA_PATH)/lib64 -lcudart -lm

SRC_CU := src/contamination.cu
SRC_C := src/utils.c
OBJ := $(patsubst %.cu,%.o,$(SRC_CU)) $(patsubst %.c,%.o,$(SRC_C))

TARGET := contamination

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	 $(NVCC) $(INCLUDES) $(OBJ) -o $@ -L$(CUDA_PATH)/lib64 -lcudart -lm \
	 -Xcompiler "-fopenmp" \
	 -Xlinker -lgomp \
	 -Xlinker -lpthread

# Garanta que a linha a seguir comece com um TAB
%.o: %.cu
	 $(NVCC) $(NVCCFLAGS) $(INCLUDES) -c $< -o $@

# Garanta que a linha a seguir comece com um TAB
%.o: %.c
	 $(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	 rm -f $(OBJ) $(TARGET)
```// filepath: /home/lhslobato/projetos-pcp/projeto-3/Makefile