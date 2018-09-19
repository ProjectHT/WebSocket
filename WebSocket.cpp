/*************************************************************************************************/
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
/*************************************************************************************************/
/* 
 * File:   WebSocket.cpp
 * Author: Thong_Laptop
 * 
 * Created on September 19, 2018, 11:43 AM
 */
/*************************************************************************************************/
#include "WebSocket.h"
/*************************************************************************************************/
static void *threadServer(void *threadid) {
    WsServer* server = (WsServer*)threadid;
    cout << "Server start" << endl;
    server->start();
    cout << "Server stop" << endl;
}
/*************************************************************************************************/
WSClient::WSClient(string ip, int port) {
    this->ip = ip;
    this->port = port;
}
/*************************************************************************************************/
WSClient::~WSClient() {
    
}
/*************************************************************************************************/
bool WSClient::checked(PConnection connection) {
    if ((this->ip.compare(connection.get()->remote_endpoint_address()) == 0) && (connection.get()->remote_endpoint_port() == port)) {
        return true;
    } else {
        return false;
    }
}
/*************************************************************************************************/
void WSClient::error(WsError* ec) {
    
}
/*************************************************************************************************/
void WSClient::sendMessage(PConnection connection, string message) {
    auto send_stream = make_shared<SendStream>();
    *send_stream << message;
    connection->send(send_stream, [](const SimpleWeb::error_code &ec) {
        if(ec) {
            cout << "Sended error client" << endl;
        }
    });
}
/*************************************************************************************************/
void WSClient::hadMessage(PConnection connection, string message) {
    Object m_obj;
    int index = 0;
    if(m_obj.convertFromData(message, index) == false) {
        string t_str = "error decoded msg";
        Object t_obj;
        t_obj.set("error",t_str);
        cout << "Test | " << t_obj.convertToData() << endl;
        sendMessage(connection, t_obj.convertToData());
    } else {
        if(m_obj.name.compare("request_frame") == 0) {
            if(m_obj.size == 3) {
                string* p_str = (string*)(m_obj.data);
                string name_camera = p_str[0];
                int width_camera = atoi(p_str[1].c_str());
                int height_camera = atoi(p_str[2].c_str());
                for(int i = 0; i < cameras.size(); i++) {
                    if(cameras.at(i).getName().compare(name_camera) == 0) {
                        Mat buffer = cameras.at(i).getBuffer();
                        Mat resize_buffer;
                        cv::resize(buffer, resize_buffer, cv::Size(width_camera, height_camera));
                        vector<int> compression_params;
                        compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
                        compression_params.push_back(70);
                        vector<uchar> send_data_buffer;
                        cv::imencode(".jpg",resize_buffer,send_data_buffer,compression_params);
                        Object t_obj;
                        t_obj.set("frame",(char*)(send_data_buffer.data()), send_data_buffer.size());
                        sendMessage(connection, t_obj.convertToData());
                        buffer.release();
                        resize_buffer.release();
                        send_data_buffer.clear();
                        break;
                    }
                }
            } else {
                string t_str = "error info";
                Object t_obj;
                t_obj.set("error",t_str);
                sendMessage(connection, t_obj.convertToData());
            }
        } else if(m_obj.name.compare("request_add_camera") == 0) {
            if(m_obj.size == 3) {
                string* p_str = (string*)(m_obj.data);
                string name_camera = p_str[0];
                string link_camera = p_str[1];
                string link_store = p_str[2];
                bool _flag_ = false;
                for(int i = 0; i < cameras.size(); i++) {
                    if(cameras.at(i).getName().compare(name_camera) == 0) {
                        Object t_obj;
                        string t_str = "error name camera";
                        t_obj.set("result",t_str);
                        sendMessage(connection, t_obj.convertToData());
                        _flag_ = true;
                        break;
                    }
                    if(cameras.at(i).getLink().compare(link_camera) == 0) {
                        Object t_obj;
                        string t_str = "error link camera";
                        t_obj.set("result",t_str);
                        sendMessage(connection, t_obj.convertToData());
                        _flag_ = true;
                        break;
                    }
                }
                if(_flag_ == false) {
                    int index_camera = cameras.size();
                    cameras.push_back(Interface());
                    cameras.at(index_camera).init(name_camera,link_camera,link_store);
                    cameras.at(index_camera).setrun();
                    Object t_obj;
                    string t_str = "ok";
                    t_obj.set("result",t_str);
                    sendMessage(connection, t_obj.convertToData());
                }
            } else {
                Object t_obj;
                string t_str = "error info";
                t_obj.set("error",t_str);
                sendMessage(connection, t_obj.convertToData());
            }
        } else if(m_obj.name.compare("request_remove_camera") == 0) {
            if(m_obj.size == 1) {
                string* p_str = (string*)(m_obj.data);
                string name_camera = p_str[0];
                for(int i = 0; i < cameras.size(); i++) {
                    if(cameras.at(i).getName().compare(name_camera) == 0) {
                        cameras.at(i).setstop();
                        cameras.erase(cameras.begin()+i);
                        break;
                    }
                }
                Object t_obj;
                string t_str = "ok";
                t_obj.set("result",t_str);
                sendMessage(connection, t_obj.convertToData());
            } else {
                Object t_obj;
                string t_str = "error info";
                t_obj.set("error",t_str);
                sendMessage(connection, t_obj.convertToData());
            }
        } else if(m_obj.name.compare("request_list_camera") == 0) {
            if(cameras.size() > 0) {
                string* list_camera = new string[cameras.size()];
                for (int i = 0; i < cameras.size(); i++) {
                    list_camera[i] = cameras.at(i).getName();
                }
                Object t_obj;
                t_obj.set("list_camera", list_camera, cameras.size());
                sendMessage(connection, t_obj.convertToData());
                delete[] list_camera;
            } else {
                Object t_obj;
                string t_str = "empty";
                t_obj.set("list_camera", t_str);
                sendMessage(connection, t_obj.convertToData());
            }
        } else if(m_obj.name.compare("request_info_camera") == 0) {
            if(m_obj.size == 1) {
                bool _flag_ = false;
                string* p_str = (string*)(m_obj.data);
                string name_camera = p_str[0];
                for(int i = 0; i < cameras.size(); i++) {
                    if(cameras.at(i).getName().compare(name_camera) == 0) {
                        Object t_obj;
                        string t_str[3];
                        t_str[0] = cameras.at(i).getName();
                        t_str[1] = cameras.at(i).getLink();
                        t_str[2] = cameras.at(i).getPath();
                        t_obj.set("result",t_str);
                        sendMessage(connection, t_obj.convertToData());
                        _flag_ = true;
                        break;
                    }
                }
                if(_flag_ == false) {
                    Object t_obj;
                    string t_str = "didn't find camera";
                    t_obj.set("result",t_str);
                    sendMessage(connection, t_obj.convertToData());
                }
            } else {
                Object t_obj;
                string t_str = "error info";
                t_obj.set("error",t_str);
                sendMessage(connection, t_obj.convertToData());
            }
        } 
        else if(m_obj.name.compare("get_status_detect_person")) {
            if(m_obj.size == 1) {
                bool _flag_ = false;
                string* p_str = (string*)(m_obj.data);
                string name_camera = p_str[0];
                for(int i = 0; i < cameras.size(); i++) {
                    if(cameras.at(i).getName().compare(name_camera) == 0) {
                        Object t_obj;
                        string t_str = cameras.at(i).getStateDetectPerson()?"true":"false";
                        t_obj.set("result",t_str);
                        sendMessage(connection, t_obj.convertToData());
                        _flag_ = true;
                        break;
                    }
                }
                if(_flag_ == false) {
                    Object t_obj;
                    string t_str = "didn't find camera";
                    t_obj.set("result",t_str);
                    sendMessage(connection, t_obj.convertToData());
                }
            } else {
                Object t_obj;
                string t_str = "error info";
                t_obj.set("error",t_str);
                sendMessage(connection, t_obj.convertToData());
            }
        } else if(m_obj.name.compare("get_status_detect_face")) {
            if(m_obj.size == 1) {
                bool _flag_ = false;
                string* p_str = (string*)(m_obj.data);
                string name_camera = p_str[0];
                for(int i = 0; i < cameras.size(); i++) {
                    if(cameras.at(i).getName().compare(name_camera) == 0) {
                        Object t_obj;
                        string t_str = cameras.at(i).getStateDetectFace()?"true":"false";
                        t_obj.set("result",t_str);
                        sendMessage(connection, t_obj.convertToData());
                        _flag_ = true;
                        break;
                    }
                }
                if(_flag_ == false) {
                    Object t_obj;
                    string t_str = "didn't find camera";
                    t_obj.set("result",t_str);
                    sendMessage(connection, t_obj.convertToData());
                }
            } else {
                Object t_obj;
                string t_str = "error info";
                t_obj.set("error",t_str);
                sendMessage(connection, t_obj.convertToData());
            }
        } else if(m_obj.name.compare("set_status_detect_person")) {
            if(m_obj.size == 2) {
                bool _flag_ = false;
                string* p_str = (string*)(m_obj.data);
                string name_camera = p_str[0];
                for(int i = 0; i < cameras.size(); i++) {
                    if(cameras.at(i).getName().compare(name_camera) == 0) {
                        string t_status =  p_str[0];
                        if(t_status.compare("true") == 0) {
                            cameras.at(i).setDetectPerson(true);
                        } else if(t_status.compare("false") == 0) {
                            cameras.at(i).setDetectPerson(false);
                        }
                        Object t_obj;
                        string t_str = "ok";
                        t_obj.set("result",t_str);
                        sendMessage(connection, t_obj.convertToData());
                        _flag_ = true;
                        break;
                    }
                }
                if(_flag_ == false) {
                    Object t_obj;
                    string t_str = "didn't find camera";
                    t_obj.set("result",t_str);
                    sendMessage(connection, t_obj.convertToData());
                }
            } else {
                Object t_obj;
                string t_str = "error info";
                t_obj.set("error",t_str);
                sendMessage(connection, t_obj.convertToData());
            }
        } else if(m_obj.name.compare("set_status_detect_face")) {
            if(m_obj.size == 2) {
                bool _flag_ = false;
                string* p_str = (string*)(m_obj.data);
                string name_camera = p_str[0];
                for(int i = 0; i < cameras.size(); i++) {
                    if(cameras.at(i).getName().compare(name_camera) == 0) {
                        string t_status =  p_str[0];
                        if(t_status.compare("true") == 0) {
                            cameras.at(i).setDetectFace(true);
                        } else if(t_status.compare("false") == 0) {
                            cameras.at(i).setDetectFace(false);
                        }
                        Object t_obj;
                        string t_str = "ok";
                        t_obj.set("result",t_str);
                        sendMessage(connection, t_obj.convertToData());
                        _flag_ = true;
                        break;
                    }
                }
                if(_flag_ == false) {
                    Object t_obj;
                    string t_str = "didn't find camera";
                    t_obj.set("result",t_str);
                    sendMessage(connection, t_obj.convertToData());
                }
            } else {
                Object t_obj;
                string t_str = "error info";
                t_obj.set("error",t_str);
                sendMessage(connection, t_obj.convertToData());
            }
        }
    }
}
/*************************************************************************************************/
WebSocket::WebSocket() {
    initVision();
    cameras.clear();
    clients.clear();
}
/*************************************************************************************************/
WebSocket::WebSocket(const WebSocket& orig) {
}
/*************************************************************************************************/
WebSocket::~WebSocket() {
}
/*************************************************************************************************/
void WebSocket::init(string link, int port) {
    server.config.port = port;
    string str = "^/" + link + "/?$";
#ifdef M_DEBUG
    my_log.write("WebSocket", "init with link : " + link + " - port : " + to_string(port));
#endif
    auto &echo = server.endpoint[str];
    echo.on_message = [](PConnection connection, shared_ptr<WsMessage> message) {
        string msg = message->string();
        for(int i = 0; i < clients.size(); i++) {
            if(clients.at(i).checked(connection)) {
                clients.at(i).hadMessage(connection, msg);
                break;
            }
        }
    };

    echo.on_open = [](PConnection connection) {
        clients.push_back(WSClient(connection.get()->remote_endpoint_address(), connection.get()->remote_endpoint_port()));
#ifdef M_DEBUG
        my_log.write("WebSocket", "added client - ip : " + connection.get()->remote_endpoint_address() + " - port : " + to_string(connection.get()->remote_endpoint_port()));
#endif
        cout << "New client - ip : " + connection.get()->remote_endpoint_address() + " - port : " + to_string(connection.get()->remote_endpoint_port());
    };

    echo.on_close = [](PConnection connection, int status, const string & /*reason*/) {
        for(int i = 0; i < clients.size(); i++) {
            if(clients.at(i).checked(connection)) {
                clients.erase(clients.begin() + i);
                break;
            }
        }
    };

    echo.on_error = [](PConnection connection, const WsError &ec) {
        for(int i = 0; i < clients.size(); i++) {
            if(clients.at(i).checked(connection)) {
                clients.at(i).error((WsError*)&ec);
                break;
            }
        }
    };
    
    pthread_t thread_server;
    int rc;
    rc = pthread_create(&thread_server, NULL, threadServer, (void *)&server);
}
/*************************************************************************************************/
