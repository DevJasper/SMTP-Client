#include "smtpclient.hpp"

#include <arpa/nameser.h>
#include <cstring>
#include <err.h>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <resolv.h>
#include <unistd.h>

#include "response.hpp"
#include "statuscodes.hpp"

#define MAX_IO_BUFFER_SIZE 1024


namespace hive{


  static enum{EHLO,MAIL_FROM,RCPT_TO,DATA,MSG,QUIT,SUCCESS,ERROR}state = EHLO;

  SMTPClient::~SMTPClient(){
    close(fd);
  }

  void SMTPClient::close(const int fd) noexcept{
    ::close(fd);
  }

  void SMTPClient::connect(){
    try{
      std::string _host = parseHost();
      struct addrinfo hints, *res, *res_;
      memset(&hints, 0, sizeof hints);
      int _fd = -1;

      hints.ai_family = PF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = AI_PASSIVE;
      hints.ai_protocol = IPPROTO_IP;
      hints.ai_canonname = nullptr;
      hints.ai_addr = nullptr;
      hints.ai_next = nullptr;

      host = resolveMX(_host.c_str());

      if(!host.empty()){
        if( ::getaddrinfo(host.c_str(), "25", &hints, &res) == -1 )
          throw std::runtime_error("Failed to get address info");

        for(res_ = res; res_ != nullptr; res_ = res_->ai_next){
          if ( (_fd = ::socket(res_->ai_family, res_->ai_socktype, res_->ai_protocol) ) == -1 )
            continue;
          if( ::connect(_fd, res_->ai_addr, res_->ai_addrlen ) == 0 ) break;
          close(_fd);
        }

        if(res_ == nullptr) throw std::runtime_error("No Address found");

        fd = _fd;
        connected = true;
        std::cout << "Connection Established " << host << "\n";

        freeaddrinfo(res);
      }else{
        throw std::runtime_error("Mx record not found");
      }


    }catch(std::exception const& exp){
      std::cerr << "ERROR: "<< exp.what();
      connected = false;
    }

  }


  std::string SMTPClient::parseHost(){
    std::string host = "";
    std::size_t at = to.find("@");
    if (at != std::string::npos){
      host = to.substr(at+1);
    }
    return host;

  }


  std::string SMTPClient::read() noexcept{
    std::string buffer;
    buffer.resize(MAX_IO_BUFFER_SIZE);
    ::recv(fd, &buffer[0], buffer.length(), 0);
    std::cout << buffer <<"\n";
    return buffer;
  }

  std::string SMTPClient::resolveMX(const char* host){
    try{
      std::string mx;
        const size_t size = 1024;
        unsigned char buffer[size];
        int r = ::res_query (host, C_IN, T_MX, buffer, size);
        if (r == -1) {
            throw std::runtime_error(hstrerror(h_errno));
        }
        else {
            if (r == static_cast<int> (size)) {
                throw std::runtime_error("Buffer too small reply truncated");
            }
        }
        HEADER *hdr = reinterpret_cast<HEADER*> (buffer);

        if (hdr->rcode != NOERROR) {
            switch (hdr->rcode) {
            case FORMERR:
                throw std::runtime_error("Format error");
                break;
            case SERVFAIL:
                throw std::runtime_error("Server failure");
                break;
            case NXDOMAIN:
                throw std::runtime_error("Name error");
                break;
            case NOTIMP:
                throw std::runtime_error("Not implemented");
                break;
            case REFUSED:
                throw std::runtime_error("Refused");
                break;
            default:
                throw std::runtime_error("Unknown error");
            }
        }

        int answers = ntohs (hdr->ancount);

        ns_msg m;
        int k = ns_initparse (buffer, r, &m);
        if (k == -1) {
            throw std::runtime_error(strerror(errno));
        }

        for (int i = 0; i < answers; ++i){
          ns_rr rr;
          int k = ns_parserr (&m, ns_s_an, i, &rr);
          if (k == -1) {
              throw std::runtime_error("ns_parserr failed");
          }


          const size_t size = NS_MAXDNAME;
          unsigned char name[size];
          int t = ns_rr_type (rr);

          const u_char *data = ns_rr_rdata (rr);
          if (t == T_MX) {

              ns_name_unpack (buffer, buffer + r, data + sizeof (u_int16_t),
                              name, size);
              char name2[size];
              ns_name_ntop (name, name2, size);

              mx.assign(name2,strlen(name2));
              break;

          }

       }

       return mx;

    }catch(std::exception const& exp){
      std::cerr << "ERROR: "<< host <<" "<< exp.what() <<"\n";
      return {};
    }




  }

  void SMTPClient::send(){
    connect();
    while(connected){
      std::unique_ptr<Response> response(new Response(read()));

      if(!response->parse()){
        std::cerr << "ERROR: Failed to parse response message" << '\n';
        break;
      }else{
        if(response->getStatusCode() == statusCodes::ServiceNotAvailable){
          printf("Service unavailable\n");
          break;
        }

      }
      //std::cout << response->getRawBuffer() << "\n";

      switch (state) {
        case EHLO:
          write("EHLO "+response->getStatusMessage());
          state = MAIL_FROM;
        break;

        case MAIL_FROM:
          write("MAIL FROM: <"+from+">");
          state = RCPT_TO;
        break;

        case RCPT_TO:

          if(response->getStatusCode() != statusCodes::Ok){
            state = ERROR;
            break;
          }

          write("RCPT TO:<"+to+">");
          state = DATA;
          break;

        break;

        case DATA:
          if(response->getStatusCode() != statusCodes::Ok){
            state = ERROR;
            break;
          }

          write("DATA");
          state = MSG;
        break;

        case MSG:
          if(response->getStatusCode() != statusCodes::StartMailInput){
            state = ERROR;
            break;
          }

          write("MIME-Version: 1.0\r\nSubject: "+subject+"\r\nFrom: "+senderName+"<"+from+">\r\nContent-Type: text/html; charset='UTF-8'\r\nContent-Transfer-Encoding: quoted-printable\r\nTo: <"+to+">\r\n"+message+"\r\n.");
          state = QUIT;
        break;

        case QUIT:
          if(response->getStatusCode() != statusCodes::Ok){
            state = ERROR;
            break;
          }

          write("QUIT");
          state = SUCCESS;
        break;

        case SUCCESS:
          std::cout << "Mail Sent Successfully." << "\n";
          connected = false;
        break;

        case ERROR:
        std::cout << "An error was encountered." << "\n";
          connected = false;
        break;


      }
    }

  }


  void SMTPClient::write(std::string&& buffer) noexcept{
    std::cout << buffer << "\n";
    buffer= buffer+"\r\n";
    ::send(fd, buffer.c_str(), buffer.length(), 0);
  }






}
