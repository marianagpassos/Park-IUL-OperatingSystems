#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. so_utils.sh                       ## This is required to activate the macros so_success, so_error, and so_debug

#####################################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2024/2025, Enunciado Versão 1
##
## Aluno: Nº:122660       Nome:Mariana Passos
## Nome do Módulo: S3. Script: stats.sh
## Descrição/Explicação do Módulo:
##
##
#####################################################################################

## Este script obtém informações sobre o sistema Park-IUL, afixando os resultados das estatísticas pedidas no formato standard HTML no Standard Output e no ficheiro stats.html. Cada invocação deste script apaga e cria de novo o ficheiro stats.html, e poderá resultar em uma ou várias estatísticas a serem produzidas, todas elas deverão ser guardadas no mesmo ficheiro stats.html, pela ordem que foram especificadas pelos argumentos do script.

## S3.1. Validações:
## O script valida se, na diretoria atual, existe algum ficheiro com o nome arquivo-<Ano>-<Mês>.park, gerado pelo Script: manutencao.sh. Se não existirem ou não puderem ser lidos, dá so_error S3.1 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S3.1.
## S3.2. Estatísticas:
## Cada uma das estatísticas seguintes diz respeito à extração de informação dos ficheiros do sistema Park-IUL. Caso não haja informação suficiente para preencher a estatística, poderá apresentar uma lista vazia.
## S3.2.1.  Obter uma lista das matrículas e dos nomes de todos os condutores cujas viaturas estão ainda estacionadas no parque, ordenados alfabeticamente por nome de condutor:
## <h2>Stats1:</h2>
## <ul>
## <li><b>Matrícula:</b> <Matrícula> <b>Condutor:</b> <Nome do Condutor></li>
## <li><b>Matrícula:</b> <Matrícula> <b>Condutor:</b> <Nome do Condutor></li>
## ...
## </ul>
## S3.2.2. Obter uma lista do top3 das matrículas e do tempo estacionado das viaturas que já terminaram o estacionamento e passaram mais tempo estacionadas, ordenados decrescentemente pelo tempo de estacionamento (considere apenas os estacionamentos cujos tempos já foram calculados):
## <h2>Stats2:</h2>
## <ul>
## <li><b>Matrícula:</b> <Matrícula> <b>Tempo estacionado:</b> <TempoParkMinutos></li>
## <li><b>Matrícula:</b> <Matrícula> <b>Tempo estacionado:</b> <TempoParkMinutos></li>
## <li><b>Matrícula:</b> <Matrícula> <b>Tempo estacionado:</b> <TempoParkMinutos></li>
## </ul>
## S3.2.3. Obter as somas dos tempos de estacionamento das viaturas que não são motociclos, agrupadas pelo nome do país da matrícula (considere apenas os estacionamentos cujos tempos já foram calculados):
## <h2>Stats3:</h2>
## <ul>
## <li><b>País:</b> <Nome País> <b>Total tempo estacionado:</b> <ΣTempoParkMinutos></li>
## <li><b>País:</b> <Nome País> <b>Total tempo estacionado:</b> <ΣTempoParkMinutos></li>
## ...
## </ul>
## S3.2.4. Listar a matrícula, código de país e data de entrada dos 3 estacionamentos, já terminados ou não, que registaram uma entrada mais tarde (hora de entrada) no parque de estacionamento, ordenados crescentemente por hora de entrada:
## <h2>Stats4:</h2>
## <ul>
## <li><b>Matrícula:</b> <Matrícula> <b>País:</b> <Código País> <b>Data Entrada:</b> <DataEntrada></li>
## <li><b>Matrícula:</b> <Matrícula> <b>País:</b> <Código País> <b>Data Entrada:</b> <DataEntrada></li>
## <li><b>Matrícula:</b> <Matrícula> <b>País:</b> <Código País> <b>Data Entrada:</b> <DataEntrada></li>
## </ul>
## S3.2.5. Tendo em consideração que um utilizador poderá ter várias viaturas, determine o tempo total, medido em dias, horas e minutos gasto por cada utilizador da plataforma (ou seja, agrupe os minutos em dias e horas).
## <h2>Stats5:</h2>
## <ul>
## <li><b>Condutor:</b> <NomeCondutor> <b>Tempo  total:</b> <x> dia(s), <y> hora(s) e <z> minuto(s)</li>
## <li><b>Condutor:</b> <NomeCondutor> <b>Tempo  total:</b> <x> dia(s), <y> hora(s) e <z> minuto(s)</li>
## ...
## </ul>
## S3.2.6. Liste as matrículas das viaturas distintas e o tempo total de estacionamento de cada uma, agrupadas pelo nome do país com um totalizador de tempo de estacionamento por grupo, e totalizador de tempo global.
## <h2>Stats6:</h2>
## <ul>
## <li><b>País:</b> <Nome País></li>
## <ul>
## <li><b>Matrícula:</b> <Matrícula> <b> Total tempo estacionado:</b> <ΣTempoParkMinutos></li>
## <li><b>Matrícula:</b> <Matrícula> <b> Total tempo estacionado:</b> <ΣTempoParkMinutos></li>
## ...
## </ul>
## <li><b>País:</b> <Nome País></li>
## <ul>
## <li><b>Matrícula:</b> <Matrícula> <b> Total tempo estacionado:</b> <ΣTempoParkMinutos></li>
## <li><b>Matrícula:</b> <Matrícula> <b> Total tempo estacionado:</b> <ΣTempoParkMinutos></li>
## ...
## </ul>
## ...
## </ul>
## S3.2.7. Obter uma lista do top3 dos nomes mais compridos de condutores cujas viaturas já estiveram estacionadas no parque (ou que ainda estão estacionadas no parque), ordenados decrescentemente pelo tamanho do nome do condutor:
## <h2>Stats7:</h2>
## <ul>
## <li><b> Condutor:</b> <Nome do Condutor mais comprido></li>
## <li><b> Condutor:</b> <Nome do Condutor segundo mais comprido></li>
## <li><b> Condutor:</b> <Nome do Condutor terceiro mais comprido></li>
## </ul>
## S3.3. Processamento do script:
## S3.3.1. O script cria uma página em formato HTML, chamada stats.html, onde lista as várias estatísticas pedidas.
## O ficheiro stats.html tem o seguinte formato:
## <html><head><meta charset="UTF-8"><title>Park-IUL: Estatísticas de estacionamento</title></head>
## <body><h1>Lista atualizada em <Data Atual, formato AAAA-MM-DD> <Hora Atual, formato HH:MM:SS></h1>
## [html da estatística pedida]
## [html da estatística pedida]
## ...
## </body></html>
## Sempre que o script for chamado, deverá:
## • Criar o ficheiro stats.html.
## • Preencher, neste ficheiro, o cabeçalho, com as duas linhas HTML descritas acima, substituindo os campos pelos valores de data e hora pelos do sistema.
## • Ciclicamente, preencher cada uma das estatísticas pedidas, pela ordem pedida, com o HTML correspondente ao indicado na secção S3.2.
## • No final de todas as estatísticas preenchidas, terminar o ficheiro com a última linha “</body></html>”



