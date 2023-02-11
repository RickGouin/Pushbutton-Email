/**
 * Send an email at the push of a button
 * Author: Rick Gouin 
 * Full Build Details: https://www.rickgouin.com/build-a-device-to-send-emails-at-the-push-of-a-button/
 *
 * Based on Email example from: https://github.com/mobizt/ESP-Mail-Client
 */

#include <Arduino.h>
#if defined(ESP32) || defined(PICO_RP2040)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#endif

#include <ESP_Mail_Client.h>

#define WIFI_SSID "YOUR SSID HERE"
#define WIFI_PASSWORD "YOUR WIFI PASSWORD HERE"
#define SMTP_HOST "YOUR SMTP HOST HERE"
#define SMTP_PORT 465 //Update to whatever SMTP port you are using

/* The sign in credentials for sender account */
#define AUTHOR_EMAIL "SENDER EMAIL"
#define AUTHOR_PASSWORD "SENDER EMAIL PASSWORD"

// set pin numbers
const int checkinPin = 14;     	//aka Pin D5 the number of the pushbutton pin
const int checkoutPin =  12;    //aka Pin D6 the number of the pushbutton pin
const int greenLED = 4; 		    //aka pin D2 for the success LED
const int redLED = 5; 		      //aka pin D1 for the error LED

// variable for storing the pushbutton status
int inbuttonState = 0;
int outbuttonState = 0;

/* Declare the global used SMTPSession object for SMTP transport */
SMTPSession smtp;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

void setup()
{

  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to AP");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  /*  Set the network reconnection option */
  MailClient.networkReconnect(true);
  // initialize the pushbutton pin as an input
  pinMode(checkinPin, INPUT_PULLUP);
  pinMode(checkoutPin, INPUT_PULLUP);
  //initialize the LEDs as output
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
}

void loop()
{
  // read the state of the pushbutton value
  inbuttonState = digitalRead(checkinPin);
  outbuttonState = digitalRead(checkoutPin);
  if (inbuttonState == LOW) {
    Serial.println("Check-In Button Was Pushed");
    SendEmail("This is the text for the check-in button email", "Check-In Subject"); //Edit with your message text
  }
  if (outbuttonState == LOW) {
    Serial.println("Check-Out Button Was Pushed");
    SendEmail("This is the text for the check-in button email", "Check-Out Subject"); //Edit with your message text
  }
}

/* Send an email */
void SendEmail(String htmlMsg, String MessageSubject)
{
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the ESP_Mail_Session for user defined session credentials */
  ESP_Mail_Session session;

  /* Set the session config */
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;

  /* Set the NTP config time */
  session.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  session.time.gmt_offset = -5;
  session.time.day_light_offset = 0;
/* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = F("YOUR NAME"); //edit with the name of the sender
  message.sender.email = AUTHOR_EMAIL;
  message.subject = MessageSubject;
  message.addRecipient(F("NAME OF RECIPIENT"), F("recipient@whatever.com")); //edit with the email address you are sending to

  message.html.content = htmlMsg;

  message.html.charSet = F("us-ascii");

  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_normal;

  /* Connect to the server */
  if (!smtp.connect(&session /* session credentials */))
    Serial.println("Error connecting to SMTP server, " + smtp.errorReason());

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
  {
    Serial.println("Error sending Email, " + smtp.errorReason());
    Serial.println("Turning on Red LED");
    digitalWrite(redLED, HIGH); // turn the LED on
    delay(10000); // Leave the LED on for 10 seconds
    digitalWrite(redLED, LOW); // turn the LED off
  }
  else
  {
    Serial.println("Turning on Green LED");
    digitalWrite(greenLED, HIGH); // turn the LED on
    delay(10000); // Leave the LED on for 10 seconds
    digitalWrite(greenLED, LOW); // turn the LED off
  }
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status)
{
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will increase.
    smtp.sendingResult.clear();
  }
}