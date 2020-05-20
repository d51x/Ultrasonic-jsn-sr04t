#ifndef WEBSERVER_H
#define WEBSERVER_H

const char PAGE_HEADER[] PROGMEM = R"=====(
<!DOCTYPE html>
<HTML>
    <HEAD>
      <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
      <TITLE>%s</TITLE>
      <style>%s</style>
</HEAD>
<BODY>
)=====";

const char PAGE_FOOTER[] PROGMEM = R"=====(
<footer>%s</footer>
</BODY>
</HTML>
)=====";

const char STYLE_CSS[] PROGMEM = R"=====(
html{ font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}
.button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}
.button-on {background-color: #1abc9c;}
.button-on:active {background-color: #16a085;}
.button-off {background-color: #34495e;}
.button-off:active {background-color: #2c3e50;}
p {font-size: 14px;color: #888;margin-bottom: 10px;
)=====";

#endif
