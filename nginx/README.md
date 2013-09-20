# Repsheet for nginx

This module was built and tested using nginx version 1.4.x. It has not
currently been verified to work against any other versions. If you
find any bugs, please file an
[issue](https://github.com/repsheet/repsheet/issues?state=open).

## How does it work?

When a request comes in, the IP is checked to see if it has been
flagged by Repsheet. If it has, it acts on that information based on
the configuration set. If `repsheet_action` is set to `block`,
Repsheet instructs nginx to return a 403. If `repsheet_action` is set
to `notify`, then Repsheet will log to the nginx log that the IP was
found on the repsheet but no action was taken. Repsheet will then add
the header `X-Repsheet: true` to the request to make downstream
applications aware that the request is from a known bad source.

## Compiling

This module requires [hiredis](https://github.com/redis/hiredis). If
you want to run the integration tests, you will need to have
[Ruby](http://www.ruby-lang.org/en/) and
[RubyGems](http://rubygems.org/) installed. The integration tests use
[Bundler](http://gembundler.com/), so you need to have that installed
as well.

Since nginx doesn't have dynamically loadable modules, you will have
to recompile nginx with the Repsheet module. Chances are that if you
already use nginx, you have a system in place for building your
version of nginx. To have Repsheet built into your version of nginx,
simply add `--add-module=<path-to-repsheet-source>/nginx` to your
`./configure` command. Here's an example using A fresh download of
nginx and a fresh checkout of the Repsheet repository.

```
$ curl -s -L -O "http://nginx.org/download/nginx-1.4.2.tar.gz"
$ tar xzf "nginx-1.4.2.tar.gz"
$ git clone git://github.com/repsheet/repsheet
$ cd nginx-1.4.2
$ ./configure --add-module=../repsheet/nginx
$ make
$ sudo make install
```

## Setup

To activate and configure repsheet you will need to set some
directives. The following list explains what each directive is and
what is does.

* `repsheet <on|off>` - Determines if Repsheet will do any processing
* `repsheet_action <notify|block>` - Action to take if an IP is found on the repsheet. `notify` sets a downstream header. `block` returns a `403`
* `repsheet_redis_timeout <n>` - Sets the time (in milliseconds) before the attempt to connect to redis will timeout and fail
* `repsheet_redis_host <host>` - Sets the host for the Redis connection
* `repsheet_redis_port <port>` - Sets the port for the Redis connection

Here's a minimal working configuration example:

```
events {
    worker_connections  1024;
}

http {
  repsheet_redis_host localhost;
  repsheet_redis_port 6379;
  repsheet_redis_timeout 5;

  server {
    listen 8888;
    location / {
      repsheet on;
      repsheet_action notify;
    }
  }
}
```

In this case, the Repsheet redis values are part of the main nginx
configuration. The decision to enable or disable Repsheet and what
action to take once an actor has been flagged is left up to each
location. This allows for maximum control over each part of your site.

## Additional Details

The optimal configuration for Repsheet is to run with
`repsheet_action` set to notify. This allows you to create dynamic
security controls with your application. Since actors can sometimes be
persistent, you might still want to block them from accessing your
site. You can accomplish this by setting a key
`<ip>:repsheet:blacklist true` substituting `<ip>` for the bad actors
IP address. Similarly, you can make sure that an IP address never gets
blocked by setting `<ip>:repsheet:whitelist true`.
