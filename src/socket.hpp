#pragma once

#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stack>

struct Message
{
  char text[1024];
  std::string name;
};

struct Addressee
{
  int remote_port;
  char *remote_ip;
  sockaddr_in remote_adress;
};

class Socket
{
public:
  Socket(const sockaddr_in &address);
  ~Socket();

  void send_to(const Message &message, const sockaddr_in &address);
  void receive_from(Message &message, sockaddr_in &address);
  void receive_and_send(Message &message, sockaddr_in &address);

private:
  int fd_;
};

sockaddr_in make_ip_address(const std::string &ip_address, int port);
