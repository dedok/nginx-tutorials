#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
    ngx_str_t     word_delimiter;
} ngx_http_example_body_conf_t;


typedef struct {
    ngx_int_t    words;
} ngx_http_example_body_ctx_t;


static void *ngx_http_example_body_create_conf(ngx_conf_t *cf);
static char *ngx_http_example_body_merge_conf(ngx_conf_t *cf, void *parent,
    void *child);
static ngx_int_t ngx_http_example_body_filter_init(ngx_conf_t *cf);


static ngx_command_t  ngx_http_example_body_commands[] = {

    { ngx_string("example_word_delimiter"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_example_body_conf_t, word_delimiter),
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_example_body_filter_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_example_body_filter_init,         /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_example_body_create_conf,         /* create location configuration */
    ngx_http_example_body_merge_conf           /* merge location configuration */
};


ngx_module_t  ngx_http_example_body_filter_module = {
    NGX_MODULE_V1,
    &ngx_http_example_body_filter_module_ctx,  /* module context */
    ngx_http_example_body_commands,            /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_http_output_body_filter_pt    ngx_http_next_body_filter;


static ngx_int_t
ngx_http_example_body_body_filter(ngx_http_request_t *r, ngx_chain_t *in)
{
    size_t i, end;
    ngx_buf_t *b;
    ngx_chain_t *cl;
    ngx_http_example_body_ctx_t *ctx;
    ngx_http_example_body_conf_t *lc;

    if (in == NULL || r->header_only) {
        return ngx_http_next_body_filter(r, in);
    }

    ctx = ngx_http_get_module_ctx(r, ngx_http_example_body_filter_module);
    if (ctx == NULL) {

        ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_example_body_ctx_t));
        if (ctx == NULL) {
            return NGX_ERROR;
        }

        ngx_http_set_ctx(r, ctx, ngx_http_example_body_filter_module);
    }

    lc = ngx_http_get_module_loc_conf(r, ngx_http_example_body_filter_module);

    for (cl = in; cl; cl = cl->next) {

        b = cl->buf;

        i = 0, end = b->last - b->pos;
        for (; i < end; ++i) {
            if (b->pos[i] == lc->word_delimiter.data[0]) {
                ++ctx->words;
            }
        }

        if (b->last_buf) {
            ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0,
                    "body_filter -> words: %d", ctx->words);
            ctx->words = 0;
        }
    }

    return ngx_http_next_body_filter(r, in);
}


static ngx_int_t
ngx_http_example_body_filter_init(ngx_conf_t *cf)
{
    ngx_http_next_body_filter = ngx_http_top_body_filter;
    ngx_http_top_body_filter = ngx_http_example_body_body_filter;

    return NGX_OK;
}


static void *
ngx_http_example_body_create_conf(ngx_conf_t *cf)
{
    ngx_http_example_body_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_example_body_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    return conf;
}


static char *
ngx_http_example_body_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_example_body_conf_t *prev = parent;
    ngx_http_example_body_conf_t *conf = child;

    ngx_conf_merge_str_value(conf->word_delimiter,
            prev->word_delimiter, ",");

    return NGX_CONF_OK;
}
