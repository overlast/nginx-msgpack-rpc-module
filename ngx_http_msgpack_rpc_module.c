/*

Copyright 2014  {name of copyright owner}

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_string.h>
#include <ngx_palloc.h>
#include <ngx_http_variables.h>
#include <msgpack_rpc_client.h>

typedef struct {
  ngx_str_t *ip_address;
  ngx_str_t *port_number;
  ngx_str_t *request_type;
  ngx_str_t *method_name;
  //ngx_uint_t *param_indexes;
  ngx_int_t  param_num;
  //ngx_uint_t  method_name_index;
} ngx_http_msgpack_rpc_client_loc_conf_t;

static void* ngx_http_msgpack_rpc_client_create_loc_conf(ngx_conf_t *cf);
static char* ngx_http_msgpack_rpc_client_call(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char* ngx_http_msgpack_rpc_client_notify(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


static ngx_command_t ngx_http_msgpack_rpc_client_commands[] =
{
  {
    ngx_string("msgpack_rpc_client_call"),
    NGX_HTTP_LOC_CONF|NGX_CONF_2MORE,
    ngx_http_msgpack_rpc_client_call,
    NGX_HTTP_LOC_CONF_OFFSET,
    0,
    NULL
  },
  {
    ngx_string("msgpack_rpc_client_notify"),
    NGX_HTTP_LOC_CONF|NGX_CONF_2MORE,
    ngx_http_msgpack_rpc_client_notify,
    NGX_HTTP_LOC_CONF_OFFSET,
    0,
    NULL
  },
  ngx_null_command
};


static ngx_http_module_t  ngx_http_msgpack_rpc_client_module_ctx =
{

  NULL,  /* preconfiguration */
  NULL,  /* postconfiguration */

  NULL,  /* create main configuration */
  NULL,  /* init main configuration */

  NULL,  /* create server configuration */
  NULL,  /* merge server configuration */

  ngx_http_msgpack_rpc_client_create_loc_conf,  /* create location configuration */
  NULL
};


ngx_module_t  ngx_http_msgpack_rpc_module =
{

  NGX_MODULE_V1,
  &ngx_http_msgpack_rpc_client_module_ctx, /* module context */
  ngx_http_msgpack_rpc_client_commands,   /* module directives */
  NGX_HTTP_MODULE,               /* module type */
  NULL,                          /* init master */
  NULL,                          /* init module */
  NULL,                          /* init process */
  NULL,                          /* init thread */
  NULL,                          /* exit thread */
  NULL,                          /* exit process */
  NULL,                          /* exit master */
  NGX_MODULE_V1_PADDING
};


static void* ngx_http_msgpack_rpc_client_create_loc_conf(ngx_conf_t *cf)
{
  ngx_http_msgpack_rpc_client_loc_conf_t  *conf;
  conf = (ngx_http_msgpack_rpc_client_loc_conf_t  *)ngx_pcalloc(cf->pool, sizeof(ngx_http_msgpack_rpc_client_loc_conf_t));
  if (conf == NULL) {
    return NGX_CONF_ERROR;
  }
  return conf;
}

ngx_str_t* variable_value_to_ngx_str_t(ngx_http_request_t *r, ngx_http_variable_value_t *vvalue) {
  ngx_str_t* ngx_str = NULL;
  if ((vvalue == NULL) || (vvalue->not_found)) {
    return ngx_str;
  }
  ngx_str = (ngx_str_t*)(ngx_pcalloc(r->pool, sizeof(ngx_str_t)));
  if (ngx_str == NULL) {
    return ngx_str;
  }

  ngx_str->len = vvalue->len;
  ngx_str->data = (u_char*)ngx_pnalloc(r->pool, vvalue->len + 1);
  ngx_memset(ngx_str->data, '\0', vvalue->len + 1);
  ngx_snprintf(ngx_str->data, vvalue->len, "%s", vvalue->data);

  return ngx_str;
}

