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
 *      Description: Biblioteca com as funções comuns entre cliente e servidor
 *      Author:  Francis Eduardo Ribeiro - edu_fers@hotmail.com
 *      
 */

#ifndef LIBCOMMON_H
#define LIBCOMMON_H

/**
 * Headers comuns entre os arquivos
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <assert.h>
#include <signal.h>
#include <time.h>

/**
 * [print: Imprime tudo que nunca precisará de depuração]
 * @param  stdout [Move tudo que está pendente para a tela]
 * @return        [String que pode estar em buffer]
 */
#define print(x) printf(x); fflush(stdout);

/**
 * [print: Imprime tudo que nunca precisará de depuração mas agora com argumentos]
 * @param  stdout [Move tudo que está pendente para a tela]
 * @return        [String que pode estar em buffer]
 */
#define aprint(x, a) printf(x, a); fflush(stdout);

#define True 1
#define False 0
#define Try 5 // [TEMPORIZADOR]Número de tentativas caso o pacote seja perdido. 

/**
 * Estrutura para o primeiro pacote de reconhecimento.
 */

typedef struct input_data_ {
    char file_name[50]; // Nome do arquivo
    int size; // Tamanho do arquivo
} input_data_t;


/**
 * Estrutura para os pacotes.
 */
typedef struct msg_header {
    int seqNum; // Número de sequência
    int size; // Tamanho do pacote sendo enviado
    int limit; // Variável auxiliar do temporizador
    unsigned char buff[100]; // Tamanho máximo permitido por pacote
    unsigned int checksum; // Soma de verificação
} m_h;

/**
 * Estrutura para os ACKS. 
 */
typedef struct acknowledgement {
    int seqNum; // Número de sequência
    int limit; // Variável auxiliar do temporizador
    unsigned int flag; // Flag para confirmar se pacote foi entregue.
} ack;

/**
 * [checksum: Faz a soma de verificação de palavras de @len bytes]
 * @param  buffer [Palavra de bytes em que será feito o checksum]
 * @param  len    [Tamanho da palavra de bytes]
 * @return        [Valor do checksum]
 */
unsigned checksum(void *buffer, size_t len);

/**
 * [createSocketUDP: Dado o tipo de socket, cria um socket
 * @return [Retorna o manipulador do socket]
 */
int createSocketUDP();

/**
 * [closeSocketUDP: Fecha o socket dado se estiver aberto]
 * @param sock [Socket que será fechado]
 */
void closeSocketUDP(int sock);

#endif /* LIBCOMMON_H */
