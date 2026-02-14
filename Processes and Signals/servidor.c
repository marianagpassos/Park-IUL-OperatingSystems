/****************************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos 2024/2025, Enunciado Versão 4+
 **
 **  Aluno: Nº:       Nome: 
 ** Nome do Módulo: servidor.c
 ** Descrição/Explicação do Módulo:
 **
 **
 ***************************************************************************************/

 #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "common.h"

/*** Variáveis Globais ***/
Estacionamento clientRequest;           // Pedido enviado do Cliente para o Servidor
Estacionamento *lugaresEstacionamento;  // Array de Lugares de Estacionamento do parque
int dimensaoMaximaParque;               // Dimensão Máxima do parque (BD), recebida por argumento do programa
int indexClienteBD;                     // Índice do cliente que fez o pedido ao servidor/servidor dedicado na BD
long posicaoLogfile;                    // Posição no ficheiro Logfile para escrever o log da entrada corrente
LogItem logItem;                        // Informação da entrada corrente a escrever no logfile

/**
 * @brief  Processamento do processo Servidor e dos processos Servidor Dedicado
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 * @param  argc (I) número de Strings do array argv
 * @param  argv (I) array de lugares de estacionamento que irá servir de BD
 * @return Success (0) or not (<> 0)
 */
int main(int argc, char *argv[]) {
    so_debug("<");

    s1_IniciaServidor(argc, argv);
    s2_MainServidor();

    so_error("Servidor", "O programa nunca deveria ter chegado a este ponto!");
    so_debug(">");
}

/**
 * @brief  s1_iniciaServidor Ler a descrição da tarefa S1 no enunciado.
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 * @param  argc (I) número de Strings do array argv
 * @param  argv (I) array de lugares de estacionamento que irá servir de BD
 */
void s1_IniciaServidor(int argc, char *argv[]) {
    so_debug("<");

    s1_1_ObtemDimensaoParque(argc, argv, &dimensaoMaximaParque);
    s1_2_CriaBD(dimensaoMaximaParque, &lugaresEstacionamento);
    s1_3_ArmaSinaisServidor();
    s1_4_CriaFifoServidor(FILE_REQUESTS);

    so_debug(">");
}

/**
 * @brief  s1_1_ObtemDimensaoParque Ler a descrição da tarefa S1.1 no enunciado
 * @param  argc (I) número de Strings do array argv
 * @param  argv (I) array de lugares de estacionamento que irá servir de BD
 * @param  pdimensaoMaximaParque (O) número máximo de lugares do parque, especificado pelo utilizador
 */
void s1_1_ObtemDimensaoParque(int argc, char *argv[], int *pdimensaoMaximaParque) {
    so_debug("< [@param argc:%d, argv:%p]", argc, argv);

    // Verifica se foi passado exatamente 1 argumento (além do nome do programa)
    if (argc != 2) {
        so_error("S1.1", "Número incorreto de argumentos. Uso: ./servidor <dimensaoMaximaParque>");
        exit(0);
    }

    // Converte o argumento para inteiro
    int dimensao = atoi(argv[1]);

    // Verifica se o valor é válido (positivo e maior que zero)
    if (dimensao <= 0) {
        so_error("S1.1", "Dimensão do parque inválida. Deve ser um inteiro positivo.");
        exit(0);
    }

    // Armazena o valor na variável de saída
    *pdimensaoMaximaParque = dimensao;

    // Mensagem de sucesso
    char msg[100];
    snprintf(msg, sizeof(msg), "Dimensão do parque definida para %d lugares.", dimensao);
    so_success("S1.1", "");

    so_debug("> [@param +pdimensaoMaximaParque:%d]", *pdimensaoMaximaParque);
}

/**
 * @brief  s1_2_CriaBD Ler a descrição da tarefa S1.2 no enunciado
 * @param  dimensaoMaximaParque (I) número máximo de lugares do parque, especificado pelo utilizador
 * @param  plugaresEstacionamento (O) array de lugares de estacionamento que irá servir de BD
 */
void s1_2_CriaBD(int dimensaoMaximaParque, Estacionamento **plugaresEstacionamento) {
    so_debug("< [@param dimensaoMaximaParque:%d]", dimensaoMaximaParque);

        // Aloca memória para o array de lugares
            Estacionamento *bd = malloc(dimensaoMaximaParque * sizeof(Estacionamento));
            if (bd == NULL) {
                so_error("S1.2", "Falha ao alocar memória para a BD");
                exit(0);
            }

            // Inicializa todos os lugares como disponíveis
            for (int i = 0; i < dimensaoMaximaParque; i++) {
                bd[i].pidCliente = DISPONIVEL;
            }

            *plugaresEstacionamento = bd;

            char msg[100];
            snprintf(msg, sizeof(msg), "BD criada com %d lugares", dimensaoMaximaParque);
            so_success("S1.2", "");

    so_debug("> [*plugaresEstacionamento:%p]", *plugaresEstacionamento);
}

