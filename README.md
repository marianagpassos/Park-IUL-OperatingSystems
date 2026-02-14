# Park-IUL-OperatingSystems
Parking management system developed in Bash and C for the Operating Systems course (ISCTE-IUL, 2024/2025). This project implements an automated 24/7 parking platform, covering concepts such as scripting, concurrent processes, signals, named pipes (FIFOs), shared memory, semaphores, and System V message queues.
This project demonstrates the practical application of fundamental Operating Systems concepts, divided into three distinct parts that evolve in complexity:

### Part 1 – Bash Scripting
Administration and management of the parking database through shell scripts.  
**Key Concepts:**
- File and string manipulation
- Regular expressions and data validation
- Task scheduling with `cron`
- Generation of statistical reports in HTML

### Part 2 – Processes and Signals
Implementation of a concurrent client-server architecture in C.  
**Key Concepts:**
- Process creation with `fork()` and `wait()`
- Inter-process communication with signals (`signal`, `kill`)
- Named pipes (FIFOs) for message exchange
- Basic concurrency handling

### Part 3 – System V IPC
Evolution of the architecture with advanced communication mechanisms.  
**Key Concepts:**
- Shared Memory (`shmget`, `shmat`, `shmdt`)
- Semaphores for mutual exclusion (`semget`, `semop`)
- Message Queues (`msgget`, `msgsnd`, `msgrcv`)
- Race condition and deadlock prevention


## Features

- [X] Registration of vehicle entries and exits with license plate validation by country
- [X] Automatic archiving of completed records in monthly files
- [X] Parking duration calculation
- [X] Detailed statistical reports in HTML format
- [X] Interactive command-line menu interface
- [X] Simulation of multiple concurrent clients
- [X] Robust synchronization with System V semaphores

## Technologies Used

| Language   | Technologies / APIs                                             |
|------------|-----------------------------------------------------------------|
| **Bash**   | `grep`, `sed`, `awk`, `cron`, regular expressions               |
| **C**      | `fork`, `wait`, `signal`, `kill`, `alarm`, `time`               |
| **C**      | `mkfifo`, `open`, `read`, `write` (FIFOs)                       |
| **C**      | `shmget`, `shmat`, `semget`, `semop`, `msgget` (System V IPC)   |
| **Others** | Binary files, dynamic memory allocation, timestamp formatting   |



## How to Run (Example for Part 3):

1. **Navigate to the project directory:**
    ```bash
   cd parte-3
    ```

## Compile the programs
    ```shell
    gcc -o servidor servidor.c -lrt -lpthread
    gcc -o cliente cliente.c -lrt -lpthread
    ```

## Start the server (Terminal 1 - Creates a parking lot with 10 spaces):
   ```shell
    ./servidor 10
   ```

## Start one or more clients (Terminal 2, 3, ...):
    ```bash
     ./cliente
    ```



> [!NOTE]
> This project was developed for the Operating Systems course at ISCTE-IUL. The code follows the specifications provided in the assignment and was validated using the official validator scripts.

> [!TIP]
> Always run the validator scripts after making changes to ensure your implementation passes all tests. Use the `-d` flag to see debug messages and identify issues more easily.

> [!IMPORTANT]
> The IPC_KEY in defines.h must be changed to your student number. Failure to do so may cause conflicts with other students' running processes on the Tigre server.
