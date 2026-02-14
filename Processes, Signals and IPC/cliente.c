/****************************************************************************************
 ** ISCTE-IUL: Trabalho prático 3 de Sistemas Operativos 2024/2025, Enunciado Versão 1+
 **
 ** Aluno: Nº:122660       Nome: Mariana Passos
 ** Nome do Módulo: cliente.c
 ** Descrição/Explicação do Módulo:
 **
 **
 ***************************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "defines.h"

/*** Variáveis Globais ***/
int msgId = -1;                         // Variável que tem o ID da Message Queue
MsgContent clientRequest;               // Pedido enviado do Cliente para o Servidor
int recebeuRespostaServidor = FALSE;    // Variável que determina se o Cliente já recebeu uma resposta do Servidor

/**
 * @brief Processamento do processo Cliente.
 *        OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 */
int main () {
    so_debug("<");

    // c1_IniciaCliente:
    c1_1_GetMsgQueue(IPC_KEY, &msgId);
    c1_2_ArmaSinaisCliente();

    // c2_CheckinCliente:
    c2_1_InputEstacionamento(&clientRequest);
    c2_2_EscrevePedido(msgId, clientRequest);

    c3_ProgramaAlarme(MAX_ESPERA);

    // c4_AguardaRespostaServidor:
    c4_1_EsperaRespostaServidor(msgId, &clientRequest);
    c4_2_DesligaAlarme();

    c5_MainCliente(msgId, &clientRequest);

    so_error("Cliente", "O programa nunca deveria ter chegado a este ponto!");
    so_debug(">");
    return 0;
}

/**
 * @brief c1_1_GetMsgQueue Ler a descrição da tarefa C1.1 no enunciado
 * @param ipcKey (I) Identificador de IPC a ser usada para o projeto
 * @param pmsgId (O) identificador aberto de IPC
 */
void c1_1_GetMsgQueue(key_t ipcKey, int *pmsgId) {
    so_debug("< [@param ipcKey:0x0%x]", ipcKey);


  // Abre a fila de mensagens existente (0666 para permissões)
    *pmsgId = msgget(ipcKey, 0666);
    if (*pmsgId == -1) {
        so_error("C1.1", "Erro ao abrir a fila de mensagens");
        exit(EXIT_FAILURE);  // Termina o Cliente como especificado
    }

    so_success("C1.1", "Message queue aberta com ID: %d", *pmsgId);

    so_debug("> [@return *pmsgId:%d]", *pmsgId);
}

/**
 * @brief c1_2_ArmaSinaisCliente Ler a descrição da tarefa C1.2 no enunciado
 */
void c1_2_ArmaSinaisCliente() {
    so_debug("<");

    if (signal(SIGINT, c6_TrataCtrlC) == SIG_ERR || signal(SIGALRM, c7_TrataAlarme) == SIG_ERR ) {
        so_error("C1.2", "");     //a função signal() arma os três sinais, chamando as funções de cada sinal e é verificado se nenhum sinal falhou             //se pelo menos o valor de uma operação der SIG_ERR, dá erro
        exit(0);                
    } else {
        so_success("C1.2", "");
        //return 0;
    }

    so_debug(">");
}

/**
 * @brief c2_1_InputEstacionamento Ler a descrição da tarefa C2.1 no enunciado
 * @param pclientRequest (O) pedido a ser enviado por este Cliente ao Servidor
 */
