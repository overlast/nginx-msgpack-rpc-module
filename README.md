nginx-msgpack-rpc-module
=======================

Nginx module to provide client methods of MessagePack RPC
 +
## Motive

Request to MessagePack RPC server using Nginx as MessagePack RPC client.

When you need a MessagePack RPC client, you need only write the address
and port number of a MessagePack RPC server to configure the nginx.

There is no need to implement MessagePack RPC clients corresponding to new MessagePack RPC servers.

## Requirements

Following programs are required to build:

- gcc >= 4.1 with C++ support
- [MessagePack for C++](http://msgpack.org/) >= 0.5.2
- [mpio](http://github.com/frsyuki/mpio) >= 0.3.5
- [MessagePack-RPC for C++](https://github.com/msgpack-rpc/msgpack-rpc-cpp/) >= 0.3.1
- [MessagePack-RPC for C](https://github.com/overlast/msgpack-rpc-c/) >= 0.0.1

## License

    Copyright (C) 2014 Toshinori SATO <overlasting _attt_ gmail _dottt_ com>

    Licensed under the Apache License, Version 2.0 (the "License");
    You may not use this file except in compliance with the License.
    You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

 See also NOTICE file.
