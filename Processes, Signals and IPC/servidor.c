/****************************************************************************************
 ** ISCTE-IUL: Trabalho prático 3 de Sistemas Operativos 2024/2025, Enunciado Versão 1+
 **
 ** Aluno: Nº:      Nome:
 ** Nome do Módulo: servidor.c
 ** Descrição/Explicação do Módulo:
 **
 **
 ***************************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "defines.h"

/*** Variáveis Globais ***/
int nrServidoresDedicados = 0;          // Número de servidores dedicados (só faz sentido no processo Servidor)
int shmId = -1;                         // Variável que tem o ID da Shared Memory
int msgId = -1;                         // Variável que tem o ID da Message Queue
int semId = -1;                         // Variável que tem o ID do Grupo de Semáforos
MsgContent clientRequest;               // Pedido enviado do Cliente para o Servidor
Estacionamento *lugaresEstacionamento = NULL;   // Array de Lugares de Estacionamento do parque
int dimensaoMaximaParque;               // Dimensão Máxima do parque (BD), recebida por argumento do programa
int indexClienteBD = -1;                // Índice do cliente que fez o pedido ao servidor/servidor dedicado na BD
long posicaoLogfile = -1;               // Posição no ficheiro Logfile para escrever o log da entrada corrente
LogItem logItem;                        // Informação da entrada corrente a escrever no logfile
int shmIdFACE = -1;                     // Variável que tem o ID da Shared Memory da entidade externa FACE
int semIdFACE = -1;                     // Variável que tem o ID do Grupo de Semáforos da entidade externa FACE
int *tarifaAtual = NULL;                // Inteiro definido pela entidade externa FACE com a tarifa atual do parque

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
    return 0;
}

/**
 * @brief s1_iniciaServidor Ler a descrição da tarefa S1 no enunciado.
 *        OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 * @param argc (I) número de Strings do array argv
 * @param argv (I) array de lugares de estacionamento que irá servir de BD
 */
void s1_IniciaServidor(int argc, char *argv[]) {
    so_debug("<");

    s1_1_ObtemDimensaoParque(argc, argv, &dimensaoMaximaParque);
    s1_2_ArmaSinaisServidor();
    s1_3_CriaMsgQueue(IPC_KEY, &msgId);
    s1_4_CriaGrupoSemaforos(IPC_KEY, &semId);
    s1_5_CriaBD(IPC_KEY, &shmId, dimensaoMaximaParque, &lugaresEstacionamento);

    so_debug(">");
}

/**
 * @brief s1_1_ObtemDimensaoParque Ler a descrição da tarefa S1.1 no enunciado
 * @param argc (I) número de Strings do array argv
 * @param argv (I) array de lugares de estacionamento que irá servir de BD
 * @param pdimensaoMaximaParque (O) número máximo de lugares do parque, especificado pelo utilizador
 */
void s1_1_ObtemDimensaoParque(int argc, char *argv[], int *pdimensaoMaximaParque) {
    so_debug("< [@param argc:%d, argv:%p]", argc, argv);

    // Verify if the correct number of arguments was provided
    if (argc != 2) {
        so_error("S1.1", "Número de argumentos inválido. Uso: ./servidor <dimensaoMaximaParque>");
        exit(0);
    }

    // Convert the argument to integer with proper validation
    char *endptr;
    long dimensao = strtol(argv[1], &endptr, 10);
    
    // Check if conversion was successful and value is positive
    if (*endptr != '\0' || dimensao <= 0) {
        so_error("S1.1", "Dimensão do parque deve ser um número positivo");
        exit(0);
    }

    *pdimensaoMaximaParque = (int)dimensao;

    so_success("S1.1", "%d", *pdimensaoMaximaParque);

    so_debug("> [@return *pdimensaoMaximaParque:%d]", *pdimensaoMaximaParque);
}

/**
 * @brief s1_2_ArmaSinaisServidor Ler a descrição da tarefa S1.2 no enunciado
 */
void s1_2_ArmaSinaisServidor() {
    so_debug("<");

    // Configura os sinais SIGUSR2 e SIGINT para o Servidor Dedicado.
    if (signal(SIGCHLD, s5_TrataTerminouServidorDedicado) == SIG_ERR || 
        signal(SIGINT, s3_TrataCtrlC) == SIG_ERR) {
        so_error("S1.2", "Falha ao armar sinais");
        exit(0);
    }

    so_success("S1.2", "");

    so_debug(">");
}

/**
 * @brief s1_3_CriaMsgQueue Ler a descrição da tarefa s1.3 no enunciado
 * @param ipcKey (I) Identificador de IPC a ser usada para o projeto
 * @param pmsgId (O) identificador aberto de IPC
 */
void s1_3_CriaMsgQueue(key_t ipcKey, int *pmsgId) {
    so_debug("< [@param ipcKey:0x0%x]", ipcKey);

    // First try to get the existing message queue
    *pmsgId = msgget(ipcKey, 0);
    
    // If it exists, remove it
    if (*pmsgId != -1) {
        if (msgctl(*pmsgId, IPC_RMID, NULL) == -1) {
            so_error("S1.3", "Failed to remove existing message queue");
            exit(0);
        }
        so_debug("Removed existing message queue");
    }

    // Create new message queue with read/write permissions for owner
    *pmsgId = msgget(ipcKey, IPC_CREAT | IPC_EXCL | 0600);
    if (*pmsgId == -1) {
        so_error("S1.3", "Failed to create new message queue");
        exit(0);
    }

    so_success("S1.3", "Message queue created successfully");

    so_debug("> [@return *pmsgId:%d]", *pmsgId);
}

