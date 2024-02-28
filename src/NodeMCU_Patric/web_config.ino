//Raw content of index.html in plain text
static const char *index_html = R"--espform--(
<!DOCTYPE html>
<html lang="en">
<head>
    <title>Karen Smart System</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            padding: 20px;
            margin: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            box-sizing: border-box;
        }

        form {
            background: white;
            padding: 20px;
            border-radius: 5px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
        }

        label {
            font-size: 17px;
            margin-top: 10px;
        }

        input[type=text] {
            display: block;
            width: calc(100% - 20px);
            height: 40px;
            padding: 4px 10px;
            margin: 10px 0;
            border: 1px solid #03A9F4;
            background: #cce6ff;
            color: #1c87c9;
            font-size: 17px;
            box-sizing: border-box;
        }

        #submit {
            cursor: pointer;
            width: 100%;
            height: 40px;
            padding: 4px 10px;
            margin: 10px 0;
            border: none;
            background: #39ad50;
            color: #ffffff;
            font-size: 17px;
            border-radius: 5px;
        }

        @media (max-width: 600px) {
            input[type=text] {
                width: 100%;
            }
        }
    </style>
</head>
<body>
    <form method="POST" action="/submit">
        <label for="ssid">SSID to device</label>
        <input type="text" id="ssid" name="ssid"/>
        <label for="pass">PASS to device</label>
        <input type="text" id="pass" name="pass"/>
        <input type="submit" id="submit" value="Save"/>
    </form>
</body>
</html>
)--espform--";


//-----------------------------------------------------------------------------------------------------

void handleFormSubmit() {
  if (server.hasArg("ssid") && server.hasArg("pass")) {
    String ssid = server.arg("ssid");
    String password = server.arg("pass");
    Serial.println(ssid);
    Serial.println(password);

    server.send(200, "text/plain", "OK");

    memoryService.startEEPROMSession(1024);
    memoryService.writeStatus(1);
    memoryService.writeSsidAndPass(ssid, password);
    memoryService.endEEPROMSession();

    Serial.println("Reset..");
    ESP.restart();
  } else {
    server.send(400, "text/plain", "Bad Request: Missing ssid or pass");
  }
}
//-----------------------------------------------------------------------------------------------------

void handleMainHtmlPage() {
  server.send(200, "text/html", index_html);
}