/*

// arg_を前につける
ngx_int_t
get_variable_param_indexes(ngx_conf_t *cf, ngx_uint_t *indexes) {
  ngx_int_t max_index_num = 16;
  indexes = (ngx_uint_t *)(ngx_pcalloc(cf->pool, sizeof(ngx_uint_t) * (max_index_num + 1)));
  ngx_str_t *ngx_param_name_key;
  ngx_int_t i = 1;
  ngx_int_t tmp_index = -1;
  ngx_int_t param_num = 0;
  for (i = 1; i <= 16; i++) {
    ngx_memset(request_type, '\0', request_type_len);
    ngx_sprintf(request_type, "arg_a%d", i);
    ngx_param_name_key = get_ngx_str_t(cf, request_type);
    indexes[i] = ngx_http_get_variable_index(cf, ngx_param_name_key);
    if (tmp_index == -1) {
      break;
    } else {
      indexes[i] = (ngx_uint_t)tmp_index;
      param_num++;
    }
  }
return param_num;
}

//ngx_http_get_indexed_variable の返り値はngx_uint_tじゃないとおかしくね、、というバグは残っているのか
ngx_str_t** get_http_parameters(ngx_http_request_t *r, ngx_http_msgpack_rpc_client_loc_conf_t *conf) {
  ngx_int_t max_index_num = 16;
  ngx_str_t** params = NULL;
  ngx_int_t i = 0;
  ngx_http_variable_value_t  *vvalue_method_name;
  ngx_http_variable_value_t  *vvalue_param;

  params = (ngx_str_t**)ngx_pcalloc(r->pool, sizeof(ngx_str_t*) * (max_index_num + 1));
  if (conf->method_name == NULL) {
    vvalue_method_name = ngx_http_get_indexed_variable(r, conf->method_name_index);
    conf->method_name = variable_value_to_ngx_str_t(r, vvalue_method_name);
  }
  if ((conf->method_name != NULL) && (conf->param_num > 0)) {
    for (i = 1; i <= (conf->param_num); i++) {

      vvalue_param = ngx_http_get_indexed_variable(r, conf->param_indexes[i]);
      params[i] = variable_value_to_ngx_str_t(r, vvalue_param);
      if (params[i] == NULL) {
        break;
      }
    }
  }
  return params;
}

*/

ngx_str_t* get_ngx_str_t(ngx_conf_t *cf, u_char* str) {
  ngx_str_t *res = (ngx_str_t *)ngx_palloc(cf->pool, sizeof(ngx_str_t));
  res->len = (size_t)(sizeof(str));
  res->data = (u_char*)ngx_pnalloc(cf->pool, res->len);
  ngx_memset(res->data, '\0', res->len);
  ngx_sprintf(res->data, "%s", str);
  return res;
}

ngx_str_t* get_ngx_str_t_with_r(ngx_http_request_t *r, u_char* str) {
  ngx_str_t *res = (ngx_str_t *)ngx_palloc(r->pool, sizeof(ngx_str_t));
  res->len = (size_t)(sizeof(str));
  res->data = (u_char*)ngx_pnalloc(r->pool, res->len);
  ngx_memset(res->data, '\0', res->len);
  ngx_sprintf(res->data, "%s", str);
  return res;
}

ngx_str_t** get_http_parameters(ngx_http_request_t *r, ngx_http_msgpack_rpc_client_loc_conf_t *conf) {
  ngx_int_t max_index_num = 16;
  ngx_str_t** params = NULL;
  ngx_int_t i = 0;
  ngx_http_variable_value_t  *vvalue_method_name;
  ngx_http_variable_value_t  *vvalue_param;
  ngx_str_t *tmp_ngx_str;
  ngx_uint_t tmp_key_hash = 0;
  ngx_int_t request_type_len = 10;
  u_char* request_type = (u_char *)ngx_pcalloc(r->pool, sizeof(u_char) * request_type_len);

  params = (ngx_str_t**)ngx_pcalloc(r->pool, sizeof(ngx_str_t*) * (max_index_num + 1));
  ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "testt%s", conf->method_name);

  if (conf->method_name == NULL) {
    tmp_ngx_str = get_ngx_str_t_with_r(r, (u_char*)"arg_method_name");
    tmp_key_hash = ngx_hash_key(tmp_ngx_str->data, tmp_ngx_str->len);
    vvalue_method_name = ngx_http_get_variable(r, tmp_ngx_str, tmp_key_hash);
    conf->method_name = variable_value_to_ngx_str_t(r, vvalue_method_name);
  }
  if (conf->method_name != NULL) {
    for (i = 1; i <= max_index_num; i++) {
      ngx_memset(request_type, '\0', request_type_len);
      ngx_sprintf(request_type, "arg_a%d", i);
      tmp_ngx_str = get_ngx_str_t_with_r(r, request_type);
      tmp_key_hash = ngx_hash_key(tmp_ngx_str->data, tmp_ngx_str->len);
      vvalue_param = ngx_http_get_variable(r, tmp_ngx_str, tmp_key_hash);
      params[i] = variable_value_to_ngx_str_t(r, vvalue_param);
      if (params[i] == NULL) {
        conf->param_num = i - 1;
        break;
      }
    }
    if (params[max_index_num] != NULL) {
      conf->param_num = max_index_num;
    }
  }
  return params;
}