ARQUIVO_HTML="stats.html"

# Função para verificar a existência e permissões dos arquivos (S3.1)
verificar_arquivos() {
    # Verifica se o arquivo paises.txt existe e pode ser lido
    if [[ ! -f "paises.txt" || ! -r "paises.txt" ]]; then
        so_error S3.1 "O arquivo paises.txt não pode ser lido"
        exit 1
    fi

    # Verifica se o arquivo estacionamentos.txt existe e pode ser lido
    if [[ ! -f "estacionamentos.txt" || ! -r "estacionamentos.txt" ]]; then
        so_error S3.1 "O arquivo estacionamentos.txt não pode ser lido"
        exit 1
    fi

    # Verifica se há arquivos com o padrão "arquivo-*.park"
    if ! compgen -G "arquivo-*.park" > /dev/null; then
        so_error S3.1 "Nenhum arquivo .park encontrado"
        exit 1
    fi

    # Verifica se todos os arquivos .park podem ser lidos
    for arquivo in arquivo-*.park; do
        if [[ ! -r "$arquivo" ]]; then
            so_error S3.1 "O arquivo $arquivo não pode ser lido"
            exit 1
        fi
    done

    so_success S3.1
}

# Função para gerar o cabeçalho do HTML (S3.3.1)
gerar_cabecalho_html() {
    cat <<EOF >"$ARQUIVO_HTML"
<html><head><meta charset="UTF-8"><title>Park-IUL: Estatísticas de estacionamento</title></head>
<body><h1>Lista atualizada em $(date +'%Y-%m-%d %H:%M:%S')</h1>
EOF
}