void c2_1_InputEstacionamento(MsgContent *pclientRequest) {
    so_debug("<");

printf("Park-IUL: Check-in Viatura\n");
    printf("----------------------------\n");

 printf("Introduza a matrícula da viatura: ");
    scanf("%s", pclientRequest->msgData.est.viatura.matricula);

    printf("Introduza o país da viatura: ");
    scanf("%s", pclientRequest->msgData.est.viatura.pais);

    printf("Introduza a categoria da viatura: ");
    scanf(" %c", &pclientRequest->msgData.est.viatura.categoria);

    printf("Introduza o nome do condutor: ");
    scanf(" %[^\n]", pclientRequest->msgData.est.viatura.nomeCondutor);

    // Preenche os PIDs
    pclientRequest->msgData.est.pidCliente = getpid();
    pclientRequest->msgData.est.pidServidorDedicado = -1;

    // Define o tipo da mensagem
    pclientRequest->msgType = MSGTYPE_LOGIN;

    so_success("C2.1", "%s %s %c %s %d %d",
               pclientRequest->msgData.est.viatura.matricula,
               pclientRequest->msgData.est.viatura.pais,
               pclientRequest->msgData.est.viatura.categoria,
               pclientRequest->msgData.est.viatura.nomeCondutor,
               pclientRequest->msgData.est.pidCliente,
               pclientRequest->msgData.est.pidServidorDedicado);

    so_debug("> [*pclientRequest:[%ld:%d:%s:%s:%c:%s:%d:%d:%s]]", pclientRequest->msgType, pclientRequest->msgData.status, pclientRequest->msgData.est.viatura.matricula, pclientRequest->msgData.est.viatura.pais, pclientRequest->msgData.est.viatura.categoria, pclientRequest->msgData.est.viatura.nomeCondutor, pclientRequest->msgData.est.pidCliente, pclientRequest->msgData.est.pidServidorDedicado, pclientRequest->msgData.infoTarifa);
}

/**
 * @brief c2_2_EscrevePedido Ler a descrição da tarefa C2.2 no enunciado
 * @param msgId (I) identificador aberto de IPC
 * @param clientRequest (I) pedido a ser enviado por este Cliente ao Servidor
 */
void c2_2_EscrevePedido(int msgId, MsgContent clientRequest) {
    so_debug("< [@param msgId:%d, clientRequest:[%ld:%d:%s:%s:%c:%s:%d:%d:%s]]", msgId, clientRequest.msgType, clientRequest.msgData.status, clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado, clientRequest.msgData.infoTarifa);

    // Send the message to the server
    if (msgsnd(msgId, &clientRequest, sizeof(clientRequest.msgData), 0) == -1) {
        so_error("C2.2", "Erro ao enviar mensagem para o servidor");
        exit(0);  // Terminate on error as specified
    }

    so_success("C2.2", "");

    so_debug(">");
}

/**
 * @brief c3_ProgramaAlarme Ler a descrição da tarefa C3 no enunciado
 * @param segundos (I) número de segundos a programar no alarme
 */
void c3_ProgramaAlarme(int segundos) {
    so_debug("< [@param segundos:%d]", segundos);

    signal(SIGINT, c7_TrataAlarme);    //arma o sinal SIGINT
    alarm(segundos);                      //define o tempo após o qual o sinal deve ser enviado
    so_success("C3", "Espera resposta em %d segundos", segundos);

    so_debug(">");
}

/**
 * @brief c4_1_EsperaRespostaServidor Ler a descrição da tarefa C4.1 no enunciado
 * @param msgId (I) identificador aberto de IPC
 * @param pclientRequest (O) mensagem enviada por um Servidor Dedicado
 */