const char* get_mrc_call_responce(ngx_http_msgpack_rpc_client_loc_conf_t *conf, ngx_str_t** params) {
  mrclient* client = mrc_create((char *)conf->ip_address->data, (int)(ngx_atoi(conf->port_number->data, conf->port_number->len)));
  const char* res;
  switch (conf->param_num) {
    case 0:
      res = mrc_call(client, (char *)(conf->method_name->data));
      break;
    case 1:
      res = mrc_call(client, (char *)(conf->method_name->data), (char *)(params[1]->data));
      break;
    case 2:
      res = mrc_call(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data));
      break;
    case 3:
      res = mrc_call(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data));
      break;
    case 4:
      res = mrc_call(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data));
      break;
    case 5:
      res = mrc_call(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data));
      break;
    case 6:
      res = mrc_call(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data));
      break;
    case 7:
      res = mrc_call(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data));
      break;
    case 8:
      res = mrc_call(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data), (char *)(params[8]->data));
      break;
    case 9:
      res = mrc_call(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data), (char *)(params[8]->data), (char *)(params[9]->data));
      break;
    case 10:
      res = mrc_call(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data), (char *)(params[8]->data), (char *)(params[9]->data), (char *)(params[10]->data));
      break;
    case 11:
      res = mrc_call(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data), (char *)(params[8]->data), (char *)(params[9]->data), (char *)(params[10]->data), (char *)(params[11]->data));
      break;
    case 12:
      res = mrc_call(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data), (char *)(params[8]->data), (char *)(params[9]->data), (char *)(params[10]->data), (char *)(params[11]->data), (char *)(params[12]->data));
      break;
    case 13:
      res = mrc_call(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data), (char *)(params[8]->data), (char *)(params[9]->data), (char *)(params[10]->data), (char *)(params[11]->data), (char *)(params[12]->data), (char *)(params[13]->data));
      break;
    case 14:
      res = mrc_call(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data), (char *)(params[8]->data), (char *)(params[9]->data), (char *)(params[10]->data), (char *)(params[11]->data), (char *)(params[12]->data), (char *)(params[13]->data), (char *)(params[14]->data));
      break;
    case 15:
      res = mrc_call(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data), (char *)(params[8]->data), (char *)(params[9]->data), (char *)(params[10]->data), (char *)(params[11]->data), (char *)(params[12]->data), (char *)(params[13]->data), (char *)(params[14]->data), (char *)(params[15]->data));
      break;
    case 16:
      res = mrc_call(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data), (char *)(params[8]->data), (char *)(params[9]->data), (char *)(params[10]->data), (char *)(params[11]->data), (char *)(params[12]->data), (char *)(params[13]->data), (char *)(params[14]->data), (char *)(params[15]->data), (char *)(params[16]->data));
      break;
    default:
      res = mrc_call(client, (char *)(conf->method_name->data));
      break;
  }
  return res;
}

