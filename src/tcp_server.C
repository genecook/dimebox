#include <tcp_server.h>
#include <string.h>

//**************************************************************************************************
//**************************************************************************************************

TCPServer::TCPServer(int portnum, bool do_block) {
    if (do_block)
      sockfd = socket(AF_INET, SOCK_STREAM, 0);
    else
      sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    if (sockfd < 0) {
      throw std::runtime_error("ERROR opening socket");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portnum);

    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
      throw std::runtime_error("ERROR on binding");
    }

    listen(sockfd,5);
    clilen = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen);

    if (newsockfd < 0) { 
      throw std::runtime_error("ERROR on accept");
    }
}

TCPServer::~TCPServer() {
    close(sockfd);
}

//**************************************************************************************************
//**************************************************************************************************

int TCPServer::Read(unsigned char *mbuf,unsigned int size) {
    memset(mbuf,0,size);

    int numread = read(newsockfd,mbuf,size);

    if (numread < 0) { 
      throw std::runtime_error("ERROR reading from socket");
    }

    return numread;
}

//**************************************************************************************************
//**************************************************************************************************

int TCPServer::Write(unsigned char *mbuf,unsigned int size) {
    int numwritten = write(newsockfd,mbuf,size);

    if (numwritten != (int) size) {
      throw std::runtime_error("ERROR writing to socket");
    }

    return (int) size;
}

