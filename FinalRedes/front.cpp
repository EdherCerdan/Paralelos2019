#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <map>
#include <vector>
#include <math.h>

using namespace std;


///FRONT'S IP ADDRESSS
string front_ip_address = "0.0.0.0";

///FRONT'S PORT
int front_port = 121213;

map<string, int> hashi; 
map<int, string> slaves;
map<int, string> nodes;
vector<string> rela; 



//Check the size of a string (usuario, mensaje)
string read_size(string text)
{
	char buffer[2];
	sprintf(buffer,"%02d",(unsigned int)text.size());
	return buffer;
}


//check if there is not another slave with the same port number
bool port_ok(int port)
{
	bool answer = true;
	map<int,string>::iterator iter_slave; //iterator on list of slaves
	for(iter_slave=slaves.begin(); iter_slave!=slaves.end(); ++iter_slave)
	{
		if(port==iter_slave->first)
		{
			answer = false;
		}
	}
	return answer;
}


//front's client will ping a slave's server or update every slave's list (ping/update)
void front_client(string server_ip, int server_port, string instruction, string node="", string valor="",int nivel=0,string segundo="") //instruction:ping/update
{
	struct sockaddr_in stSockAddr;
	int Res;
	int socketToServer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	int n;
	char buffer[3];
	string message;
	string primer_nodo;
	string segundo_nodo;
	if (-1 == socketToServer)
	{
		perror("cannot create socket");
		//exit(EXIT_FAILURE);
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
		close(socketToServer);
		return;
	}
	else if (0 == Res)
	{
		perror("char string (second parameter does not contain valid ipaddress");
		close(socketToServer);
		return;
	}

	if (-1 == connect(socketToServer, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
	{
		perror("connect failed");
		close(socketToServer);
		//if we can not reach slave's server, it churned
		cout<<"slave (Ip: " << server_ip <<" , Port: " << server_port << ") does not answer."<< endl; 
		slaves.erase(server_port);
		map<int,string>::iterator iter_slave; //iterator on list of slaves
		for(iter_slave=slaves.begin(); iter_slave!=slaves.end(); ++iter_slave)
		{
			front_client(iter_slave->second, iter_slave->first, "update"); //we update every list of slaves
		}
		return;
	}
	//If we have to ping a slave's server
	if(instruction == "ping")
	{
		write(socketToServer, "?", 1); //Ping 
		n = read(socketToServer, buffer ,1); //read just the first byte to check the slave's answer 
		//if there's no answer, the slave churned
		if(n==0)
		{
			cout<<"slave (Ip: " << server_ip <<" , Port: " << server_port << ") does not answer."<< endl; 
			slaves.erase(server_port);
			map<int,string>::iterator iter_slave; //iterator on list of slaves
			for(iter_slave=slaves.begin(); iter_slave!=slaves.end(); ++iter_slave)
			{
				front_client(iter_slave->second, iter_slave->first, "update"); //we update every list of slaves
			}
		}
	}

	if(instruction == "IN")
	{
		
		message = "IN" + read_size(node) + node + read_size(valor) + valor  ;
		write(socketToServer, message.c_str(), message.length());
	}
	if(instruction == "t")
	{
		
		message = "t" + read_size(node) + node + read_size(segundo) + segundo ;
		write(socketToServer, message.c_str(), message.length());
	}
	if(instruction == "QR")
	{
		int num;
		int size;
		int tam_mesa;
		string node_name;
		message = "R" + read_size(node) + node ;
		write(socketToServer, message.c_str(), message.length());
		read(socketToServer, buffer,2);
		tam_mesa=atoi(buffer);
		cout<<"buffer"<<tam_mesa<<endl;
		bzero(buffer,2); //cleaning the buffer
		cout<<"Relaciones : "<<endl;
			
		for(int i=0;i<tam_mesa;i++){
			//cout<<"Relaciones : "<<endl;
			read(socketToServer, buffer, 2);
			//cout<<"tam"<<buffer<<endl;
			size = atoi(buffer);
			bzero(buffer,2); //cleaning the buffer
			read(socketToServer, buffer, size);
			node_name = string(buffer);
			cout<<"Relacion: "<<node_name<<endl;
			rela.push_back(node_name);
			bzero(buffer,size); //cleaning the buffer
			
		}
			
	}
	if(instruction == "U")
	{
		int num;
		int size;
		int tam_mesa;
		string node_name;
		message = "U" + read_size(node) + node + read_size(valor) + valor ;
		write(socketToServer, message.c_str(), message.length());
		read(socketToServer, buffer,2);
		tam_mesa=atoi(buffer);
		cout<<"buffer"<<tam_mesa<<endl;
		bzero(buffer,2); 
		cout<<"Relaciones : "<<endl;
			
		for(int i=0;i<tam_mesa;i++){
			//cout<<"Relaciones : "<<endl;
			read(socketToServer, buffer, 2);
			//cout<<"tam"<<buffer<<endl;
			size = atoi(buffer);
			bzero(buffer,2); //cleaning the buffer
			read(socketToServer, buffer, size);
			node_name = string(buffer);
			cout<<"Relacion: "<<node_name<<"-";
			rela.push_back(node_name);
			bzero(buffer,size); //cleaning the buffer
			
		}
			
	}
	else if(instruction == "update")
	{
		write(socketToServer, "p", 1);
		sprintf(buffer,"%02d",(unsigned int)slaves.size());
		write(socketToServer, buffer, 2);
		map<int,string>::iterator iter_slave; //iterator on list of slaves
		for(iter_slave=slaves.begin(); iter_slave!=slaves.end(); ++iter_slave)
		{
			//ip
			sprintf(buffer,"%02d",(unsigned int)(iter_slave->second.length()) );
			write(socketToServer, buffer, 2);
			write(socketToServer, iter_slave->second.c_str(), iter_slave->second.length());
			//port
			sprintf(buffer,"%02d",(unsigned int)(to_string(iter_slave->first).length()) );
			write(socketToServer, buffer, 2);
			write(socketToServer, to_string(iter_slave->first).c_str(), to_string(iter_slave->first).length());
		}
	}
	shutdown(socketToServer, SHUT_RDWR);
	close(socketToServer);
}


void handle_client_request(int socketToClient)
{
	char buffer[1000];
   	string message;
	string ip;
	string node_;
   	int size;
 	int port;
 	int n;
 	for(;;)
 	{
 		buffer[0] = 'X';
 		n = read(socketToClient, buffer, 1);
 		if(n==0)
 		{
 			continue;
 		}
 		if(buffer[0] == 'I') //Login
 		{
			//Reading Ip
   			read(socketToClient, buffer, 2);
			size = atoi(buffer);
			bzero(buffer,2); //cleaning the buffer
			read(socketToClient, buffer, size);
			ip = string(buffer);
			bzero(buffer,size); //cleaning the buffer
			//Reading port
   			read(socketToClient, buffer, 2);
			size = atoi(buffer);
			bzero(buffer,2); //cleaning the buffer
			read(socketToClient, buffer, size);
			port = atoi(buffer);
			bzero(buffer,size); //cleaning the buffer
			//Check if there is not another slave with the same port
			if(port_ok(port))
			{
				slaves[port] = ip; //add slave to the map (port,ip)
				write(socketToClient,"A10",3); //acknowledgment ok
				map<int,string>::iterator iter_slave; //iterator on list of slaves
				for(iter_slave=slaves.begin(); iter_slave!=slaves.end(); ++iter_slave)
				{
					if(port != iter_slave->first) //For every new slave, except the new one...
					{
					front_client(iter_slave->second, iter_slave->first, "update"); //...we update the list of slaves
					}
				}
				cout<<"New slave (Ip: " << ip <<" , Port: " << port << ") has joined."<< endl; 
			}
			else
			{
				write(socketToClient,"E10",3); 
			}			
		}
		else if(buffer[0] == 'P') 
		{
			write(socketToClient, "p", 1);
			sprintf(buffer,"%02d",(unsigned int)slaves.size());
			write(socketToClient, buffer, 2);
			map<int,string>::iterator iter_slave; 
			for(iter_slave=slaves.begin(); iter_slave!=slaves.end(); ++iter_slave)
			{
				//ip
				sprintf(buffer,"%02d",(unsigned int)(iter_slave->second.length()) );
				write(socketToClient, buffer, 2);
				write(socketToClient, iter_slave->second.c_str(), iter_slave->second.length());
				//port
				sprintf(buffer,"%02d",(unsigned int)(to_string(iter_slave->first).length()) );
				write(socketToClient, buffer, 2);
				write(socketToClient, to_string(iter_slave->first).c_str(), to_string(iter_slave->first).length());
			}
		}
		else if(buffer[0] == 'T')
		{
			string valor;
			int nivel;
			string primer;
			string segundo;
			int direccion;
			read(socketToClient, buffer, 2);
			size = atoi(buffer);
			bzero(buffer,2); //cleaning the buffer
			read(socketToClient, buffer, size);
			primer = string(buffer);
			bzero(buffer,size);
			////////////////
			read(socketToClient, buffer, 2);
			size = atoi(buffer);
			bzero(buffer,2); //cleaning the buffer
			read(socketToClient, buffer, size);
			segundo = string(buffer);
			bzero(buffer,size);
			map<string,int>::iterator it;
			it = hashi.find(primer);
			direccion=(*it).second;
			front_client(front_ip_address, direccion, "t", primer, valor="",nivel=0, segundo); 
		}	
	}
}



void relation_node()
{
	string valor;
	int nivel;
	string segundo;
	int direccion;
	string node;
	cout<<"Node"<<endl;
	cin>>node;
	map<string,int>::iterator it;
	it = hashi.find(node);
	direccion=(*it).second;
	front_client(front_ip_address, direccion, "QR", node, valor="",nivel=0, segundo=""); //ping every slave
	
}
void update_value(){
	string valor;
	int nivel;
	string segundo;
	int direccion;
	string node;
	cout<<"Node: ";
	cin>>node;
	cout<<endl;
	cout<<"Valor a cambiar: ";
	cin>>valor;
	cout<<endl;
	map<string,int>::iterator it;
	it = hashi.find(node);
	direccion=(*it).second;
	front_client(front_ip_address, direccion, "U", node, valor,nivel=0, segundo=""); //ping every slave

}

void insert_node()
{
	int count=1;
	int slave_;
	char a;
	string node;
	cout<<"Node"<<endl;
	cin>>a;
	string valor;
	cout<<"Valor"<<endl;
	cin>>valor;
	map<int,string>::iterator iter_slave;
	//cout<<int(a)<<endl;
	slave_=int(a)%4;
	if(int(a)%4==0){
		slave_=4;
	}
	//cout<<slave_;
	node=a;
	//cout<<"string"<<node;
	for(iter_slave=slaves.begin(); iter_slave!=slaves.end(); ++iter_slave)
	{	
		if(count==slave_){
			cout<<iter_slave->second<<" "<< iter_slave->first<<endl;
			front_client(iter_slave->second, iter_slave->first, "IN",node,valor); //ping every slave
			hashi[node]=iter_slave->first;///port and node
			//cout<<"tamaño hashii"<< (unsigned int)hashi.size()<<endl;

		}
		count++;
		//cout<<"cont "<<count<<endl;
	}
}

/*
void relation_node_level()
{
	int nivel;
	string node;
	cout<<"Escoja Node"<<endl;
	cin>>node;
	cout<<"Escoja Nivel"<<endl;
	cin>>nivel;
	
	//node="T";
	map<int,string>::iterator iter_slave; //iterator on list of slaves
	//sleep
	for(iter_slave=slaves.begin(); iter_slave!=slaves.end(); ++iter_slave)
	{
		front_client(iter_slave->second, iter_slave->first, "QR_nivel",nivel); //ping every slave
	}
}*/
//front's server, listening for incoming requests from clients
void front_server()
{
	string instruction;
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
	stSockAddr.sin_port = htons(front_port);
	stSockAddr.sin_addr.s_addr = INADDR_ANY;
	if(-1 == bind(SocketToClient,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
	{
		perror("error bind failed");
		close(SocketToClient);
	}
	if(-1 == listen(SocketToClient, 10))
	{
		perror("error listen failed");
		close(SocketToClient);
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
		//cin>>instruction;
		//if(instruction=="QR")
		//	relation_node();
		
	}		
}



//Ping every registered slave to check if they are still connected
void ping_slaves()
{

	map<int,string>::iterator iter_slave; //iterator on list of slaves
	//sleep
	for(iter_slave=slaves.begin(); iter_slave!=slaves.end(); ++iter_slave)
	{
		front_client(iter_slave->second, iter_slave->first, "ping"); //ping every slave
	}
}




int main(void)
{
	string instructions;
	//cout<<"INSTR :"<<endl;
	//We launch front's server
	thread(front_server).detach();
	cout<<"front server has been initialized."<<endl;
	for(;;)
	{
		cout<<endl<<endl<<"---------- Select an instruction ------------"<<endl;
		cout<<"IN: Insertar Nodo"<<endl;
		//cout<<"QV: Consulta Nodo-Valor"<<endl;
		//cout<<"QN: Consulta Nodo"<<endl;
		cout<<"QR: Consulta Relacion-Nodo"<<endl;
		cout<<"U: Actualizar Valor-Nodo"<<endl;
		
		cout<<"Instruction: ";
		cin>>instructions;
		
		if(instructions=="QR")	
					relation_node();

		//else if(instructions=="QRL")
					//relation_node_level();
		else if(instructions=="IN")
					insert_node();
		else if(instructions=="U")
					update_value();

					
	}	
	
	return 0;
}