ngx_int_t get_mrc_notify_responce(ngx_http_msgpack_rpc_client_loc_conf_t *conf, ngx_str_t** params) {
  mrclient* client = mrc_create((char *)conf->ip_address->data, (int)(ngx_atoi(conf->port_number->data, conf->port_number->len)));
  ngx_int_t res = 1;
  switch (conf->param_num) {
    case 0:
      mrc_notify(client, (char *)(conf->method_name->data));
      res = 0;
      break;
    case 1:
      mrc_notify(client, (char *)(conf->method_name->data), (char *)(params[1]->data));
      res = 0;
      break;
    case 2:
      mrc_notify(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data));
      res = 0;
      break;
    case 3:
      mrc_notify(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data));
      res = 0;
      break;
    case 4:
      mrc_notify(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data));
      res = 0;
      break;
    case 5:
      mrc_notify(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data));
      res = 0;
      break;
    case 6:
      mrc_notify(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data));
      res = 0;
      break;
    case 7:
      mrc_notify(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data));
      res = 0;
      break;
    case 8:
      mrc_notify(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data), (char *)(params[8]->data));
      res = 0;
      break;
    case 9:
      mrc_notify(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data), (char *)(params[8]->data), (char *)(params[9]->data));
      res = 0;
      break;
    case 10:
      mrc_notify(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data), (char *)(params[8]->data), (char *)(params[9]->data), (char *)(params[10]->data));
      res = 0;
      break;
    case 11:
      mrc_notify(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data), (char *)(params[8]->data), (char *)(params[9]->data), (char *)(params[10]->data), (char *)(params[11]->data));
      res = 0;
      break;
    case 12:
      mrc_notify(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data), (char *)(params[8]->data), (char *)(params[9]->data), (char *)(params[10]->data), (char *)(params[11]->data), (char *)(params[12]->data));
      res = 0;
      break;
    case 13:
      mrc_notify(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data), (char *)(params[8]->data), (char *)(params[9]->data), (char *)(params[10]->data), (char *)(params[11]->data), (char *)(params[12]->data), (char *)(params[13]->data));
      res = 0;
      break;
    case 14:
      mrc_notify(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data), (char *)(params[8]->data), (char *)(params[9]->data), (char *)(params[10]->data), (char *)(params[11]->data), (char *)(params[12]->data), (char *)(params[13]->data), (char *)(params[14]->data));
      res = 0;
      break;
    case 15:
      mrc_notify(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data), (char *)(params[8]->data), (char *)(params[9]->data), (char *)(params[10]->data), (char *)(params[11]->data), (char *)(params[12]->data), (char *)(params[13]->data), (char *)(params[14]->data), (char *)(params[15]->data));
      res = 0;
      break;
    case 16:
      mrc_notify(client, (char *)(conf->method_name->data), (char *)(params[1]->data), (char *)(params[2]->data), (char *)(params[3]->data), (char *)(params[4]->data), (char *)(params[5]->data), (char *)(params[6]->data), (char *)(params[7]->data), (char *)(params[8]->data), (char *)(params[9]->data), (char *)(params[10]->data), (char *)(params[11]->data), (char *)(params[12]->data), (char *)(params[13]->data), (char *)(params[14]->data), (char *)(params[15]->data), (char *)(params[16]->data));
      res = 0;
      break;
    default:
      mrc_notify(client, (char *)(conf->method_name->data));
      res = 0;
      break;
  }
  return res;
}

static ngx_int_t
ngx_http_msgpack_rpc_client_handler(ngx_http_request_t *r)
{
  ngx_http_msgpack_rpc_client_loc_conf_t *conf;

  ngx_int_t     rc;
  ngx_buf_t    *b;
  ngx_chain_t   out;
  u_char *buff;
  u_char* client_res = NULL;
  conf = (ngx_http_msgpack_rpc_client_loc_conf_t *)ngx_http_get_module_loc_conf(r, ngx_http_msgpack_rpc_module);
  size_t client_res_len = 0;
  ngx_str_t** params;
  ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "testip:%s", conf->ip_address->data);
  ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "testport:%s", conf->port_number->data);
  ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "testmethod_name:%s", conf->method_name->data);
  ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "testmethod_name_len:%d", conf->method_name->len);
  ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "testreqest_type:%s", conf->request_type->data);
  params = get_http_parameters(r, conf);
  if ((ngx_strncmp(conf->request_type->data, "call", conf->request_type->len)) == 0) {
    client_res = (u_char *)get_mrc_call_responce(conf, params);
    client_res_len = ngx_strlen(client_res);
  } else if ((ngx_strncmp(conf->request_type->data, "notify", conf->request_type->len)) == 0) {
    if(get_mrc_notify_responce(conf, params)) {
      // notify response error_log
      client_res = (u_char*)ngx_pcalloc(r->pool, sizeof((u_char*)"Fail") + 1);
      ngx_memset(client_res, '\0', sizeof((u_char*)"Fail") + 1);
      ngx_snprintf(client_res, sizeof((u_char*)"Fail"), "%s", (u_char*)"Fail");
      client_res_len = ngx_strlen(client_res);
    } else {
      client_res = (u_char*)ngx_pcalloc(r->pool, sizeof((u_char*)"Success") + 1);
      ngx_memset(client_res, '\0', sizeof((u_char*)"Success") + 1);
      ngx_snprintf(client_res, sizeof((u_char*)"Success"), "%s", (u_char*)"Success");
      client_res_len = ngx_strlen(client_res);
    }
  } else {
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "request_type_error");
    // request_type error
  }

  ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "test%d", client_res_len);

  // header設定
  r->headers_out.content_type.len = sizeof("text/plain") - 1;
  r->headers_out.content_type.data = (u_char *) "text/plain";
  r->headers_out.status = NGX_HTTP_OK;
  r->headers_out.content_length_n = client_res_len;
  if (r->method == NGX_HTTP_HEAD) {
    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
      return rc;
    }
  }
  b = (ngx_buf_t *)(ngx_pcalloc(r->pool, sizeof(ngx_buf_t)));
  if (b == NULL) {
    return NGX_HTTP_INTERNAL_SERVER_ERROR;
  }

  out.buf = b;
  out.next = NULL;

  buff = (u_char *)ngx_palloc(r->pool, client_res_len);
  ngx_snprintf(buff, client_res_len, "%s", client_res);

  b->pos = buff;
  b->last = buff + client_res_len;
  b->memory = 1;
  b->last_buf = 1;

  rc = ngx_http_send_header(r);
  if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
    return rc;
  }
  ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "Success! msgpack_rpc_client");

  return ngx_http_output_filter(r, &out);
}



