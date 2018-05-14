#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
    ngx_str_t cond;
    ngx_str_t value;
} ngx_http_example_header_filter_loc_conf_t;


static ngx_int_t ngx_http_example_header_filter_init(ngx_conf_t *cf);
static void* ngx_http_example_header_filter_create_loc_conf(ngx_conf_t *cf);
static char* ngx_http_example_header_filter_merge_loc_conf(ngx_conf_t *cf,
        void *parent, void *child);

static ngx_int_t ngx_header_get(ngx_http_request_t *r,
        const ngx_str_t *header_name,
        ngx_str_t *k, ngx_str_t *v);
static ngx_int_t ngx_header_set(ngx_http_request_t *r,
        const ngx_str_t *n, const ngx_str_t *v);


static ngx_command_t ngx_http_example_header_filter_commands[] = {

    { ngx_string("example_if_header"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_example_header_filter_loc_conf_t, cond),
      NULL },

    { ngx_string("example_set_header_value"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_example_header_filter_loc_conf_t, value),
      NULL },

      ngx_null_command
};


static ngx_http_module_t ngx_http_example_header_filter_ctx = {
    NULL,                                   /* preconfiguration */
    ngx_http_example_header_filter_init,              /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_example_header_filter_create_loc_conf,  /* create location configuration */
    ngx_http_example_header_filter_merge_loc_conf    /* merge location configuration */
};


ngx_module_t ngx_http_example_header_filter_module = {
    NGX_MODULE_V1,
    &ngx_http_example_header_filter_ctx,         /* module context */
    ngx_http_example_header_filter_commands,     /* module directives */
    NGX_HTTP_MODULE,                   /* module type */
    NULL,                              /* init master */
    NULL,                              /* init module */
    NULL,                              /* init process */
    NULL,                              /* init thread */
    NULL,                              /* exit thread */
    NULL,                              /* exit process */
    NULL,                              /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_http_output_header_filter_pt ngx_http_next_header_filter;


static ngx_int_t
ngx_http_example_header_filter(ngx_http_request_t *r)
{
    ngx_int_t rc;
    ngx_http_example_header_filter_loc_conf_t *lc;
    ngx_str_t k = { .data = NULL, .len = 0 },
              v = { .data = NULL, .len = 0 };

    lc = ngx_http_get_module_loc_conf(r, ngx_http_example_header_filter_module);

    if (r != r->main || !lc->cond.data) {
        return ngx_http_next_header_filter(r);
    }

    rc = ngx_header_get(r, &lc->cond, &k, &v);
    if (rc == NGX_ERROR) {
        return rc;
    }

    if (k.data == NULL) {
        rc = ngx_header_set(r, &lc->cond, &lc->value);
        if (rc == NGX_ERROR) {
            return rc;
        }
    }

    return ngx_http_next_header_filter(r);
}


static void *
ngx_http_example_header_filter_create_loc_conf(ngx_conf_t *cf)
{
    (void)cf;

    ngx_http_example_header_filter_loc_conf_t *conf;
    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_example_header_filter_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }

    return conf;
}


static char *
ngx_http_example_header_filter_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    (void)cf;

    ngx_http_example_header_filter_loc_conf_t *prev = parent;
    ngx_http_example_header_filter_loc_conf_t *conf = child;

    ngx_conf_merge_str_value(conf->cond, prev->cond, "");
    ngx_conf_merge_str_value(conf->value, prev->value, "");

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_example_header_filter_init(ngx_conf_t *cf)
{
    (void)cf;

    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_example_header_filter;

    return NGX_OK;
}

static ngx_int_t
ngx_header_get(ngx_http_request_t *r, const ngx_str_t *header_name,
        ngx_str_t* k, ngx_str_t *v)
{
    ngx_table_elt_t *h;
    ngx_list_part_t *part;

    part = &r->headers_in.headers.part;
    h = part->elts;

    size_t i = 0;
    for (;; i++) {

        if (i >= part->nelts) {
            if (part->next == NULL) {
                break;
            }
            part = part->next;
            h = part->elts;
            i = 0;
        }

        if (ngx_strncmp(h[i].key.data, header_name->data, header_name->len)
                == 0)
        {
            k->data = h[i].key.data;
            k->len = h[i].key.len;
            v->data = h[i].value.data;
            v->len = h[i].value.len;
            break;
        }
    }

    return NGX_OK;
}


static ngx_int_t
ngx_header_set(ngx_http_request_t *r, const ngx_str_t *n, const ngx_str_t *v)
{
	ngx_table_elt_t *header;
	u_char *name, *name_end, *value, *value_end;

	name = ngx_pnalloc(r->pool, n->len);
	if (name == NULL) {
		return NGX_ERROR;
	}

    value = ngx_pnalloc(r->pool, v->len);
    if (value == NULL) {
      return NGX_ERROR;
    }

	name_end = ngx_copy(name, n->data, n->len);
	value_end = ngx_copy(value, v->data, v->len);

	header = ngx_list_push(&r->headers_out.headers);
	if (header == NULL) {
		return NGX_ERROR;
	}

	header->hash = 1;
    header->key.data = name;
    header->key.len = name_end - name;
	header->value.data = value;
	header->value.len = value_end - value;

	return NGX_OK;
}