# --------------------- CÁLCULO DAS ESTATÍSTICAS ---------------------

# S3.2.1: Lista de veículos ainda estacionados
estatistica_1() {
   printf "<h2>Stats1:</h2>\n<ul>\n" >>"$ARQUIVO_HTML"
    
    awk -F: '!($6 ~ /T/) {print $1, $4}' estacionamentos.txt | sort -u -k1,1 | sort -t ' ' -k2,2 |
    while read -r matricula nome; do
        printf "<li><b>Matrícula:</b> %s <b>Condutor:</b> %s</li>\n" "$matricula" "$nome"
    done >>"$ARQUIVO_HTML"
    
    echo "</ul>" >>"$ARQUIVO_HTML"
}

# S3.2.2: Top 3 veículos com mais tempo de estacionamento
estatistica_2() {
    printf "<h2>Stats2:</h2>\n<ul>\n" >>"$ARQUIVO_HTML"

    awk -F: '{tempo[$1]+=$NF} END {for (mat in tempo) print tempo[mat], mat}' arquivo-*.park |
    sort -nr | head -3 |
    while read -r tempo matricula; do
        printf "<li><b>Matrícula:</b> %s <b>Tempo estacionado:</b> %s</li>\n" "$matricula" "$tempo"
    done >>"$ARQUIVO_HTML"

    echo "</ul>" >>"$ARQUIVO_HTML"

}

# S3.2.3: Tempo total de estacionamento por país (exceto motocicletas)
estatistica_3() {
    printf "<h2>Stats3:</h2>\n<ul>\n" >>"$ARQUIVO_HTML"
    awk -F: '$3 != "M" {pais[$2]+=$NF} END {for (p in pais) print p, pais[p]}' arquivo-*.park |
    while read -r codigo total; do
        pais=$(awk -F'###' -v c="$codigo" '$1==c {print $2}' paises.txt)
        printf "<li><b>País:</b> %s <b>Total tempo estacionado:</b> %s</li>\n" "$pais" "$total"
    done >>"$ARQUIVO_HTML"
    echo "</ul>" >>"$ARQUIVO_HTML"
}

# S3.2.4: Top 3 entradas mais tardias
estatistica_4() {
     printf "<h2>Stats4:</h2>\n<ul>\n" >>"$ARQUIVO_HTML"
    awk -F: '{split($5, d, "T"); print d[2], $1, $2, $5}' arquivo-*.park estacionamentos.txt |
    sort -r | head -3 | while read -r _ matricula pais data; do
        printf "<li><b>Matrícula:</b> %s <b>País:</b> %s <b>Data Entrada:</b> %s</li>\n" \
        "$matricula" "$pais" "$data"
    done >>"$ARQUIVO_HTML"
    echo "</ul>" >>"$ARQUIVO_HTML"

}

# S3.2.5: Tempo total de estacionamento por condutor
estatistica_5() {
    printf "<h2>Stats5:</h2>\n<ul>\n" >>"$ARQUIVO_HTML"

    awk -F: '
    {
        tempo[$4] += $NF  # Acumula tempo total por condutor
    }
    END {
        for (c in tempo) {
            dias = int(tempo[c] / 1440)
            horas = int((tempo[c] % 1440) / 60)
            minutos = tempo[c] % 60
            printf "<li><b>Condutor:</b> %s <b>Tempo total:</b> %d dia(s), %d hora(s) e %d minuto(s)</li>\n", c, dias, horas, minutos
        }
    }' arquivo-*.park >>"$ARQUIVO_HTML"

    echo "</ul>" >>"$ARQUIVO_HTML"

}

