/*
 *   httpget.c
 *
 *   Oliver Fromme  <oliver.fromme@heim3.tu-clausthal.de>
 *   Wed Apr  9 20:57:47 MET DST 1997
 */

#undef ALSA

#if !defined(WIN32) && !defined(GENERIC)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <ctype.h>

extern int errno;

#include "mpg123.h"

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

void writestring (int fd, char *string)
{
    int result, bytes = strlen(string);

    while (bytes) {
        if ((result = write(fd, string, bytes)) < 0 && errno != EINTR) {
            perror ("write");
            exit (1);
        }
        else if (result == 0) {
            fprintf (stderr, "write: %s\n",
                "socket closed unexpectedly");
            exit (1);
        }
        string += result;
        bytes -= result;
    }
}

void readstring (char *string, int maxlen, FILE *f)
{
#if 0
    char *result;
#endif
    int pos = 0;

    while(1) {
        if( read(fileno(f),string+pos,1) == 1) {
            pos++;
            if(string[pos-1] == '\n') {
                string[pos] = 0;
                break;
            }
        }
        else if(errno != EINTR) {
            fprintf (stderr, "Error reading from socket or unexpected EOF.\n");
            exit(1);
        }
    }
#if 0
    do {
        result = fgets(string, maxlen, f);
    } while (!result  && errno == EINTR);
    if (!result) {
        fprintf (stderr, "Error reading from socket or unexpected EOF.\n");
        exit (1);
    }
#endif

}

void encode64 (char *source,char *destination)
{
  static char *Base64Digits =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  int n = 0;
  int ssiz=strlen(source);
  int i;

  for (i = 0 ; i < ssiz ; i += 3) {
    unsigned int buf;
    buf = ((unsigned char *)source)[i] << 16;
    if (i+1 < ssiz)
      buf |= ((unsigned char *)source)[i+1] << 8;
    if (i+2 < ssiz)
      buf |= ((unsigned char *)source)[i+2];

    destination[n++] = Base64Digits[(buf >> 18) % 64];
    destination[n++] = Base64Digits[(buf >> 12) % 64];
    if (i+1 < ssiz)
      destination[n++] = Base64Digits[(buf >> 6) % 64];
    else
      destination[n++] = '=';
    if (i+2 < ssiz)
      destination[n++] = Base64Digits[buf % 64];
    else
      destination[n++] = '=';
  }
  destination[n++] = 0;
}

/* VERY  simple auth-from-URL grabber */
int getauthfromURL(char *url,char *auth)
{
  char *pos;

  *auth = 0;

  if (!(strncmp(url, "http://", 7)))
    url += 7;

  if( (pos = strchr(url,'@')) ) {
    int i;
    for(i=0;i<pos-url;i++) {
      if( url[i] == '/' )
         return 0;
    }
    strncpy(auth,url,pos-url);
    auth[pos-url] = 0;
    strcpy(url,pos+1);
    return 1;
  }
  return 0;
}

static char *defaultportstr = "80";

char *url2hostport (char *url, char **hname, unsigned long *hip, unsigned char **port)
{
    char *h, *p;
    char *hostptr;
    char *r_hostptr;
    char *pathptr;
    char *portptr;
    char *p0;
    size_t stringlength;

    p = url;
    if (strncmp(p, "http://", 7) == 0)
        p += 7;
    hostptr = p;
    while (*p && *p != '/')
        p++;
    pathptr = p;

    r_hostptr = --p;
    while (*p && hostptr < p && *p != ':' && *p != ']')
        p--;

    if (!*p || p < hostptr || *p != ':') {
        portptr = NULL;
    }
    else{
        portptr = p + 1;
        r_hostptr = p - 1;
    }
    if (*hostptr == '[' && *r_hostptr == ']') {
        hostptr++;
        r_hostptr--;
    }

    stringlength = r_hostptr - hostptr + 1;
    h = malloc(stringlength + 1); /* removed the strndup for better portability */
    if (h == NULL) {
        *hname = NULL;
        *port = NULL;
        return NULL;
    }
    strncpy(h, hostptr, stringlength);
    *(h+stringlength) = '\0';
    *hname = h;

    if (portptr) {
        stringlength = (pathptr - portptr);
        if(!stringlength) portptr = NULL;
    }
    if (portptr == NULL) {
        portptr = defaultportstr;
        stringlength = strlen(defaultportstr);
    }
    p0 = malloc(stringlength + 1);
    if (p0 == NULL) {
        free(h);
        *hname = NULL;
        *port = NULL;
        return NULL;
    }
    strncpy(p0, portptr, stringlength);
    *(p0 + stringlength) = '\0';

    for (p = p0; *p && isdigit((unsigned char) *p); p++) ;

    *p = '\0';
    *port = (unsigned char *) p0;

    return pathptr;
}

char *proxyurl = NULL;
unsigned long proxyip = 0;
unsigned char *proxyport;

#define ACCEPT_HEAD "Accept: audio/mpeg, audio/x-mpegurl, */*\r\n"

char *httpauth = NULL;
char httpauth1[256];

