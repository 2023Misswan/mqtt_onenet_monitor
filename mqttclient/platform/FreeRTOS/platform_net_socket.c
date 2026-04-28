/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2020-01-10 23:45:59
 * @LastEditTime: 2020-04-25 17:50:58
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#include "platform_net_socket.h"
#include "platform_timer.h"
#include "usart.h"
#include "stdio.h"

 
int platform_net_socket_connect(const char *host, const char *port, int proto)
{
	char cmd[400];
	/*配置连接服务器的参数，根据自己的情况进行修改*/
	char *ssid = "手机热点ID";
	char *pwd  = "手机热点密码";
	char *dev_name = "设备名称";
	char *pro_id = "产品ID";
	char *token = "获取的Token";
	/*配置连接服务器的参数，根据自己的情况进行修改*/
	
	unsigned int rc;

	rc = ESP8266_SendCommand("AT+RST", "OK", 2000);
	if (rc)
		return -1;
	
	HAL_Delay(800);

	rc = ESP8266_SendCommand("AT+CWMODE=1", "OK", 1000);
	if (rc)
		return -1;
	
	rc = ESP8266_SendCommand("AT+CWDHCP=1,1", "OK", 1000);
	if (rc)
		return -1;

	sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", ssid, pwd);
	rc = ESP8266_SendCommand(cmd, "OK", 5000);
	if (rc)
		return -1;
	
	sprintf(cmd, "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"", dev_name, pro_id, token);
	rc = ESP8266_SendCommand(cmd, "OK", 1000);
	if (rc)
		return -1;
	
	rc = ESP8266_SendCommand("AT+CIPCLOSE", "OK", 500);
	
	/*连接OneNet云服务器，激活设备*/
	sprintf(cmd, "AT+MQTTCONN=0,\"%s\",%s,1", host, port);
	rc = ESP8266_SendCommand(cmd, "OK", 5000);
	if (rc)
		return -1;
	
	/*订阅主题*/
	sprintf(cmd, "AT+MQTTSUB=0,\"$sys/%s/%s/thing/property/post/reply\",0", pro_id , dev_name);
	rc = ESP8266_SendCommand(cmd, "OK", 1000);
	if (rc)
		return -1;
	
	sprintf(cmd, "AT+MQTTSUB=0,\"$sys/%s/%s/thing/property/set\",0", pro_id , dev_name);
	rc = ESP8266_SendCommand(cmd, "OK", 1000);
	if (rc)
		return -1;

	return 0;
}

int platform_net_socket_recv(int fd, void *buf, size_t len, int flags)
{
	/* 在mbedtls_net_recv里才会调用platform_net_socket_recv函数
	 * 我们没有使用mbedtls_net_recv
	 * 所以不用实现platform_net_socket_recv
	 */
    return 0;
}

int platform_net_socket_recv_timeout(int fd, unsigned char *buf, int len, int timeout)
{
	extern int ESP8266ReadPacket(unsigned char *buf, int len);
	int time = platform_timer_now();
	int rlen = 0;
	int ret = len;

	timeout += time;
	
	while ((platform_timer_now() < timeout) && len)
	{
		rlen = ESP8266ReadPacket(buf, len);
		buf += rlen;
		len -= rlen;
	}

	return (ret - len);
}

int platform_net_socket_write(int fd, void *buf, size_t len)
{
	/* 在mbedtls_net_send里才会调用platform_net_socket_write函数
	 * 我们没有使用mbedtls_net_recv
	 * 所以不用实现platform_net_socket_write
	 */
	return 0;
}

int platform_net_socket_write_timeout(int fd, unsigned char *buf, int len, int timeout)
{
	extern int ESP8266_Send_Packet(unsigned char *data, int len);
	int i;
	if (len == 2)
		printf("datas: 0x%x, 0x%x\r\n", buf[0], buf[1]);
	printf("Send Packet: ");
	for (i = 0; i < len; i++)
		printf("%02x ", buf[i]);
	printf("\r\n");
	return ESP8266_Send_Packet(buf, len);
}

int platform_net_socket_close(int fd)
{
	ESP8266_SendCommand("AT+CIPCLOSE", "OK", 500);
	return 0;
}

