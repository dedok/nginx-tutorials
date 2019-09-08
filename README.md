# nginx-tutorials

# The tutorials of creating modules of nginx.

About nginx: https://nginx.org

## Content
----------
* [Compilation and install](#compilation-and-install)
* [Tutorials](#tutorials)
* [See also](#see-also)

## Compilation and install
--------------------------
```bash
git clone https://github.com/dedok/nginx-tutorials
cd nginx-tutorials
git clone https://github.com/nginx/nginx.git nginx
sudo apt-get install libpcre-dev zlib1-dev # install dependencies to build nginx
make configure
make
./nginx/objs/nginx
wget --server-response --header "X-My-header:1" "127.0.0.1:8081/example"
```
[Back to content](#content)

## Tutorials
------------
### Header filter, a file: body_filter.c

  A header filter consists of three basic steps:
  - Decide whether to operate on this response
  - Operate on the response
  - Call the next filter.

### Body filter, a file: header_filter.c or it's filtering the nginx chain buffer. 

The buffer chain makes it a little tricky to write a body filter,
because the body filter can only operate on one buffer (chain link) at a time.
The module must decide whether to overwrite the input buffer, replace the buffer with a newly allocated buffer,
or insert a new buffer before or after the buffer in question. To complicate things,
sometimes a module will receive several buffers so that it has an incomplete buffer chain that it must operate on.

### Handlers

  Content handler (Non-proxying), a file: content_handler.c
  Handlers typically do four things:
   - get the location configuration;
   - generate an appropriate response;
   - send the header, and send the body.
  A handler has one argument, the request struct.
  A request struct has a lot of useful information about the client request, such as the request method, URI, and headers.

### NJS

#### NJS module example
  - Get NGINX NJS
  - Run NGINX with conf/njs_nginx.conf configuration
#### NJS and add_header example
  - Get NJS sources
  - Patch original add_header sources and complie patched NGINX
  - Run compiled NGINX with conf/njs_add_header.conf configuration

Feel free to ask, if you have any issues.

## See also
------------
The links to the examples implementation of nginx upstream:
https://github.com/tarantool/nginx_upstream_module
https://github.com/openresty/redis2-nginx-module

An old but the good an article about nginx anatomy:
http://www.evanmiller.org/nginx-modules-guide.html#filters-body

NginX wiki:
https://www.nginx.com/resources/wiki/
