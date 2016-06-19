# UDP_FTP
As transferências consideram uma transmissão confiável por meio da implementação dos tópicos: soma de verificação (checksum), número de sequência, temporizadores e pacotes de reconhecimento ACK e NAK.

# EXEMPLO DE UTILIZAÇÃO:

Abra dois terminais dentro da pasta UDP_FTP.

Digite --> <b>"make"</b> para compilar o programa

# PARA O SERVIDOR:

Uso: server <numero_porta_servidor>
Digite --> ./server 2020 (por exemplo pode ser qualquer porta livre).

A seguinte mensagem deverá aparecer.

Socket criado!
Aguardando por conexão...

# PARA O CLIENTE:

Uso: client <nome_arquivo_local> <arquivo_servidor> <nome_servidor> <numero_porta>
Digite --> ./client README.MD README2.MD  localhost 2020

<b>OBS 1:</b> README.MD  é o arquivo que irá ser transferido, poder ser qualquer tipo de arquivo, a extensão é obrigatória.

<b>OBS 2:</b> README2.MD é o nome do arquivo que será copiado, pode ser qualquer nome mas a extensão tem que ser a mesma do arquivo original.
 
<b>OBS 3:</b> A porta deve ser a mesma do servidor, e o nome do servidor deve ser o IP do servidor, ou localhost para uso local.

<b>Se tudo ocorreu como o esperado você foi capaz de transferir o arquivo!!</b>
