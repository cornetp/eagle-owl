Why this fork?
==============

This is my fork of cornetp/eagle-owl. Eagleowl is already a fantastic piece of software, but it's quite oriented to be
executed as a desktop program. Since what I need are just get the power values from a Raspberry Pi, in order to send them
to external services, I've made some changes to the code:

- Removed Sqlite DB support amb web interface.
- Removed most debug output and file writing, in order to not tear down SD cards.
- Added several different configurable outputs:
  - Text file: Power readings will be written to a simple text file, one per line.
  - MQTT: Publish power values to a MQTT host
  - System command: Custom command to be executed for every power value (NOT IMPLEMENTED YET)
- Updated configuration file syntax, in order to support all new options, and use standard config file parsing mechanisms
(libconfig)

Those new output options allow things impossible to achieve with official version, like minimal disk writing, or even no
disk writing at all (using MQTT to publish power values), thus allowing eagle-owl to be executed from a read only file
system. Text file support also makes easier to get raw power values, without the need of sqlite queries or complex shell
script acrobatics (using screen or similar tools).

What's changed
==============

Usage is more or less the same as the mainstream version. Main difference is the config file, which should be now
something like this:

```
# eagleowl configuration file

# Execute a command for every consumption value (NOT YET IMPLEMENTED)
#system_command=""

# Output consumption values to a file in disk. Disabled by default
#output_file=""

# MQTT broker connection information for publishing values as a MQTT topic
#mqtt_host="mqttbroker"
#mqtt_port=1883
#mqtt_topic="eagleowl/w"

# install_path: path where the eagleowl software is installed
install_path="/opt/eagleowl"
```

It's still needed to specify install path, and then some output method should be activated by uncommenting the corresponding
lines and filling the right values, which are pretty straightforward.

Jesús Jiménez <jesjimenez@gmail.com>

