#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include<string.h>  
#include<errno.h>  
#include<sys/types.h>  
#include<sys/socket.h>  
#include<netinet/in.h> 
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#define bu_printf
void set_reuseport(int sockfd, int optval)
{
#ifdef SO_REUSEPORT
    int on = (optval != 0) ? 1 : 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) < 0)
    {
        perror("setsockopt SO_REUSEADDR");
	exit(0);
    }
#else
    fprintf(stderr, "SO_REUSEPORT is not supported.\n");
#endif //SO_REUSEPORT
}

void set_reuseaddr(int sockfd, int optval)
{
    int on = (optval != 0) ? 1 : 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        perror("setsockopt SO_REUSEADDR");
	exit(0);
    }
}

void set_tcpnodelay(int sockfd, int optval)
{
    int on = (optval != 0) ? 1 : 0;
    if(setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)) == -1)
    {
        perror("setsockopt SO_REUSEADDR");
	exit(0);
    }
}

void set_keepalive(int sockfd, int optval)
{
    int on = (optval != 0) ? 1 : 0;
    if(setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on)) == -1)
    {
        perror("setsockopt SO_REUSEADDR");
	exit(0);
    }
}


int set_nonblocking(int fd)
{
    int flags;
    if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void comu_printf(char * data_start, int length)
{
	bu_printf("\r\ndata_start:%x,length:%d\r\n", data_start,length);
	int i;
	for (i= 0; i < length; i++ )
	{
		bu_printf("%02x ", (unsigned char)*(data_start + i));
		if ((i%16) == 15) {
			bu_printf("\r\n");
		}
	}
	bu_printf("\r\n");
}

int readEnd(unsigned char*temp,int offset, int length)
{
		for(int i=offset+1;i<length;i++) { //
			if(temp[i]==0x7e) {//
				return i;
			}
		}
		return -1;
	}

int readHead(unsigned char * temp, int length)
{
		bu_printf("\n0:%x,1:%x\n", temp[0], temp[1]);
		if(temp[0]==0x7e&&temp[1]!=0x7e) { //
			return 0;
		}else {
			for(int i=0;i<length;i++) {
				if(temp[i]==0x7e) {
					if(length>i+1) { //
						if(temp[i+1]!=0x7e) { 
							return i;
						}else {
							return i+1;  //
						}
					}else {//
						return i;
					}
				}
			}
		}
	return -1;
}

void copy_array_front(char * front, char * back, int length)
{
	int i;
	for (i = 0; i < length; i++) {
		*(front + i) = *(back + i);
	}
	return;
}

void copy_array_back(char * back, char * front, int length, int total_lenth)
{
	int i;
#if 1
	for (i = 0; i < length; i++) {
		*(back + length - i -1) = *(back + length - i -1);
	}
#endif	
//	memcpy(back, front, length);
	return;
	
}

int decode_info(char * message, int & msg_length, char * remaining, int & remain_byte_length)
{
	int old_remain_byte_length = remain_byte_length;
	int mark;
	if (old_remain_byte_length >= 11) {
		int startIndex=readHead((unsigned char *)remaining, old_remain_byte_length);//
		if(startIndex==-1) {//
			//temp=null;
			return false;
		}else if(startIndex>=0) {//
			mark=startIndex;  //
			bu_printf("mark:%d\r\n",mark);
		}
		int endPosition=readEnd((unsigned char *)remaining, startIndex+1, old_remain_byte_length);
		if(endPosition==-1) { //
			return false;
		}else {
			bu_printf("endPosition:%d\r\n",endPosition);
			int new_remain_byte_length = old_remain_byte_length - endPosition - 1;
			if(endPosition  + 1 - mark>=11) {//
				memcpy(message, remaining, endPosition + 1 - mark);
				msg_length = endPosition  + 1 - mark;
			}
			if (new_remain_byte_length >= 0) {
				copy_array_front(remaining, remaining + endPosition + 1, new_remain_byte_length);//
				remain_byte_length = new_remain_byte_length;
			}

			//if(old_remain_byte_length>=endPosition+11) {//
				return true;
			//}
		}
	}
	return false;
}

int get_end_char(char* buf, int start, int length) 
{
	int i;
	for (i = start; i < length; i ++) {
		if (buf[i] == 0x7e) {
			return 1;
		}
	}
	return 0;
}

void encode_escape_word(char *msg_out, int& out_length, char *msg_in, int in_length) 
{
	int i = 0;
	int j = 0;
	msg_out[j] = msg_in[i];		
	i += 1;
	j += 1;
	while (i < in_length - 1) {
		if (msg_in[i] == 0x7e) {
			msg_out[j] = 0x7d;
			msg_out[j+1] = 0x02;
			j += 2;
			i += 1;
		} else if (msg_in[i] == 0x7d) {
			msg_out[j] = 0x7d;
			msg_out[j+1] = 0x01;
			j += 2;
			i += 1;
		}  else {
			msg_out[j] = msg_in[i];
			i += 1;
			j += 1;
		}
		
	}
	msg_out[j] = msg_in[i];
	i += 1;
	j += 1;	
	out_length= j;
	return;
}

void delete_escape_word(char *msg_out, int& out_length, char *msg_in, int in_length)
{
	int i = 0;
	int j = 0;
	int out_length_flag  = 0 ;
	while(i < in_length) {
		if (*(msg_in + i) == 0x7d) {
			if (*(msg_in + i + 1) == 0x01) {
				*(msg_out + j ) = 0x7d;
				j += 1;
				i += 2;
			} else if (*(msg_in + i + 1) == 0x02) {
				*(msg_out + j ) = 0x7e;
				j += 1;
				i += 2;	
			} else {
				//bu_printf("i=%d\r\n", i);
				out_length_flag = 1;
				break;
			}
		} else {
			*(msg_out + j) = *(msg_in + i);
			i += 1;
			j += 1;
		}
	}
	if (out_length_flag != 0) {
		out_length = 0;
	} else {
		out_length = j;
	}	
	return;
	
}

int check_data_valid(char * src, int length)
{
	int j;
	unsigned char check_value = src[1];
	for (j = 2; j < length - 2 ; j++)
		check_value ^= src[j];
	bu_printf("check_value:%x, orgin:%x\n",check_value, (unsigned char)src[length-2]);
	if (check_value == (unsigned char)src[length - 2]) {
		return 1;	
	}else{
		return 0;
	}
}

