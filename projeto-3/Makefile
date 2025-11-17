# filepath: /home/lhslobato/projetos-pcp/projeto-3/Makefile

CUDA_PATH ?= /usr/local/cuda
NVCC := /usr/bin/nvcc
CC := gcc

NVCCFLAGS := -O3 -Xcompiler -fopenmp
CFLAGS := -O3 -fopenmp
INCLUDES := -I$(CUDA_PATH)/include
# LDFLAGS inclui bibliotecas CUDA e bibliotecas comuns
LDFLAGS := -L$(CUDA_PATH)/lib64 -lcudart -lcurand -lm -lpthread -lgomp

SRC_CU := src/gpu.cu
SRC_C := src/utils.c src/cpu.c # Adicionei src/cpu.c aqui
OBJ_GPU := $(patsubst %.cu,%.o,$(SRC_CU)) $(patsubst %.c,%.o,$(filter-out src/cpu.c,$(SRC_C))) # gpu.o e utils.o
OBJ_CPU := $(patsubst %.c,%.o,$(filter-out src/gpu.cu,$(SRC_C))) # cpu.o e utils.o

TARGET_GPU := gpu
TARGET_CPU := cpu

.PHONY: all clean

all: $(TARGET_GPU) $(TARGET_CPU)

# Regra para o alvo GPU
$(TARGET_GPU): $(OBJ_GPU)
	$(NVCC) $(INCLUDES) $^ -o $@ $(LDFLAGS) \
	-Xcompiler "-fopenmp"

# Regra para o alvo CPU
$(TARGET_CPU): $(OBJ_CPU)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Regra explícita para compilar arquivos .cu em .o
%.o: %.cu
	$(NVCC) $(NVCCFLAGS) $(INCLUDES) -c $< -o $@

# Regra explícita para compilar arquivos .c em .o
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ_GPU) $(OBJ_CPU) $(TARGET_GPU) $(TARGET_CPU)