/****************************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos 2024/2025, Enunciado Versão 4+
 **
 ** Aluno: Nº:122660       Nome: Mariana Passos
 ** Nome do Módulo: cliente.c
 ** Descrição/Explicação do Módulo:
 **
 **
 ***************************************************************************************/

 #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "common.h"

/*** Variáveis Globais ***/
Estacionamento clientRequest;           // Pedido enviado do Cliente para o Servidor
int recebeuRespostaServidor = FALSE;    // Variável que determina se o Cliente já recebeu uma resposta do Servidor


/**
 * @brief  Processamento do processo Cliente.
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 */
int main () {
    so_debug("<");

    // c1_IniciaCliente:
    c1_1_ValidaFifoServidor(FILE_REQUESTS);
    c1_2_ArmaSinaisCliente();

    // c2_CheckinCliente:
    c2_1_InputEstacionamento(&clientRequest);
    FILE *fFifoServidor;
    c2_2_AbreFifoServidor(FILE_REQUESTS, &fFifoServidor);
    c2_3_EscrevePedido(fFifoServidor, clientRequest);

    c3_ProgramaAlarme(MAX_ESPERA);

    // c4_AguardaRespostaServidor:
    c4_1_EsperaRespostaServidor();
    c4_2_DesligaAlarme();
    c4_3_InputEsperaCheckout();

    c5_EncerraCliente();

    so_error("Cliente", "O programa nunca deveria ter chegado a este ponto!");
    so_debug(">");
}

/**
 * @brief  c1_1_ValidaFifoServidor Ler a descrição da tarefa C1.1 no enunciado
 * @param  filenameFifoServidor (I) O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 */
void c1_1_ValidaFifoServidor(char *filenameFifoServidor) {
    so_debug("< [@param filenameFifoServidor:%s]", filenameFifoServidor);

    struct stat fileStat;
    if (stat(filenameFifoServidor, &fileStat) != 0) {
        so_error("C1.1", "O FIFO do Servidor não existe");

        exit(1);
    }
    if (!S_ISFIFO(fileStat.st_mode)) {
        so_error("C1.1", "O arquivo não é um FIFO");
        exit(0);
       
    }

    so_success("C1.1", "O FIFO do Servidor existe");
    so_debug(">");
}

/**
 * @brief  c1_2_ArmaSinaisCliente Ler a descrição da tarefa C1.3 no enunciado
 */
 void c1_2_ArmaSinaisCliente() {
    so_debug("<");

    // Configura os sinais SIGUSR1, SIGHUP, SIGINT e SIGALRM
    // Se houver um erro ao configurar qualquer um desses sinais, exibe uma mensagem de erro e termina o Cliente
    // SIGUSR1 com sigaction (SA_SIGINFO)
    struct sigaction sa;
    sa.sa_sigaction = c6_TrataSigusr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    if (sigaction(SIGUSR1, &sa, NULL) != 0) {
        so_error("C1.2", "Erro ao armar o sinal SIGUSR1");
        exit(1);
    }
    
    if (signal(SIGHUP, c7_TrataSighup) == SIG_ERR) {
        so_error("C1.2", "Erro ao armar o sinal SIGHUP");
        exit(0);
    }

    if (signal(SIGINT, c8_TrataCtrlC) == SIG_ERR) {
        so_error("C1.2", "Erro ao armar o sinal SIGINT");
        exit(0);
    }

    if (signal(SIGALRM, c9_TrataAlarme) == SIG_ERR) {
        so_error("C1.2", "Erro ao armar o sinal SIGALRM");
        exit(0);
    }

    // Se todos os sinais forem configurados com sucesso, exibe uma mensagem de sucesso
    so_success("C1.2", "Sinais armados com sucesso");

    so_debug(">");
}

