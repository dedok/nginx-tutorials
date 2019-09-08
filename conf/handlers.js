
// js_set
function get_cache_key(r) {
  // See also:
  // http://nginx.org/en/docs/njs/reference.html#http
  //
  if (r.headersIn['Header-A']) {

    if (r.headersIn['Header-A'] == 'xyz') {
      r.return(500, "Sorry, bad request\n");
      return "";
    }

    return r.headersIn['Header-A'] + '_default_key';
  }
  return 'default_key';
}


// js_content
function subrequests(r) {
  // See also:
  // 1. http://nginx.org/en/docs/njs/reference.html#http
  // 2. subrequests
  //
  var result = '';

  r.subrequest('/a', null, function(resp) {
    result = resp.responseBody;

    r.subrequest('/b', null, function(resp) {
      result += "; " + resp.responseBody + '\n';
      r.return(200, result);
    });

  });
}


// NJS + add_header
function if_status_200(r) {
  if (r.status == 200) {
      return true;
  }
  return false;
}

