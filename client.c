/*      Este programa é um software livre; você pode redistribuí-lo e/ou 
 *      modificá-lo dentro dos termos da Licença Pública Geral GNU como 
 *      publicada pela Fundação do Software Livre (FSF); na versão 3 da 
 *      Licença, ou (na sua opinião) qualquer versão.
 *
 *      Este programa é distribuído na esperança de que possa ser útil, 
 *      mas SEM NENHUMA GARANTIA; sem uma garantia implícita de ADEQUAÇÃO
 *      a qualquer MERCADO ou APLICAÇÃO EM PARTICULAR. Veja a
 *      Licença Pública Geral GNU para maiores detalhes.
 *
 *      Você deve ter recebido uma cópia da Licença Pública Geral GNU junto
 *      com este programa. Se não, veja <http://www.gnu.org/licenses/>.
 *
 *      File: client.c
 *      Description: Executa as funções de cliente
 *      Author:  Francis Eduardo Ribeiro - edu_fers@hotmail.com
 *      
 */

/**
 * Header das funções comuns
 */
#include "libcommon.h"

/**
 * [file_open: Abre o arquivo que será enviado ao servidor]
 * @param  x [Nome do arquivo]
 * @return   [Arquivo aberto]
 */
FILE *file_open(char *x) {
    FILE *fp = fopen(x, "rb");

    if (!fp) {
        perror("O arquivo não existe!");
        fclose(fp);
        exit(1);
    }

    return fp;
}

/**
 * [file_size: O tamanho do arquivo a ser enviado]
 * @param  fp [Ponteiro para o arquivo]
 * @return    [Tamanho do arquivo]
 */
int file_size(FILE *fp) {
    int sz;

    fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    printf("O arquivo tem %d bytes\n", sz);

    return sz;
}

/**
 * [send_file description]
 * @param fp       [Ponteiro para o arquivo a ser enviado]
 * @param sock     [socket do cliente]
 * @param si_other [dados do socket do cliente]
 */