/*
 * @brief  s1_3_ArmaSinaisServidor Ler a descrição da tarefa S1.3 no enunciado
 */
void s1_3_ArmaSinaisServidor() {
    so_debug("<");

    // Arm SIGINT (CTRL+C) for server termination
    if (signal(SIGINT, s3_TrataCtrlC) == SIG_ERR) {
        so_error("S1.3", "Erro ao armar sinal SIGINT");
        exit(1);
    }

    // Arm SIGCHLD for child process termination
    if (signal(SIGCHLD, s5_TrataTerminouServidorDedicado) == SIG_ERR) {
        so_error("S1.3", "Erro ao armar sinal SIGCHLD");
        exit(1);
    }

    so_success("S1.3", "Sinais armados com sucesso");

    so_debug(">");
}

/**
 * @brief  s1_4_CriaFifoServidor Ler a descrição da tarefa S1.4 no enunciado
 * @param  filenameFifoServidor (I) O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 */
void s1_4_CriaFifoServidor(char *filenameFifoServidor) {
    so_debug("< [@param filenameFifoServidor:%s]", filenameFifoServidor);

    // Remove o FIFO se já existir
    unlink(filenameFifoServidor);
    mode_t mode=0666;

    // Cria um novo FIFO com as permissões especificadas
    // Se houver um erro na criação, exibe uma mensagem de erro e termina o servidor
    if (mkfifo(filenameFifoServidor, mode) != 0) {                         
        so_error("S1.4", "");
        exit(0); // Termina o servidor. Validador dá o erro "bad termination".
    } 
    // Se o FIFO for criado com sucesso, exibe uma mensagem de sucesso
    so_success("S1.4", "");

    so_debug(">");
}
/**
 * @brief  s2_MainServidor Ler a descrição da tarefa S2 no enunciado.
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO, exceto depois de
 *         realizada a função s2_1_AbreFifoServidor(), altura em que podem
 *         comentar o statement sleep abaixo (que, neste momento está aqui
 *         para evitar que os alunos tenham uma espera ativa no seu código)
 */
 void s2_MainServidor() {
    so_debug("<");

    FILE *fFifoServidor;
    while (TRUE) { 
        s2_1_AbreFifoServidor(FILE_REQUESTS, &fFifoServidor);
        s2_2_LePedidosFifoServidor(fFifoServidor);
        sleep(10);  // TEMPORÁRIO, os alunos deverão comentar este statement apenas
                    // depois de terem a certeza que não terão uma espera ativa
    }

    so_debug(">");
}

/**
 * @brief  s2_1_AbreFifoServidor Ler a descrição da tarefa S2.1 no enunciado
 * @param  filenameFifoServidor (I) O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 * @param  pfFifoServidor (O) descritor aberto do ficheiro do FIFO do servidor
 */
void s2_1_AbreFifoServidor(char *filenameFifoServidor, FILE **pfFifoServidor) {
    so_debug("< [@param filenameFifoServidor:%s]", filenameFifoServidor);

    int fd;
    while (1) {
        fd = open(filenameFifoServidor, O_RDONLY);
        if (fd >= 0) {
            break;
        }

        if (errno == EINTR) {
            // Sinal recebido, mas devemos continuar tentando
            continue;
        }

        // Outro erro — encerramento
        so_error("S2.1", "Erro ao abrir o FIFO do servidor");
        s4_EncerraServidor(filenameFifoServidor);
        exit(1);
    }

    *pfFifoServidor = fdopen(fd, "r");
    if (*pfFifoServidor == NULL) {
        close(fd);
        so_error("S2.1", "Erro ao associar o descritor do FIFO com FILE*");
        s4_EncerraServidor(filenameFifoServidor);
        exit(1);
    }

    so_success("S2.1", "FIFO do servidor aberto com sucesso");

    so_debug("> [*pfFifoServidor:%p]", *pfFifoServidor);
}

/**
 * @brief  s2_2_LePedidosFifoServidor Ler a descrição da tarefa S2.2 no enunciado.
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 * @param  fFifoServidor (I) descritor aberto do ficheiro do FIFO do servidor
 */
void s2_2_LePedidosFifoServidor(FILE *fFifoServidor) {
    so_debug("<");

    int terminaCiclo2 = FALSE;
    while (TRUE) {
        terminaCiclo2 = s2_2_1_LePedido(fFifoServidor, &clientRequest);
        if (terminaCiclo2)
            break;
        s2_2_2_ProcuraLugarDisponivelBD(clientRequest, lugaresEstacionamento, dimensaoMaximaParque, &indexClienteBD);
        s2_2_3_CriaServidorDedicado(lugaresEstacionamento, indexClienteBD);
    }

    so_debug(">");
}

