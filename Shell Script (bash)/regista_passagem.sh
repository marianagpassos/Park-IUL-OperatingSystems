#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. so_utils.sh                       ## This is required to activate the macros so_success, so_error, and so_debug

#####################################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2024/2025, Enunciado Versão 1
##
## Aluno: Nº: 122660      Nome:Mariana Passos
## Nome do Módulo: S1. Script: regista_passagem.sh
## Descrição/Explicação do Módulo:
##
##
#####################################################################################

## Este script é invocado quando uma viatura entra/sai do estacionamento Park-IUL. Este script recebe todos os dados por argumento, na chamada da linha de comandos, incluindo os <Matrícula:string>, <Código País:string>, <Categoria:char> e <Nome do Condutor:string>.

## S1.1. Valida os argumentos passados e os seus formatos:
## • Valida se os argumentos passados são em número suficiente (para os dois casos exemplificados), assim como se a formatação de cada argumento corresponde à especificação indicada. O argumento <Categoria> pode ter valores: L (correspondente a Ligeiros), P (correspondente a Pesados) ou M (correspondente a Motociclos);
## • A partir da indicação do argumento <Código País>, valida se o argumento <Matrícula> passada cumpre a especificação da correspondente <Regra Validação Matrícula>;
## • Valida se o argumento <Nome do Condutor> é o “primeiro + último” nomes de um utilizador atual do Tigre;
## • Em caso de qualquer erro das condições anteriores, dá so_error S1.1 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S1.1.


# Verifica se o arquivo paises.txt existe
if [[ ! -e "paises.txt" ]]; then
    so_error S1.1 "Arquivo paises.txt não encontrado"
    exit 1
fi

