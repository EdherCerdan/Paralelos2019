#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <map>
#include <sstream>
#include "protocol.h"
#include <vector>
#include <fstream>
#include <mutex>          // std::mutex
#include <algorithm>
#include <queue> 

std::mutex mtx;           // mutex for critical section

using namespace std;

///TRACKER'S IP ADDRESSS
string tracker_ip_address = "0.0.0.0";

///TRACKER'S PORT
int tracker_port = 121213;
int verif_size_file;

///THIS DEVICE'S IP ADDRESS
string my_ip_address = "0.0.0.0";

///THIS DEVICE'S PORT
int my_port = -1;




vector<string> chunks; //list of chunks that THIS PEER has
map<int, string> peers; //saving peers from tracker to ask all each peer (port, ip)
map<string, string> relaciones; //saving peers from tracker to ask all each peer (port, ip)
map<string, string> nodes; //saving peers from tracker to ask all each peer (port, ip)
map<int, string> hash_slave; //saving peers from tracker to ask all each peer (port, ip)



map<int, string> chunks_from_peers; //(port, list_of_chunks)




int StringToNumber(string a)
{
   	int number;
	std::istringstream iss (a);
	iss >> number;
	return number;
}

//Check the size of a string
string read_size(string text)
{
	char buffer[2];
	sprintf(buffer,"%02d",(unsigned int)text.size());
	return buffer;
}