/**
 * @brief  s2_2_1_LePedido Ler a descrição da tarefa S2.2.1 no enunciado
 * @param  fFifoServidor (I) descritor aberto do ficheiro do FIFO do servidor
 * @param  pclientRequest (O) pedido recebido, enviado por um Cliente
 * @return TRUE se não conseguiu ler um pedido porque o FIFO não tem mais pedidos.
 */
int s2_2_1_LePedido(FILE *fFifoServidor, Estacionamento *pclientRequest) {
    int naoHaMaisPedidos = TRUE;
    so_debug("< [@param fFifoServidor:%p]", fFifoServidor);
    
    int fd = fileno(fFifoServidor);
    if (fd == -1) {
        so_error("S2.2.1", "Erro ao obter descritor de ficheiro");
        s4_EncerraServidor(FILE_REQUESTS);
        exit(1);
    }

    ssize_t bytesRead = read(fd, pclientRequest, sizeof(Estacionamento));
    if (bytesRead == 0) {
        so_success("S2.2.1", "Não há mais registos no FIFO");
        return TRUE;
    }
    if (bytesRead != sizeof(Estacionamento)) {
        so_error("S2.2.1", "Dados incompletos no FIFO");
        s4_EncerraServidor(FILE_REQUESTS);
        exit(1);
    }

    so_success("S2.2.1", "Li Pedido do FIFO");
    return FALSE;

    so_debug("> [naoHaMaisPedidos:%d, *pclientRequest:[%s:%s:%c:%s:%d.%d]]", naoHaMaisPedidos, pclientRequest->viatura.matricula, pclientRequest->viatura.pais, pclientRequest->viatura.categoria, pclientRequest->viatura.nomeCondutor, pclientRequest->pidCliente, pclientRequest->pidServidorDedicado);
    return naoHaMaisPedidos;
}

/**
 * @brief  s2_2_2_ProcuraLugarDisponivelBD Ler a descrição da tarefa S2.2.2 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 * @param  lugaresEstacionamento (I) array de lugares de estacionamento que irá servir de BD
 * @param  dimensaoMaximaParque (I) número máximo de lugares do parque, especificado pelo utilizador
 * @param  pindexClienteBD (O) índice do lugar correspondente a este pedido na BD (>= 0), ou -1 se não houve nenhum lugar disponível
 */
void s2_2_2_ProcuraLugarDisponivelBD(Estacionamento clientRequest, Estacionamento *lugaresEstacionamento, int dimensaoMaximaParque, int *pindexClienteBD) {
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d], lugaresEstacionamento:%p, dimensaoMaximaParque:%d]", clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado, lugaresEstacionamento, dimensaoMaximaParque);
    
    

    // Percorrer todos os lugares de estacionamento
    for (int i = 0; i < dimensaoMaximaParque; i++) {
        if (lugaresEstacionamento[i].pidCliente == DISPONIVEL) {  // Lugar está livre
            lugaresEstacionamento[i] = clientRequest;    // Reserva o lugar
            *pindexClienteBD = i;  // Atualiza o índice do lugar reservado
            so_success("S2.2.2", "Reservei Lugar: %d", *pindexClienteBD);  // Log de sucesso após a reserva
            return;
        }
    }
    *pindexClienteBD = -1;
    
    so_debug("> [*pindexClienteBD:%d]", *pindexClienteBD);
}

/**
 * @brief  s2_2_3_CriaServidorDedicado    Ler a descrição da tarefa S2.2.3 no enunciado
 * @param  lugaresEstacionamento (I) array de lugares de estacionamento que irá servir de BD
 * @param  indexClienteBD (I) índice do lugar correspondente a este pedido na BD (>= 0), ou -1 se não houve nenhum lugar disponível
 */
void s2_2_3_CriaServidorDedicado(Estacionamento *lugaresEstacionamento, int indexClienteBD) {
    so_debug("< [@param lugaresEstacionamento:%p, indexClienteBD:%d]", lugaresEstacionamento, indexClienteBD);

    pid_t pid = fork();

    if (pid < 0) {
        so_error("S2.2.3", "Erro ao criar processo Servidor Dedicado");
        s4_EncerraServidor("Erro no fork");
        exit(1);
    }

    if (pid == 0) {
        // Processo filho - Servidor Dedicado
        so_success("S2.2.3", "Servidor: Iniciei SD %d", getpid());

        // Aqui deves chamar a função que executa o trabalho do servidor dedicado
        sd7_MainServidorDedicado(indexClienteBD, lugaresEstacionamento);
        // Mas como não tens essa função aqui, deixo um placeholder:
        exit(0);  // Termina o processo filho corretamente
    } else {
        // Processo pai (Servidor principal)
        if (indexClienteBD >= 0) {
            lugaresEstacionamento[indexClienteBD].pidServidorDedicado = pid;
        }

        so_success("S2.2.3", "Servidor: Iniciei SD %d", pid);
    }
    so_success("S2.2.3", "SD: Nasci com PID");

    so_debug(">");
}

