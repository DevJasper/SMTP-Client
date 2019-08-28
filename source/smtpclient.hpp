#ifndef SMTP_CLIENT_HPP
#define SMTP_CLIENT_HPP

#include <string>
#include <utility>

namespace hive
{

  class SMTPClient
  {
    public:
      explicit SMTPClient() = default;
      ~SMTPClient();
      std::string parseHost();
      std::string resolveMX(const char*);
      void send();

      inline void setFrom(std::string&& _from) noexcept{
        from = std::move(_from);
      }

      inline void setMessage(std::string&& _message) noexcept{
        message = std::move(_message);
      }


      inline void setSenderName(std::string&& _senderName) noexcept{
        senderName = std::move(_senderName);
      }

      inline void setSubject(std::string&& _subject) noexcept{
        subject = std::move(_subject);
      }

      inline void setTo(std::string&& _to) noexcept{
        to = std::move(_to);
      }
    
      inline void setTo(std::string& _to) noexcept{
        to = std::move(_to);
      }

    private:
      void close(const int) noexcept;
      void connect();
      std::string read() noexcept;
      void write(std::string&&) noexcept;

      std::string host, senderName, subject, from, to, message;
      int fd, port;
      bool connected;
  };

}



#endif
