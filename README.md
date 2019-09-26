# ESP8266_KeyPad
Testing a MQTT-based KeyPad in Home Assistant

Bought a couple of keypads (0..9,*,#) from China and had to test toward Home Assistant when used as a sort of button.

The keypad has been fitted into a 3D-printed enclosure with a Wemos D1 Mini module.

Keypress are transfered using MQTT.

By long-press of the #-key the module switches between sending keys as they are hit or building up a string that is transmitted using the #-sign (short-pressed).

The unit have two LED's; one for indicating connection to the MQTT-server and one that is blinking when data is transmitted.

The default topic used is "KEYPAD/C0".
This may be changed by hitting * followed by a number-key. For instance hitting '*' followed by '5' will change the topic to "KEYPAD/C5".
This gives a keypad with 10 x 10 keys !!!
Example: Channel 0 and 1 are controlling 20 light toggles while channel 2 is controlling up to 10 blind-toggling.

Improvement: Add a small 128x32 OLED display to show mode, channel and digit pressed.
This will require a small change of pins in moving the "keyboard scanner" pins D1 and D2 to the Tx/Rx pins to free up the I2C pins. After this modification all I/O pins of the Wemos D1 will be in use !!

Home Assistant
==============
The keypad is set up as a sensor in HA (using the default 'channel' 0:
- platform: mqtt
  name: "KeyPad"
  unit_of_measurement: ""
  state_topic: "KEYPAD/C0"

Usage:
======
Example 1: Use digit 1 and 2 to turn on/off a light
===================================================
- id: Keypad1
  alias: Keypad1
  trigger: 
    platform: template
    value_template: "{{states('sensor.keypad') == '1'}}"
  action:
    - service: light.turn_on
      entity_id: light.ikea_large_light
- id: Keypad2
  alias: Keypad2
  trigger: 
    platform: template
    value_template: "{{states('sensor.keypad') == '2'}}"
  action:
    - service: light.turn_off
      entity_id: light.ikea_large_light

Example 2: Use one digit to toggle a light
==========================================
- id: Keypad4
  alias: Keypad4
  trigger: 
    platform: template
    value_template: "{{states('sensor.keypad') == '4'}}"
  action:
    service_template: >
      {% if is_state('light.ikea_small_light','off') %}
        light.turn_on
      {% else %}
        light.turn_off
      {% endif %}
    entity_id: light.ikea_small_light

- id: Keypad4a
  alias: Keypad4a
  trigger: 
    platform: template
    value_template: "{{states('sensor.keypad') == '4'}}"
  action:
    service: python_script.set_state
    data_template:
      entity_id: sensor.keypad
      state: 12

Example 2 is using a small script by "rodpyne" (set_state.py). This is required as HA automation will not detect that the same digit is pressed twice. (Have to dive into the documentation to check if possible). But for now I just change the value of the sensor if it has been set to '4'. It's working....
