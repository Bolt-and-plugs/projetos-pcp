
# compilando o projeto
```sh 
make
```

e execute com o comando:
```sh
Usage:
./server
./client 127.0.0.1
```

# ToDo

- [X] Multiple conns -> criar threads no servidor ao receber conexoes do clientes.
- [ ] o client espera que o servidor envie uma fatia do buffer inicial e, ao receber essa fatia, ele envia de volta para o servidor uma media dos valores daquela fatia. O servidor, ao receber a media, o escreve no buffer compartilhado na posicao correspondente a fatia enviada e envia uma das N porções faltantes.
- [ ] a medição de tempo será feita no servidor, que iniciará a contagem ao enviar a primeira fatia e terminará ao solicitar a escrita da imagem completa no disco. Devemos salvar o tempo total por imagem num arquivo de log.
- [ ] é necessário uma forma de criar os CLI_NUM clientes simultaneamente, talvez usando um shell script (no momento, não consegui fazer com um sh pois a execução de um `./client` é bloqueante, ou seja, o shell espera o client retornar 0) 