/**
 * @brief s1_4_CriaGrupoSemaforos Ler a descrição da tarefa s1.4 no enunciado
 * @param ipcKey (I) Identificador de IPC a ser usada para o projeto
 * @param psemId (O) identificador aberto de IPC
 */
 void s1_4_CriaGrupoSemaforos(key_t ipcKey, int *psemId) {
    so_debug("< [@param ipcKey:0x0%x]", ipcKey);

    // Remove existing semaphore if it exists
    if ((*psemId = semget(ipcKey, 0, 0)) != -1) {
        if (semctl(*psemId, 0, IPC_RMID) == -1) {
            so_error("S1.4", "Falha ao remover SEM existente");
            exit(0);
        }
    }

    // Create new semaphore set with 4 semaphores
    *psemId = semget(ipcKey, 4, IPC_CREAT | IPC_EXCL | 0666);
    if (*psemId == -1) {
        so_error("S1.4", "Falha ao criar SEM");
        exit(0);
    }

    // Initialize semaphores
    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } arg;

    // SEM_MUTEX_BD (index 0) - initialized to 1 (mutex)
    arg.val = 1;
    if (semctl(*psemId, SEM_MUTEX_BD, SETVAL, arg) == -1) {
        so_error("S1.4", "Falha ao inicializar SEM_MUTEX_BD");
        exit(0);
    }

    // SEM_MUTEX_LOGFILE (index 1) - initialized to 1 (mutex)
    arg.val = 1;
    if (semctl(*psemId, SEM_MUTEX_LOGFILE, SETVAL, arg) == -1) {
        so_error("S1.4", "Falha ao inicializar SEM_MUTEX_LOGFILE");
        exit(0);
    }

    // SEM_SRV_DEDICADOS (index 2) - initialized to 0 (barrier)
    arg.val = 0;
    if (semctl(*psemId, SEM_SRV_DEDICADOS, SETVAL, arg) == -1) {
        so_error("S1.4", "Falha ao inicializar SEM_SRV_DEDICADOS");
        exit(0);
    }

    // SEM_LUGARES_PARQUE (index 3) - initialized to dimensaoMaximaParque
    arg.val = dimensaoMaximaParque;
    if (semctl(*psemId, SEM_LUGARES_PARQUE, SETVAL, arg) == -1) {
        so_error("S1.4", "Falha ao inicializar SEM_LUGARES_PARQUE");
        exit(0);
    }

    so_success("S1.4", "");

    so_debug("> [@return *psemId:%d]", *psemId);
}

/**
 * @brief s1_5_CriaBD Ler a descrição da tarefa S1.5 no enunciado
 * @param ipcKey (I) Identificador de IPC a ser usada para o projeto
 * @param pshmId (O) identificador aberto de IPC
 * @param dimensaoMaximaParque (I) número máximo de lugares do parque, especificado pelo utilizador
 * @param plugaresEstacionamento (O) array de lugares de estacionamento que irá servir de BD
 */
void s1_5_CriaBD(key_t ipcKey, int *pshmId, int dimensaoMaximaParque, Estacionamento **plugaresEstacionamento) {
    so_debug("< [@param ipcKey:0x0%x, dimensaoMaximaParque:%d]", ipcKey, dimensaoMaximaParque);

    int created_new = 0;
    size_t size = dimensaoMaximaParque * sizeof(Estacionamento);

    // Try to get existing shared memory
    *pshmId = shmget(ipcKey, size, 0666);
    
    if (*pshmId == -1) {
        // Create new shared memory if it doesn't exist
        *pshmId = shmget(ipcKey, size, IPC_CREAT | IPC_EXCL | 0666);
        if (*pshmId == -1) {
            so_error("S1.5", "Falha ao criar SHM");
            exit(0);
        }
        created_new = 1;
    }

    // Attach to shared memory
    *plugaresEstacionamento = (Estacionamento *)shmat(*pshmId, NULL, 0);
    if (*plugaresEstacionamento == (void *)-1) {
        so_error("S1.5", "Falha ao anexar SHM");
        exit(0);
    }

    // Initialize if newly created
    if (created_new) {
        for (int i = 0; i < dimensaoMaximaParque; i++) {
            (*plugaresEstacionamento)[i].pidCliente = DISPONIVEL;
            (*plugaresEstacionamento)[i].pidServidorDedicado = -1;
            // Initialize other fields if needed
            memset((*plugaresEstacionamento)[i].viatura.matricula, 0, sizeof((*plugaresEstacionamento)[i].viatura.matricula));
            memset((*plugaresEstacionamento)[i].viatura.pais, 0, sizeof((*plugaresEstacionamento)[i].viatura.pais));
            (*plugaresEstacionamento)[i].viatura.categoria = '\0';
            memset((*plugaresEstacionamento)[i].viatura.nomeCondutor, 0, sizeof((*plugaresEstacionamento)[i].viatura.nomeCondutor));
        }
    }

    so_success("S1.5", "");

    so_debug("> [@return *pshmId:%d, *plugaresEstacionamento:%p]", *pshmId, *plugaresEstacionamento);
}

/**
 * @brief s2_MainServidor Ler a descrição da tarefa S2 no enunciado.
 *        OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO
 */
void s2_MainServidor() {
    so_debug("<");

    while (TRUE) {
        s2_1_LePedidoCliente(msgId, &clientRequest);
        s2_2_CriaServidorDedicado(&nrServidoresDedicados);
    }

    so_debug(">");
}

/**
 * @brief s2_1_LePedidoCliente Ler a descrição da tarefa S2.1 no enunciado.
 * @param msgId (I) identificador aberto de IPC
 * @param pclientRequest (O) pedido recebido, enviado por um Cliente
 */
