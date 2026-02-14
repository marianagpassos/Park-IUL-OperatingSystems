#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. so_utils.sh                       ## This is required to activate the macros so_success, so_error, and so_debug

#####################################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2024/2025, Enunciado Versão 1
##
## Aluno: Nº:122660       Nome:Mariana Passos
## Nome do Módulo: S2. Script: manutencao.sh
## Descrição/Explicação do Módulo:
##
##
#####################################################################################

## Este script não recebe nenhum argumento, e permite realizar a manutenção dos registos de estacionamento. 

## S2.1. Validações do script:
## O script valida se, no ficheiro estacionamentos.txt:
## • Todos os registos referem códigos de países existentes no ficheiro paises.txt;
## • Todas as matrículas registadas correspondem à especificação de formato dos países correspondentes;
## • Todos os registos têm uma data de saída superior à data de entrada;
## • Em caso de qualquer erro das condições anteriores, dá so_error S2.1 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S2.1.


# Ficheiros de entrada
ESTACIONAMENTOS="estacionamentos.txt"
PAISES="paises.txt"

# Verifica se os ficheiros existem
if [[ ! -f "$ESTACIONAMENTOS" ]]; then
    so_success S2.1
    so_success S2.2
    exit 0  # Garante que o script termine sem erro se o arquivo não existir
fi

if [[ ! -f "$PAISES" ]]; then
    so_error S2.1 "Arquivo paises.txt não encontrado"
    exit 1
fi

# Validação dos registos
while IFS=: read -r matricula pais categoria nome_condutor data_entrada data_saida; do
# Validação do país
    if ! grep -q "^$pais###" "$PAISES"; then
        so_error S2.1 "Código país inválido: $pais"
        exit 1
    fi

    # Obtém a regra de validação da matrícula do ficheiro paises.txt
    regra=$(awk -F'###' -v pais="$pais" '$1 == pais {print $3}' "$PAISES")
    if [[ -z "$regra" ]]; then
        so_error S2.1 "Regra de validação de matrícula não encontrada para o país: $pais"
        exit 1
    fi

    # Validação da matrícula
    if ! [[ "$matricula" =~ $regra ]]; then
        so_error S2.1 "Matrícula inválida para o país $pais: $matricula"
        exit 1
    fi

    # Validação das datas (apenas se a data de saída não estiver vazia)
    if [[ -n "$data_saida" ]]; then
        # Corrige o formato removendo 'T' e 'h'
        data_entrada_formatada=$(echo "$data_entrada" | sed -E 's/T/ /; s/h/:/')
        data_saida_formatada=$(echo "$data_saida" | sed -E 's/T/ /; s/h/:/')

        # Converte as datas para segundos
        entrada_segundos=$(date -d "$data_entrada_formatada" +%s 2>/dev/null)
        saida_segundos=$(date -d "$data_saida_formatada" +%s 2>/dev/null)

        # Verifica erro na conversão de datas
        if [[ -z "$entrada_segundos" || -z "$saida_segundos" ]]; then
            so_error S2.1 "Erro ao converter datas: $data_entrada -> $data_entrada_formatada, $data_saida -> $data_saida_formatada"
            exit 1
        fi

        if [[ "$entrada_segundos" -ge "$saida_segundos" ]]; then
            so_error S2.1 "Data de saída menor ou igual à data de entrada: $data_entrada >= $data_saida"
            exit 1
        fi
    fi

done < "$ESTACIONAMENTOS"

so_success S2.1

## S2.2. Processamento:

## • O script move, do ficheiro estacionamentos.txt, todos os registos que estejam completos (com registo de entrada e registo de saída), mantendo o formato do ficheiro original, para ficheiros separados com o nome arquivo-<Ano>-<Mês>.park, com todos os registos agrupados pelo ano e mês indicados pelo nome do ficheiro. Ou seja, os registos são removidos do ficheiro estacionamentos.txt e acrescentados ao correspondente ficheiro arquivo-<Ano>-<Mês>.park, sendo que o ano e mês em questão são os do campo <DataSaída>.
## • Quando acrescentar o registo ao ficheiro arquivo-<Ano>-<Mês>.park, este script acrescenta um campo <TempoParkMinutos> no final do registo, que corresponde ao tempo, em minutos, que durou esse registo de estacionamento (correspondente à diferença em minutos entre os dois campos anteriores).
## • Em caso de qualquer erro das condições anteriores, dá so_error S2.2 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S2.2.
## • O registo em cada ficheiro arquivo-<Ano>-<Mês>.park, tem então o formato:
## <Matrícula:string>:<Código País:string>:<Categoria:char>:<Nome do Condutor:string>: <DataEntrada:AAAA-MM-DDTHHhmm>:<DataSaída:AAAA-MM-DDTHHhmm>:<TempoParkMinutos:int>
## • Exemplo de um ficheiro arquivo-<Ano>-<Mês>.park, para janeiro de 2025:



# Verifica se o arquivo é gravável
if [[ ! -w "." ]]; then # Verifica permissões de escrita na diretoria atual
    so_error S2.2 "Diretoria local sem permissões de escrita"
    exit 1
fi

# Processamento dos registos
while IFS=: read -r matricula pais categoria nome_condutor data_entrada data_saida; do

    # Verifica se o registo está completo (tem data de saída)
    if [[ -n "$data_saida" ]]; then
        # Extrai o ano e mês da data de saída
        ano_mes=$(echo "$data_saida" | cut -c 1-7)

        # Cria o nome do ficheiro de destino
        ficheiro_destino="arquivo-${ano_mes}.park"

        # Corrige o formato das datas removendo 'T' e 'h'
        data_entrada_formatada=$(echo "$data_entrada" | sed -E 's/T/ /; s/h/:/')
        data_saida_formatada=$(echo "$data_saida" | sed -E 's/T/ /; s/h/:/')

        # Calcula a diferença em minutos entre as datas de entrada e saída
        entrada_segundos=$(date -d "$data_entrada_formatada" +%s 2>/dev/null)
        saida_segundos=$(date -d "$data_saida_formatada" +%s 2>/dev/null)

        if [[ -z "$entrada_segundos" || -z "$saida_segundos" ]]; then
            so_error S2.2 "Formato de data inválido: $data_entrada ou $data_saida"
            exit 1
        fi

        diferenca_minutos=$(( (saida_segundos - entrada_segundos) / 60 ))

        # Adiciona o registo ao ficheiro de destino com a diferença em minutos
        echo "$matricula:$pais:$categoria:$nome_condutor:$data_entrada:$data_saida:$diferenca_minutos" >> "$ficheiro_destino"

        # Remove o registo do ficheiro estacionamentos.txt
        sed -i "/^$matricula:$pais:$categoria:$nome_condutor:$data_entrada:$data_saida/d" "estacionamentos.txt"
    fi
done < "estacionamentos.txt"

# Se todas as validações passaram
so_success S2.2