/**
 * @brief  c2_1_InputEstacionamento Ler a descrição da tarefa C2.1 no enunciado
 * @param  pclientRequest (O) pedido a ser enviado por este Cliente ao Servidor
 */
 void c2_1_InputEstacionamento(Estacionamento *pclientRequest) {
    so_debug("<");
    printf("Park-IUL: Check-in Viatura\n");
    printf("----------------------------\n");

    char input[128];

    // Matrícula
    while (1) {
        printf("Introduza a matrícula da viatura: ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;
        int vazia = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isspace((unsigned char)input[i])) {
                vazia = 0;
                break;
            }
        }
        if (!vazia) break;
    }
    strncpy(pclientRequest->viatura.matricula, input, sizeof(pclientRequest->viatura.matricula));

    // País
    while (1) {
        printf("Introduza o país da viatura: ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;
        int vazia = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isspace((unsigned char)input[i])) {
                vazia = 0;
                break;
            }
        }
        if (!vazia) break;
    }
    strncpy(pclientRequest->viatura.pais, input, sizeof(pclientRequest->viatura.pais));

    // Categoria
    while (1) {
        printf("Introduza a categoria da viatura: ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;
        int vazia = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isspace((unsigned char)input[i])) {
                vazia = 0;
                break;
            }
        }
        if (!vazia) break;
    }
    pclientRequest->viatura.categoria = input[0];

    // Nome do condutor
    while (1) {
        printf("Introduza o nome do condutor: ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;
        int vazia = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isspace((unsigned char)input[i])) {
                vazia = 0;
                break;
            }
        }
        if (!vazia) break;
    }
    strncpy(pclientRequest->viatura.nomeCondutor, input, sizeof(pclientRequest->viatura.nomeCondutor));

    // PIDs
    pclientRequest->pidCliente = getpid();
    pclientRequest->pidServidorDedicado = -1;

    // Sucesso
    so_success("C2.1", "%s %s %c %s %d %d",
        pclientRequest->viatura.matricula,
        pclientRequest->viatura.pais,
        pclientRequest->viatura.categoria,
        pclientRequest->viatura.nomeCondutor,
        pclientRequest->pidCliente,
        pclientRequest->pidServidorDedicado
    );

    so_debug("> [*pclientRequest:[%s:%s:%c:%s:%d:%d]]", pclientRequest->viatura.matricula, pclientRequest->viatura.pais, pclientRequest->viatura.categoria, pclientRequest->viatura.nomeCondutor, pclientRequest->pidCliente, pclientRequest->pidServidorDedicado);
}

/**
 * @brief  c2_2_AbreFifoServidor Ler a descrição da tarefa C2.2 no enunciado
 * @param  filenameFifoServidor (I) O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 * @param  pfFifoServidor (O) descritor aberto do ficheiro do FIFO do servidor
 */
void c2_2_AbreFifoServidor(char *filenameFifoServidor, FILE **pfFifoServidor) {
    so_debug("< [@param filenameFifoServidor:%s]", filenameFifoServidor);

    // Abre o FIFO do Servidor
    FILE *file = fopen(filenameFifoServidor, "w");
    if (file == NULL) {
        so_error("C2.2", "");
        exit(1);
    }



    so_success("C2.2", "");

    so_debug("> [*pfFifoServidor:%p]", *pfFifoServidor);
}

/**
 * @brief  c2_3_EscrevePedido Ler a descrição da tarefa C2.3 no enunciado
 * @param  fFifoServidor (I) descritor aberto do ficheiro do FIFO do servidor
 * @param  clientRequest (I) pedido a ser enviado por este Cliente ao Servidor
 */
void c2_3_EscrevePedido(FILE *fFifoServidor, Estacionamento clientRequest) {
    so_debug("< [@param fFifoServidor:%p, clientRequest:[%s:%s:%c:%s:%d:%d]]", fFifoServidor, clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

// Tenta escrever no FIFO
if (fwrite(&clientRequest, sizeof(Estacionamento), 1, fFifoServidor) != 1) {
    so_error("C2.3", "Erro ao escrever pedido no FIFO");
    exit(1);  // <- Não fecha!
}

// fwrite correu bem, fecha agora
if (fclose(fFifoServidor) != 0) {
    so_error("C2.3", "Erro ao fechar FIFO do Servidor");
    exit(1);
}

so_success("C2.3", "Pedido escrito no FIFO com sucesso");


    so_debug(">");
}

/**
 * @brief  c3_ProgramaAlarme Ler a descrição da tarefa C3 no enunciado
 * @param  segundos (I) número de segundos a programar no alarme
 */
void c3_ProgramaAlarme(int segundos) {
    so_debug("< [@param segundos:%d]", segundos);

    signal(SIGALRM, c9_TrataAlarme);
    
    alarm(segundos);
    so_success("C3","Espera resposta em %d segundos", segundos);

    so_debug(">");
}

/**
 * @brief  c4_1_EsperaRespostaServidor Ler a descrição da tarefa C4 no enunciado
 */
void c4_1_EsperaRespostaServidor() {
    so_debug("<");

    // Aguarda um sinal (sem bloquear ativamente, sem usar sleep)
    pause();  // Aguardar até receber um sinal

    // Após o sinal ser recebido e tratado, indica que o check-in foi realizado com sucesso
    so_success("C4.1", "Check-in realizado com sucesso");

    so_debug(">");
}

/**
 * @brief  c4_2_DesligaAlarme Ler a descrição da tarefa C4.1 no enunciado
 */
void c4_2_DesligaAlarme() {
    so_debug("<");

    alarm(0);  // Cancela o alarme pendente
    so_success("C4.2" ,"Desliguei alarme");

    so_debug(">");
}

/**
 * @brief  c4_3_InputEsperaCheckout Ler a descrição da tarefa C4.2 no enunciado
 */
void c4_3_InputEsperaCheckout() {
    so_debug("<");

    char input[64];

    while (1) {
        printf("Escreva 'sair' para terminar o estacionamento: ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            continue; // Se der erro, tenta outra vez
        }

        // Remover newline (\n) no final, se existir
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "sair") == 0) {
            so_success("C4.3", "Utilizador pretende terminar estacionamento");
            c5_EncerraCliente();
            break;
        }
    }
    c5_EncerraCliente();
    so_debug(">");
}

