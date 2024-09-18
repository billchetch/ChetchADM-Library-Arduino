//Board setup
#define DEVPIN 12 //set to false for production values
#define RESETPIN 49 //pin to control hardware reset on ethernet
#define STATUSPIN 48
#define MAX_SETUP_ATTEMPTS 3 //Setup completed means ethernet server started and this is registered as a service
#define RESET_IF_NO_CLIENT_TIMEOUT 0 //how long in millis to wait before trying a reset if client (set to 0 to not try a reset)
#define RESET_IF_HARDWARE_FAILURE_TIMEOUT 10000 //how long in millis since last hardware reset to try if there is a genuine hardware failure (set to 0 to not bother with a reset)
#define RESET_IF_NO_SERVER_TIMEOUT 10000 //how long in millis since last hardware reset to try again in the case that the ethernet server didn't start

//Dev or production
#define DEVIPCOMPONENT 4
#define PROIPCOMPONENT 2


//Ethernet setup ... normally only these need to change per board
#define ETHERNET_INIT_PIN 53 //This is the SCK pin set to -1 to use default (normally pin 10) 
#define ETHERNET_BEGIN_TIMEOUT 0 //this is so we get the opportunity to do an led flash or something, set to 0 to try once only
#define HOSTNAME "plankton" //change this per board
#define PORT 8091 
byte mac[] = {  0x00, 0xAD, 0xBE, 0xEF, 0xFE, 0xEC }; //change this per board
IPAddress ip(192, 168, 0, 13); //change this per board


//Regsiter this as a service setup (normally these seetings stay the same)
#define NETWORK_SERVICE_SERVER_PORT 8001
#define NETWORK_SERVICE_CONNECT_TIMEOUT 20000 
IPAddress router(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress networkServiceServerIP(192, 168, 0, 88);


//SFC values (these are good for Mega)
#define LOCAL_UART_BUFFER 256
#define REMOTE_UART_BUFFER 256
#define RECEIVE_BUFFER 512
#define SEND_BUFFER 512
#define CTS_TIMEOUT 4000