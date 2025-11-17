# ...existing code...
# all:
# 	gcc -o cuda src/cuda.c src/utils.c -lm -lpthread -fopenmp -lcudart -L/usr/local/cuda/lib64 -I/usr/local/cuda/include

CUDA_PATH ?= /usr/local/cuda
NVCC := /usr/bin/nvcc
CC := gcc

NVCCFLAGS := -O3 -Xcompiler -fopenmp
CFLAGS := -O3 -fopenmp
INCLUDES := -I$(CUDA_PATH)/include
LDFLAGS := -L$(CUDA_PATH)/lib64 -lcudart -lcurand -lm

SRC_GPU_CU := src/gpu.cu
SRC_CPU_CU := src/cpu.cu
SRC_C := src/utils.c

OBJ_GPU := $(patsubst %.cu,%.o,$(SRC_GPU_CU)) $(patsubst %.c,%.o,$(SRC_C))
OBJ_CPU := $(patsubst %.cu,%.o,$(SRC_CPU_CU))

TARGET_GPU := gpu
TARGET_CPU := cpu

.PHONY: all clean

all: $(TARGET_GPU) $(TARGET_CPU)

$(TARGET_GPU): $(OBJ_GPU)
	 $(NVCC) $(INCLUDES) $(OBJ_GPU) -o $@ $(LDFLAGS) \
	 -Xcompiler "-fopenmp" \
	 -Xlinker -lgomp \
	 -Xlinker -lpthread

$(TARGET_CPU): $(OBJ_CPU)
	 $(NVCC) $(INCLUDES) $(OBJ_CPU) -o $@ $(LDFLAGS) \
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
	 rm -f $(OBJ_GPU) $(OBJ_CPU) $(TARGET_GPU) $(TARGET_CPU)
```// filepath: /home/lhslobato/projetos-pcp/projeto-3/Makefile