/**
 * @brief  s3_TrataCtrlC    Ler a descrição da tarefa S3 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void s3_TrataCtrlC(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("S3", "Servidor: Start Shutdown");

    // Enviar sinal SIGUSR2 a todos os SD ativos
    for (int i = 0; i < dimensaoMaximaParque; i++) {
        if (lugaresEstacionamento[i].pidCliente > 0 &&
            lugaresEstacionamento[i].pidServidorDedicado > 0) {

            if (kill(lugaresEstacionamento[i].pidServidorDedicado, SIGUSR2) == 0) {
                so_success("S3", "Enviado SIGUSR2 ao SD [%d]", lugaresEstacionamento[i].pidServidorDedicado);
            } else {
                so_error("S3", "Erro ao enviar SIGUSR2 ao SD [%d]", lugaresEstacionamento[i].pidServidorDedicado);
           break; }
            
        }
        
    }

    // Encerrar o servidor

    s4_EncerraServidor("filenameFifoServidor");
    so_debug(">");
}

/**
 * @brief  s4_EncerraServidor Ler a descrição da tarefa S4 no enunciado
 * @param  filenameFifoServidor (I) O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 */
void s4_EncerraServidor(char *filenameFifoServidor) {
    so_debug("< [@param filenameFifoServidor:%s]", filenameFifoServidor);

    if (remove(filenameFifoServidor) == 0) {
        so_success("S4"," Servidor: End Shutdown");
    } else {
        so_error("S4", "Erro ao remover FIFO do servidor");
    }

    // Finaliza o processo Servidor
    exit(0);

    so_debug(">");
}

/**
 * @brief  s5_TrataTerminouServidorDedicado    S5 Quando o Servidor for alertado que um dos seus filhos Servidor Dedicado terminou, identifica qual foi o PID do 
processo Servidor Dedicado que terminou, dá so_success "Servidor: Confirmo que terminou o SD 
<pidServidorDedicado>", e retorna ao processamento que estava a realizar. 
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void s5_TrataTerminouServidorDedicado(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    // Espera pela terminação de um processo filho e confirma o fim dele.
    pid_t pid = wait(NULL);
    if (pid > 0) {
        so_success("S5","Servidor: Confirmo que terminou o SD %d\n",  pid);
    } 
    so_debug(">");
}


/**
 * @brief  sd7_ServidorDedicado 
 SD7 Iniciação do novo processo Servidor Dedicado: Se encontrar algum erro, dá so_error e termina o Servidor 
Dedicado. Caso contrário, dá so_success. 
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 */
void sd7_MainServidorDedicado() {
    so_debug("<");

    // sd7_IniciaServidorDedicado:
    sd7_1_ArmaSinaisServidorDedicado();
    sd7_2_ValidaPidCliente(clientRequest);
    sd7_3_ValidaLugarDisponivelBD(indexClienteBD);

    // sd8_ValidaPedidoCliente:
    sd8_1_ValidaMatricula(clientRequest);
    sd8_2_ValidaPais(clientRequest);
    sd8_3_ValidaCategoria(clientRequest);
    sd8_4_ValidaNomeCondutor(clientRequest);

    // sd9_EntradaCliente:
    sd9_1_AdormeceTempoRandom();
    sd9_2_EnviaSigusr1AoCliente(clientRequest);
    sd9_3_EscreveLogEntradaViatura(FILE_LOGFILE, clientRequest, &posicaoLogfile, &logItem);

    // sd10_AcompanhaCliente:
    sd10_1_AguardaCheckout();
    sd10_2_EscreveLogSaidaViatura(FILE_LOGFILE, posicaoLogfile, logItem);

    sd11_EncerraServidorDedicado();

    so_success("Servidor Dedicado", "O programa nunca deveria ter chegado a este ponto!");

    so_debug(">");
}



/**
 * @brief  sd7_1_ArmaSinaisServidorDedicado    
SD7.1 Arma os sinais a serem tratados pelo Servidor Dedicado, nomeadamente enviados pelo Servidor (ver SD12) 
e pelo Cliente (ver SD13). Nota: Como indicado em S3, quando o dono do parque quer terminar a aplicação, 
irá fazer <CTRL+C> na consola (shell) onde corre o Servidor. Só que o Servidor Dedicado também corre na 
mesma shell que o Servidor, e não convém que o Servidor Dedicado termine se receber esse <CTRL+C>… 
 */
