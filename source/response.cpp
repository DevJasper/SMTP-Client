#include "response.hpp"

#include <iostream>
#include <sstream>
#include <utility>

namespace hive
{

  Response::Response(const std::string&& buffer) : statusCode(-1), rawBUffer(std::move(buffer)){}

  bool Response::parse(){

    std::istringstream ss(rawBUffer);
    std::string temp;
    int line = 0;

    while(getline(ss,temp,' '))
    {
        ++ line;
        if(line == 1){
          statusCode = std::stoi(temp);
          continue;
        }

        if(line == 2){
          statusMessage = temp;
          break;
        }
    }

    return (!statusCode) ? false : true;
  }


}