void s2_1_LePedidoCliente(int msgId, MsgContent *pclientRequest) {
    so_debug("< [@param msgId:%d]", msgId);

  ssize_t bytesRead = msgrcv(msgId, pclientRequest, sizeof(pclientRequest->msgData), MSGTYPE_LOGIN, 0);
    if (bytesRead == -1) {
        if (errno == EINTR) {
            // Signal received, continue processing
            so_debug("> [Signal received, continuing]");
            return;
        }
        so_error("S2.1", "Erro ao ler mensagem");
        s4_EncerraServidor();
    }

    so_success("S2.1", "%s %d", pclientRequest->msgData.est.viatura.matricula, pclientRequest->msgData.est.pidCliente);


    //sleep(10);  // TEMPORÁRIO, os alunos deverão comentar este statement apenas
                // depois de terem a certeza que não terão uma espera ativa

    so_debug("> [@return *pclientRequest:[%ld:%d:%s:%s:%c:%s:%d:%d:%s]]", pclientRequest->msgType, pclientRequest->msgData.status, pclientRequest->msgData.est.viatura.matricula, pclientRequest->msgData.est.viatura.pais, pclientRequest->msgData.est.viatura.categoria, pclientRequest->msgData.est.viatura.nomeCondutor, pclientRequest->msgData.est.pidCliente, pclientRequest->msgData.est.pidServidorDedicado, pclientRequest->msgData.infoTarifa);
}

/**
 * @brief s2_2_CriaServidorDedicado Ler a descrição da tarefa S2.2 no enunciado
 * @param pnrServidoresDedicados (O) número de Servidores Dedicados que foram criados até então
 */
void s2_2_CriaServidorDedicado(int *pnrServidoresDedicados) {
    so_debug("<");

   pid_t pid = fork(); // Create a new process

    if (pid == -1) {
        // Error handling for fork failure
        so_error("S2.2", "Erro ao criar processo Servidor Dedicado");
        s4_EncerraServidor(); // Added missing system call
    } else if (pid == 0) {
        // Child process (Servidor Dedicado)
        so_success("S2.2", "SD: Nasci com PID %d", getpid());
        sd7_MainServidorDedicado();
    } else {
        // Parent process (Servidor)
        (*pnrServidoresDedicados)++;
        so_success("S2.2", "Servidor: Iniciei SD %d", pid);
    }

    so_debug("> [@return *pnrServidoresDedicados:%d", *pnrServidoresDedicados);
}

/**
 * @brief s3_TrataCtrlC Ler a descrição da tarefa S3 no enunciado
 * @param sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void s3_TrataCtrlC(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

   so_success("S3", "Servidor: Start Shutdown");
    s4_EncerraServidor();


    so_debug(">");
}

/**
 * @brief s4_EncerraServidor Ler a descrição da tarefa S4 no enunciado
 *        OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO
 */
void s4_EncerraServidor() {
    so_debug("<");

    s4_1_TerminaServidoresDedicados(lugaresEstacionamento, dimensaoMaximaParque);
    s4_2_AguardaFimServidoresDedicados(nrServidoresDedicados);
    s4_3_ApagaElementosIPCeTermina(shmId, semId, msgId);

    so_debug(">");
}

/**
 * @brief s4_1_TerminaServidoresDedicados Ler a descrição da tarefa S4.1 no enunciado
 * @param lugaresEstacionamento (I) array de lugares de estacionamento que irá servir de BD
 * @param dimensaoMaximaParque (I) número máximo de lugares do parque, especificado pelo utilizador
 */
void s4_1_TerminaServidoresDedicados(Estacionamento *lugaresEstacionamento, int dimensaoMaximaParque) {
    so_debug("< [@param lugaresEstacionamento:%p, dimensaoMaximaParque:%d]", lugaresEstacionamento, dimensaoMaximaParque);

     // Enter critical section - lock the semaphore for BD access
    struct sembuf sem_op;
    sem_op.sem_num = SEM_MUTEX_BD;  // Semaphore index for BD mutex
    sem_op.sem_op = SEM_DOWN;       // Decrement (lock)
    sem_op.sem_flg = 0;
    if (semop(semId, &sem_op, 1) == -1) {
        so_error("S4.1", "Failed to lock BD semaphore");
        return;
    }

    // Send SIGUSR2 to all active dedicated servers
    for (int i = 0; i < dimensaoMaximaParque; i++) {
        int pidSD = lugaresEstacionamento[i].pidServidorDedicado;
        if (pidSD > 0) {
            kill(pidSD, SIGUSR2);
        }
    }

    // Exit critical section - unlock the semaphore
    sem_op.sem_op = SEM_UP;  // Increment (unlock)
    if (semop(semId, &sem_op, 1) == -1) {
        so_error("S4.1", "Failed to unlock BD semaphore");
    }

    so_success("S4.1", "Todos os sinais SIGUSR2 enviados aos Servidores Dedicados");

    so_debug(">");
}

/**
 * @brief s4_2_AguardaFimServidoresDedicados Aguarda que TODOS os Servidores Dedicados terminem (usando um algoritmo do tipo “barreira” usando o 
semáforo SEM_SRV_DEDICADOS como ensinado na aula SO-T08), para garantir que não haverá mais 
alterações à lista de estacionamentos. Depois de todos eles terminarem, dá so_success. 
 * @param nrServidoresDedicados (I) número de Servidores Dedicados que foram criados até então
 */
void s4_2_AguardaFimServidoresDedicados(int nrServidoresDedicados) {
    so_debug("< [@param nrServidoresDedicados:%d]", nrServidoresDedicados);


   struct sembuf op = {SEM_SRV_DEDICADOS, -nrServidoresDedicados, 0};
    if (semop(semId, &op, 1) == -1) {
        so_error("S4.2", "Erro ao aguardar servidores dedicados");
    } else {
        so_success("S4.2", "Todos os servidores dedicados terminaram");
    }


    so_debug(">");
}

/**
 * @brief s4_3_ApagaElementosIPCeTermina Ler a descrição da tarefa S4.2 no enunciado
 * @param shmId (I) identificador aberto de IPC
 * @param semId (I) identificador aberto de IPC
 * @param msgId (I) identificador aberto de IPC
 */