void c4_1_EsperaRespostaServidor(int msgId, MsgContent *pclientRequest) {
    so_debug("< [@param msgId:%d]", msgId);

    // Tenta ler uma mensagem com tipo igual ao PID do processo
    ssize_t result = msgrcv(msgId, pclientRequest, sizeof(MsgContent) - sizeof(long), getpid(), 0);
    
    if (result == -1) {
        // Erro ao ler mensagem
        so_error("C4.1", "Erro na leitura da mensagem do servidor");
        exit(0);
    }

    // Verifica o status retornado
    if (pclientRequest->msgData.status == CLIENT_ACCEPTED) {
        so_success("C4.1", "Check-in realizado com sucesso");
    } else if (pclientRequest->msgData.status == ESTACIONAMENTO_TERMINADO) {
        so_success("C4.1", "Não é possível estacionar");
        exit(0); // Termina o cliente
    } else {
        so_error("C4.1", "Status inesperado recebido do servidor");
        exit(0); // Trata como erro
    }
    so_debug("> [*pclientRequest:[%ld:%d:%s:%s:%c:%s:%d:%d:%s]]", pclientRequest->msgType, pclientRequest->msgData.status, pclientRequest->msgData.est.viatura.matricula, pclientRequest->msgData.est.viatura.pais, pclientRequest->msgData.est.viatura.categoria, pclientRequest->msgData.est.viatura.nomeCondutor, pclientRequest->msgData.est.pidCliente, pclientRequest->msgData.est.pidServidorDedicado, pclientRequest->msgData.infoTarifa);
}

/**
 * @brief c4_2_DesligaAlarme Ler a descrição da tarefa C4.2 no enunciado
 */
void c4_2_DesligaAlarme() {
    so_debug("<");

    alarm(0);
    so_success("C4.2", "Desliguei alarme");

    so_debug(">");
}

/**
 * @brief c5_MainCliente Ler a descrição da tarefa C5 no enunciado
 * @param msgId (I) identificador aberto de IPC
 * @param pclientRequest (O) mensagem enviada por um Servidor Dedicado
 */
void c5_MainCliente(int msgId, MsgContent *pclientRequest) {
    so_debug("< [@param msgId:%d]", msgId);

    while (1) {
        if (msgrcv(msgId, pclientRequest, sizeof(MsgContent) - sizeof(long), getpid(), 0) == -1) {
            so_error("C5", "");
            exit(0);
        }

        if (pclientRequest->msgData.status == INFO_TARIFA) {
            so_success("C5", "%s", pclientRequest->msgData.infoTarifa);
        } else if (pclientRequest->msgData.status == ESTACIONAMENTO_TERMINADO) {
            so_success("C5", "Estacionamento terminado");
            exit(0);
        }
    }

    so_debug("> [*pclientRequest:[%ld:%d:%s:%s:%c:%s:%d:%d:%s]]", pclientRequest->msgType, pclientRequest->msgData.status, pclientRequest->msgData.est.viatura.matricula, pclientRequest->msgData.est.viatura.pais, pclientRequest->msgData.est.viatura.categoria, pclientRequest->msgData.est.viatura.nomeCondutor, pclientRequest->msgData.est.pidCliente, pclientRequest->msgData.est.pidServidorDedicado, pclientRequest->msgData.infoTarifa);
}

/**
 * @brief  c6_TrataCtrlC Ler a descrição da tarefa C6 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void c6_TrataCtrlC(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d, msgId:%d, clientRequest:[%ld:%d:%s:%s:%c:%s:%d:%d:%s]]", sinalRecebido, msgId, clientRequest.msgType, clientRequest.msgData.status, clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado, clientRequest.msgData.infoTarifa);

    // Preenche os dados da mensagem para encerrar estacionamento
    clientRequest.msgType = clientRequest.msgData.est.pidServidorDedicado;  // ou outro valor esperado pelo servidor
    clientRequest.msgData.status = TERMINA_ESTACIONAMENTO;

    // Envia a mensagem
    if (msgsnd(msgId, &clientRequest, sizeof(MsgContent) - sizeof(long), 0) == -1) {
        so_error("C6", "Erro ao enviar mensagem de shutdown");
        exit(0);
    }

    // Mensagem enviada com sucesso
    so_success("C6", "Cliente: Shutdown");

    so_debug(">");
}

/**
 * @brief  c7_TrataAlarme Ler a descrição da tarefa C7 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void c7_TrataAlarme(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_error("C7", "Cliente: Timeout");        //caso receba o sinal SIGALRM
    exit(0);

    so_debug(">");
}