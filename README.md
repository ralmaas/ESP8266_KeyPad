# ESP8266_KeyPad
Testing a MQTT-based KeyPad in Home Assistant

Bought a couple of keypads (0..9,*,#) from China and had to test toward Home Assistant when used as a sort of button.

The keypad has been fitted into a 3D-printed enclosure with a Wemos D1 Mini module.

Keypress are transfered using MQTT.

By long-press of the #-key the module switches between sending keys as they are hit or building up a string that is transmitted using the #-sign (short-pressed).

The unit have two LED's; one for indicating connection to the MQTT-server and one that is blinking when data is transmitted.

Home Assistant
==============
The keypad is set up as a sensor in HA:
- platform: mqtt
  name: "KeyPad"
  unit_of_measurement: ""
  state_topic: "KEYPAD/C0"

Usage:
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