void s4_3_ApagaElementosIPCeTermina(int shmId, int semId, int msgId) {
    so_debug("< [@param shmId:%d, semId:%d, msgId:%d]", shmId, semId, msgId);

    // Remove shared memory
    if (shmId != -1) {
        if (shmctl(shmId, IPC_RMID, NULL) == -1) {
            so_error("S4.3", "Erro ao remover shared memory");
        }
    }

    // Remove semaphore
    if (semId != -1) {
        if (semctl(semId, 0, IPC_RMID) == -1) {
            so_error("S4.3", "Erro ao remover semáforos");
        }
    }

    // Remove message queue
    if (msgId != -1) {
        if (msgctl(msgId, IPC_RMID, NULL) == -1) {
            so_error("S4.3", "Erro ao remover message queue");
        }
    }

    so_success("S4.3", "Servidor: End Shutdown");
    exit(0);


    so_debug(">");
}

/**
 * @brief s5_TrataTerminouServidorDedicado Ler a descrição da tarefa S5 no enunciado
 * @param sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void s5_TrataTerminouServidorDedicado(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    pid_t pid = wait(NULL);
    if (pid > 0) {
        nrServidoresDedicados--;  // Atualiza o número de SDs ativos
        so_success("S5", "Servidor: Confirmo que terminou o SD %d", pid);
    } else {
        so_error("S5", "Erro ao esperar pelo processo filho");
    }

   
   
    so_debug("> [@return nrServidoresDedicados:%d", nrServidoresDedicados);
}

/**
 * @brief sd7_ServidorDedicado Ler a descrição da tarefa SD7 no enunciado
 *        OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 */
void sd7_MainServidorDedicado() {
    so_debug("<");

    // sd7_IniciaServidorDedicado:
    sd7_1_ArmaSinaisServidorDedicado();
    sd7_2_ValidaPidCliente(clientRequest);
    sd7_3_GetShmFACE(KEY_FACE, &shmIdFACE);
    sd7_4_GetSemFACE(KEY_FACE, &semIdFACE);
    sd7_5_ProcuraLugarDisponivelBD(semId, clientRequest, lugaresEstacionamento, dimensaoMaximaParque, &indexClienteBD);

    // sd8_ValidaPedidoCliente:
    sd8_1_ValidaMatricula(clientRequest);
    sd8_2_ValidaPais(clientRequest);
    sd8_3_ValidaCategoria(clientRequest);
    sd8_4_ValidaNomeCondutor(clientRequest);

    // sd9_EntradaCliente:
    sd9_1_AdormeceTempoRandom();
    sd9_2_EnviaSucessoAoCliente(msgId, clientRequest);
    sd9_3_EscreveLogEntradaViatura(FILE_LOGFILE, clientRequest, &posicaoLogfile, &logItem);

    // sd10_AcompanhaCliente:
    sd10_1_AguardaCheckout(msgId);
    sd10_2_EscreveLogSaidaViatura(FILE_LOGFILE, posicaoLogfile, logItem);

    sd11_EncerraServidorDedicado();

    so_error("Servidor Dedicado", "O programa nunca deveria ter chegado a este ponto!");
    so_debug(">");
}

/**
 * @brief sd7_1_ArmaSinaisServidorDedicado Ler a descrição da tarefa SD7.1 no enunciado
 */
void sd7_1_ArmaSinaisServidorDedicado() {
    so_debug("<");

    // Ignorar SIGINT (CTRL+C)
    if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
        so_error("SD7.1", "Erro ao armar sinal SIGINT");
        exit(1);
    }

    // Armar SIGUSR2
    if (signal(SIGUSR2, sd12_TrataSigusr2) == SIG_ERR) {
        so_error("SD7.1", "Erro ao armar sinal SIGUSR2");
        exit(1);
    }

    // [IMPORTANTE] Provavelmente também esperam armar o sinal SIGALRM aqui:
    if (signal(SIGALRM, sd10_1_1_TrataAlarme) == SIG_ERR) {
        so_error("SD7.1", "Erro ao armar sinal SIGALRM");
        exit(1);
    }

    so_success("SD7.1", "");

    so_debug(">");
}

/**
 * @brief sd7_2_ValidaPidCliente Ler a descrição da tarefa SD7.2 no enunciado
 * @param clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd7_2_ValidaPidCliente(MsgContent clientRequest) {
    so_debug("< [@param clientRequest:[%ld:%d:%s:%s:%c:%s:%d:%d:%s]]", clientRequest.msgType, clientRequest.msgData.status, clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado, clientRequest.msgData.infoTarifa);

    if (clientRequest.msgData.est.pidCliente <= 0) {
        so_error("SD7.2", "PID do cliente inválido");
        exit(1);
    }

    so_success("SD7.2", "");
    so_debug(">");
}

/**
 * @brief sd7_3_GetShmFACE Ler a descrição da tarefa SD7.3 no enunciado
 * @param ipcKeyFace (I) Identificador de IPC a ser definida pela FACE
 * @param pshmIdFACE (O) identificador aberto de IPC da FACE
 */
void sd7_3_GetShmFACE(key_t ipcKeyFace, int *pshmIdFACE) {
    so_debug("< [@param ipcKeyFace:0x0%x]", ipcKeyFace);

    // Get existing shared memory segment
    *pshmIdFACE = shmget(ipcKeyFace, sizeof(int), 0);
    if (*pshmIdFACE == -1) {
        so_error("SD7.3", "Erro ao obter ID da SHM da FACE");
        exit(1);
    }

    // Attach to the shared memory segment
    tarifaAtual = (int *)shmat(*pshmIdFACE, NULL, 0);
    if (tarifaAtual == (int *)-1) {
        so_error("SD7.3", "Erro ao anexar à SHM da FACE");
        exit(1);
    }

    so_success("SD7.3", "");
    so_debug("> [@return *pshmIdFACE:%d, tarifaAtual:%p]", *pshmIdFACE, tarifaAtual);
}

/**
 * @brief sd7_4_GetSemFACE Ler a descrição da tarefa SD7.4 no enunciado
 * @param ipcKeyFace (I) Identificador de IPC a ser definida pela FACE
 * @param psemIdFACE (O) identificador aberto de IPC da FACE
 */
