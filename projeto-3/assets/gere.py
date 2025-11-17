import random

LINHAS = 4096
COLUNAS = 3060
VALOR_MIN = -2
VALOR_MAX = 1
NOME_ARQUIVO = f"input-{LINHAS}-{COLUNAS}.txt"
# ---------------------

print(f"Iniciando a geração do arquivo '{NOME_ARQUIVO}'...")
print(f"Dimensões: {LINHAS}x{COLUNAS} | Valores: [{VALOR_MIN}, {VALOR_MAX}]")

try:
    with open(NOME_ARQUIVO, 'w') as f:
        f.write(f"{LINHAS} {COLUNAS}\n")

        for i in range(LINHAS):
            numeros_da_linha = [
                str(random.randint(VALOR_MIN, VALOR_MAX)) for _ in range(COLUNAS)
            ]
            # Junta todos os números da linha com um espaço entre eles
            # e adiciona uma quebra de linha no final
            f.write(" ".join(numeros_da_linha) + "\n")
            # Fornece um feedback de progresso a cada 100 linhas
            if (i + 1) % 100 == 0:
                print(f"  ... {i + 1} de {LINHAS} linhas escritas.")

    print(f"\nArquivo '{NOME_ARQUIVO}' gerado com sucesso!")

except IOError as e:
    print(f"Erro ao tentar escrever o arquivo: {e}")
except Exception as e:
    print(f"Ocorreu um erro inesperado: {e}")
