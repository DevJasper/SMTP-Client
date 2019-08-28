#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>


namespace hive
{

  class Response
  {
    public:
      Response(const std::string&&);
      ~Response() = default;

      inline std::string getRawBuffer()const noexcept{
        return rawBUffer;
      }

      inline int getStatusCode()const noexcept{
        return statusCode;
      }


      inline std::string getStatusMessage()const noexcept{
        return statusMessage;
      }

      bool parse();

    private:

      int statusCode;
      std::string rawBUffer, domain, statusMessage;


  };


}


#endif