/**
 * @brief  c5_EncerraCliente      Ler a descrição da tarefa C5 no enunciado
 */
void c5_EncerraCliente() {
    so_debug("<");

    // Substituir este comentário pelo código da função a ser implementado pelo aluno

    so_debug(">");
}

/**
 * @brief  c5_1_EnviaSigusr1AoServidor      Ler a descrição da tarefa C5.1 no enunciado
 * @param  clientRequest (I) pedido a ser enviado por este Cliente ao Servidor
 */
void c5_1_EnviaSigusr1AoServidor(Estacionamento clientRequest) {
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]", clientRequest.viatura.matricula, clientRequest.viatura.pais, clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor, clientRequest.pidCliente, clientRequest.pidServidorDedicado);

  
    if (kill(clientRequest.pidServidorDedicado, SIGUSR1) != 0) {
        so_error("C5.1","");
        exit(1);
    }

    so_success("C5.1","");

    so_debug(">");
}

/**
 * @brief  c5_2_EsperaRespostaServidorETermina      Ler a descrição da tarefa C5.2 no enunciado
 */
void c5_2_EsperaRespostaServidorETermina() {
    so_debug("<");

    // Espera por um sinal (ex: SIGUSR1 do Servidor Dedicado)
    pause();  // A função pause() faz o processo aguardar até que um sinal seja recebido.

    // Assim que o sinal for recebido, registra sucesso e termina o Cliente
    so_success("C5.2", "Resposta do Servidor Dedicado recebida. Cliente terminado.");

    // O processo termina automaticamente após a execução da função
    // O sistema operacional encerra o processo ao retornar da função main ou equivalente
    exit(0);

    so_debug(">");
}

/**
 * @brief  c6_TrataSigusr1      Ler a descrição da tarefa C6 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 * @param  siginfo (I) informação sobre o sinal
 * @param  context (I) contexto em que o sinal foi chamado
 */
void c6_TrataSigusr1(int sinalRecebido, siginfo_t *siginfo, void *context) {
    so_debug("< [@param sinalRecebido:%d, siginfo:%p, context:%p]", sinalRecebido, siginfo, context);

    so_success("C6", "Check-in concluído com sucesso pelo Servidor Dedicado %d", siginfo->si_pid);

    so_debug(">");
}

/**
 * @brief  c7_TrataSighup      Ler a descrição da tarefa C7 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 * @param  siginfo (I) informação sobre o sinal
 */
void c7_TrataSighup(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("C7", "Estacionamento terminado");

    // O Cliente deve ser encerrado após o SIGHUP
    exit(0); // Exit com código 0 para indicar sucesso
    

    so_debug(">");
}

/**
 * @brief  c8_TrataCtrlC      Ler a descrição da tarefa c8 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void c8_TrataCtrlC(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    if(sinalRecebido = SIGINT){
    // Exibe mensagem de sucesso e encerra o Cliente
    so_success("C8", "Cliente: Shutdown");
    c5_EncerraCliente();
    }
    
    so_debug(">");
}

/**
 * @brief  c9_TrataAlarme      Ler a descrição da tarefa c9 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void c9_TrataAlarme(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_error("C9", "Cliente: Timeout"); //mensagem de erro
    exit(0); //termina o cliente

    so_debug(">");
}
