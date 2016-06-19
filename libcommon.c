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
 *      File: libcommon.h
 *      Description: Biblioteca com as funções comuns entre cliente e servidor
 *      Author:  Francis Eduardo Ribeiro - edu_fers@hotmail.com
 *      
 */

#include "libcommon.h"

/**
 * [checksum: Faz a soma de verificação de palavras de @len bytes]
 * @param  buffer [Palavra de bytes em que será feito o checksum]
 * @param  len    [Tamanho da palavra de bytes]
 * @return        [Valor do checksum]
 */
unsigned int checksum(void *buffer, size_t len) {
    unsigned int seed = 0;
    unsigned char *buf = (unsigned char *) buffer;
    size_t i;

    for (i = 0; i < len; ++i) {
        seed += (unsigned int) (*buf++);
    }

    return seed;
}

/**
 * [createSocketUDP: Dado o tipo de socket, cria um socket]
 * @return [Retorna o manipulador do socket]
 */
int createSocketUDP() {
    int sock, socktype;

    socktype = SOCK_DGRAM; //socket UDP

    if ((sock = socket(AF_INET, socktype, 0)) == -1) {
        perror("FALHA ao criar socket!\n");
        exit(-1);
    }

    print("\nSocket criado!\n");

    return sock;
}

/**
 * [closeSocketUDP: Fecha o socket dado se estiver aberto]
 * @param sock [Socket que será fechado]
 */
void closeSocketUDP(int sock) {
    if (sock == -1) return;

    if (close(sock) == -1) {
        perror("ERRO ao fechar o socket! \n");
        exit(-1);
    }

    print("Socket fechado!\n\n");
}