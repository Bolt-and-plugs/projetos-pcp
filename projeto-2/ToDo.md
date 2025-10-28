
- [ ] Multiple conns -> criar threads multiplas no servidor para receber multiplas conexoes do clientes. Cada uma das conns terá acesso a uma memoria compartilhada (um buffer de N posições) que, no entanto, tem uma separação clara entre as threads (cada thread só pode acessar a sua parte do buffer).
    - obs: lista circular: no momento, desnecessário, visto que os N clientes são conhecidos ao iniciar-se o servidor. No entando, numa aplicação real, isso seria estritamente necessário.

- [ ] o client espera que o servidor envie uma fatia do buffer inicial e, ao receber essa fatia, ele envia de volta para o servidor uma media dos valores daquela fatia. O servidor, ao receber a media, o escreve no buffer compartilhado na posicao correspondente a fatia enviada e envia uma das porções faltantes.
 
- [ ] a medição de tempo será feita no servidor, que iniciará a contagem ao enviar a primeira fatia e terminará ao solicitar a escrita da imagem completa no disco. Devemos salvar o tempo total por imagem num arquivo de log.
