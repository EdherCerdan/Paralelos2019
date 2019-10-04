#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <string>
#include <sstream>
#include <fstream>
#include <iterator>
#include <vector>

using namespace std;


string FirstNumberToString(int n)
{
     ostringstream ss;
     ss << n;
     return ss.str();
}

string number_to_string_with_padding(int n, int n_bytes){
    string ret = FirstNumberToString(n);
    ret = string(n_bytes-ret.length(), '0').append(ret);
    return ret;
}

string encode_simple_message(string message){
    char tmp = message[0];
    message.erase(0,1);
    int size_message = message.length();
    string size_str = FirstNumberToString(size_message);
    size_str = string(4-size_str.length(), '0').append(size_str);
    size_str += tmp;
    size_str += message;
    return size_str;
}


namespace file_utils
{
    static string read_all_bytes(char const* filename)
    {
        ifstream ifs(filename, ios::binary|ios::ate);
        ifstream::pos_type pos = ifs.tellg();
        std::vector<char>  vec_buffer(pos);
        ifs.seekg(0, ios::beg);
        ifs.read(&vec_buffer[0], pos);
        std::string ret = string(vec_buffer.begin(), vec_buffer.end());
        return ret;
    }

    void get_size_string_of_file(string &filename, string &binary_file, int &size_file){
        binary_file = read_all_bytes(filename.c_str());
        size_file = binary_file.length();
    }

    string prepare_file_message(string binary_file, int size_file, string filename){
        string ret = "F";
	ret += filename;
	ret += number_to_string_with_padding(size_file, 4);
	ret += binary_file;
        return ret;
    }
 
}

#endif // PROTOCOL_H
