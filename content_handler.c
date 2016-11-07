#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
    ngx_str_t content;
} ngx_http_content_loc_conf_t;


static ngx_int_t ngx_http_content_init(ngx_conf_t *cf);

static void* ngx_http_content_create_loc_conf(ngx_conf_t *cf);
static char* ngx_http_content_merge_loc_conf(ngx_conf_t *cf,
        void *parent, void *child);


static ngx_command_t  ngx_http_content_commands[] = {

	{ ngx_string("example_set_content"),
      NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
	  ngx_conf_set_str_slot,
	  NGX_HTTP_LOC_CONF_OFFSET,
	  offsetof(ngx_http_content_loc_conf_t, content),
	  NULL },

	ngx_null_command
};


static ngx_http_module_t  ngx_http_content_module_ctx = {
	NULL,                             /* preconfiguration */
	ngx_http_content_init,            /* postconfiguration */

	NULL,                              /* create main configuration */
	NULL,                              /* init main configuration */

	NULL,                              /* create server configuration */
	NULL,                              /* merge server configuration */

	ngx_http_content_create_loc_conf,  /* create location configuration */
	ngx_http_content_merge_loc_conf    /* merge location configuration */
};


ngx_module_t  ngx_http_content_handler_module = {
	NGX_MODULE_V1,
	&ngx_http_content_module_ctx,    /* module context */
	ngx_http_content_commands,       /* module directives */
	NGX_HTTP_MODULE,                 /* module type */
	NULL,                            /* init master */
	NULL,                            /* init module */
	NULL,                            /* init process */
	NULL,                            /* init thread */
	NULL,                            /* exit thread */
	NULL,                            /* exit process */
	NULL,                            /* exit master */
	NGX_MODULE_V1_PADDING
};

/**
 * Module entry point
 */
static ngx_int_t
ngx_http_content_handler(ngx_http_request_t *r)
{
	ngx_buf_t *b;
    ngx_int_t rc;
	ngx_chain_t out;
    ngx_http_content_loc_conf_t *lc;

	lc = ngx_http_get_module_loc_conf(r, ngx_http_content_handler_module);

    if (lc->content.data == NULL || lc->content.len == 0) {
		return NGX_DECLINED;
	}

	rc = ngx_http_discard_request_body(r);
	if (rc != NGX_OK && rc != NGX_AGAIN) {
		return rc;
	}

	b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
	if (b == NULL) {
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}

	r->headers_out.status = NGX_HTTP_OK;
	r->headers_out.content_length_n = lc->content.len;
	r->headers_out.content_type.len = sizeof("plain/text") - 1;
	r->headers_out.content_type.data = (u_char *) "plain/text";

	rc = ngx_http_send_header(r);
	if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
		return rc;
	}

    b->pos = b->start = lc->content.data;
    b->end = b->last = lc->content.data + lc->content.len;
	b->last_buf = b->memory = 1;

	out.buf = b;
	out.next = NULL;

	return ngx_http_output_filter(r, &out);
}


static ngx_int_t
ngx_http_content_init(ngx_conf_t *cf)
{
	ngx_http_handler_pt *h;
	ngx_http_core_main_conf_t *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
	if (h == NULL) {
		return NGX_ERROR;
	}

	*h = ngx_http_content_handler;

	return NGX_OK;
}


static void *
ngx_http_content_create_loc_conf(ngx_conf_t *cf)
{
    (void)cf;

	ngx_http_content_loc_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_content_loc_conf_t));
	if (conf == NULL) {
		return NGX_CONF_ERROR;
	}

	return conf;
}


static char *
ngx_http_content_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    (void)cf;

	ngx_http_content_loc_conf_t *prev = parent;
	ngx_http_content_loc_conf_t *conf = child;

	ngx_conf_merge_str_value(conf->content, prev->content, "");

	return NGX_CONF_OK;
}

