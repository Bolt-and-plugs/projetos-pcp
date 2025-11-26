
for i in {1..3}; do
    # casos para input_small.dat
    ./cpu assets/input_small.dat # caso teste 1. apenas CPU
    ./gpu assets/input_small.dat 1 1 # caso teste 2. GPU com 1 kernel e 1 bloco
    ./gpu assets/input_small.dat 1024 1 # caso teste 3. GPU com n kernel em 1 bloco
    ./gpu assets/input_small.dat 512 2 # caso teste 4. GPU com n kernel em 2 blocos
    ./gpu assets/input_small.dat 256 4 # caso teste 5. GPU com n kernel em 4 blocos
    ./gpu assets/input_small.dat 128 8  # caso teste 6. GPU com n kernel em 8 blocos
    ./gpu assets/input_small.dat 32 32 # caso teste 7. GPU com n kernel em n blocos
    ./gpu assets/input_small.dat 1024 16 # caso teste 8. GPU com n kernel em m blocos (n/m kernels por bloco)

    # casos para realdata.dat
    ./cpu assets/realdata.dat # caso teste 1. apenas CPU
    ./gpu assets/realdata.dat 1 1 # caso teste 2. GPU com 1 kernel e 1 bloco -> precisa de shared memory
    ./gpu assets/realdata.dat 1024 1 # caso teste 3. GPU com n kernel em 1 bloco -> precisa de shared memory
    ./gpu assets/realdata.dat 1024 2 # caso teste 4. GPU com n kernel em 2 blocos -> precisa de shared memory
    ./gpu assets/realdata.dat 1024 4 # caso teste 5. GPU com n kernel em 4 blocos -> precisa de shared memory
    ./gpu assets/realdata.dat 1024 8 # caso teste 6. GPU com n kernel em 8 blocos -> precisa de shared memory
    ./gpu assets/realdata.dat 1024 1024 # caso teste 7. GPU com n kernel em n blocos
    ./gpu assets/realdata.dat 1048576 2048  # caso teste 8. GPU com n kernel em m blocos (n/m kernels por bloco)
done