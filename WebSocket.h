/*************************************************************************************************/
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
/*************************************************************************************************/
/* 
 * File:   WebSocket.h
 * Author: Thong_Laptop
 *
 * Created on September 19, 2018, 11:43 AM
 */
/*************************************************************************************************/
#ifndef WEBSOCKET_H
#define WEBSOCKET_H
/*************************************************************************************************/
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include <sstream>
#include <typeinfo>
#include <memory>
/*************************************************************************************************/
#include "../Object/Object.h"
#include "../Vision/Vision.h"
#include "server_ws.hpp"
/*************************************************************************************************/
using namespace std;
/*************************************************************************************************/
using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
using WsConnection = WsServer::Connection;
using WsMessage = WsServer::Message;
using WsError = SimpleWeb::error_code;
using SendStream = WsServer::SendStream;
/*************************************************************************************************/
typedef std::shared_ptr<WsConnection> PConnection;
/*************************************************************************************************/
class WSClient {
public:
    WSClient(string ip, int port);
    virtual ~WSClient();
    bool checked(PConnection connection);
    void error(WsError* ec);
    void hadMessage(PConnection connection, string message);
private:
    string ip;
    int port;
    void sendMessage(PConnection connection, string message);
};
/*************************************************************************************************/
static vector<WSClient> clients;
static vector<Interface> cameras;
/*************************************************************************************************/
class WebSocket {
public:
    WebSocket();
    WebSocket(const WebSocket& orig);
    virtual ~WebSocket();
    
    void init(string link, int port);
private:
    WsServer server;
};
/*************************************************************************************************/
#endif /* WEBSOCKET_H */
/*************************************************************************************************/
