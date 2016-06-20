#Development environment setup
1. Ensure that you have Ruby installed. I recommend using [rbenv](https://github.com/rbenv/rbenv).
2. Ensure that you can invoke bundler: ```bundle --version```.
	If that doesn't execute successfully, install bundler via ```gem install bundler```.
3. From the ```ruby``` directory, run ```bundle install``` to install the application's dependencies.

#Running the automated test suite
From the ```ruby``` directory, run ```bundle exec rspec```.


#Starting smith-server locally for development
From the ```ruby``` directory, run:

```
cd lib/smith/server
bundle exec rackup -p 9292
```
Now access the application at [http://localhost:9292](http://localhost:9292).

#Rake tasks
* ```rake import_definitions```: Generate constant definition file using preprocessor definitions from C++ header files
* ```rake check_definitions```: Check if the definition file is up to date with respect to the C++ header files
* ```rake deploy```: Installs the smith gem and dependencies to a remote host (by default 192.168.7.2 for the BeagleBone Black or Ember booted from development SD card)

#Usage
The gem provides the following commands after installation through ```rake deploy``` or by booting from a production firmware image:

* ```smith-client```: Starts the web client
* ```smith-config```: Command line utility for performing configuration tasks
* ```smith-server```: Starts the local web server that provides the local web interface and API