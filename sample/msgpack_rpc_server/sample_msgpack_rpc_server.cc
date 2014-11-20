#include <msgpack/rpc/server.h>

using namespace std;

class myserver : public msgpack::rpc::server::base {
 public:
  myserver() {
  }
  void call_sample(msgpack::rpc::request req, string a1) {
    string res;
    res = "you call call_sample(";
    res += a1;
    res += ")";
    cout << res << endl;
    req.result(res);
  }
  void notify_sample(msgpack::rpc::request req, string a1) {
    string res;
    res = "you call notify_sample(";
    res += a1;
    res += ")";
    cout << res << endl;
    req.result(res);
  }

 public:
  void dispatch(msgpack::rpc::request req)
      try {
        std::string method;
        req.method().convert(&method);
        cout << method << endl;
        if(method == "call_sample") {
          msgpack::type::tuple<std::string> params;
          req.params().convert(&params);
          call_sample(req, params.get<0>());
        } else if(method == "notify_sample") {
          msgpack::type::tuple<std::string> params;
          req.params().convert(&params);
          notify_sample(req, params.get<0>());
        } else {
          req.error(msgpack::rpc::NO_METHOD_ERROR);
        }
      } catch (msgpack::type_error& e) {
        req.error(msgpack::rpc::ARGUMENT_ERROR);
        return;
      } catch (std::exception& e) {
        req.error(std::string(e.what()));
        return;
      }
};

int main(void) {
  myserver svr;
  svr.instance.listen("0.0.0.0", 9090);
  svr.instance.run(4);
  // run 4 threads
}
