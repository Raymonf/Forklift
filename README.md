# Forklift

A mod loader for Shenmue I/II (by d3t) which allows the user to toggle modifications on/off from within the game, without needing to restart.

## Overview

Forklift comes in two versions:-

* GUI version (Recommended for most users)
* Non-GUI version

The only difference between the two versions is that the GUI version has a menu that can be used in-game. The non-GUI version does not have this menu. Additionally, the GUI version allows you to disable Forklift with the <kbd>F11</kbd> key and toggle the menu with the <kbd>F12</kbd> key.

The core functionality of Forklift is built in a separate library, named `libForklift`, which contains all of the main patching code. 