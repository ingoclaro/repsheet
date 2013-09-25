require 'rake'
require 'rspec/core/rake_task'
require 'redis'

RSpec::Core::RakeTask.new(:integration) do |t|
  t.pattern = "spec/**/*_spec.rb"
end

namespace :apache do
  task :start do
    `build/apache/bin/apachectl restart`
    sleep 1
  end

  task :stop do
    `build/apache/bin/apachectl stop`
  end

  task :compile do
    sh "script/apache_compile"
  end
end

namespace :nginx do
  task :start do
    `build/nginx/sbin/nginx`
    sleep 1
  end

  task :stop do
    `build/nginx/sbin/nginx -s stop`
  end

  task :compile do
    sh "script/nginx_compile"
  end
end

namespace :redis do
  task :start do
    `redis-server etc/redis.conf`
  end

  task :stop do
    redis = Redis.new
    redis.shutdown
  end
end

namespace :repsheet do
  task :bootstrap do
    unless Dir.exists?("build")
      puts "Run make install_local before running the tests"
      exit(1)
    end
  end
end

desc "Run the integration tests against Apache"
task :apache => ["repsheet:bootstrap", "apache:compile", "apache:start", "redis:start", :integration, "apache:stop", "redis:stop"]

desc "Run the integration tests against nginx"
task :nginx => ["repsheet:bootstrap", "nginx:compile", "nginx:start", "redis:start", :integration, "nginx:stop", "redis:stop"]
