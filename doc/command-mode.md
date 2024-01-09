# Command mode

The following commands of the original K3NG command mode are implemented:

NONE

## What happens when command mode is activated

1. Keying is stopped
2. Buffered message is discarded
3. Protocol commands are completed, but they are ignored: any incoming characters are dropped. Commands are accepted with complete parameters, but not executed.