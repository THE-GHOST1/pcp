#include <uv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct sockaddr_in req_addr;



void on_close(uv_handle_t * handle)
{
	printf("\nclosed!! \n");
}

void on_read(uv_stream_t* tcp, ssize_t nread, uv_buf_t buf)
{
	if(nread >= 0) 
	{
		printf("\nread: %s\n", buf.base);
	}
	else 
	{
	    uv_close((uv_handle_t*)tcp, on_close);
	}
	free(buf.base);
}
			  
void alloc_buffer(uv_handle_t *handle, size_t suggested_size,uv_buf_t *buf) 
{       	
	buf->base = (char*) malloc(suggested_size);
	buf->len = suggested_size;
}

void on_write_end(uv_write_t *req, int status)
{
	if(status < 0)
	{
		fprintf(stderr,"uv_write Error : %s\n",uv_strerror(status));
		return;
	}
	printf("\nwrote !!\n");
	//uv_read_start(req->handle,alloc_buffer,on_read);

}
void on_connect(uv_connect_t *req, int status)
{
	if(status < 0)
	{
		fprintf(stderr,"Connection error %s\n", uv_strerror(status));
		return;
	}
	char *message = "*2\r\n$3\r\nGET\r\n$3\r\nkey\r\n";
	int len = strlen(message);
	char buffer[100];
	uv_buf_t buf = uv_buf_init(buffer,sizeof(buffer));
	buf.len = len;
	buf.base = message;

	printf("buf.base : %s",buf.base);
	uv_stream_t* tcp = req->handle;
	uv_write_t write_req;
	int buf_count = 1;
	uv_write(&write_req, tcp, &buf,buf_count, on_write_end);
	uv_read_start(tcp, alloc_buffer, on_read);
}

int main()
{
	
	 uv_tcp_t client;
	    
	 uv_tcp_init(uv_default_loop(), &client);
	
	 uv_tcp_keepalive(&client,1,50);

	 uv_ip4_addr("0.0.0.0", 44323,&req_addr);
	    
	 uv_connect_t connect_req;
			
	 uv_tcp_connect(&connect_req, &client,(const struct sockaddr*)&req_addr, on_connect);
			    
	 return uv_run(uv_default_loop(),UV_RUN_DEFAULT);
}