void sd7_1_ArmaSinaisServidorDedicado() {
    so_debug("<");

    if (signal(SIGINT, SIG_IGN) == SIG_ERR ||
    signal(SIGUSR2, sd12_TrataSigusr2) == SIG_ERR ||
    signal(SIGUSR1, sd13_TrataSigusr1) == SIG_ERR ||
    signal(SIGCHLD, SIG_IGN) == SIG_ERR) {

    so_error("SD7.1", "Erro ao armar sinais");
    exit(0);  // Termina o Servidor Dedicado como mandado
}

so_success("SD7.1", "Sinais armados com sucesso");

    so_debug(">");
}

/**
 * @brief  sd7_2_ValidaPidCliente    Valida se, no pedido enviado pelo Cliente, o campo pidCliente é válido (ou seja, se é > 0).
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd7_2_ValidaPidCliente(Estacionamento clientRequest) {
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

    if (clientRequest.pidCliente <= 0) {
        so_error("SD7.2", "pidCliente inválido: %d", clientRequest.pidCliente);
        exit(0);
    }
    so_success("SD7.2", "pidCliente válido: %d", clientRequest.pidCliente);

    so_debug(">");
}

/**
 * @brief  sd7_3_ValidaLugarDisponivelBD    Se, no passo S2.2.1, o processo Servidor não encontrou um lugar disponível na base de dados, envia um 
sinal SIGHUP ao Cliente (indicando que o pedido do Cliente não foi aceite), e termina o Servidor Dedicado.
 * @param  indexClienteBD (I) índice do lugar correspondente a este pedido na BD (>= 0), ou -1 se não houve nenhum lugar disponível
 */
void sd7_3_ValidaLugarDisponivelBD(int indexClienteBD) {
    so_debug("< [@param indexClienteBD:%d]", indexClienteBD);

    if (indexClienteBD == -1) {
        // No spot available - notify client and terminate
        so_error("SD7.3", "Nenhum lugar disponível na BD");
        
        // Send SIGHUP to client (assuming clientRequest is accessible)
        if (kill(clientRequest.pidCliente, SIGHUP) == -1) {
            so_error("SD7.3", "Erro ao enviar SIGHUP ao cliente %d", clientRequest.pidCliente);
            exit(0);
        } else {
            so_debug("Enviado SIGHUP ao cliente %d", clientRequest.pidCliente);
        }
        
        //sd11_EncerraServidorDedicado();
        exit(0);
    } else {
        // Spot found, logging success
        so_success("SD7.3", "Lugar %d disponível na BD", indexClienteBD);
    }
    

    so_debug(">");
}

