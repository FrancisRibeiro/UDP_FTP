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
 *      File: libcommon.c
 *      Description: Executas as funções de servidor
 *      Author:  Francis Eduardo Ribeiro - edu_fers@hotmail.com
 *      
 */

/**
 * Header das funções comuns
 */
#include "libcommon.h"

int main(int argc, char *argv[]) {
    input_data_t id;
    int sock; 
    struct sockaddr_in server;
    int mysock;
    char buff[5];
    int rval;
    int count = 0;
    FILE *fp1;
    m_h msg_hdr;
    ack ack_msg;
    int slen = sizeof (server);
    int seqNum = 1;
    int flag = 0;

    /**
     *  Enquanto não for informada uma porta válida,
     *  o servidor ficará impedido de ser ativado.
     */
    if (argc != 2) {
        print("Uso: server <numero_porta_servidor>\n");
        return 0;
    }

    /**
     * Criação do socket para comunicão entre cliente e servidor.
     */
    sock = createSocketUDP();

    /**
     * Setando as opoções para permitir que uma porta seja
     * reutilizada, logo após o servidor ser fechado.
     */
    {
        int reuseAddr = 1;
        if (-1 == setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                &reuseAddr, sizeof (reuseAddr))) {
            perror("ERRO ao setar as opções do socket!\n");
            closeSocketUDP(sock);
            exit(-1);
        }
    }

    {
        /**
         * Configurando o endereço do socket do servidor.
         */
        memset((void *) &server, 0, sizeof (server));
        server.sin_family = AF_INET;
        server.sin_port = htons(atoi(argv[1]));
        server.sin_addr.s_addr = htonl(INADDR_ANY);

        /**
         * Vinculando a porta informada ao socket do servidor.
         */
        if (bind(sock, (struct sockaddr *) &server, sizeof (server)) == -1) {
            perror("FALHA ao vincular a porta ao socket!");
            closeSocketUDP(sock);
            exit(-1);
        }
    }

    print("Aguardando por conexão...\n\n");

    /**
     *  Os dados de cabeçalho são enviados para a criação do novo arquivo.
     *  @param file_name    [nome do novo arquivo]
     *  @param size         [tamanho do novo arquivo]
     */
    mysock = sock;
    if ((rval = recvfrom(mysock, &id, sizeof (id), 0,
            (struct sockaddr *) &server, &slen)) < 0) {
        perror("ERRO ao ler o fluxo de mensagens!\n");
    } else if (rval == 0) {
        print("Finalizando conexão...\n");
    } else {
        aprint("MSG RECEBIDA -->[arquivo]: %s ", id.file_name);
        aprint("- [tamanho]: %d bytes\n", id.size);
    }

    /**
     * De fato aqui o novo arquivo é criado no servidor.
     */
    memset(buff, 0, sizeof (buff));
    print("Copia do arquivo iniciada...\n\n");
    fp1 = fopen(id.file_name, "wb");
    int read_bytes = 0;

    /**
     * Finalmente iniciamos o envido dos pacotes do cliente para o servidor.
     */
    while (read_bytes < id.size) {
        fd_set select_fds; /* fd's usado por select */
        struct timeval timeout; /* Valor de tempo para timeout */

        /* -----------------------------------------
         Setando a descrição para o select()
         ----------------------------------------- */
        FD_ZERO(&select_fds); /* Limpando o fd's */
        FD_SET(mysock, &select_fds); /* Setando o bit que corresponde ao socket */

        /* -----------------------------------------
         Setando o valor do timeout
         ----------------------------------------- */
        timeout.tv_sec = 5; /* Timeout setado para 5 segundos + 0 micro segundos*/
        timeout.tv_usec = 0;

        print("Esperando pela mensagem...\n");

        /**
         * Espera pela mensagem do cliente.
         */
        if (select(32, &select_fds, NULL, NULL, &timeout) == 0) {

            /**
	     * [TEMPORIZADOR]
             * Perda de conexão.
             * O tempo maximo permitido é de 5 segundos;
             */
            print("\nO temporizador estourou!\nFALHA na transferência do arquivo!\n\n");
            fclose(fp1);
            closeSocketUDP(mysock);
            exit(1);
        } else {
            rval = recvfrom(mysock, &msg_hdr, sizeof (msg_hdr), 0,
                    (struct sockaddr *) &server, &slen);
        }

        if (rval < 0) {
            perror("ERRO ao ler o fluxo de mensagens!\n");
        } else if (rval == 0) {
            print("Finalizando conexão...\n");
        } else {
            unsigned int x = checksum(msg_hdr.buff, msg_hdr.size);

            /**
             * Se o checksum calculado for o mesmo que o checksum recebido,
             * enviamos um ACK para o pacote correspondente.
             */
            aprint("Checksum calculado : %d\n", x);
            aprint("MSG VÁLIDA %d recebida: ", msg_hdr.seqNum);
            aprint("tamanho:%d ", msg_hdr.size);
            aprint("checksum:%d\n", msg_hdr.checksum);

            ack_msg.seqNum = seqNum; // número de sequencia esperado
            ack_msg.limit = msg_hdr.limit; // temporizador

            /**
             * Tentamos enviar o pacote pelo numero de sequência solicitado.
             */
            if ((x == msg_hdr.checksum) && (msg_hdr.seqNum == seqNum)) {
                ack_msg.flag = 0;
                ++seqNum;

                /**
                 * Enviando ACK de confirmação ao cliente.
                 */
                aprint("Enviando ACK %d\n", ack_msg.seqNum);
                if ((rval = sendto(mysock, &ack_msg, sizeof (ack_msg), 0,
                        (struct sockaddr *) &server, slen)) < 0) {
                    perror("ERRO ao ler o fluxo de mensagens!\n");
                } else if (rval == 0) {
                    print("Finalizando conexão...\n");
                } else {

                    /**
                     * Gravação dos bytes que vem dos pacotes no arquivo no servidor.
                     */
                    fwrite(&msg_hdr.buff[0], msg_hdr.size, 1, fp1);
                    print("ACK ENVIADO.\n\n");
                    read_bytes += msg_hdr.size;
                }

                /**
                 * Caso o pacote tenha sido perdido, é feita a solicitação novamente.
                 * E também é iniciado o temporizador.
                 */
            } else {
                ack_msg.flag = 1;
                print("PACOTE PERDIDO OU DEFEITUOSO...\n");
                ack_msg.limit;
                ack_msg.seqNum -= 1; 
                if (ack_msg.limit <= (Try - 1)) {
                    aprint("Reenviando ACK %d - ", ack_msg.seqNum);
                    aprint("Tentativa %d\n", ack_msg.limit + 1);
                    if ((rval = sendto(mysock, &ack_msg, sizeof (ack_msg), 0,
                            (struct sockaddr *) &server, slen)) < 0) {
                        perror("ERRO ao ler o fluxo de mensagens!\n");
                    } else if (rval == 0) {
                        print("Finalizando conexão...\n");
                    } else {
                        print("ACK REENVIADO.\n\n");
                    }
                } else {
                    print("\nO temporizador estourou!\n FALHA na transferência do arquivo!\n");
                    fclose(fp1);
                    closeSocketUDP(mysock);
                    return 0;
                }
            }

        }
    }

    /**
     * O arquivo foi transferido com sucesso.
     * Todos os arquivos e sockets são fechados.
     */
    print("Cópia do arquivo completada com sucesso!\n");
    fclose(fp1);
    closeSocketUDP(mysock);

    return 0;
}