//Function that reads the reply of a server
void read_socketToServer(int socketToServer, int port_number=-1)
{
	int size;
	char buffer[1035]; //max size of message
	string type;
	int cantidad_usuarios = 0;
	int cantidad_chunks = 0;
	int n;
	string message;
	string node_name;
	string node_name_2;
	string valor;
	n = read(socketToServer, buffer ,1); //read just the first byte to check the message's type
	if(n==0)
	{
		return;
	}
	if (buffer[0]=='p') //Peers' list
	{
		bzero(buffer,1); //cleaning the buffer of the header-command
		read(socketToServer,buffer,2); //reading header with number of users
		cantidad_usuarios = atoi(buffer);
		bzero(buffer,2); //cleaning the buffer of the header-number of users
		cout<<"List of available peers:"<<endl;
		string ip_;
		int port_;
		int i=0;			
		for(int i=0; i<cantidad_usuarios; i++)
		{
			read(socketToServer,buffer,2); //reading header with size of ip address
			size = atoi(buffer);
			bzero(buffer,2); //cleaning the buffer of the header-size of ip address
			read(socketToServer,buffer,size); //reading ip address
			ip_= string(buffer);
			cout<< "IP: " << buffer << "   -   "; //print ip address
			bzero(buffer,size); //cleaning the buffer of the nickname
			read(socketToServer,buffer,2); //reading header with size of port
			size = atoi(buffer);
			bzero(buffer,2); //cleaning the buffer of the header-size of port
			read(socketToServer,buffer,size); //reading port
			cout<< "Port: " << buffer <<endl; //print port
			port_= atoi(buffer);
			bzero(buffer,size); //cleaning the buffer of the port
			peers[port_] = ip_; //add peer to the map (port,ip)
			

		}
	}
	else if (buffer[0]=='h') //Peers' list
	{
		cout<<"ENTRO A h"<<endl;
		bzero(buffer,1); //cleaning the buffer of the header-command
		read(socketToServer,buffer,2); //reading header with number of users
		cantidad_usuarios = atoi(buffer);
		bzero(buffer,2); //cleaning the buffer of the header-number of users
		cout<<"List of available peers:"<<endl;
		string ip_;
		int port_;
		int i=0;			
		for(int i=0; i<cantidad_usuarios; i++)
		{
			read(socketToServer,buffer,2); //reading header with size of ip address
			size = atoi(buffer);
			bzero(buffer,2); //cleaning the buffer of the header-size of ip address
			read(socketToServer,buffer,size); //reading ip address
			ip_= string(buffer);
			cout<< "PORT: " << buffer << "   -   "; //print ip address
			bzero(buffer,size); //cleaning the buffer of the nickname
			read(socketToServer,buffer,2); //reading header with size of port
			size = atoi(buffer);
			bzero(buffer,2); //cleaning the buffer of the header-size of port
			read(socketToServer,buffer,size); //reading port
			cout<< "NODO: " << buffer <<endl; //print port
			port_= atoi(buffer);
			bzero(buffer,size); //cleaning the buffer of the port
			//peers[port_] = ip_; //add peer to the map (port,ip)
			

		}
	}
	else if (buffer[0]=='A') //acknowledgments
	{
		bzero(buffer,1); //cleaning the buffer of the header-command
		read(socketToServer,buffer,2);
		if(buffer[0]=='1')
		{
			cout<<"Registration was succesful"<<endl;
		}
		bzero(buffer,2); //cleaning the buffer of the type of acknowledgment
	}
	else if (buffer[0]=='E') //errors
	{
		bzero(buffer,1); //cleaning the buffer of the header-command
		read(socketToServer,buffer,2);
		if(buffer[0]=='1') //registration error
		{
			cout<<"Register ERROR"<<endl;
			cout<<"Enter your port: ";
			cin>>my_port;
			message = "I" + read_size(my_ip_address) + my_ip_address + read_size(to_string(my_port)) + to_string(my_port);
			write(socketToServer, message.c_str(), message.length());
			read_socketToServer(socketToServer); //we read registration answer
		}
		if(buffer[0]=='2') //chunk request error
		{
			cout<<"Requested chunk not found."<<endl;
		}
		bzero(buffer,2); //cleaning the buffer of the type of error
	}
	else if(buffer[0] == 'I' ) //INSERT NODE
 	{
 			bzero(buffer,1); //cleaning the buffer
			
			if(buffer[0]=='N'){ //NODE
				//Reading node
	   			read(socketToServer, buffer, 2);
				size = atoi(buffer);
				bzero(buffer,2); //cleaning the buffer
				read(socketToServer, buffer, size);
				node_name = string(buffer);
				bzero(buffer,size); //cleaning the buffer
				read(socketToServer, buffer, 2);
				size = atoi(buffer);
				bzero(buffer,2); //cleaning the buffer
				read(socketToServer, buffer, size);
				valor = string(buffer);
				bzero(buffer,size); //cleaning the buffer
				nodes[node_name]=valor;

			}
			if(buffer[0]=='R'){ //RELATION
			 		//Reading node
	   			read(socketToServer, buffer, 2);
				size = atoi(buffer);
				bzero(buffer,2); //cleaning the buffer
				read(socketToServer, buffer, size);
				node_name = string(buffer);
				bzero(buffer,size); //cleaning the buffer
				
				read(socketToServer, buffer, 2);
				size = atoi(buffer);
				bzero(buffer,2); //cleaning the buffer
				read(socketToServer, buffer, size);
				node_name_2 = string(buffer);
				bzero(buffer,size); //cleaning the buffer
				relaciones.insert (std::pair<string,string>(node_name,node_name_2));
				//relaciones[node_name] = node_name_2;			
				
			}
			map<int,string>::iterator iter_peer; //iterator on list of peers
			
	}

}

//Function that send a file through a given socket 
void send_file(int socketToClient, string filename)
{
	string to_send;
        string binary_file;
        string message_to_client = "";
        int size_file;
        file_utils::get_size_string_of_file(filename, binary_file, size_file);
        to_send = file_utils::prepare_file_message(binary_file, size_file, filename);
        write(socketToClient, to_send.c_str(), to_send.size());
}