/*
void set_str_to_ngx_str_t(ngx_str_t *ngx_str, char* str) {
  ngx_str->len = (size_t)(strlen(str));
  ngx_str->data = (u_char*)ngx_pnalloc(cf->pool, ngx_str->len);
  ngx_memset(ngx_str->data, '\0', BUFFER_SIZE);
  ngx_sprintf(ngx_str->data, "%s", str);
  return res;
}
*/

ngx_str_t* get_copy_ngx_str_t(ngx_conf_t *cf, ngx_str_t base) {
  ngx_str_t *target = (ngx_str_t *)ngx_pnalloc(cf->pool, sizeof(ngx_str_t));
  target->len = base.len;
  target->data = (u_char*)ngx_pnalloc(cf->pool, target->len);
  ngx_memset(target->data, '\0', target->len);
  ngx_snprintf(target->data, target->len, "%s", base.data);
  return target;
}

static char *
ngx_http_msgpack_rpc_client_call(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
  ngx_http_msgpack_rpc_client_loc_conf_t  *mrclcf = (ngx_http_msgpack_rpc_client_loc_conf_t  *)conf;
  ngx_uint_t   args;
  ngx_str_t   *argv;
  u_char* request_type = (u_char*)"call";
  ngx_http_core_loc_conf_t  *clcf;

  args = cf->args->nelts;
  argv = cf->args->elts;

  if (args == 4) {
    mrclcf->ip_address = get_copy_ngx_str_t(cf, argv[1]);
    mrclcf->port_number = get_copy_ngx_str_t(cf, argv[2]);
    mrclcf->request_type = get_ngx_str_t(cf, request_type);
    mrclcf->method_name= get_copy_ngx_str_t(cf, argv[3]);
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "method_name!!!!!!!!!!!!!!!!!!!!:%s", argv[3].data);
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "method_name!!!!!!!!!!!!!!!!!!!!:%d", argv[3].len);
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "method_name!!!!!!!!!!!!!!!!!!!!:%s", mrclcf->method_name->data);
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "method_name!!!!!!!!!!!!!!!!!!!!:%d", mrclcf->method_name->len);

  } else if (args == 3) {
    mrclcf->ip_address = get_copy_ngx_str_t(cf, argv[1]);
    mrclcf->port_number = get_copy_ngx_str_t(cf, argv[2]);
    mrclcf->request_type = get_ngx_str_t(cf, request_type);
    mrclcf->method_name = NULL;
  } else {
    //return "NGX_CONF_ERROR";
  }

  // location設定を取得しHandler設定
  clcf = (ngx_http_core_loc_conf_t  *)ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
  clcf->handler = ngx_http_msgpack_rpc_client_handler;
  return NGX_CONF_OK;
}

static char *
ngx_http_msgpack_rpc_client_notify(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
  ngx_http_msgpack_rpc_client_loc_conf_t  *mrclcf = (ngx_http_msgpack_rpc_client_loc_conf_t *)conf;
  ngx_uint_t   args;
  ngx_str_t   *argv;
  u_char* request_type = (u_char*)"notify";
  ngx_http_core_loc_conf_t  *clcf;

  args = cf->args->nelts;
  argv = (ngx_str_t *)cf->args->elts;

  if (args == 4) {
    mrclcf->ip_address = get_copy_ngx_str_t(cf, argv[1]);
    mrclcf->port_number = get_copy_ngx_str_t(cf, argv[2]);
    mrclcf->request_type = get_ngx_str_t(cf, request_type);
    mrclcf->method_name= get_copy_ngx_str_t(cf, argv[3]);
  } else if (args == 3) {
    mrclcf->ip_address = get_copy_ngx_str_t(cf, argv[1]);
    mrclcf->port_number = get_copy_ngx_str_t(cf, argv[2]);
    mrclcf->request_type = get_ngx_str_t(cf, request_type);
    mrclcf->method_name = NULL;
  } else {
    //return NGX_CONF_ERROR;
  }
  // location設定を取得しHandler設定
  clcf = (ngx_http_core_loc_conf_t  *)ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
  clcf->handler = ngx_http_msgpack_rpc_client_handler;

  return NGX_CONF_OK;
}