void sd7_4_GetSemFACE(key_t ipcKeyFace, int *psemIdFACE) {
    so_debug("< [@param ipcKeyFace:0x0%x]", ipcKeyFace);

    *psemIdFACE = semget(ipcKeyFace, 1, 0);  // tenta obter o semáforo existente, sem criar

    if (*psemIdFACE == -1) {
        so_error("SD7.4", "Erro ao ligar ao SEM da FACE");
        exit(1);
    }

    so_success("SD7.4", "");

    so_debug("> [@return *psemIdFACE:%d]", *psemIdFACE);
}

/**
 * @brief sd7_5_ProcuraLugarDisponivelBD Ler a descrição da tarefa SD7.5 no enunciado
 * @param semId (I) identificador aberto de IPC
 * @param clientRequest (I) pedido recebido, enviado por um Cliente
 * @param lugaresEstacionamento (I) array de lugares de estacionamento que irá servir de BD
 * @param dimensaoMaximaParque (I) número máximo de lugares do parque, especificado pelo utilizador
 * @param pindexClienteBD (O) índice do lugar correspondente a este pedido na BD (>= 0), ou -1 se não houve nenhum lugar disponível
 */
void sd7_5_ProcuraLugarDisponivelBD(int semId, MsgContent clientRequest, Estacionamento *lugaresEstacionamento, int dimensaoMaximaParque, int *pindexClienteBD) {
    so_debug("< [@param semId:%d, clientRequest:[%ld:%d:%s:%s:%c:%s:%d:%d:%s], lugaresEstacionamento:%p, dimensaoMaximaParque:%d]", semId, clientRequest.msgType, clientRequest.msgData.status, clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado, clientRequest.msgData.infoTarifa, lugaresEstacionamento, dimensaoMaximaParque);

    // Wait for a parking spot to become available
    struct sembuf sem_op;
    sem_op.sem_num = SEM_LUGARES_PARQUE;  // Semaphore for available spots
    sem_op.sem_op = SEM_DOWN;             // Decrement (wait)
    sem_op.sem_flg = 0;
    if (semop(semId, &sem_op, 1) == -1) {
        so_error("SD7.5", "Erro ao esperar por lugar disponível");
        exit(1);
    }

    // Enter critical section for BD access
    sem_op.sem_num = SEM_MUTEX_BD;        // BD mutex semaphore
    sem_op.sem_op = SEM_DOWN;             // Decrement (lock)
    if (semop(semId, &sem_op, 1) == -1) {
        so_error("SD7.5", "Erro ao bloquear BD");
        exit(1);
    }

    // Find and reserve an available spot
    *pindexClienteBD = -1;
    for (int i = 0; i < dimensaoMaximaParque; i++) {
        if (lugaresEstacionamento[i].pidCliente == DISPONIVEL) {
            *pindexClienteBD = i;
            
            // Reserve the spot
            memcpy(&lugaresEstacionamento[i].viatura, 
                  &clientRequest.msgData.est.viatura, 
                  sizeof(Viatura));
            lugaresEstacionamento[i].pidCliente = clientRequest.msgData.est.pidCliente;
            lugaresEstacionamento[i].pidServidorDedicado = clientRequest.msgData.est.pidServidorDedicado;

            break;
        }
    }

    // Exit critical section
    sem_op.sem_op = SEM_UP;               // Increment (unlock)
    if (semop(semId, &sem_op, 1) == -1) {
        so_error("SD7.5", "Erro ao desbloquear BD");
        exit(1);
    }

    if (*pindexClienteBD == -1) {
        so_error("SD7.5", "Nenhum lugar disponível após reserva");
        exit(1);
    }

    so_success("SD7.5", "Reservei Lugar: %d", *pindexClienteBD);

    so_debug("> [*pindexClienteBD:%d]", *pindexClienteBD);
}

/**
 * @brief  sd8_1_ValidaMatricula Ler a descrição da tarefa SD8.1 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd8_1_ValidaMatricula(MsgContent clientRequest) {
    so_debug("< [@param clientRequest:[%ld:%d:%s:%s:%c:%s:%d:%d:%s]]", clientRequest.msgType, clientRequest.msgData.status, clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado, clientRequest.msgData.infoTarifa);

   for (int i = 0; clientRequest.msgData.est.viatura.matricula[i] != '\0'; i++) {
        char c = clientRequest.msgData.est.viatura.matricula[i];
        if (!(isupper(c) || isdigit(c))) {
            so_error("SD8.1", "Matrícula inválida");
            sd11_EncerraServidorDedicado();
        }
    }

    so_success("SD8.1", "Matrícula válida");

    so_debug(">");
}

/**
 * @brief  sd8_2_ValidaPais Ler a descrição da tarefa SD8.2 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd8_2_ValidaPais(MsgContent clientRequest) {
    so_debug("< [@param clientRequest:[%ld:%d:%s:%s:%c:%s:%d:%d:%s]]", clientRequest.msgType, clientRequest.msgData.status, clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado, clientRequest.msgData.infoTarifa);

     if (strlen(clientRequest.msgData.est.viatura.pais) != 2) {
        so_error("SD8.2", "País inválido - tamanho incorreto");
        sd11_EncerraServidorDedicado();
    }

    for (int i = 0; i < 2; i++) {
        if (!isupper(clientRequest.msgData.est.viatura.pais[i])) {
            so_error("SD8.2", "País inválido - caracteres não maiúsculos");
            sd11_EncerraServidorDedicado();
        }
    }

    so_success("SD8.2", "País válido");

    so_debug(">");
}

/**
 * @brief  sd8_3_ValidaCategoria Ler a descrição da tarefa SD8.3 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd8_3_ValidaCategoria(MsgContent clientRequest) {
    so_debug("< [@param clientRequest:[%ld:%d:%s:%s:%c:%s:%d:%d:%s]]", clientRequest.msgType, clientRequest.msgData.status, clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado, clientRequest.msgData.infoTarifa);

     char cat = clientRequest.msgData.est.viatura.categoria;
    if (cat != 'P' && cat != 'L' && cat != 'M') {
        so_error("SD8.3", "Categoria inválida");
        sd11_EncerraServidorDedicado();
    }

    so_success("SD8.3", "Categoria válida");

    so_debug(">");
}

/**
 * @brief  sd8_4_ValidaNomeCondutor Ler a descrição da tarefa SD8.4 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd8_4_ValidaNomeCondutor(MsgContent clientRequest) {
    so_debug("< [@param clientRequest:[%ld:%d:%s:%s:%c:%s:%d:%d:%s]]", clientRequest.msgType, clientRequest.msgData.status, clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado, clientRequest.msgData.infoTarifa);

    // Check if name is empty
    if (strlen(clientRequest.msgData.est.viatura.nomeCondutor) == 0) {
        so_error("SD8.4", "Nome do condutor vazio");
        sd11_EncerraServidorDedicado();
        return;  // Ensure we don't continue after error
    }

    // Check against system users
    FILE *fp = fopen(FILE_USERS, "r");
    if (fp == NULL) {
        so_error("SD8.4", "Erro ao abrir ficheiro de utilizadores");
        sd11_EncerraServidorDedicado();
        return;
    }

    char line[256];
    int found = 0;
    const char *search_name = clientRequest.msgData.est.viatura.nomeCondutor;

    while (fgets(line, sizeof(line), fp)) {
        // Parse the line to get the full name (5th field)
        char *name = strtok(line, ":");
        for (int i = 0; i < 4 && name != NULL; i++) {
            name = strtok(NULL, ":");
        }
        
        if (name != NULL && strstr(name, search_name) != NULL) {
            found = 1;
            break;
        }
    }

    fclose(fp);

    if (!found) {
        so_error("SD8.4", "Condutor não encontrado na lista de utilizadores");
        sd11_EncerraServidorDedicado();
        return;
    }

    so_success("SD8.4", "Condutor válido");

    so_debug(">");
}

/**
 * @brief sd9_1_AdormeceTempoRandom Ler a descrição da tarefa SD9.1 no enunciado
 */
