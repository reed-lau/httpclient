#include <stdio.h>
#include <string>
#include <vector>

#include <stdint.h>
#include <memory.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

class HttpClient {
    public:
        HttpClient(const char *host, uint16_t port = 8080) {
            host_ = host;

            socket_ = ::socket(AF_INET, SOCK_STREAM, 0);
            if (socket_ < 0) {
                status_ = -1;
            }

            std::vector<std::string> ips = GetAddrInfo(host);

            for (size_t i = 0; i < ips.size(); ++i) {
                struct sockaddr_in *addr = (struct sockaddr_in*)(ips[i].data());
                addr->sin_port = htons(port);
                addr->sin_family = AF_INET; 
                int ret = ::connect(socket_, (struct sockaddr*)(addr), ips[i].size());
                if (ret != 0) {
                    status_ = ret;
                    continue;
                } else {
                    status_ = 0;
                    break;
                }
            }
        }

        void Get(const char *uri) {
            if (socket_ <0 || status_ !=0) {
                return ;
            }

            std::string req = "GET " + std::string(uri) + " HTTP/1.1\r\n"
                + "Host:" +  host_ + " \r\n"
                + "Connection: keep-alive\r\n"
                + "Pragma: no-cache\r\n"
                + "Cache-Control: no-cache\r\n"
                + "User-Agent: HttpClient/0.1\r\n"
                + "\r\n\r\n";

            int ret = write(socket_, req.data(), req.size());

            std::string header;
            std::string data;

            // read header
            while (true) {
                char buf[1024] = {0};
                ret = ::read(socket_, buf, 1024);

                if (ret <= 0) {
                    printf("error occur ----\n");
                    continue;
                }

                header.append(buf, ret);
                std::size_t pos = header.find("\r\n\r\n"); 
                if (pos != std::string::npos) {
                    data.append(&header[pos + 4], header.size() - pos + 4);
                    header.erase(pos + 2);
                    break;
                }
            }

            printf("Response:\n");
            printf("%s\n", header.data());

            // get Status Line

            // get Content-Length
            std::size_t pos = header.find("Content-Length:");
            std::size_t len = 0;
            sscanf(&header[pos + 15], "%zu", &len);

            printf("Content-Length: = %zu\n", len);

            // read body
            while (data.size() < len) {
                char buf[1024] = {0};
                size_t left = len - data.size();
                if (left > 1024)
                    left = 1024;
                int ret = ::read(socket_, buf, left);
                data.append(buf, ret);
            }

            printf("body: %zu\n", data.size());

            /*
            for (int i=0; i<data.size(); ++i) {
                if (isprint(data[i]))
                  printf("(%d.%c)", data[i], data[i]);
                else
                  printf("(%d.*)", data[i]);
            }
            printf("\n");
            for (int i=0; i <header.size(); ++i) {
                if (isprint(header[i]))
                  printf("<%d.%c>", header[i], header[i]);
                else
                  printf("<%d.*>", header[i]);
            }*/
        }

        std::vector<std::string> GetAddrInfo(const char *host) {
            struct addrinfo hints;
            struct addrinfo *resolved, *current;
            memset(&hints, 0, sizeof(hints));
            hints.ai_flags = 0;
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = 0;
            int ret = getaddrinfo(host, NULL, &hints, &resolved);

            std::vector<std::string> addrs;
            if (ret != 0) {
                return addrs;
            }

            for (current = resolved; current != NULL; current = current->ai_next) {
                std::string addr;
                addr.assign((char*)(current->ai_addr), current->ai_addrlen);
                addrs.push_back(addr);
            }
            return addrs;
        }
    private:
        std::string host_;
        int socket_;
        int status_;
};

std::vector<std::string> GetAddrInfo(const char *host) {
    struct addrinfo hints;
    struct addrinfo *resolved, *current;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = 0;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    int ret = getaddrinfo(host, NULL, &hints, &resolved);

    std::vector<std::string> addrs;
    if (ret != 0) {
        return addrs;
    }

    for (current = resolved; current != NULL; current = current->ai_next) {
        std::string addr;
        addr.assign((char*)(current->ai_addr), current->ai_addrlen);
        addrs.push_back(addr);
    }
    return addrs;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage http_client www.baidu.com");
        return 0;
    }

    HttpClient client(argv[1]);

    client.Get("/glibc-2.19.tar.gz");

}
