
# compilando o projeto
```sh 
make
```

e execute com o comando:
```sh
Usage:
./mpi
```

Loop principal:
- A cada iteração temporal uma pessoa saudável será contaminada se tiver uma pessoa contaminada ou morta em sua vizinhança (horizontal ou vertical).
- A cada iteração uma pessoa contaminada pode ser curada, com probabilidade 0,1, continuar doente, com probabilidade 0,3, ou morrer, com probabilidade 0,6.
- Uma pessoa morta permanece como contaminante por mais uma iteração, antes de desaparecer do mapa da região.

## Testes
1. apenas um processo sequencial
2. apenas um processo MPI
3. dois processos MPI
4. quatro processos MPI
5. oito processos MPI

# ToDo 

- [ ] implementar a lógica em mpi e sequencial de espalhamento da doença, considerando a troca de bordas entre processos
- [ ] implementar um sorteio de 0 a 9999 (rand() % 10000) para decidir o destino da pessoa contaminada
    - 0 - 999 - curada 
    - 1000 - 3999 - continua doente
    - 4000 - 9999 - morre
- [ ] implementar a remoção da pessoa morta após uma iteração
- [ ] medir o tempo de execução de cada versão e comparar os tempos de execução
