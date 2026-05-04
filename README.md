# Quick Start
```shell!=
$ make
$ log_receiver

Log will be saved to: 2026-05-04_15:02:55_fan_control.log
Listening on  :8080
Writing to    2026-05-04_15:02:55_fan_control.log
Board config  remote_host <laptop-ip>, remote_port 8080

{"device":"Switch","ts":1776154577,"temp_4a":81.10,"temp_4b":53.30,"fan_pwm":100,"profile":"default"}
{"device":"Switch","ts":1776154583,"temp_4a":81.10,"temp_4b":53.30,"fan_pwm":100,"profile":"default"}
{"device":"Switch","ts":1776154589,"temp_4a":81.10,"temp_4b":53.30,"fan_pwm":100,"profile":"default"}


$ make clean # Remove log_receiver
$ make clean-logs # remove all logs file
$ make clean-all # remove log_receiver and all logs file
```

