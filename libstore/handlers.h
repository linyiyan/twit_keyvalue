#ifndef HANDLERS_H
#define HANDLERS_H

typedef void (*func_t)(evhttp_request *, void *);

/* handle http request from twit server */
void twit_server_http_req_handler(struct evhttp_request *req, void *arg);

/* handle http request from twit_store */
void store_server_http_req_handler(struct evhttp_request *req, void *arg);

/* handle setup response from twit_store */
void twit_store_setup_resp_handler(evhttp_request *req, void *arg);

/* handle set/get request-response from twit_store */
void twit_store_opr_resp_handler(evhttp_request *req, void *arg);

void send_setup_req(const server_config& server,
		const map<string, string>& params, func_t cb, void* cbArg);

void send_http_req(const server_config& server,
		const map<string, string>& params, func_t cb, void* cbArg);

void set(storage_server_manager& mgr, string szKey, string szValue,
		evhttp_request *req);

void get(storage_server_manager& mgr, string szKey, evhttp_request *req);

#endif