//Function that reads the request from a client
void handle_client_request(int socketToClient){
	int size;
	char buffer[1000];
	string type;
	int cantidad_usuarios=0;
	int n;
	string message = "";
	string node_name;
	string node_name_2;
	string valor;
		

	
	n = read(socketToClient, buffer ,1); //read just the first byte to check the message's type
	if(n==0)
	{
		return;
	}
	
	//From tracker client
	if (buffer[0]=='p') //Peers' list
	{
		bzero(buffer,1); //cleaning the buffer of the header-command
		read(socketToClient,buffer,2); //reading header with number of users
		cantidad_usuarios = atoi(buffer);
		bzero(buffer,2); //cleaning the buffer of the header-number of users
		cout<<"List of available peers:"<<endl;
		for(int i=0; i<cantidad_usuarios; i++)
		{
			read(socketToClient,buffer,2); //reading header with size of ip address
			size = atoi(buffer);
			bzero(buffer,2); //cleaning the buffer of the header-size of ip address
			read(socketToClient,buffer,size); //reading ip address
			cout<< "IP: " << buffer << "   -   "; //print ip address
			bzero(buffer,size); //cleaning the buffer of the nickname
			read(socketToClient,buffer,2); //reading header with size of port
			size = atoi(buffer);
			bzero(buffer,2); //cleaning the buffer of the header-size of port
			read(socketToClient,buffer,size); //reading port
			cout<< "Port: " << buffer <<endl; //print port
			bzero(buffer,size); //cleaning the buffer of the port
		}			
	}
	else if (buffer[0]=='?') //Checking connection
	{
		bzero(buffer,1); //cleaning the buffer of the header-command
		write(socketToClient,"A",1);//We reply, probing that we are connected		
	}		
	
	
	else if(buffer[0] == 'I' ) //INSERT 
	 	{
	 			bzero(buffer,1); //cleaning the buffer
				read(socketToClient,buffer,1);
				
				if(buffer[0]=='N'){
					bzero(buffer,1);
					//Reading node
		   			read(socketToClient, buffer, 2);
					size = atoi(buffer);
					bzero(buffer,2); //cleaning the buffer
					read(socketToClient, buffer, size);
					node_name = string(buffer);
					bzero(buffer,size); //cleaning the buffer
					read(socketToClient, buffer, 2);
					size = atoi(buffer);
					bzero(buffer,2); //cleaning the buffer
					read(socketToClient, buffer, size);
					valor = string(buffer);
					bzero(buffer,size); //cleaning the buffer
					//cout<<node_name<<" "<<valor<<endl;
					cout<<node_name<<" "<<valor<<endl;
					
					nodes[node_name] = valor;

				}
				if(buffer[0]=='R'){
					bzero(buffer,1);
				 		//Reading node
		   			read(socketToClient, buffer, 2);
					size = atoi(buffer);
					bzero(buffer,2); //cleaning the buffer
					read(socketToClient, buffer, size);
					node_name = string(buffer);
					bzero(buffer,size); //cleaning the buffer
					
					read(socketToClient, buffer, 2);
					size = atoi(buffer);
					bzero(buffer,2); //cleaning the buffer
					read(socketToClient, buffer, size);
					node_name_2 = string(buffer);
					bzero(buffer,size); 

				}
				map<string,string>::iterator iter_peer; //iterator on list of peers
				//map<int,string>::iterator iter_peer; //iterator on list of peers
				for(iter_peer=nodes.begin(); iter_peer!=nodes.end(); ++iter_peer)
				{
					cout<<"Nodo : "<< iter_peer->first;
					cout<<"Value: "<< iter_peer->second<<endl;					
				}
		}
	else if(buffer[0] == 'Q' ) //Login
	 	{
	 			bzero(buffer,1); //cleaning the buffer
				read(socketToClient,buffer,1);
				
				if(buffer[0]=='N'){
					bzero(buffer,1);
					//Reading node
		   			read(socketToClient, buffer, 2);
					size = atoi(buffer);
					bzero(buffer,2); //cleaning the buffer
					read(socketToClient, buffer, size);
					node_name = string(buffer);
					bzero(buffer,size); 
					map<string,string>::iterator iter_peer; //iterator on list of peers
					for(iter_peer=nodes.begin(); iter_peer!=nodes.end(); ++iter_peer)
					{
						if(node_name==iter_peer->first)
							cout<<"Si hay nodo"<<endl;
						else cout<<"No hay nodo"<<endl;
					}	
				}
				if(buffer[0]=='R'){
					bzero(buffer,1);
				 		//Reading node
		   			read(socketToClient, buffer, 2);
					size = atoi(buffer);
					bzero(buffer,2); //cleaning the buffer
					read(socketToClient, buffer, size);
					node_name = string(buffer);
					bzero(buffer,size); //cleaning the buffer
					
					read(socketToClient, buffer, 2);
					size = atoi(buffer);
					bzero(buffer,2); //cleaning the buffer
					read(socketToClient, buffer, size);
					node_name_2 = string(buffer);
					bzero(buffer,size); //cleaning the buffer
					//relaciones[node_name] = node_name_2;			
				
					map<string,string>::iterator iter_peer; //iterator on list of peers
				//map<int,string>::iterator iter_peer; //iterator on list of peers
					for(iter_peer=relaciones.begin(); iter_peer!=relaciones.end(); ++iter_peer)
					{
						
						if(node_name == iter_peer->first && node_name_2 == iter_peer->second )
							cout<<"Si hay relacion: "<<endl;
						else cout<<"No hay relacion: "<<endl;
					}	
				}
			
		}
	else if(buffer[0]=='V'){
					bzero(buffer,1);
					//Reading node
		   			read(socketToClient, buffer, 2);
					size = atoi(buffer);
					bzero(buffer,2); //cleaning the buffer
					read(socketToClient, buffer, size);
					node_name = string(buffer);
					bzero(buffer,size); //cleaning the buffer
					//cout<<node_name<<" "<<valor<<endl;
					map<string,string>::iterator iter_peer; //iterator on list of peers
					for(iter_peer=nodes.begin(); iter_peer!=nodes.end(); ++iter_peer)
					{
						if(node_name==iter_peer->first)
							cout<<"VALOR "<<iter_peer->second<<endl;
						else cout<<"No hay nodo"<<endl;
					}	
	    }
	else if(buffer[0]=='U'){
					bzero(buffer,1);
					//Reading node
		   			read(socketToClient, buffer, 2);
					size = atoi(buffer);
					bzero(buffer,2); //cleaning the buffer
					read(socketToClient, buffer, size);
					node_name = string(buffer);
					bzero(buffer,size); //cleaning the buffer
					read(socketToClient, buffer, 2);
					size = atoi(buffer);
					bzero(buffer,2); //cleaning the buffer
					read(socketToClient, buffer, size);
					valor= string(buffer);
					bzero(buffer,size); //cleaning the buffer
					
					//cout<<node_name<<" "<<valor<<endl;
					map<string,string>::iterator iter_peer; //iterator on list of peers
					for(iter_peer=nodes.begin(); iter_peer!=nodes.end(); ++iter_peer)
					{
						if(node_name==iter_peer->first){
							cout<<"Cambiando de valor de: "<<iter_peer->second<<" a "<<valor;
							nodes[node_name]=valor;
							//iter_peer->first=valor;
						}
						else cout<<"No hay nodo"<<endl;
					}	
	    }
	else if(buffer[0]=='t'){
					int size;
					string node_name;
					string node_name_2;
					bzero(buffer,1);
				 		//Reading node
		   			read(socketToClient, buffer, 2);
					size = atoi(buffer);
					bzero(buffer,2); //cleaning the buffer
					read(socketToClient, buffer, size);
					node_name = string(buffer);
					bzero(buffer,size); //cleaning the buffer
					
					read(socketToClient, buffer, 2);
					size = atoi(buffer);
					bzero(buffer,2); //cleaning the buffer
					read(socketToClient, buffer, size);
					node_name_2 = string(buffer);
					bzero(buffer,size); //cleaning the buffer
					relaciones.insert (std::pair<string,string>(node_name,node_name_2));
						
					map<string,string>::iterator iter_peer; //iterator on list of peers
				
					for(iter_peer=relaciones.begin(); iter_peer!=relaciones.end(); ++iter_peer)
					{
						cout<<"Nodo 1: "<< iter_peer->first;
						cout<<"  Nodo 2: "<< iter_peer->second <<endl;					
					}	
	    }   
	 else if(buffer[0] == 'D' ) //Login
	 	{
	 			//cout<<"entrojejeje"<<endl;
	 			bzero(buffer,1); //cleaning the buffer
				read(socketToClient,buffer,1);
				
				if(buffer[0]=='N'){
					bzero(buffer,1);
					//Reading node
		   			read(socketToClient, buffer, 2);
					size = atoi(buffer);
					bzero(buffer,2); //cleaning the buffer
					read(socketToClient, buffer, size);
					node_name = string(buffer);
					bzero(buffer,size); //cleaning the buffer
					//cout<<node_name<<" "<<valor<<endl;
					map<string,string>::iterator iter_peer; //iterator on list of peers
					for(iter_peer=nodes.begin(); iter_peer!=nodes.end(); ++iter_peer)
					{
						if(node_name==iter_peer->first)
							cout<<"Si hay nodo"<<endl;
						else cout<<"No hay nodo"<<endl;
					}	
				}
				if(buffer[0]=='R'){
					bzero(buffer,1);
				 		//Reading node
		   			read(socketToClient, buffer, 2);
					size = atoi(buffer);
					bzero(buffer,2); //cleaning the buffer
					read(socketToClient, buffer, size);
					node_name = string(buffer);
					bzero(buffer,size); //cleaning the buffer
					
					read(socketToClient, buffer, 2);
					size = atoi(buffer);
					bzero(buffer,2); //cleaning the buffer
					read(socketToClient, buffer, size);
					node_name_2 = string(buffer);
					bzero(buffer,size); //cleaning the buffer
					
					map<string,string>::iterator iter_peer; //iterator on list of peers
					for(iter_peer=relaciones.begin(); iter_peer!=relaciones.end(); ++iter_peer)
					{
						
						if(node_name == iter_peer->first && node_name_2 == iter_peer->second ){
							cout<<"Eliminado :("<<endl;
							relaciones.erase(iter_peer->first);
						}
						else cout<<"No hay relacion: "<<endl;
					}	
				}
				map<string,string>::iterator iter_peer; //iterator on list of peers
				//map<int,string>::iterator iter_peer; //iterator on list of peers
				for(iter_peer=nodes.begin(); iter_peer!=nodes.end(); ++iter_peer)
				{
					cout<<"Nodo: "<< iter_peer->first;
					cout<<"  Value: "<< iter_peer->second<<endl;					
				}	
				//for(int i=0;i<10;i++)
				//	cout<<i<<endl;	
		}
	else if(buffer[0]=='R'){
					string message ="";
					int cunt=0;
					bzero(buffer,1);
					//Reading node
		   			read(socketToClient, buffer, 2);
					size = atoi(buffer);
					bzero(buffer,2); //cleaning the buffer
					read(socketToClient, buffer, size);
					node_name = string(buffer);
					bzero(buffer,size); //cleaning the buffer
					//cout<<node_name<<" "<<valor<<endl;
					map<string,string>::iterator iter_peer; //iterator on list of peers
					cout<<"ENTRE"<<endl;

					for(iter_peer=relaciones.begin(); iter_peer!=relaciones.end(); ++iter_peer)
					{		
						cout<<iter_peer->first<<"   "<<iter_peer->second<<endl;
							
						if(node_name==iter_peer->first){
							//cout<<"lista"<<endl;
							cout<<node_name<<"   "<<iter_peer->second<<endl;
							message = message + read_size(iter_peer->second) + iter_peer->second; 
							cunt++;
							cout<<cunt<<endl;
						}
						//else cout<<"No hay nodo"<<endl;
					}

					message = "0"+to_string(cunt) + message; 
					//cout<<"mesnaje"<<message<<endl;
			
					write(socketToClient, message.c_str(), message.length());//We reply the list of chunks	

	    }

}
void peer_client(string server_ip, int server_port, string instruction, string node="")
{
	string nombre;
	string primer_nodo;
	string segundo_nodo;
	string valor_;
	string relacion;
	char buffer[1000];
	int cantidad_usuarios;
	int size;
	struct sockaddr_in stSockAddr;
	int Res;
	int SocketToServer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	int n;
	string message;	
	if (-1 == SocketToServer)
	{
		perror("cannot create socket");
	}
	memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
	//connection{
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(server_port);
	Res = inet_pton(AF_INET, server_ip.c_str(), &stSockAddr.sin_addr);
	//}connection
	if (0 > Res)
	{
		perror("error: first parameter is not a valid address family");
		close(SocketToServer);
		exit(EXIT_FAILURE);
	}
	else if (0 == Res)
	{
		perror("char string (second parameter does not contain valid ipaddress");
		close(SocketToServer);
		exit(EXIT_FAILURE);
	}

	if (-1 == connect(SocketToServer, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
	{
		perror("connect failed");
		close(SocketToServer);
		exit(EXIT_FAILURE);
	}

	if(instruction == "I") //Requesting list of available peers
	{
		message = "I" + read_size(my_ip_address) + my_ip_address + read_size(to_string(my_port)) + to_string(my_port);
		write(SocketToServer, message.c_str(), message.length());
		read_socketToServer(SocketToServer); //we read answer
	}
	else if(instruction == "IR") //Requesting list of available peers
	{
						
				
		cout<<"Primer Nodo"<<endl;
		cin>>primer_nodo;
		cout<<"Segundo Nodo"<<endl;		
		cin>>segundo_nodo;
		//peer_client
    	relaciones.insert (std::pair<string,string>(primer_nodo,segundo_nodo));
		
    	map<string,string>::iterator iter_peer; //iterator on list of peers
				
		for(iter_peer=relaciones.begin(); iter_peer!=relaciones.end(); ++iter_peer)
		{
						cout<<"Nodo 1: "<< iter_peer->first;
						cout<<"Nodo 2: "<< iter_peer->second <<endl;					
		}
		//message = "IR"  + read_size(primer_nodo) + primer_nodo + read_size(segundo_nodo) + segundo_nodo;
		//write(SocketToServer, message.c_str(), message.length());
		//write(SocketToServer, message.c_str(), message.length());
		
		//peer_client(my_ip_address, (*it).second, "R"); //execute instruction
			
		//read_socketToServer(SocketToServer); //we read answer
	}
	else if(instruction == "T") //Requesting list of available peers
	{
		
		cout<<"Primer Nodo"<<endl;
		cin>>primer_nodo;
		cout<<"Segundo Nodo"<<endl;		
		cin>>segundo_nodo;
		//peer_client
		message = "T" + read_size(primer_nodo) + primer_nodo + read_size(segundo_nodo) + segundo_nodo  ;
		write(SocketToServer, message.c_str(), message.length());
		//write(SocketToServer, message.c_str(), message.length());
		
		//peer_client(my_ip_address, (*it).second, "R"); //execute instruction
			
		//read_socketToServer(SocketToServer); //we read answer
	}
	else if(instruction == "QN") //Requesting list of available peers
	{
		cout<<"Nodo: "<<endl;
		cin>>nombre;
		
		message = "QN" + read_size(nombre) + nombre  ;
		write(SocketToServer, message.c_str(), message.length());
		//read_socketToServer(SocketToServer); //we read answer
	}
	else if(instruction == "QR") //Requesting list of available peers
	{
		cout<<"Primer Nodo"<<endl;
		cin>>primer_nodo;
		cout<<"Segundo Nodo"<<endl;		
		cin>>segundo_nodo;

		message = "QR" + read_size(primer_nodo) + primer_nodo + read_size(segundo_nodo) + segundo_nodo  ;
		write(SocketToServer, message.c_str(), message.length());
		//read_socketToServer(SocketToServer); //we read answer
	}
	else if(instruction == "V") //Requesting list of available peers
	{
		cout<<"Nodo: "<<endl;
		cin>>nombre;
		
		message = "V" +  read_size(nombre) + nombre ;
		write(SocketToServer, message.c_str(), message.length());
		//read_socketToServer(SocketToServer); //we read answer
	}
	else if(instruction == "H") //Requesting list of available peers
	{	
		string relacion_;
		//cout<<"Primer_Nodo"<<endl;
		//cin>>relacion_;

		//string relacion_2;
		//cout<<"Segundo_nodo"<<endl;
		//cin>>relacion;
		message = "H";
		write(SocketToServer, message.c_str(), message.length());
		read(SocketToServer,buffer,2); //reading header with number of users
		cantidad_usuarios = atoi(buffer);
		bzero(buffer,2); //cleaning the buffer of the header-number of users
		cout<<"List of available peers:"<<endl;
		string ip_;
		int port_;
		int i=0;			
		for(int i=0; i<cantidad_usuarios; i++)
		{
			read(SocketToServer,buffer,2); //reading header with size of ip address
			size = atoi(buffer);
			bzero(buffer,2); //cleaning the buffer of the header-size of ip address
			read(SocketToServer,buffer,size); //reading ip address
			ip_= string(buffer);
			cout<< "PORT: " << buffer << "   -   "; //print ip address
			bzero(buffer,size); //cleaning the buffer of the nickname
			read(SocketToServer,buffer,2); //reading header with size of port
			size = atoi(buffer);
			bzero(buffer,2); //cleaning the buffer of the header-size of port
			read(SocketToServer,buffer,size); //reading port
			cout<< "NODO: " << buffer <<endl; //print port
			port_= atoi(buffer);
			bzero(buffer,size); //cleaning the buffer of the port
			
			hash_slave[port_] = ip_; //add peer to the map (port,ip)
		}	
		//peer_client(tracker_ip_address, tracker_port, "H"); //execute instruction
			
		//read_socketToServer(SocketToServer); //we read answer
	}

	else if(instruction == "DN") //Requesting list of available peers
	{
		cout<<"Nodo: "<<endl;
		cin>>nombre;
		
		message = "DN" +  read_size(nombre) + nombre ;
		write(SocketToServer, message.c_str(), message.length());
		//read_socketToServer(SocketToServer); //we read answer
	}
	else if(instruction == "DR") //Requesting list of available peers
	{
		map<string,string>::iterator iter_peer; //iterator on list of peers
		for(iter_peer=relaciones.begin(); iter_peer!=relaciones.end(); ++iter_peer){				
				cout<<iter_peer->first<<" - " <<iter_peer->second<<endl;
		}
		cout<<"Primer Nodo"<<endl;
		cin>>primer_nodo;
		cout<<"Segundo Nodo"<<endl;		
		cin>>segundo_nodo;

		message = "DR" + read_size(primer_nodo) + primer_nodo + read_size(segundo_nodo) + segundo_nodo  ;
		write(SocketToServer, message.c_str(), message.length());
		
	}






	else if(instruction == "P") //Requesting list of available peers
	{
		message = "P";
		write(SocketToServer, message.c_str(), message.length());
		read_socketToServer(SocketToServer); //we read answer
	}

	
	else if(instruction == "U") //Upload chunk to the torrent (just for testing)
	{
		string name_file;
		cout<<"Write File Part Name: ";
		cin>>name_file;	
		chunks.push_back(name_file);
		//Print chunks that I have
		cout<<"Chunks that I have:"<<endl;
		for(vector<string>::iterator it = chunks.begin() ; it != chunks.end(); ++it)
		{
			cout<<(*it)<<endl;
		}
	}
	
	shutdown(SocketToServer, SHUT_RDWR);
	close(SocketToServer);
}


//Function that ckeck how many chunks of certain movie We have
bool movie_is_complete(string movie_id)
{
	int total = 0; //total number of parts
	int progress = 0; //number of parts that We have
	for(vector<string>::iterator it = chunks.begin() ; it != chunks.end(); ++it)
	{
		if( (*it).substr(0,2) == movie_id) 
		{
			total = atoi( (*it).substr(4,2).c_str() );
			progress = progress + 1;			
		}
	}
	if( total == 0)
	{
		cout<<"Movie not found among unchocked peers."<<endl;
	}
	else
	{
		cout<<"Downloaded parts: "<< progress << "/" << total << endl; 
	}	
	if( (total!=0) and (progress==total) )
	{
		return true;
	}
	return false;
}


//Menu to search a movie among unchocked peers
void movies()
{
	string movie_id;
	map<int,string>::iterator iter_peer; //iterator on list of peers
	map<int,string>::iterator iter_list; //iterator on list of chunk_lists
	int cantidad_chunks = 0;
	string chunk_id = "";	

	cout<<"/---- MOVIES ------/"<<endl;	
	cout<<"01 : Harry Potter"<<endl;
	cout<<"02 : Star Wars"<<endl;
	cout<<"03 : Avengers"<<endl;
	cout<<"Write movie :";
	cin>>movie_id;

	bool stop = false; //loop control

	while( stop == false )
	{
		for(iter_peer=peers.begin(); iter_peer!=peers.end(); ++iter_peer) //de la estructura de peers que tengo
		{
			if(my_port != iter_peer->first) //pregunto por todos los peers expecto por mi
			{
				peer_client(iter_peer->second, iter_peer->first, "D" , movie_id);
			}			
		}
		//now, that we have the list chunks_from_peers updated, we begin to request chunks
		for(iter_list=chunks_from_peers.begin(); iter_list!=chunks_from_peers.end(); ++iter_list)
		{
			if(my_port != iter_list->first) //pregunto por todos los peers expecto por mi
			{
				cantidad_chunks = atoi( (iter_list->second).substr(0,2).c_str() );
				for(int i=0; i<cantidad_chunks; i++)
				{
					chunk_id = (iter_list->second).substr(i+2,6); //reading every chunk's id
					if(chunk_id != "")
					{
						peer_client(peers[iter_list->first], iter_list->first, "d" , chunk_id);//...we ask for it.
					}
				}
			}			
		}
		chunks_from_peers.clear(); //We clean the list, it will be used for the next attempt to download
		stop = movie_is_complete(movie_id);
		//We wait for neighbor peers change
		this_thread::sleep_for(chrono::milliseconds(3000));
	}
}
//Peer's server, listening for incoming requests
void peer_server()
{
	struct sockaddr_in stSockAddr;
	int SocketToClient = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	char buffer[1000];
	int n;
	if(-1 == SocketToClient)
	{
		perror("can not create socket");
		exit(EXIT_FAILURE);
	}
	memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(my_port);
	stSockAddr.sin_addr.s_addr = INADDR_ANY;
	if(-1 == bind(SocketToClient,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
	{
		perror("error bind failed");
		close(SocketToClient);
		exit(EXIT_FAILURE);
	}
	if(-1 == listen(SocketToClient, 10))
	{
		perror("error listen failed");
		close(SocketToClient);
		exit(EXIT_FAILURE);
	}	
	for(;;)
	{
		int ConnectionSocket = accept(SocketToClient, NULL, NULL);
		if(0 > ConnectionSocket)
		{
			perror("error accept failed");
			close(SocketToClient);
		}
		else
		{
			thread(handle_client_request,ConnectionSocket).detach();
		}
	}		
}


int main()
{
	string instruction;
	string sub_instruction;
	
	cout<<"- - - - - WELCOME - SAVE- - "<<endl;		
	cout<<"Enter your port: ";
	cin>>my_port;
	//We "login" in tracker's server
	peer_client(tracker_ip_address, tracker_port, "I");

	//We ask for the list of peers
	peer_client(tracker_ip_address, tracker_port, "P");

	//After being registered, we launch our peer's server
	thread(peer_server).detach();
	
	//Command line
	for(;;)
	{
		cout<<endl<<endl<<"---------- Select an instruction ------------"<<endl;
		cout<<"IR: Insert Relation"<<endl;
		cout<<"DR: Delete Relation"<<endl;
		cout<<"QR: Consulta Relacion "<<endl;
		cout<<"QN: Consulta Nodo "<<endl;
		cout<<"V: Valor Nodo "<<endl;
		
		//cout<<string(D)<<endl;
		cout<<"Instruction: ";
		cin>>instruction;
		if(instruction=="DR")
		{
			peer_client(my_ip_address, my_port, instruction); //execute instruction
			
		}
		if(instruction=="QN")
		{
			peer_client(my_ip_address, my_port, instruction); //execute instruction
			
		}

		if(instruction=="QR")
		{
			peer_client(my_ip_address, my_port, instruction); //execute instruction
			
		}

		if(instruction=="V")
		{
			peer_client(my_ip_address, my_port, instruction); //execute instruction
			
		}

		if(instruction=="IR")
		{
			cout<<"OBTENIEDNO HASH"<<endl;
			//peer_client(tracker_ip_address, tracker_port, "H"); //execute instruction
			
			peer_client(my_ip_address, my_port, instruction); //execute instruction
			peer_client(tracker_ip_address, tracker_port, "T"); //execute instruction
			
			//peer_client(my_ip_address, my_port, instruction); //execute instruction
			
			cout<<"OBTENIEDNO otro"<<endl;
			
			//peer_client(my_ip_address, my_port, instruction); //execute instruction
			
		}		
		//if(instructions==)	
		/*cout<<"Instruction: ";

		if(instruction=="P")
		{
			peer_client(tracker_ip_address, tracker_port, instruction); //execute instruction
		}
		else if(instruction=="D")
		{
			movies(); 
		}
		else if(instruction=="U")
		{
			peer_client(tracker_ip_address, tracker_port, instruction); //execute instruction
		}*/
		//peer_client(my_ip_address, my_port, instruction); //execute instruction
		//peer_client(instruction); //execute instruction
		//peer_server_(instruction)
			
	}	

	return 0;
}
