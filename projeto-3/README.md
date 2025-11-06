
# compilando o projeto
```sh 
make
```

e execute com o comando:
```sh
Usage:
./cuda
```

Loop principal:
- A cada iteração temporal uma pessoa saudável será contaminada se tiver uma pessoa contaminada ou morta em sua vizinhança (horizontal ou vertical).
- A cada iteração uma pessoa contaminada pode ser curada, com probabilidade 0,1, continuar doente, com probabilidade 0,3, ou morrer, com probabilidade 0,6.
- Uma pessoa morta permanece como contaminante por mais uma iteração, antes de desaparecer do mapa da região.

## Testes
1. apenas a cpu executando, sem uso de gpu
2. gpu executando um kernel em um bloco
3. gpu executando n kernels em um bloco
4. gpu executando n kernels em 2 blocos
5. gpu executando n kernels em 4 blocos
6. gpu executando n kernels em 8 blocos
7. gpu executando n kernels em n blocos (um kernel por bloco)
8. gpu executando n kernels em m blocos (n/m kernels por bloco)


# ToDo 

- [ ] implementar a lógica em cuda com gpu e cpu de espalhamento da doença
- [ ] implementar um sorteio de 0 a 9999 (rand() % 10000) para decidir o destino da pessoa contaminada
    - 0 - 999 - curada 
    - 1000 - 3999 - continua doente
    - 4000 - 9999 - morre
- [ ] implementar a remoção da pessoa morta após uma iteração
- [ ] medir o tempo de execução de cada versão e comparar os tempos de execução
