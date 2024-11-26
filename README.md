# Arduino "heart monitor"
This was made to monitor if a machine is still turned on or properly running, as it is hard to rely on software on the host machine to send a notification if it unexpectedly turns off or kernel panics, as the computer is still turned on after a kernel panic.

[heartbeat_client](https://github.com/devNat0/heartbeat_client) is required on the host machine for this to work.
This works by the client constantly sending every second to the microcontroller. If it doesn't receive data after a second, it will turn on all LEDs and send a [ntfy](https://ntfy.sh) notification.

I understand that it is bad to block the main loop, but the code currently does nothing during timeout and this keeps the code simpler.

## Usage:
This project uses [PlatformIO](https://platformio.org) VSCode extension.\
Include your microcontroller in the platformio.ini file.\
Build and flash from the extension.
