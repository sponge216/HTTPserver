#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string.h>
#include <windows.h>
#include <dbnetlib.h>
#include <math.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT "80"
#define BACKLOG 15
#define MAX_REQ_SIZE 1024
#define ROOT "C:\\Users\\yoava\\Downloads\\free_robux"
#define GET "GET"
#define OK "HTTP/1.1 200 OK\r\n"
#define OKsize 17
#define CONTENT_TYPE "Content-type: "
#define CTsize 14
#define CHARSET "; charset=utf-8"
#define CHARSETsize 15
#define CONTENT_LENGTH "Content-Length: "
#define CLENsize 16
#define RN "\r\n"
#define RNsize 2
#define READ_SIZE 2048

typedef struct client {

	SOCKET socket;

}client_t, * clientPtr;

char* getReqMethod(char* req, int* index) {
	*index = 0;
	char method[30] = { 0 };

	while (req[*index] != ' ') {
		method[*index] = req[*index];
		(*index)++;
	}
	return method;
}

char* getReqService(char* req) {

	char service[MAX_REQ_SIZE] = { 0 };
	int i = 0;
	while (req[i] != ' ') {
		if (req[i] == '/')	service[i] = '\\';
		else service[i] = req[i];
		i++;
	}
	return service;

}
char* getFileExtension(char* path) {
	int pSize = strlen(path);
	int psTemp = pSize;
	int i = 0;
	char* extension;
	while (path[psTemp - 1] != '.') {
		psTemp--;
	}
	extension = (char*)malloc(pSize - psTemp + 1); // malloc according to extension length.
	while (path[psTemp] != '\0') {
		extension[i] = path[psTemp];
		psTemp++;
		i++;
	}
	extension[i] = '\0';// since we malloced, we need to add \0 to the end of the string.
	return extension;
}
char* getContentTypeByExtension(char* extension) {
	if (!strcmp(extension, "css")) return "text/css";
	if (!strcmp(extension, "csv")) return "text/csv";
	if (!strcmp(extension, "gif")) return "image/gif";
	if (!strcmp(extension, "html")) return "text/html";
	if (!strcmp(extension, "ico")) return "image/vnd.microsoft.icon";
	if (!strcmp(extension, "jpeg") || !(strcmp(extension, "jpg"))) return "image/jpeg";
	if (!strcmp(extension, "js")) return "text/javascript";
	if (!strcmp(extension, "json")) return "application/json";
	if (!strcmp(extension, "otf")) return "font/otf";
	if (!strcmp(extension, "png")) return "image/png";
	if (!strcmp(extension, "pdf")) return "application/pdf";
	if (!strcmp(extension, "tif") || !strcmp(extension, "tiff")) return "image/tiff";
	if (!strcmp(extension, "ttf")) return "font/ttf";
	if (!strcmp(extension, "txt")) return "text/plain";
	if (!strcmp(extension, "webp")) return "image/webp";
	if (!strcmp(extension, "zip")) return "application/zip";

	return "*/*";

}
void end_function(clientPtr client, char* msg) {
	perror(msg);
	closesocket(client->socket);
}
void* client_handler(void* data) {

	fprintf(stdout, "IN THREAD\n");

	clientPtr client = (clientPtr)data;
	FILE* fp;

	char buffer[MAX_REQ_SIZE] = { 0 }; // a buffer to hold the received request from the client
	char* method; // the request's method
	char* service; // the requested file
	char path[MAX_REQ_SIZE] = ROOT; // the root dir where we store files we want clients to reach
	char* response; // the first response to the client, including the headers and such things.
	char* contentType; // the type of content requested by the client.
	char* extension; // the extension representing the content requested by the client.
	char fBuffer[READ_SIZE] = { 0 }; // a buffer to hold the file requested by the client. Also used to send the file separate from the headers.

	int finalSize = 0; //the size of everything together.
	int fSizeSize = 0; //size of the string.
	int contentTypeLen = 0; // length of contentType.
	int valread = 0; // used to store values from read/recv functions.
	int serviceIndex = 0; // used in getReqMethod. Signifies the start of the service.
	int fSize = 0; // file size

	if ((valread = recv(client->socket, buffer, MAX_REQ_SIZE - 1, 0)) < 0) { //receive from client
		end_function(client, "RECV FUCKED");
		fprintf(stdout, "%d\n", valread);
		ExitThread(1);
	}
	method = getReqMethod(buffer, &serviceIndex); // get method from the request.
	if (strcmp(method, GET)) { // validate method
		end_function(client, "GET FUCKED");
		ExitThread(1);
	}
	service = getReqService(buffer + serviceIndex + 1); // add serviceIndex to buffer in order to skip the method and get the service.
	strcat(path, service); // add the requested service to the root path.
	fprintf(stdout, "%s\n", path);
	fp = fopen(path, "rb"); // open the file requested.
	if (fp <= 2) { // validate file pointer.
		end_function(client, "FILE IS FUCKED");
		ExitThread(1);
	}

	fseek(fp, 0, SEEK_END); // go to end of file.
	fSize = ftell(fp); // compare value of beginning and end of file to get the size.
	fseek(fp, 0, SEEK_SET); // go back to start of file.

	extension = getFileExtension(path); // get the extension from the requested service.
	contentType = getContentTypeByExtension(extension); // get the appropriate content-type response according to the file extension.
	contentTypeLen = strlen(contentType); // get length of contentType
	fSizeSize = (int)(log10((double)fSize) + 1); // get length of fSize

	if (contentType[0] == 't') { // if content-type starts with "text", we need to include CHARSET.
		finalSize = OKsize + CTsize + contentTypeLen + CHARSETsize + CLENsize + fSizeSize + 3 * RNsize; // calculate the combined size of all variables.
		if ((response = (char*)malloc(finalSize + 1)) == NULL) { // malloc for response.
			end_function(client, "MALLOC FUCKED");
			ExitThread(1);
		}
		sprintf(response, "%s%s%s%s%s%s%d%s%s", OK, CONTENT_TYPE, contentType, CHARSET, RN, CONTENT_LENGTH, fSize, RN, RN); //load the needed text into response
	}
	else {// if "text" isnt in content-type
		finalSize = OKsize + CTsize + contentTypeLen + CLENsize + fSizeSize + 3 * RNsize; // calculate the combined size of all variables.
		if ((response = (char*)malloc(finalSize + 1)) == NULL) { // malloc for response.
			end_function(client, "MALLOC FUCKED");
			ExitThread(1);
		}
		sprintf(response, "%s%s%s%s%s%d%s%s", OK, CONTENT_TYPE, contentType, RN, CONTENT_LENGTH, fSize, RN, RN); //load the needed text into response
	}

	send(client->socket, response, finalSize, 0); // SEND to client the header response

	while ((valread = fread(fBuffer, sizeof(char), READ_SIZE, fp)) > 0) {

		send(client->socket, fBuffer, valread, 0); // SEND to client the file in fragments.
		//fprintf(stdout, "FROM %d SENDING %d\n", client->socket, valread);
	}

	free(response);
	free(extension);
	end_function(client, "\nGOOD JOB!!!!");
	ExitThread(1);
	return NULL;
}
int main(int argc, char** argv) {

	WSADATA wsa_data; // needed in order to use sockets in windows.

	if (WSAStartup(MAKEWORD(2, 2), &wsa_data)) { // start the window socket application
		perror("SETUP FUCKED"); exit(1);
	}

	SOCKET server_socket; // socket for the server.
	struct addrinfo* server_addr = NULL, // the server's address info struct, that holds all info about the address.
		hints; // used to set the socket's behavior and address.

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; // set internet family.
	hints.ai_socktype = SOCK_STREAM; //set socket type. We use TCP so we set it to sock_stream.
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	int res = getaddrinfo(NULL, PORT, &hints, &server_addr);
	if (res != 0) {
		printf("getaddrinfo failed: %d\n", res);
		WSACleanup();
		exit(1);
	}

	if ((server_socket = socket(server_addr->ai_family, server_addr->ai_socktype, server_addr->ai_protocol)) <= 1) {
		perror("SOCKET SUCKS ASS");
		exit(1);
	}
	if (server_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(server_addr);
		WSACleanup();
		exit(1);
	}

	if (bind(server_socket, server_addr->ai_addr, (int)server_addr->ai_addrlen) == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(server_addr);
		closesocket(server_socket);
		WSACleanup();
		exit(1);
	}
	printf("all good!\n");
	if (listen(server_socket, BACKLOG) < 0) {
		perror("LISTEN FUCKED");
		exit(1);
	}
	while (1) {

		client_t client;
		SOCKET client_socket;
		if ((client_socket = accept(server_socket, NULL, NULL)) != INVALID_SOCKET) {

			fprintf(stdout, "NEW CLIENT!!!  :%d\n", client_socket);;
			client.socket = client_socket;
			CreateThread(NULL, 0, client_handler, (void*)&client, 0, NULL);

		}

	}

	return 0;
}