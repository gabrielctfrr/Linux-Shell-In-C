//Maria Luiza Saldanha - 22153140
//Mateus Mota Nobrega - 21953021
//Gabriel César Tavares Ferreira - 21854868

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_TOKENS 20 
#define MAX_TOKEN_LENGTH 100

char initialPath[MAX_TOKEN_LENGTH]; // Variável global para armazenar o caminho inicial

// Função para obter o diretório atual e extrair a parte desejada para ficar algo semelhante a mateusnobrega@MANDVS323PCJ3:~/Desktop/Terminal---SO$
void diretorioCd(char *cwd) {
    if (getcwd(cwd, MAX_TOKEN_LENGTH) == NULL) {
        perror("Erro ao obter diretório atual");
        exit(EXIT_FAILURE);
    }

    char *substring = strstr(cwd, initialPath);
    if (substring != NULL) {
        strcpy(cwd, substring + strlen(initialPath)); // Copie a parte desejada para o caminho atual
    }
}


int parseEntrada(char *input, char **tokens) {
    int numTokens = 0;
    char *token = strtok(input, " \t\n");

    while (token != NULL && numTokens < MAX_TOKENS) {
        tokens[numTokens] = token;
        numTokens++;
        token = strtok(NULL, " \t\n");
    }

    tokens[numTokens] = NULL; // Marca o final da lista de tokens
    return numTokens;
}

void comandoCd(char *dir) {
    if (chdir(dir) == 0) {
        //printf("Diretório alterado para: %s\n", dir);
    } else {
        perror("Erro ao alterar o diretório");
    }
}

void executaComando(char **tokens) { 
    pid_t pid = fork(); 
     
    if (pid == 0) { 
        // Verifica se o primeiro token é um caminho de arquivo válido 
        if (access(tokens[0], X_OK) == 0) { 
            execv(tokens[0], tokens); 
        } else { 
            // Se não for um caminho válido, tenta executar o comando com o PATH 
            execvp(tokens[0], tokens); 
            // Erro ao executar o comando, sair com código de erro 
            perror("Comando não encontrado"); 
            exit(EXIT_FAILURE); 
        } 
    } else if (pid > 0) { 
        // Processo pai 
        wait(NULL); 
    } else { 
        perror("Erro ao criar o processo filho"); 
    } 
}

void executaComPipe(char **tokens1, char **tokens2) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        execvp(tokens1[0], tokens1);
        perror("Comando não encontrado");
        exit(EXIT_FAILURE);
    } else if (pid1 > 0) {
        pid_t pid2 = fork();
        if (pid2 == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);

            execvp(tokens2[0], tokens2);
            perror("Comando não encontrado");
            exit(EXIT_FAILURE);
        } else if (pid2 > 0) {
            close(pipefd[0]);
            close(pipefd[1]);
            wait(NULL);
            wait(NULL);
        } else {
            perror("Erro ao criar o processo filho");
        }
    } else {
        perror("Erro ao criar o processo filho");
    }
}

void executaComandoEmSegundoPlano(char **tokens){
    int pid = fork();

    if(pid == 0){
        execvp(tokens[0], tokens);
        perror("Comando não encontrado");
        exit(EXIT_FAILURE);
    }else if(pid > 0){
        //t
    }else{
        perror("Erro ao criar o processo filho");
    }
}

void redireciona(char **tokens) {
    pid_t pid = fork();

    if (pid == 0) {
        // Processo filho
        int in_fd = -1;
        int out_fd = -1;

        for (int i = 0; tokens[i] != NULL; i++) {
            if (strcmp(tokens[i], "<") == 0) {
                in_fd = open(tokens[i + 1], O_RDONLY);
                if (in_fd == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
                tokens[i] = NULL;
            } else if (strcmp(tokens[i], ">") == 0) {
                out_fd = open(tokens[i + 1], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
                if (out_fd == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
                tokens[i] = NULL;
            }
        }

        execvp(tokens[0], tokens);

        // Se execvp falhar, exibir mensagem de erro e sair
        perror("Comando não encontrado");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Processo pai
        wait(NULL);
    } else {
        perror("Erro ao criar o processo filho");
    }
}



int main() {
    
    char command[100];
    char *tokens[MAX_TOKENS];
    int numTokens;
    char cwd[MAX_TOKEN_LENGTH];

    if (getcwd(initialPath, MAX_TOKEN_LENGTH) == NULL) {
        perror("Erro ao obter diretório inicial");
        return 1;
    }

    while (1) {
        diretorioCd(cwd);  // Obtém o diretório atual a partir de Terminal---SO para o comanodo cd
        printf("ShellSO>:%s$ ", cwd);      // Exibe o prompt com o diretório atual

        fgets(command, sizeof(command), stdin);

        if (command[strlen(command) - 1] == '\n') {
            command[strlen(command) - 1] = '\0';
        }

        numTokens = parseEntrada(command, tokens);

        if (numTokens > 0) {
            if (strcmp(tokens[0], "cd") == 0) { //cd -> comando interno do proprio shell
                if (numTokens > 1) {
                 comandoCd(tokens[1]);
                } else {
                    printf("\nVoce quis dizer:\n\ncd <diretório>\n\n");
                }
            } else if (numTokens > 1 && ((strcmp(tokens[1], ">") == 0) || (strcmp(tokens[1], "<") == 0))) {
                redireciona(tokens);
            } else if (strcmp(tokens[0], "exit") == 0) { 
                break;
            } else if(numTokens > 1 && strcmp(tokens[numTokens - 1], "&") == 0){
                tokens[numTokens-1] = NULL;
                executaComandoEmSegundoPlano(tokens);
            }else {
                int pipeIndex = -1;
                for (int i = 0; i < numTokens; i++) {
                    if (strcmp(tokens[i], "|") == 0) {
                        tokens[i] = NULL;
                        pipeIndex = i;
                        break;
                    }
                }

                if (pipeIndex >= 0) {
                    char **tokens1 = tokens;
                    char **tokens2 = tokens + pipeIndex + 1;
                    executaComPipe(tokens1, tokens2);
                } else {
                    executaComando(tokens);
                }
            }
        }

    }
    return 0;

}