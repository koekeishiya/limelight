## Limelight

Simple C99 sample showing how to create a border that will follow the focused window on macOS.

Requires access to the accessibility API. Supports macOS High Sierra and newer.

```sh
# add the following to the end of your yabairc to have it launch automatically when yabai starts.
# substitute the path to the limelight binary in the 3rd line below.

# tell limelight to refresh when the focused application changes (see issue #3)
yabai -m signal --add event=application_activated action="pkill -SIGUSR1 limelight &> /dev/null"

# kill any existing limelight process if one exists, before we launch a new one
killall limelight &> /dev/null
limelight &> /dev/null &
```

### Build

Requires xcode command line tools

```sh
# simply clone repo and run make
  make
  make install
```