# Valida o número de argumentos
if [[ $# -eq 0 ]]; then
    so_error "S1.1" "Nenhum argumento fornecido"
    exit 1
elif [[ $# -ne 1 && $# -ne 4 ]]; then
    so_error "S1.1" "Número de argumentos inválido"
    exit 1
fi

# Caso de Entrada no Parque de Estacionamento
if [[ $# -eq 4 ]]; then
    # Atribui os argumentos a variáveis
    matricula=$1
    codigo_pais=$2
    categoria=$3
    nome_condutor=$4

    # Valida a categoria
    if [[ ! "$categoria" =~ ^[LPM]$ ]]; then
        so_error S1.1 "Categoria inválida"
        exit 1
    fi

    # Valida a matrícula com a regra do país
    regra=$(awk -F'###' -v pais="$codigo_pais" '$1 == pais {print $3}' paises.txt)
    if [[ -z "$regra" ]]; then
        so_error S1.1 "Código País não existe em paises.txt"
        exit 1
    fi

    # Valida a matrícula com a expressão regular do país
    if ! [[ "$matricula" =~ $regra ]]; then
        so_error S1.1 "Formato Matrícula não corresponde à RegExp do Código País"
        exit 1
    fi

    # Valida o nome do condutor
    if [[ ! "$nome_condutor" =~ ^[A-Za-z]+\ [A-Za-z]+$ ]]; then
        so_error S1.1 "Nome do condutor inválido. Deve ser 'Primeiro Último'."
        exit 1
    fi

    # Separar primeiro e último nome
    PrimeiroNome=$(echo "$nome_condutor" | cut -f1 -d" ") 
    UltimoNome=$(echo "$nome_condutor" | awk '{print $NF}')
    NomeCompleto="$PrimeiroNome $UltimoNome"

    # Obtém os nomes cadastrados no sistema
    Nomes=$(cut -d: -f5 /etc/passwd | cut -d, -f1 | awk '{print $1, $NF}')

    # Verifica se o nome do condutor existe no sistema
    if ! echo "$Nomes" | grep -q "^$NomeCompleto$"; then
        so_error S1.1 "Nome do condutor não existe no sistema: $NomeCompleto"
        exit 1
    fi
fi


# Caso de Saída do Parque de Estacionamento
if [[ $# -eq 1 ]]; then
    # Atribui os argumentos a variáveis
    codigo_pais=$(echo "$1" | cut -d '/' -f1)
    matricula=$(echo "$1" | cut -d '/' -f2-)

    # Valida se o código do país e a matrícula foram fornecidos
    if [[ -z "$codigo_pais" || -z "$matricula" ]]; then
        so_error S1.1 "Formato inválido. Use 'Código_País/Matrícula'."
        exit 1
    fi

    # Valida a matrícula com a regra do país
    regra=$(awk -F'###' -v pais="$codigo_pais" '$1 == pais {print $3}' paises.txt)
    if [[ -z "$regra" ]]; then
        so_error S1.1 "Código País não existe em paises.txt"
        exit 1
    fi

    # Valida a matrícula com a expressão regular do país
    if ! [[ "$matricula" =~ $regra ]]; then
        so_error S1.1 "Formato Matrícula não corresponde à RegExp do Código País"
        exit 1
    fi
fi
so_success S1.1

## S1.2. Valida os dados passados por argumento para o script com o estado da base de dados de estacionamentos especificada no ficheiro estacionamentos.txt:
## • Valida se, no caso de a invocação do script corresponder a uma entrada no parque de estacionamento, se ainda não existe nenhum registo desta viatura na base de dados;
## • Valida se, no caso de a invocação do script corresponder a uma saída do parque de estacionamento, se existe um registo desta viatura na base de dados;
## • Em caso de qualquer erro das condições anteriores, dá so_error S1.2 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S1.2.
# Verifica se o arquivo estacionamentos.txt existe
# Verifica se o arquivo estacionamentos.txt existe

# Verifica se o arquivo estacionamentos.txt existe; se não, cria-o
if [[ ! -e "estacionamentos.txt" ]]; then
    touch estacionamentos.txt
fi

if [[ $# -eq 1 ]]; then
  # Saída do parque
  codigo_pais=$(echo "$1" | cut -d '/' -f1)
  matricula=$(echo "$1" | cut -d '/' -f2 | tr -d ' ')
  matricula_normalizada=$(echo "$matricula" | tr -d -c '[:alnum:]')
elif [[ $# -eq 4 ]]; then
  # Entrada no parque
  matricula_normalizada=$(echo "$matricula" | tr -d -c '[:alnum:]')
fi

# Obtém o registro mais recente da viatura
registro_viatura=$(grep "^$matricula_normalizada:" "estacionamentos.txt" | tail -n 1)

if [[ $# -eq 1 ]]; then
  # Saída do parque
  if [ -z "$registro_viatura" ]; then
    so_error S1.2 "A viatura com a matrícula $matricula_normalizada não está no estacionamento."
exit 1  
  else
    timestamp_saida=$(echo "$registro_viatura" | awk -F ':' '{print $6}')
    if [ -n "$timestamp_saida" ]; then
      so_error S1.2 "A viatura com a matrícula $matricula_normalizada já tem registo de saída."
    exit 1
    fi
  fi
elif [[ $# -eq 4 ]]; then
  # Entrada no parque
  if [ -n "$registro_viatura" ]; then
    timestamp_saida=$(echo "$registro_viatura" | awk -F ':' '{print $6}')
    if [ -z "$timestamp_saida" ]; then
      so_error S1.2 "A viatura com a matrícula $matricula_normalizada já está no estacionamento e não tem registo de saída."
        exit 1
    fi
  fi
fi

so_success S1.2



## S1.3. Atualiza a base de dados de estacionamentos especificada no ficheiro estacionamentos.txt:
## • Remova do argumento <Matrícula> passado todos os separadores (todos os caracteres que não sejam letras ou números) eventualmente especificados;
## • Especifique como data registada (de entrada ou de saída, conforme o caso) a data e hora do sistema Tigre;
## • No caso de um registo de entrada, crie um novo registo desta viatura na base de dados;
## • No caso de um registo de saída, atualize o registo desta viatura na base de dados, registando a data de saída;
## • Em caso de qualquer erro das condições anteriores, dá so_error S1.3 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S1.3.

# Remove separadores da matrícula
matricula_limpa=$(echo "$matricula" | sed 's/[^a-zA-Z0-9]//g')

# Verifica se o arquivo é gravável
if [[ ! -w "estacionamentos.txt" ]]; then
    so_error S1.3 "Arquivo estacionamentos.txt sem permissões de escrita"
    exit 1
fi

# Caso de Entrada no Parque de Estacionamento
if [[ $# -eq 4 ]]; then
    # Adiciona novo registro com a matrícula limpa e data/hora atual
    echo "$matricula_limpa:$codigo_pais:$categoria:$nome_condutor:$(date +'%Y-%m-%dT%Hh%M')" >> estacionamentos.txt
    so_success S1.3
fi

# Caso de Saída do Parque de Estacionamento
if [[ $# -eq 1 ]]; then
    # Atualiza o registro existente com a data de saída
    if grep -q "^$matricula_limpa:$codigo_pais:" estacionamentos.txt; then
        sed -i "s/^\($matricula_limpa:$codigo_pais:[^:]*:[^:]*:[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}T[0-9]\{2\}h[0-9]\{2\}\)$/\1:$(date +'%Y-%m-%dT%Hh%M')/" estacionamentos.txt
        so_success S1.3
    else
        so_error S1.3 "Registro de entrada não encontrado"
        exit 1
    fi
fi


## S1.4. Lista todos os estacionamentos registados, mas ordenados por saldo:
## • O script deve criar um ficheiro chamado estacionamentos-ordenados-hora.txt igual ao que está no ficheiro estacionamentos.txt, com a mesma formatação, mas com os registos ordenados por ordem crescente da hora (e não da data) de entrada das viaturas.
## • Em caso de qualquer erro das condições anteriores, dá so_error S1.4 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S1.4.

# Verifica se o arquivo estacionamentos.txt existe
if [[ ! -e "estacionamentos.txt" ]]; then
    so_error S1.4 "Arquivo estacionamentos.txt não encontrado"
    exit 1
fi

# Ordena os registos por hora de entrada e cria o ficheiro estacionamentos-ordenados-hora.txt
sort -t ':' -k 5.12,5.16 estacionamentos.txt > estacionamentos-ordenados-hora.txt

# Verifica se o ficheiro estacionamentos-ordenados-hora.txt foi criado
if [[ ! -e "estacionamentos-ordenados-hora.txt" ]]; then
    so_error S1.4 "Erro ao criar o ficheiro estacionamentos-ordenados-hora.txt"
    exit 1
fi

so_success S1.4