int http_open (char *url)
{
    char *purl, *host, *request, *sptr;
    int linelength;
    unsigned long myip;
    unsigned char *myport;
    int sock;
    int relocate, numrelocs = 0;
    FILE *myfile;
#ifdef INET6
    struct addrinfo hints, *res, *res0;
    int error;
#else
    struct hostent *hp;
    struct sockaddr_in sin;
#endif

    host = NULL;
    proxyport = NULL;
    myport = NULL;
    if (!proxyip) {
        if (!proxyurl)
            if (!(proxyurl = getenv("MP3_HTTP_PROXY")))
                if (!(proxyurl = getenv("http_proxy")))
                    proxyurl = getenv("HTTP_PROXY");
        if (proxyurl && proxyurl[0] && strcmp(proxyurl, "none")) {
            if (!(url2hostport(proxyurl, &host, &proxyip, &proxyport))) {
                fprintf (stderr, "Unknown proxy host \"%s\".\n",
                    host ? host : "");
                exit (1);
            }
#if 0
            if (host)
                free (host);
#endif
        }
        else
            proxyip = INADDR_NONE;
    }

    if ((linelength = strlen(url)+200) < 1024)
        linelength = 1024;
    if (!(request = malloc(linelength)) || !(purl = malloc(1024))) {
        fprintf (stderr, "malloc() failed, out of memory.\n");
        exit (1);
    }
    strncpy (purl, url, 1023);
    purl[1023] = '\0';

        getauthfromURL(purl,httpauth1);

    do {
        strcpy (request, "GET ");
        if (proxyip != INADDR_NONE) {
            if (strncmp(url, "http://", 7))
                strcat (request, "http://");
            strcat (request, purl);
            myport = proxyport;
            myip = proxyip;
        }
        else {
            if (host) {
                free(host);
                host=NULL;
            }
            if (proxyport) {
                free(proxyport);
                proxyport=NULL;
            }
            if (!(sptr = url2hostport(purl, &host, &myip, &myport))) {
                fprintf (stderr, "Unknown host \"%s\".\n",
                    host ? host : "");
                exit (1);
            }
            strcat (request, sptr);
        }
        sprintf (request + strlen(request),
            " HTTP/1.0\r\nUser-Agent: %s/%s\r\n",
            prgName, prgVersion);
        if (host) {
            sprintf(request + strlen(request),
                "Host: %s:%s\r\n", host, myport);
#if 0
            free (host);
#endif
        }
        strcat (request, ACCEPT_HEAD);

#ifdef INET6
        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_STREAM;
        error = getaddrinfo(host, (char *)myport, &hints, &res0);
        if (error) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
            exit(1);
        }

        sock = -1;
        for (res = res0; res; res = res->ai_next) {
            if ((sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
                continue;
            }
            if (connect(sock, res->ai_addr, res->ai_addrlen)) {
                close(sock);
                sock = -1;
                continue;
            }
            break;
        }

        freeaddrinfo(res0);
#else
        sock = -1;
        hp = gethostbyname(host);
        if (!hp)
            goto fail;
        if (hp->h_length != sizeof(sin.sin_addr))
            goto fail;
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock < 0)
            goto fail;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        /* sin.sin_len = sizeof(struct sockaddr_in); */
        memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
        if (connect(sock, (struct sockaddr *)&sin, sizeof(struct sockaddr_in) ) < 0) {
            close(sock);
            sock = -1;
        }
fail:
#endif

        if (sock < 0) {
            perror("socket");
            exit(1);
        }

        if (strlen(httpauth1) || httpauth) {
            char buf[1023];
            strcat (request,"Authorization: Basic ");
                        if(strlen(httpauth1))
                          encode64(httpauth1,buf);
                        else
              encode64(httpauth,buf);
            strcat (request,buf);
            strcat (request,"\r\n");
        }
        strcat (request, "\r\n");

        writestring (sock, request);
        if (!(myfile = fdopen(sock, "rb"))) {
            perror ("fdopen");
            exit (1);
        };
        relocate = FALSE;
        purl[0] = '\0';
        readstring (request, linelength-1, myfile);
        if ((sptr = strchr(request, ' '))) {
            switch (sptr[1]) {
                case '3':
                    relocate = TRUE;
                case '2':
                    break;
                default:
                    fprintf (stderr, "HTTP request failed: %s",
                        sptr+1); /* '\n' is included */
                    exit (1);
            }
        }
        do {
            readstring (request, linelength-1, myfile);
            if (!strncmp(request, "Location:", 9))
                strncpy (purl, request+10, 1023);
        } while (request[0] != '\r' && request[0] != '\n');
    } while (relocate && purl[0] && numrelocs++ < 5);
    if (relocate) {
        fprintf (stderr, "Too many HTTP relocations.\n");
        exit (1);
    }
    free (purl);
    free (request);
    free(host);
    free(proxyport);
    free(myport);

    return sock;
}

#else
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern int errno;

#include "mpg123.h"

void writestring (int fd, char *string)
{
}

void readstring (char *string, int maxlen, FILE *f)
{
}

char *url2hostport (char *url, char **hname, unsigned long *hip, unsigned int *port)
{
}

char *proxyurl = NULL;
unsigned long proxyip = 0;
unsigned int proxyport;

#define ACCEPT_HEAD "Accept: audio/mpeg, audio/x-mpegurl, */*\r\n"

int http_open (char *url)
{
}
#endif

/* EOF */

