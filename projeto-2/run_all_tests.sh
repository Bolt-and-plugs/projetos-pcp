#!/bin/bash

# isto aqui vai rodar todos os testes automaticamente

# --- Configurações ---
HOSTNAME="localhost"
IMAGE_FILE="assets/imagem.txt"    # Coloque o nome do seu arquivo de imagem de entrada
SERVER_EXEC="./server"
CLIENT_EXEC="./client"
NUM_EXECUCOES=3                 # Média de 3 execuções por teste 

# --- Limpa o log anterior ---
echo "Iniciando suíte de testes - $(date)"
echo "-----------------------------------"

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
# ATUALIZADO para corresponder à sua lista de 14 casos.
get_block_dims() {
    TEST_CASE=$1
    case $TEST_CASE in
        1)  echo "2000 2000" ;;  # 1 cliente, imagem inteira
        2)  echo "2000 1000" ;;  # 1 cliente, 2 blocos de 2000x1000
        3)  echo "1000 2000" ;;  # 1 cliente, 2 blocos de 1000x2000
        4)  echo "1000 1000" ;;  # 1 cliente, 4 blocos de 1000x1000
        5)  echo "500 2000"  ;;  # 1 cliente, 4 blocos de 500x2000 (assumindo que eram 4)
        6)  echo "1000 2000" ;;  # 2 clientes, 2 blocos de 1000x2000
        7)  echo "2000 1000" ;;  # 2 clientes, 2 blocos de 2000x1000
        8)  echo "500 2000"  ;;  # 4 clientes, 4 blocos de 500x2000
        9)  echo "2000 500"  ;;  # 4 clientes, 4 blocos de 2000x500
	10) echo "1000 1000" ;;  # 4 clientes, 4 blocos de 1000x1000
        11) echo "2000 250"  ;;  # 8 clientes, 8 blocos de 2000x250
        12) echo "250 2000"  ;;  # 8 clientes, 8 blocos de 250x2000
        13) echo "1000 500"  ;;  # 8 clientes, 8 blocos de 1000x500
        14) echo "500 1000"  ;;  # 8 clientes, 8 blocos de 500x1000
        *)  echo "100 100" ;;  # Padrão
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
# Itera por todos os 14 casos de teste
for ((test_id=1; test_id<=14; test_id++)); do
    
    echo "--- INICIANDO CASO DE TESTE $test_id ---"
    NUM_CLIENTES=$(get_num_clientes $test_id)
    
    if [ "$NUM_CLIENTES" -eq "0" ]; then
        echo "ID de teste inválido: $test_id"
        continue
    fi

    echo "Caso $test_id: $NUM_CLIENTES cliente(s)"
    echo "--- Caso de Teste $test_id ($NUM_CLIENTES cliente(s)) ---"

    # obtém dimensões do bloco para este caso de teste
    read X Y <<< $(get_block_dims $test_id)
    echo "  Usando dimensões de bloco: X=$X Y=$Y"
    echo "  Usando dimensões de bloco: X=$X Y=$Y" 

    # Itera 3 vezes para a média 
    for ((exec_num=1; exec_num<=$NUM_EXECUCOES; exec_num++)); do
        echo "  Iniciando execução $exec_num/$NUM_EXECUCOES..."
        
        # 1. Inicia o servidor em background
        # Passamos: 1) caminho do arquivo 2) número de clientes 3) X 4) Y
        echo "  Executando: $SERVER_EXEC $IMAGE_FILE $NUM_CLIENTES $X $Y"
        $SERVER_EXEC $IMAGE_FILE $NUM_CLIENTES $X $Y &
        SERVER_PID=$! # Salva o ID do processo (PID) do servidor

        sleep 2

        for ((c=0; c<$NUM_CLIENTES; c++)); do
            echo "  Iniciando cliente $c: $CLIENT_EXEC $HOSTNAME $X $Y"
            $CLIENT_EXEC $HOSTNAME $X $Y &
        done

        wait $SERVER_PID
        
        echo "  Execução $exec_num concluída."
        
        sleep 2
    done
done
