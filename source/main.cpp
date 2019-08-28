#include "smtpclient.hpp"

int main(int argc, char const *argv[]) {
  hive::SMTPClient client;
  client.setSubject("Jasper SMTP Server Test");
  client.setSenderName("Viva Jasper");
  client.setFrom("jasper@trygot.com");
  client.setTo("famewind@gmail.com");
  client.setMessage("<h1> Just a test from Jasper's SMTP Client </h1>");
  client.send();


  return 0;
}