void sd9_1_AdormeceTempoRandom() {
    so_debug("<");

    // Inicializa gerador de números aleatórios (só uma vez é ideal)
    static int seed_initialized = 0;
    if (!seed_initialized) {
        srand(time(NULL) ^ getpid());
        seed_initialized = 1;
    }

    int tempo = (rand() % MAX_ESPERA) + 1;  // entre 1 e MAX_ESPERA

    if (tempo <= 0) {
        so_error("SD9.1", "Tempo inválido");
        exit(1);
    }

    // Log do sucesso com o tempo
    so_success("SD9.1", "%d", tempo);

    // Dorme o tempo calculado
    sleep(tempo);
    
    so_debug(">");
}

/**
 * @brief sd9_2_EnviaSucessoAoCliente Ler a descrição da tarefa SD9.2 no enunciado
 * @param msgId (I) identificador aberto de IPC
 * @param clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd9_2_EnviaSucessoAoCliente(int msgId, MsgContent clientRequest) {
    so_debug("< [@param msgId:%d, clientRequest:[%ld:%d:%s:%s:%c:%s:%d:%d:%s], indexClienteBD:%d]", msgId, clientRequest.msgType, clientRequest.msgData.status, clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado, clientRequest.msgData.infoTarifa, indexClienteBD);

    
    MsgContent response;
    response.msgType = clientRequest.msgData.est.pidCliente;
    response.msgData.status = CLIENT_ACCEPTED;
    response.msgData.est = clientRequest.msgData.est;
    strcpy(response.msgData.infoTarifa, clientRequest.msgData.infoTarifa);

    // Correção aqui ↓↓↓
    if (msgsnd(msgId, &response, sizeof(response.msgData), 0) == -1) {
        so_error("SD9.2", "Erro envio mensagem");
        sd11_EncerraServidorDedicado();
        return;
    }

    so_success("SD9.2", "SD: Confirmei Cliente Lugar %d", indexClienteBD);

    so_debug(">");
}

/**
 * @brief sd9_3_EscreveLogEntradaViatura Ler a descrição da tarefa SD9.3 no enunciado
 * @param logFilename (I) O nome do ficheiro de Logfile (i.e., FILE_LOGFILE)
 * @param clientRequest (I) pedido recebido, enviado por um Cliente
 * @param pposicaoLogfile (O) posição do ficheiro Logfile mesmo antes de inserir o log desta viatura
 * @param plogItem (O) registo de Log para esta viatura
 */
void sd9_3_EscreveLogEntradaViatura(char *logFilename, MsgContent clientRequest, long *pposicaoLogfile, LogItem *plogItem) {
    so_debug("< [@param logFilename:%s, clientRequest:[%ld:%d:%s:%s:%c:%s:%d:%d:%s]]", logFilename, clientRequest.msgType, clientRequest.msgData.status, clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado, clientRequest.msgData.infoTarifa);

 // Abrir o ficheiro no modo binário de acrescentar
    FILE *fp = fopen(logFilename, "ab+");
    if (fp == NULL) {
        so_error("SD9.3", "Erro abertura");
        sd11_EncerraServidorDedicado();
        return;
    }

    // Obter a posição atual no ficheiro
    if (fseek(fp, 0, SEEK_END) != 0) {
        so_error("SD9.3", "Erro posição ficheiro");
        fclose(fp);
        sd11_EncerraServidorDedicado();
        return;
    }

    *pposicaoLogfile = ftell(fp);
    if (*pposicaoLogfile == -1L) {
        so_error("SD9.3", "Erro posição ficheiro");
        fclose(fp);
        sd11_EncerraServidorDedicado();
        return;
    }

    // Preencher plogItem com os dados da viatura e timestamp atual
    plogItem->viatura = clientRequest.msgData.est.viatura;
    time_t agora = time(NULL);
    struct tm *tm_info = localtime(&agora);
    strftime(plogItem->dataEntrada, sizeof(plogItem->dataEntrada), "%Y-%m-%dT%Hh%M", tm_info);
    strcpy(plogItem->dataSaida, "");  // ainda não saiu

    // Escrever no ficheiro
    if (fwrite(plogItem, sizeof(LogItem), 1, fp) != 1) {
        so_error("SD9.3", "Erro escrita");
        fclose(fp);
        sd11_EncerraServidorDedicado();
        return;
    }

    // Fechar o ficheiro
    fclose(fp);


    // Success message
    so_success("SD9.3", "SD: Guardei log na posição %ld: Entrada Cliente %s em %s", 
               *pposicaoLogfile, plogItem->viatura.matricula, plogItem->dataEntrada);


    so_debug("> [*pposicaoLogfile:%ld, *plogItem:[%s:%s:%c:%s:%s:%s]]", *pposicaoLogfile, plogItem->viatura.matricula, plogItem->viatura.pais, plogItem->viatura.categoria, plogItem->viatura.nomeCondutor, plogItem->dataEntrada, plogItem->dataSaida);
}

