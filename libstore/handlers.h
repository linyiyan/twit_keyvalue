#ifndef HANDLERS_H
#define HANDLERS_H

/* handle http request from twit server */
void twit_server_http_req_handler(struct evhttp_request *req, void *arg);

/* handle http request from twit_store */
void store_server_http_req_handler(struct evhttp_request *req , void *arg);

#endif