# S3.2.6: Tempo total por país em ordem específica
estatistica_6() {
    echo "<h2>Stats6:</h2>" >> "$ARQUIVO_HTML"
    echo "<ul>" >> "$ARQUIVO_HTML"

    # Mapeamento de códigos para nomes de países
    declare -A paises
    paises=( ["PT"]="Portugal" ["UK"]="Reino Unido" ["ES"]="Espanha" ["FR"]="França" )

    # Lista de países na ordem correta
    paises_ordenados=("PT" "UK" "ES" "FR")

    # Iterar sobre os países na ordem correta
    for pais in "${paises_ordenados[@]}"; do
        # Obter o tempo total estacionado para o país atual
        total_pais=$(awk -F: -v pais="$pais" '$2 == pais {sum += $NF} END {print sum}' arquivo-*.park)

        # Converter código de país para nome completo
        nome_pais="${paises[$pais]}"

        # Escrever o país e o tempo total
        echo "<li><b>País:</b> $nome_pais <b>Total tempo estacionado:</b> $total_pais</li>" >> "$ARQUIVO_HTML"
        echo "<ul>" >> "$ARQUIVO_HTML"

        # Obter tempo total de cada matrícula dentro do país, agrupando por matrícula e ordenando alfabeticamente
        awk -F: -v pais="$pais" '$2 == pais {sum[$1] += $NF} END {for (v in sum) print v "|" sum[v]}' arquivo-*.park | sort -t'|' -k1,1 | while IFS='|' read -r matricula tempo; do
            # Escrever a matrícula e o tempo total de estacionamento
            echo "<li><b>Matrícula:</b> $matricula <b> Total tempo estacionado:</b> $tempo</li>" >> "$ARQUIVO_HTML"
        done

        # Fechar a lista para o país atual
        echo "</ul>" >> "$ARQUIVO_HTML"
    done

    # Fechar a lista geral
    echo "</ul>" >> "$ARQUIVO_HTML"

}

# S3.2.7: Top 3 condutores com nomes mais longos
estatistica_7() {
    printf "<h2>Stats7:</h2>\n<ul>\n" >>"$ARQUIVO_HTML"

    awk -F: '{print length($4), $4}' estacionamentos.txt arquivo-*.park | sort -nr | head -3 | cut -d' ' -f2- |
    while read -r nome; do
        printf "<li><b> Condutor:</b> %s</li>\n" "$nome"  # Adiciona o espaço corretamente após <b>
    done >>"$ARQUIVO_HTML"

    echo "</ul>" >>"$ARQUIVO_HTML"

}

# --------------------- PROCESSAMENTO FINAL ---------------------

# Validação dos argumentos fornecidos
validar_argumentos() {
    for argumento in "$@"; do
        if ! [[ "$argumento" =~ ^[1-7]$ ]]; then
            so_error S3.1 "Argumento inválido: $argumento"
            exit 1
        fi
    done
}

# Validar os argumentos fornecidos pelo usuário
if [[ $# -gt 0 ]]; then
    validar_argumentos "$@"
fi

# Verificar a existência e permissões dos arquivos
verificar_arquivos

# Gerar o cabeçalho do arquivo HTML
gerar_cabecalho_html

# Determinar quais estatísticas processar
if [[ $# -eq 0 ]] || [[ " $* " =~ " 8 " ]]; then
    estatisticas=(1 2 3 4 5 6 7)
else
    estatisticas=("$@")
fi

# Processar as estatísticas solicitadas
for estatistica in "${estatisticas[@]}"; do
    case $estatistica in
        1) estatistica_1 ;;
        2) estatistica_2 ;;
        3) estatistica_3 ;;
        4) estatistica_4 ;;
        5) estatistica_5 ;;
        6) estatistica_6 ;;
        7) estatistica_7 ;;
        8) continue ;; # Já tratado acima
    esac
done

# Fechar o HTML
echo "</body></html>" >>"$ARQUIVO_HTML"

# Exibir sucesso para S3.3
so_success S3.3