/**
 * @brief  sd10_1_AguardaCheckout Ler a descrição da tarefa SD10.1 no enunciado
 * @param msgId (I) identificador aberto de IPC
 */
void sd10_1_AguardaCheckout(int msgId) {
    so_debug("< [@param msgId:%d]", msgId);

    MsgContent msg;
    long msgtyp = getpid();

    // Set up alarm for tariff updates
    alarm(60);

    while (1) {
        ssize_t ret = msgrcv(msgId, &msg, sizeof(msg.msgData), msgtyp, 0);

        if (ret == -1) {
            if (errno == EINTR) {
                // Signal received, check if we should continue
                continue;
            }
            so_error("SD10.1", "Erro %d na leitura da mensagem", errno);
            sd11_EncerraServidorDedicado();
            return;
        }

        if (msg.msgData.status == TERMINA_ESTACIONAMENTO) {
            alarm(0); // Cancel any pending alarm
            // Use the matricula from the received message for the success log
            so_success("SD10.1", "SD: A viatura %s deseja sair do parque",
                       msg.msgData.est.viatura.matricula);
            // Update the global clientRequest with the received message for sd10_2_EscreveLogSaidaViatura and sd11_2_EnviaTerminarAoClienteETermina
            clientRequest = msg;
            break;
        }
    }

    
    so_debug(">");
}

/**
 * @brief  sd10_1_1_TrataAlarme Ler a descrição da tarefa SD10.1.1 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void sd10_1_1_TrataAlarme(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    // Get current time
    time_t agora = time(NULL);
    struct tm *tm_info = localtime(&agora);
    char dataHora[64];
    strftime(dataHora, sizeof(dataHora), "%Y-%m-%dT%Hh%M", tm_info);

    // Try to lock FACE semaphore with non-blocking operation to prevent deadlock
    struct sembuf op = {0, -1, IPC_NOWAIT}; // SEM_FACE_MUTEX at index 0, non-blocking
    if (semop(semIdFACE, &op, 1) == -1) {
        if (errno == EAGAIN) {
            // Couldn't acquire semaphore, skip this update
            so_debug("> [semaphore busy, skipping update]");
            alarm(60); // Schedule next alarm anyway
            return;
        }
        so_error("SD10.1.1", "Erro no semop (wait) para FACE");
        return;
    }

    int valorTarifa = *tarifaAtual;

    // Unlock FACE semaphore
    op.sem_op = 1; // SEM_UP
    if (semop(semIdFACE, &op, 1) == -1) {
        so_error("SD10.1.1", "Erro no semop (signal) para FACE");
        return;
    }

    // Prepare message
    MsgContent msg;
    msg.msgType = clientRequest.msgData.est.pidCliente;
    msg.msgData.status = INFO_TARIFA;

    // Copy client information
    msg.msgData.est.pidCliente = clientRequest.msgData.est.pidCliente;
    msg.msgData.est.pidServidorDedicado = getpid();
    strcpy(msg.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.matricula);
    strcpy(msg.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.pais);
    msg.msgData.est.viatura.categoria = clientRequest.msgData.est.viatura.categoria;
    strcpy(msg.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.viatura.nomeCondutor);

    snprintf(msg.msgData.infoTarifa, sizeof(msg.msgData.infoTarifa),
             "%s Tarifa atual:%d", dataHora, valorTarifa);

    // Send message
    if (msgsnd(msgId, &msg, sizeof(msg.msgData), IPC_NOWAIT) == -1) {
        if (errno != EAGAIN) {
            so_error("SD10.1.1", "Erro ao enviar mensagem de tarifa");
        }
        // If queue is full, we'll try again next time
    } else {
        so_success("SD10.1.1", "Info Tarifa");
    }

    // Schedule next alarm
    alarm(60);


    so_debug(">");
}

/**
 * @brief  sd10_2_EscreveLogSaidaViatura Ler a descrição da tarefa SD10.2 no enunciado
 * @param  logFilename (I) O nome do ficheiro de Logfile (i.e., FILE_LOGFILE)
 * @param  posicaoLogfile (I) posição do ficheiro Logfile mesmo antes de inserir o log desta viatura
 * @param  logItem (I) registo de Log para esta viatura
 */