void send_file(FILE *fp, int sock, struct sockaddr_in si_other) {
    ack ack_msg;
    m_h msg_header;
    int slen = sizeof (si_other);
    int read_from_file = True;

    memset(msg_header.buff, 0, sizeof (msg_header.buff));

    /**
     * Setando o ponteiro do arquivo para o inicio do arquivo.
     */
    fseek(fp, 0L, SEEK_SET);

    /**
     * Setando algumas variáveis de flag.
     */
    printf("Enviando arquivo...\n");
    int seqNum = 0;
    int id = 0;
    int i = 0;
    int rval;
    int sends = 0;

    /**
     * Começando a enviar o arquivo.
     */
    while (!feof(fp)) {

        /**
         * Lógica para enviar 500 bytes
         */
        if (read_from_file == True) {
            msg_header.limit = 0;
            for (i = 0; i < sizeof (msg_header.buff); i++) {
                fread(&(msg_header.buff[i]), 1, 1, fp);
                if (feof(fp)) {
                    msg_header.buff[i] = '\0';
                    break;
                }
            }
        } else {
            /**
             * Caso o pacote tenha sido perdido, aqui iniciamos o temporizador
             * no lado do cliente, e setamos o ponteiro para o pacote perdido.
             */
            ack_msg.limit = ++msg_header.limit;
            msg_header.seqNum = --seqNum;
            if (msg_header.limit <= Try) {
                aprint("\nTentativa %d\n", ack_msg.limit);

                /**
                 * Voltamos o ponteiro do arquivo para o pacote perdido.
                 */
                fseek(fp, (msg_header.seqNum)*100, SEEK_SET);

                for (i = 0; i < sizeof (msg_header.buff); i++) {
                    fread(&(msg_header.buff[i]), 1, 1, fp);
                    if (feof(fp)) {
                        msg_header.buff[i] = '\0';
                        break;
                    }
                }
            }
        }

        /**
         * Lógica para enviar menos que 500 bytes
         */
        if (i != 0) {
            /**
             * Calcula o checksum a ser enviado
             */
            msg_header.checksum = checksum(msg_header.buff, i);
            msg_header.size = i;
            msg_header.seqNum = ++seqNum;
            aprint("\nMSG Enviada: Id:%d ", msg_header.seqNum);
            aprint("tamanho:%d ", msg_header.size);
            aprint("checksum:%d\n", msg_header.checksum);

            /**
             * Caso seja um pacote válido é iniciada a transferência.
             * Enviando arquivo para o servidor.
             */
            ENVIO:
            if (sendto(sock, &msg_header, sizeof (msg_header), 0,
                    (struct sockaddr *) &si_other, slen) <= 0) {
                perror("FALHA ao enviar!");
                fclose(fp);
                closeSocketUDP(sock);
                exit(1);
            }

            fd_set select_fds; /* fd's usado por select */
            struct timeval timeout; /* Valor de tempo para timeout */

            /* -----------------------------------------
             Setando a descrição para o select()
             ----------------------------------------- */
            FD_ZERO(&select_fds); /* Limpando o fd's */
            FD_SET(sock, &select_fds); /* Setando o bit que corresponde ao socket */

            /* -----------------------------------------
             Setando o valor do timeout
             ----------------------------------------- */
            timeout.tv_sec = 1; /* Timeout setado para 5 segundos + 0 micro segundos*/
            timeout.tv_usec = 0;

            print("Esperando pela mensagem...\n");

            /**
             * Perda de pacotes.
             * O tempo de espera máximo permitido é de Try tentativas.
             */
            if (msg_header.limit == Try) {
                print("\nO temporizador estourou!\nFALHA na transferência do arquivo!\n\n");
                fclose(fp);
                closeSocketUDP(sock);
                exit(1);
            }

            /**
	     * [TEMPORIZADOR]
             * Perda de conexão.
             * O tempo maximo permitido é de 5 segundos;
             */
            if (select(32, &select_fds, NULL, NULL, &timeout) == 0) {
                if(sends < 5) {
                    ++sends;
                    aprint("\nReenviando, tentativa %d\n", sends);
                    goto ENVIO;
                }
                /**
                 * Perda de conexão.
                 * O tempo maximo permitido é de 5 segundos;
                 */
                print("\nO temporizador estourou!\nFALHA na transferência do arquivo!\n\n");
                fclose(fp);
                closeSocketUDP(sock);
                exit(1);
            } else {
                sends = 0;
                rval = recvfrom(sock, &ack_msg, sizeof (ack_msg), 0,
                        (struct sockaddr *) &si_other, &slen);
            }

            /**
             * Recebendo o ACK.
             */
            if (rval < 0) {
                perror("ERRO ao ler o fluxo de mensagens!\n");
            } else if (rval == 0) {
                print("Finalizando conexão...\n");
            } else {
                printf("ACK %d RECEBIDO.\n", ack_msg.seqNum);
            }

            /**
             * Verificando o ACK.
             * Se flag = 0 não ocorreu erro.
             */
            if (seqNum == ack_msg.seqNum && ack_msg.flag == 0) {
                read_from_file = True;
            } else {
                read_from_file = False;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in client;
    FILE *fp;
    struct hostent *hp;
    char buff[10] = "/0";
    int rval;
    int count = 0;
    int sz;
    struct sockaddr_in si_me;
    int slen = sizeof (client);

    /**
     * Enquanto os argumentos enviados não forem válidos
     * o cliente ficará impedido de ser ativado.
     */
    if (argc != 5) {
        print("Uso: client <nome_arquivo_local> <arquivo_servidor> ");
        print("<nome_servidor> <numero_porta>");
        exit(1);
    }

    aprint("\nServidor: %s", argv[3]);
    aprint(":%s\n", argv[4]);
    aprint("Arquivo enviado: %s\n", argv[1]);

    fp = file_open(argv[1]);
    sz = file_size(fp);

    /**
     * Os dados do arquivo a ser enviado são alocados
     * na struct correspondente.
     */
    input_data_t id;
    strcpy(id.file_name, argv[2]);
    id.size = sz;

    /**
     * Criação do socket para comunicão entre cliente e servidor
     */
    sock = createSocketUDP();

    /**
     * Configurando o endereço do socket do servidor.
     */
    client.sin_family = AF_INET;

    /**
     * Estabelecimento de conexão se o cliente e servidor estiverem
     * rodando no mesmo computador.
     */
    if (strcmp(argv[3], "localhost") == 0) {
        if (strcmp(argv[1], argv[2]) == 0) {
            perror("Tentativa de sobreescrever o arquivo no mesmo computador!\n");
            fclose(fp);
            closeSocketUDP(sock);
            exit(1);
        }

        /**
         * Pega o endereço de IP de 32-bit do host
         */
        hp = gethostbyname(argv[3]);
        if (hp == 0) {
            perror("FALHA ao pegar endereço de IP!");
            fclose(fp);
            closeSocketUDP(sock);
            exit(1);
        }
        memcpy(&client.sin_addr, hp->h_addr, hp->h_length);
    } else {
        /**
         * Estabelecimento de conexão se o cliente e servidor estiverem
         * rodando em computadores diferentes.
         */
        client.sin_addr.s_addr = inet_addr(argv[3]);

    }

    /**
     * Vinculando a porta informada ao socket do cliente.
     */
    client.sin_port = htons(atoi(argv[4]));

    /**
     * Enviando os dados para o servidor.
     */
    if (sendto(sock, &id, sizeof (id), 0, (struct sockaddr *) &client, slen) <= 0) {
        perror("Falha ao enviar!");
        fclose(fp);
        closeSocketUDP(sock);
        exit(1);
    }

    aprint("\nChecando o estabelecimento de conexão: enviando %s ", id.file_name);
    aprint("com %d bytes\n", id.size);

    /**
     * Enviando o arquivo
     */
    send_file(fp, sock, client);

    /**
     * O arquivo foi transferido com sucesso.
     * Todos os arquivos e sockets são fechados.
     */
    print("\nO arquivo foi enviado com sucesso!\n");
    fclose(fp);
    closeSocketUDP(sock);

    return 0;
}