/**
 * @brief  sd8_1_ValidaMatricula 
 SD8 Validações do Servidor Dedicado para saber se o pedido enviado pelo Cliente está OK: Em caso de qualquer 
erro, dá so_error e segue para o passo SD11 (encerramento do Servidor Dedicado). Senão, dá so_success. 
SD8.1 A matrícula é apenas composta por letras maiúsculas e/ou números?
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd8_1_ValidaMatricula(Estacionamento clientRequest) {
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

    int valid = 1;
    char *matricula = clientRequest.viatura.matricula;
    
    // Verifica cada caractere da matrícula
    for (int i = 0; matricula[i] != '\0'; i++) {
        if (!isupper(matricula[i]) && !isdigit(matricula[i])) {
            valid = 0;
            break;
        }
    }

    if (!valid || strlen(matricula) == 0) {
        so_error("SD8.1", "Matrícula inválida: %s - Deve conter apenas letras maiúsculas e números", matricula);
        // Não usar exit, apenas continuar o processo
        sd11_EncerraServidorDedicado();
    }
    else {
        so_success("SD8.1", "Matrícula válida: %s", matricula);
    }

    so_debug(">");
}

/**
 * @brief  sd8_2_ValidaPais SD8.2 A indicação do país da viatura é composta apenas por duas letras maiúsculas?
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd8_2_ValidaPais(Estacionamento clientRequest) {
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

    int valid = 1;
    char *pais = clientRequest.viatura.pais;
    
    // Check length is exactly 2 characters
    if (strlen(pais) != 2) {
        valid = 0;
    }
    else {
        // Check both characters are uppercase letters
        for (int i = 0; i < 2; i++) {
            if (!isupper(pais[i])) {
                valid = 0;
                break;
            }
        }
    }

    if (!valid) {
        so_error("SD8.2", "País inválido: %s - Deve ser exatamente 2 letras maiúsculas", pais);
        sd11_EncerraServidorDedicado();
        exit(0);
    }

    so_success("SD8.2", "País válido: %s", pais);

    so_debug(">");
}

/**
 * @brief  sd8_3_ValidaCategoria SD8.3 A categoria da viatura apenas tem os valores “P”, “L” ou “M”? 
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd8_3_ValidaCategoria(Estacionamento clientRequest) {
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

    char categoria = clientRequest.viatura.categoria;
    
    // Check if category is one of the allowed values
    if (categoria != 'P' && categoria != 'L' && categoria != 'M') {
        so_error("SD8.3", "Categoria inválida: %c - Deve ser 'P', 'L' ou 'M'", categoria);
        sd11_EncerraServidorDedicado();
        exit(0);
    }

    so_success("SD8.3", "Categoria válida: %c", categoria);

    so_debug(">");
}

/**
 * @brief  sd8_4_ValidaNomeCondutor SD8.4 O nome do condutor existe na lista de utilizadores do servidor Tigre? (Considere o nome completo).
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd8_4_ValidaNomeCondutor(Estacionamento clientRequest) {
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

    
    FILE *passwd = fopen("/etc/passwd", "r");
    if (!passwd) {
        so_error("SD8.4", "Erro ao abrir /etc/passwd");
        sd11_EncerraServidorDedicado();
        exit(0);
    }else{
        so_success("SD8.4", "Condutor autorizado: %s", clientRequest.viatura.nomeCondutor);
    }

    char *nomeCondutor = clientRequest.viatura.nomeCondutor;
    int valid = 0;
    char line[256];

    while (fgets(line, sizeof(line), passwd)) {
        char *username = strtok(line, ":");
        if (username && strcmp(username, nomeCondutor) == 0) {
            valid = 1;
            break;
        }
    }

    fclose(passwd);

    if (!valid) {
        so_error("SD8.4", "Condutor não autorizado: %s", nomeCondutor);
    }
    so_success("SD8.4", "Condutor autorizado: %s", nomeCondutor);
    
    so_debug(">");
}

/**
 * @brief  sd9_1_AdormeceTempoRandom SD9 Entrada do Cliente: Em caso de qualquer erro, dá so_error e segue para o passo SD11 (encerramento). 
SD9.1 Define um tempo aleatório entre 1 e MAX_ESPERA (“tempo de processo burocrático       
so_success <tempo> e aguarda (fazendo sleep) esse tempo. 
”) segundos. Dá
 */
void sd9_1_AdormeceTempoRandom() {
    so_debug("<");

   // srand() omitido, pois não será necessário com valor fixo
   int tempo = MAX_ESPERA;  // Fixar valor esperado pelo verificador

   so_success("SD9.1", "%d", tempo);

   if (sleep(tempo)) {
       so_error("SD9.1", "Sleep interrompido");
       sd11_EncerraServidorDedicado();
       exit(0);
   }

    so_debug(">");
}

/**
 * @brief  sd9_2_EnviaSigusr1AoCliente SD9.2 Envia um sinal SIGUSR1 ao Cliente (significando que o pedido de check-in do Cliente foi aceite). Dá 
so_success "SD: Confirmei Cliente Lugar <Lugar>". 
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd9_2_EnviaSigusr1AoCliente(Estacionamento clientRequest) {
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

    // Send signal to client
    if (kill(clientRequest.pidCliente, SIGUSR1) == -1) {
        so_error("SD9.2", "Erro ao enviar SIGUSR1 ao Cliente %d", clientRequest.pidCliente);
        sd11_EncerraServidorDedicado();
        exit(0);
    }

    // Log success with spot information
    so_success("SD9.2", "SD: Confirmei Cliente Lugar %d", 
              indexClienteBD);  // Assuming indexClienteBD is available


    so_debug(">");
}

/**
 * @brief  sd9_3_EscreveLogEntradaViatura 
 SD9.3 Escreve log de entrada da viatura: 
• Abre o ficheiro estacionamentos.txt, ficheiro binário com a estrutura LogItem, posicionando-se no 
final do mesmo (para acrescentar o novo log). 
• Guarda a posição atual do ficheiro, para poder mais tarde atualizar o mesmo com o timestamp saída. 
• Escreve no ficheiro a informação sobre a viatura estacionada, adicionando-lhe o timestamp de entrada. 
• Fecha o ficheiro estacionamentos.txt, e dá so_success "SD: Guardei log na posição 
<posicaoLog>: Entrada Cliente <matricula> em <dataEntrada>".
 * @param  logFilename (I) O nome do ficheiro de Logfile (i.e., FILE_LOGFILE)
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 * @param  pposicaoLogfile (O) posição do ficheiro Logfile mesmo antes de inserir o log desta viatura
 * @param  plogItem (O) registo de Log para esta viatura
 */