void sd10_2_EscreveLogSaidaViatura(char *logFilename, long posicaoLogfile, LogItem logItem) {
    so_debug("< [@param logFilename:%s, posicaoLogfile:%ld, logItem:[%s:%s:%c:%s:%s:%s]]", logFilename, posicaoLogfile, logItem.viatura.matricula, logItem.viatura.pais, logItem.viatura.categoria, logItem.viatura.nomeCondutor, logItem.dataEntrada, logItem.dataSaida);

    FILE *f = fopen(logFilename, "rb+");
    if (f == NULL) {
        so_error("SD10.2", "Erro a abrir o ficheiro");
        sd11_EncerraServidorDedicado();
        return;
    }

    // Posicionar no local indicado
    if (fseek(f, posicaoLogfile, SEEK_SET) != 0) {
        so_error("SD10.2", "Erro a posicionar o ficheiro");
        fclose(f);
        sd11_EncerraServidorDedicado();
        return;
    }

    // Escrever o log atualizado (com dataSaida)
    size_t written = fwrite(&logItem, sizeof(LogItem), 1, f);
    if (written != 1) {
        so_error("SD10.2", "Erro na escrita do ficheiro");
        fclose(f);
       sd11_EncerraServidorDedicado();
        return;
    }

    fclose(f);

    // Log de sucesso conforme pedido: 
    // "(SD10.2) SD: Atualizei log na posição <pos>: Saída Cliente <matricula> em <dataSaida>"
    so_success("SD10.2", "SD: Atualizei log na posição %ld: Saída Cliente %s em %s",
        posicaoLogfile, logItem.viatura.matricula, logItem.dataSaida);
    sd11_EncerraServidorDedicado();
      
    so_debug(">");
}

/**
 * @brief  sd11_EncerraServidorDedicado Ler a descrição da tarefa SD11 no enunciado
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 */
void sd11_EncerraServidorDedicado() {
    so_debug("<");

    sd11_1_LibertaLugarViatura(semId, lugaresEstacionamento, indexClienteBD);
    sd11_2_EnviaTerminarAoClienteETermina(msgId, clientRequest);

    so_debug(">");
}

/**
 * @brief sd11_1_LibertaLugarViatura Ler a descrição da tarefa SD11.1 no enunciado
 * @param semId (I) identificador aberto de IPC
 * @param lugaresEstacionamento (I) array de lugares de estacionamento que irá servir de BD
 * @param indexClienteBD (I) índice do lugar correspondente a este pedido na BD (>= 0), ou -1 se não houve nenhum lugar disponível
 */
void sd11_1_LibertaLugarViatura(int semId, Estacionamento *lugaresEstacionamento, int indexClienteBD) {
    so_debug("< [@param semId:%d, lugaresEstacionamento:%p, indexClienteBD:%d]", semId, lugaresEstacionamento, indexClienteBD);

    if (indexClienteBD < 0) {
        so_error("SD11.1", "Índice inválido, nada a libertar");
        return;
    }

    struct sembuf sem_op;

    // 1. Entrar na zona crítica - P(mutex BD)
    sem_op.sem_num = SEM_MUTEX_BD;   // índice do mutex BD
    sem_op.sem_op  = SEM_DOWN;       // -1 (down)
    sem_op.sem_flg = 0;
    if (semop(semId, &sem_op, 1) == -1) {
        so_error("SD11.1", "Erro a bloquear BD");
        exit(1);
    }

    // 2. Marcar o lugar como livre
    lugaresEstacionamento[indexClienteBD].pidCliente = DISPONIVEL;

    // 3. Sair da zona crítica - V(mutex BD)
    sem_op.sem_op = SEM_UP;          // +1 (up)
    if (semop(semId, &sem_op, 1) == -1) {
        so_error("SD11.1", "Erro a desbloquear BD");
        exit(1);
    }

    // 4. Incrementar o semáforo de lugares disponíveis para indicar lugar livre
    sem_op.sem_num = SEM_LUGARES_PARQUE;
    sem_op.sem_op = SEM_UP;          // +1 (up)
    sem_op.sem_flg = 0;
    if (semop(semId, &sem_op, 1) == -1) {
        so_error("SD11.1", "Erro a incrementar semáforo de lugares disponíveis");
        exit(1);
    }

    so_success("SD11.1", "SD: Libertei Lugar: %d", indexClienteBD);

    so_debug(">");
}

/**
 * @brief sd11_2_EnviaTerminarAoClienteETermina Ler a descrição da tarefa SD11.2 no enunciado
 * @param msgId (I) identificador aberto de IPC
 * @param clientRequest (I) pedido recebido, enviado por um Cliente
 */
void sd11_2_EnviaTerminarAoClienteETermina(int msgId, MsgContent clientRequest) {
    so_debug("< [@param msgId:%d, clientRequest:[%ld:%d:%s:%s:%c:%s:%d:%d:%s]]", msgId, clientRequest.msgType, clientRequest.msgData.status, clientRequest.msgData.est.viatura.matricula, clientRequest.msgData.est.viatura.pais, clientRequest.msgData.est.viatura.categoria, clientRequest.msgData.est.viatura.nomeCondutor, clientRequest.msgData.est.pidCliente, clientRequest.msgData.est.pidServidorDedicado, clientRequest.msgData.infoTarifa);

    // Preparar mensagem para enviar ao cliente
    MsgContent msgToSend;

    // Definir o tipo da mensagem para o PID do cliente (normalmente o msgType)
    msgToSend.msgType = clientRequest.msgData.est.pidCliente;

    // Copiar dados relevantes (pode ser só necessário definir o status)
    msgToSend.msgData = clientRequest.msgData;  // copiar para manter campos, se necessário

    // Alterar o status para ESTACIONAMENTO_TERMINADO
    msgToSend.msgData.status = ESTACIONAMENTO_TERMINADO;

    // Enviar a mensagem para o cliente
    if (msgsnd(msgId, &msgToSend, sizeof(msgToSend.msgData), 0) == -1) {
        so_error("SD11.2", "Erro no envio da mensagem");
        exit(0); // ou exit(-1), conforme enunciado
    }

    so_success("SD11.2", "SD: Shutdown");

    exit(0);

    so_debug(">");
}

/**
 * @brief  sd12_TrataSigusr2    Ler a descrição da tarefa SD12 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
void sd12_TrataSigusr2(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    // Notify server we're terminating
    struct sembuf op = {SEM_SRV_DEDICADOS, 1, 0};
    semop(semId, &op, 1);
    
    so_success("SD12", "SD: Recebi pedido do Servidor para terminar");
    sd11_EncerraServidorDedicado();

    so_debug(">");
}
