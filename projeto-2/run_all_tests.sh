#!/bin/bash

# isto aqui vai rodar todos os testes automaticamente

# --- Configurações ---
HOSTNAME="localhost"
IMAGE_FILE="assets/imagem.txt"     # Coloque o nome do seu arquivo de imagem de entrada [cite: 13]
SERVER_EXEC="./server"
CLIENT_EXEC="./client"
NUM_EXECUCOES=3             # Média de 3 execuções por teste 
LOG_FILE="resultados_testes.log"

# --- Limpa o log anterior ---
echo "Iniciando suíte de testes - $(date)" > $LOG_FILE
echo "-----------------------------------" >> $LOG_FILE

# --- Funções Auxiliares ---
# Retorna o número de clientes para um dado caso de teste
get_num_clientes() {
    TEST_CASE=$1
    case $TEST_CASE in
        1|2|3|4|5)  echo 1 ;;     # Testes 1-5: 1 cliente
        6|7)        echo 2 ;;     # Testes 6-7: 2 clientes
        8|9|10)     echo 4 ;;     # Testes 8-10: 4 clientes
        11|12|13|14) echo 8 ;;    # Testes 11-14: 8 clientes
        *)          echo "0" ;;
    esac
}

# Retorna as dimensões do bloco (x y) para um dado caso de teste
# Ajuste estes valores conforme a sua configuração de imagem/experimento.
get_block_dims() {
    TEST_CASE=$1
    case $TEST_CASE in
        1|2|3|4|5)   echo "200 200" ;;  # blocos grandes (1 cliente)
        6|7)         echo "200 100" ;;  # dois clientes
        8|9|10)      echo "100 100" ;;  # quatro clientes
        11|12|13|14) echo "50 50" ;;    # muitos clientes, blocos menores
        *)           echo "100 100" ;;
    esac
}

# --- Verificações ---
if [ ! -f "$SERVER_EXEC" ] || [ ! -f "$CLIENT_EXEC" ]; then
    echo "Erro: Executáveis 'server' ou 'client' não encontrados."
    echo "Por favor, compile o projeto com 'make'."
    exit 1
fi
if [ ! -f "$IMAGE_FILE" ]; then
    echo "Erro: Arquivo de imagem '$IMAGE_FILE' não encontrado."
    exit 1
fi

# --- Loop Principal ---
# Itera por todos os 14 casos de teste [cite: 19-33]
for ((test_id=1; test_id<=14; test_id++)); do
    
    echo "--- INICIANDO CASO DE TESTE $test_id ---"
    NUM_CLIENTES=$(get_num_clientes $test_id)
    
    if [ "$NUM_CLIENTES" -eq "0" ]; then
        echo "ID de teste inválido: $test_id"
        continue
    fi

    echo "Caso $test_id: $NUM_CLIENTES cliente(s)"
    echo "--- Caso de Teste $test_id ($NUM_CLIENTES cliente(s)) ---" >> $LOG_FILE

    # obtém dimensões do bloco para este caso de teste
    read X Y <<< $(get_block_dims $test_id)
    echo "  Usando dimensões de bloco: X=$X Y=$Y"
    echo "  Usando dimensões de bloco: X=$X Y=$Y" >> $LOG_FILE

    # Itera 3 vezes para a média 
    for ((exec_num=1; exec_num<=$NUM_EXECUCOES; exec_num++)); do
        echo "  Iniciando execução $exec_num/$NUM_EXECUCOES..."
        
    # 1. Inicia o servidor em background
    # Passamos: 1) caminho do arquivo 2) número de clientes 3) X 4) Y
    $SERVER_EXEC $IMAGE_FILE $NUM_CLIENTES $X $Y &
        SERVER_PID=$! # Salva o ID do processo (PID) do servidor

        # Pequena pausa para garantir que o servidor está pronto (bind/listen)
        sleep 1

        # 2. Inicia os N clientes em background
        for ((c=0; c<$NUM_CLIENTES; c++)); do
            # O cliente agora recebe apenas X e Y (tamanhos do bloco)
            $CLIENT_EXEC $X $Y &
        done

        # 3. Espera o servidor terminar
        # (Seu servidor deve ser programado para fechar após
        # processar a imagem inteira para o teste atual)
        wait $SERVER_PID
        
        echo "  Execução $exec_num concluída."
        
        # Você deve modificar seu servidor para escrever o tempo
        # no arquivo de log $LOG_FILE, como pedido no ToDo (item 3)
        
        sleep 1 # Pausa entre as execuções para tudo "assentar"
    done
    
    echo "--- CASO DE TESTE $test_id CONCLUÍDO ---"
    echo "-----------------------------------" >> $LOG_FILE
done

echo "Suíte de testes finalizada. Resultados em $LOG_FILE"