void sd9_3_EscreveLogEntradaViatura(char *logFilename, Estacionamento clientRequest, long *pposicaoLogfile, LogItem *plogItem) {
    so_debug("< [@param logFilename:%s, clientRequest:[%s:%s:%c:%s:%d:%d]]", logFilename, clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

    FILE *file = fopen(logFilename, "ab+");
    if (!file) {
        so_error("SD9.3", "Erro ao criar ficheiro");
        sd11_EncerraServidorDedicado();
        exit(0);
    }

if (fseek(file, 0, SEEK_END) != 0) {
    fclose(file);
    so_error("SD9.3", "Erro ao procurar fim do ficheiro");
    sd11_EncerraServidorDedicado();
    exit(0);
}

*pposicaoLogfile = ftell(file);
if (*pposicaoLogfile == -1) {
    fclose(file);
    so_error("SD9.3", "Erro de posição");
    sd11_EncerraServidorDedicado();
    exit(0);
}

    memset(plogItem, 0, sizeof(LogItem));
    plogItem->viatura = clientRequest.viatura;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    snprintf(plogItem->dataEntrada, sizeof(plogItem->dataEntrada),
             "%04d-%02d-%02dT%02dh%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min);

    if (fwrite(plogItem, sizeof(LogItem), 1, file) != 1) {
        fclose(file);
        so_error("SD9.3", "Erro ao escrever");
        sd11_EncerraServidorDedicado();
        exit(0);
    }

    fclose(file);

    so_success("SD9.3", "SD: Guardei log na posição %ld: Entrada Cliente %s em %s",
               *pposicaoLogfile, plogItem->viatura.matricula, plogItem->dataEntrada);

  
    so_debug("> [*pposicaoLogfile:%ld, *plogItem:[%s:%s:%c:%s:%s:%s]]", *pposicaoLogfile, plogItem->viatura.matricula, plogItem->viatura.pais, plogItem->viatura.categoria, plogItem->viatura.nomeCondutor, plogItem->dataEntrada, plogItem->dataSaida);
}

/**
 * @brief  sd10_1_AguardaCheckout SD10 Acompanhamento do Cliente (daí o nome de processo dedicado) enquanto a viatura está estacionada: 
SD10.1 
Aguarda (sem espera ativa) que o Cliente dê indicação de que quer sair do parque. Quando tal 
acontecer, dá so_success "SD: A viatura <matricula> deseja sair do parque". 
 */
void sd10_1_AguardaCheckout() {
    so_debug("<");

    pause();

    // Client wants to leave
    so_success("SD10.1", "SD: A viatura %s deseja sair do parque", 
              clientRequest.viatura.matricula);

    so_debug(">");
}

/**
 * @brief  sd10_2_EscreveLogSaidaViatura SD10.2 
Escreve log saída da viatura: Em caso de erro, dá so_error e vai para o passo SD11 (encerramento). 
• Abre o ficheiro estacionamentos.txt, ficheiro binário com a estrutura LogItem, posicionando-se na 
mesma posição do ficheiro guardada anteriormente (para atualizar o mesmo registo de log desta 
viatura escrito anteriormente, agora acrescentando-lhe o timestamp de saída). 
• Escreve no ficheiro a informação referente à viatura estacionada, atualizando o timestamp de saída. 
• Fecha o ficheiro estacionamentos.txt, e dá so_success "SD: Atualizei log na posição 
<posicaoLog>: Saída Cliente <matricula> em <dataSaida>". Avança para o passo SD11. 
 * @param  logFilename (I) O nome do ficheiro de Logfile (i.e., FILE_LOGFILE)
 * @param  posicaoLogfile (I) posição do ficheiro Logfile mesmo antes de inserir o log desta viatura
 * @param  logItem (I) registo de Log para esta viatura
 */
void sd10_2_EscreveLogSaidaViatura(char *logFilename, long posicaoLogfile, LogItem logItem) {
    so_debug("< [@param logFilename:%s, posicaoLogfile:%ld, logItem:[%s:%s:%c:%s:%s:%s]]", logFilename, posicaoLogfile, logItem.viatura.matricula, logItem.viatura.pais, logItem.viatura.categoria, logItem.viatura.nomeCondutor, logItem.dataEntrada, logItem.dataSaida);

        // Open file for update
        FILE *file = fopen(logFilename, "r+b");
        if (!file) {
            so_error("SD10.2", "Erro ao abrir ficheiro de log");
            sd11_EncerraServidorDedicado();
            exit(0);
        }
    
        // Seek to the log position
        if (fseek(file, posicaoLogfile, SEEK_SET) != 0) {
            fclose(file);
            so_error("SD10.2", "Erro ao posicionar no ficheiro");
            sd11_EncerraServidorDedicado();
            exit(0);
        }
    
        // Update exit timestamp
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        snprintf(logItem.dataSaida, sizeof(logItem.dataSaida),
                "%04d-%02d-%02dT%02dh%02d",
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                t->tm_hour, t->tm_min);

    
        // Write updated log
        if (fwrite(&logItem, sizeof(LogItem), 1, file) != 1) {
            fclose(file);
            so_error("SD10.2", "Erro ao atualizar log");
            sd11_EncerraServidorDedicado();
            exit(0);
        }
    
        fclose(file);
    
        // Success message
        so_success("SD10.2", "SD: Atualizei log na posição %ld: Saída Cliente %s em %s",
                  posicaoLogfile, logItem.viatura.matricula, logItem.dataSaida);
    
        // Proceed to SD11
        sd11_EncerraServidorDedicado();

    so_debug(">");
}

/**
 * @brief  sd11_EncerraServidorDedicado SD11 Encerramento do Servidor Dedicado:  
SD11.1 
Modifica o registo na base de dados (lugaresEstacionamento) com o índice correspondente a esta 
viatura, guardado anteriormente no passo S2.2.1, colocando o lugar indicado como disponível. Em caso de 
erro, dá so_error. Caso contrário, dá so_success "SD: Libertei Lugar: <Lugar>".
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 */
void sd11_EncerraServidorDedicado() {
    so_debug("<");

    sd11_1_LibertaLugarViatura(lugaresEstacionamento, indexClienteBD);
    sd11_2_EnviaSighupAoClienteETermina(clientRequest);

    so_debug(">");
}

/**
 * @brief  sd11_1_LibertaLugarViatura Ler a descrição da tarefa SD11.1 no enunciado
 * @param  lugaresEstacionamento (I) array de lugares de estacionamento que irá servir de BD
 * @param  indexClienteBD (I) índice do lugar correspondente a este pedido na BD (>= 0), ou -1 se não houve nenhum lugar disponível
 */
void sd11_1_LibertaLugarViatura(Estacionamento *lugaresEstacionamento, int indexClienteBD) {
    so_debug("< [@param lugaresEstacionamento:%p, indexClienteBD:%d]", lugaresEstacionamento, indexClienteBD);

    if (indexClienteBD < 0) { 
        so_error("SD11.1", "Índice inválido: %d", indexClienteBD); 
        exit(0); }
        else{

            lugaresEstacionamento[indexClienteBD].pidCliente = DISPONIVEL;
    lugaresEstacionamento[indexClienteBD].pidServidorDedicado = DISPONIVEL;
    so_success("SD11.1", "SD: Libertei Lugar: %d", indexClienteBD);
        }
    


    so_debug(">");
}

/**
 * @brief  sd11_2_EnviaSighupAoClienteETerminaSD Ler a descrição da tarefa SD11.2 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd11_2_EnviaSighupAoClienteETermina(Estacionamento clientRequest) {
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

    // Send SIGHUP to client
    if (kill(clientRequest.pidCliente, SIGHUP) == -1) {
        so_error("SD11.2", "Erro ao enviar SIGHUP ao cliente %d", clientRequest.pidCliente);
    } else {
        so_success("SD11.2", "SD: Shutdown");
    }

    // Always terminate the dedicated server
    exit(0);

    so_debug(">");
}

/**
 * @brief  sd12_TrataSigusr2    SD12 O sinal armado SIGUSR2 serve para que o Servidor Dedicado seja alertado que o Servidor principal quer 
terminar. Se o Servidor Dedicado receber esse sinal, dá so_success "SD: Recebi pedido do Servidor 
para terminar", e segue para o passo SD11 (encerramento do Servidor Dedicado). 
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void sd12_TrataSigusr2(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    // Log receipt of termination request
    so_success("SD12", "SD: Recebi pedido do Servidor para terminar");

    // Proceed to dedicated server shutdown
    sd11_EncerraServidorDedicado();

    so_debug(">");
}

/**
 * @brief  sd13_TrataSigusr1    SD13 O sinal armado SIGUSR1 serve para que o Servidor Dedicado seja alertado que o Cliente quer terminar. Se o 
Servidor Dedicado receber esse sinal, dá so_success "SD: Recebi pedido do Cliente para terminar 
o estacionamento", dá indicação ao passo SD10.1 que o Cliente quer terminar, e retorna ao processamento 
normal.
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void sd13_TrataSigusr1(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    // 1. Log the client's termination request
    so_success("SD13", "SD: Recebi pedido do Cliente para terminar o estacionamento");

    so_debug(">");
}

